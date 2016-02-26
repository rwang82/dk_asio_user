#include "stdafx.h"
#include "dk_pd_4_render.h"
#include "dk_pd_4_asio.h"
#include "dk_pd_root.h"
#include "laudio_pack_defs.h"
#include "laudio_pack_4_asio.h"
#include "dk_pd_channel_4_asio.h"
//
static void _on_LAUDIO_ASIO_CMD_PCMDATA_BUF0( void* pParamUser, unsigned char* pBuf, unsigned int uLenBuf );
static void _on_LAUDIO_ASIO_CMD_PCMDATA_BUF1( void* pParamUser, unsigned char* pBuf, unsigned int uLenBuf );
static void _on_LAUDIO_ASIO_CMD_NEW_INSTANCE( void* pParamUser, unsigned char* pBuf, unsigned int uLenBuf );
static void _pushback_pcmdata( struct dk_pd_4_asio* pASIORoot, unsigned char* pBufPCMData, unsigned int uLenPCMData );
//
void _on_accepted_4_render( SOCKET sConnect, void* pParam ) {
	printf( "enter %s\n", __FUNCTION__ );


	printf( "exit %s\n", __FUNCTION__ );
}

void _on_disconnected_4_render( SOCKET sConnect, void* pParam ) {
	printf( "enter %s\n", __FUNCTION__ );

	
	printf( "exit %s\n", __FUNCTION__ );
}

void _on_recv_4_render( SOCKET sConnect, unsigned char* pBuf, unsigned int uLenBuf, void* pParam ) {
	struct dk_pd_4_asio* pInst = (struct dk_pd_4_asio*)pParam;

	pInst->m_pDecoder4Render->pushback( pBuf, uLenBuf );
}

void _on_whole_package_4_render( void* pParamUser, unsigned char* pBuf, unsigned int uLenBuf ) {
	struct dk_pd_4_asio* pInst = (struct dk_pd_4_asio*)pParamUser;
	laudio_header* pHeader = (laudio_header*)pBuf;
	if ( pHeader->m_sort != LAUDIO_SORT_ASIO )
		return;
	switch ( pHeader->m_cmd )
	{
	case LAUDIO_ASIO_CMD_PCMDATA_BUF0:
	{
	    _on_LAUDIO_ASIO_CMD_PCMDATA_BUF0( pParamUser, pBuf, uLenBuf );
	}
		break;
	case LAUDIO_ASIO_CMD_PCMDATA_BUF1:
	{
	    _on_LAUDIO_ASIO_CMD_PCMDATA_BUF1( pParamUser, pBuf, uLenBuf );
	}
		break;
	case LAUDIO_ASIO_CMD_NEW_INSTANCE:
	{
	     _on_LAUDIO_ASIO_CMD_NEW_INSTANCE( pParamUser, pBuf, uLenBuf );
	}
		break;
	default:
		break;
	}
}


void _on_LAUDIO_ASIO_CMD_PCMDATA_BUF0( void* pParamUser, unsigned char* pBuf, unsigned int uLenBuf ) {
	if ( !pParamUser || !pBuf || (uLenBuf == 0) )
		return;
    struct dk_pd_4_asio* pInst = (struct dk_pd_4_asio*)pParamUser;
	laudio_header* pHeader = (laudio_header*)pBuf;
	unsigned char* pBufPayload = pBuf + LAUDIO_PACKHEADER_SIZE;
	unsigned char ucChannel = *(pBufPayload);
	unsigned char ucChannelCount = *(pBufPayload + 1 );
	short sPCMSize = *((short*)(pBufPayload + 2));
	unsigned char* pPCMStart = pBufPayload + 4;
	TCHAR szMsg[50];

	if ( ucChannel >= DK_PD_CHANNEL_MAX ) {
		_stprintf_s( szMsg, _T("ucChannel(%d)>=DK_PD_CHANNEL_MAX(%d)"), ucChannel, DK_PD_CHANNEL_MAX );
		::OutputDebugString( szMsg );
		return;
	}
	if ( sPCMSize > pInst->m_uSizePCMBuf ) {
	    _stprintf_s( szMsg, _T("sPCMSize(%d)>pInst->m_uSizePCMBuf(%d)"), sPCMSize, pInst->m_uSizePCMBuf );
		return;
	}
	//
	memcpy_s( pInst->m_channels[ ucChannel ]->m_pBuf0, pInst->m_uSizePCMBuf, pPCMStart, sPCMSize );
	pInst->m_channels[ ucChannel ]->m_uLenBuf0Valid = sPCMSize;
	//
	if ( ( ucChannel + 1 ) == DK_PD_CHANNEL_MAX
		|| ( ( ucChannel + 1 ) == ucChannelCount ) ) {
		::SetEvent( pInst->m_hEventBuf0Ready );
	}
}

void _on_LAUDIO_ASIO_CMD_PCMDATA_BUF1( void* pParamUser, unsigned char* pBuf, unsigned int uLenBuf ) {
		if ( !pParamUser || !pBuf || (uLenBuf == 0) )
		return;
    struct dk_pd_4_asio* pInst = (struct dk_pd_4_asio*)pParamUser;
	laudio_header* pHeader = (laudio_header*)pBuf;
	unsigned char* pBufPayload = pBuf + LAUDIO_PACKHEADER_SIZE;
	unsigned char ucChannel = *(pBufPayload);
	unsigned char ucChannelCount = *(pBufPayload + 1 );
	short sPCMSize = *((short*)(pBufPayload + 2));
	unsigned char* pPCMStart = pBufPayload + 4;
	TCHAR szMsg[50];

	if ( ucChannel >= DK_PD_CHANNEL_MAX ) {
		_stprintf_s( szMsg, _T("ucChannel(%d)>=DK_PD_CHANNEL_MAX(%d)"), ucChannel, DK_PD_CHANNEL_MAX );
		::OutputDebugString( szMsg );
		return;
	}
	if ( sPCMSize > pInst->m_uSizePCMBuf ) {
	    _stprintf_s( szMsg, _T("sPCMSize(%d)>pInst->m_uSizePCMBuf(%d)"), sPCMSize, pInst->m_uSizePCMBuf );
		return;
	}
	//
	memcpy_s( pInst->m_channels[ ucChannel ]->m_pBuf1, pInst->m_uSizePCMBuf, pPCMStart, sPCMSize );
	pInst->m_channels[ ucChannel ]->m_uLenBuf1Valid = sPCMSize;	
	//
	if ( ( ucChannel + 1 ) == DK_PD_CHANNEL_MAX
		|| ( ( ucChannel + 1 ) == ucChannelCount ) ) {
		::SetEvent( pInst->m_hEventBuf1Ready );
	}
}

void _pushback_pcmdata( struct dk_pd_4_asio* pASIORoot, unsigned char* pBufPCMData, unsigned int uLenPCMData ) {
	if( !pASIORoot || !pBufPCMData || ( uLenPCMData == 0 ) )
		return;


}

void _do_render( struct dk_pd_4_asio* pInst, unsigned int bBuf0 ) {
	if ( !pInst || bBuf0 >= 2 )
		return;
	dk_pd_channel_4_asio* pChannel = NULL;
	DWORD dwWritten = 0;
	unsigned char* pBuf = NULL;
	unsigned int uLenBuf = 0;

	for ( unsigned int uIdx = 0; uIdx < DK_PD_CHANNEL_MAX; ++uIdx ) {
		pChannel = pInst->m_channels[ uIdx ];
		if ( !pChannel )
			return;
		pBuf = bBuf0 ? pChannel->m_pBuf0 : pChannel->m_pBuf1;
		uLenBuf = bBuf0 ? pChannel->m_uLenBuf0Valid : pChannel->m_uLenBuf1Valid;
		//pChannel->m_pBuf0 / pChannel->m_pBuf1
		//pChannel->m_uLenBuf0Valid / pChannel->m_uLenBuf1Valid
		if ( uIdx == 0 ) {
			if ( !::WriteFile( pInst->m_hPCMChannel0, pBuf, uLenBuf, &dwWritten, NULL ) ) {
			    DWORD dwErr = ::GetLastError();
				int a = 0;
			}
		} else if ( uIdx == 1 ) {
			::WriteFile( pInst->m_hPCMChannel1, pBuf, uLenBuf, &dwWritten, NULL );
		}
		//
		::Sleep( 1 );
	}
}

void _req_pcmdata( struct dk_pd_4_asio* pInst, unsigned int bBuf0 ) {
	if ( !pInst )
		return;
	unsigned char bufCmdWantData[ LAUDIO_PACKHEADER_SIZE + PAYLOAD_LEN_4_CMD_WANTDATA_BUF0 ];
	unsigned char* pPayload = &bufCmdWantData[LAUDIO_PACKHEADER_SIZE];

	LAUDIO_PACKAGE_HEADER_INIT_4_ASIO( bufCmdWantData
		, ( bBuf0 ? LAUDIO_ASIO_CMD_WANTDATA_BUF0 : LAUDIO_ASIO_CMD_WANTDATA_BUF1)
		, PAYLOAD_LEN_4_CMD_WANTDATA_BUF0 );
	memcpy_s( pPayload, PAYLOAD_LEN_4_CMD_WANTDATA_BUF0, pInst->m_pPDRoot->m_szPDID, 39 );
	pInst->m_server4Render.send( bufCmdWantData, LAUDIO_PACKHEADER_SIZE + PAYLOAD_LEN_4_CMD_WANTDATA_BUF0 );
}

void _on_LAUDIO_ASIO_CMD_NEW_INSTANCE( void* pParamUser, unsigned char* pBuf, unsigned int uLenBuf ) {
	struct dk_pd_4_asio* pInst = (struct dk_pd_4_asio*)pParamUser;

	::SetEvent( pInst->m_hEventNewInstance );
}