#include "stdafx.h"
#include "dk_pd_4_asio.h"
#include "dk_pd_root.h"
#include "dk_pd_envcfg.h"
#include "dk_bit_helper.h"
#include "laudio_port_defs.h"
#include "laudio_pack_defs.h"
#include "laudio_pack_4_acp.h"
#include "dk_pd_4_render.h"
#include "dk_pd_4_capture.h"
#include "dk_pd_channel_4_asio.h"
#include <process.h>
//
#define DK_PD_CHANNEL_4_ASIO_CACHE_SIZE (20*1024)
//
static void dk_pd_4_asio_destroy( struct dk_pd_4_asio* pInst );
//
static void _on_accepted_4_acp( SOCKET sConnect, void* pParam );
static void _on_disconnected_4_acp( SOCKET sConnect, void* pParam );
static void _on_recv_4_acp( SOCKET sConnect, unsigned char* pBuf, unsigned int uLenBuf, void* pParam );
static void _on_whole_package_4_acp( void* pParamUser, unsigned char* pBuf, unsigned int uLenBuf );
//
static void _on_cmd_LAUDIO_ACP_CMD_FIND_PD( struct dk_pd_4_asio* pInst, unsigned char* pBuf, unsigned int uLen );
//
static unsigned int __stdcall _pd_thread_4_render( void* pParam );
//
struct dk_pd_4_asio* dk_pd_4_asio_start(dk_pd_root* pPDRoot) {
	struct dk_pd_4_asio* pInstNew = NULL;
	struct dk_pd_channel_4_asio* pChannel = NULL;
		
	pInstNew = new dk_pd_4_asio();
	if ( !pInstNew )
		return NULL;
	//
	pInstNew->m_pPDRoot = pPDRoot;
	pInstNew->m_hEventAccess = ::CreateEvent( NULL, FALSE, TRUE, NULL );
	pInstNew->m_hEventExit = ::CreateEvent( NULL, TRUE, FALSE, NULL );
	pInstNew->m_hEventBuf0Ready = ::CreateEvent( NULL, FALSE, FALSE, NULL );
	pInstNew->m_hEventBuf1Ready = ::CreateEvent( NULL, FALSE, FALSE, NULL );
	pInstNew->m_hEventNewInstance = ::CreateEvent( NULL, FALSE, FALSE, NULL );
	if ( !pInstNew->m_hEventAccess 
		|| !pInstNew->m_hEventExit 
		|| !pInstNew->m_hEventBuf0Ready
		|| !pInstNew->m_hEventBuf1Ready
		|| !pInstNew->m_hEventNewInstance )
		goto err;
	pInstNew->m_pDecoder4ACP = new laudio_pack_decoder( pInstNew, _on_whole_package_4_acp );
	pInstNew->m_pDecoder4Render = new laudio_pack_decoder( pInstNew, _on_whole_package_4_render );
	pInstNew->m_pDecoder4Capture = new laudio_pack_decoder( pInstNew, _on_whole_package_4_capture );
	//
	pInstNew->m_uSizePCMBuf = DK_PD_CHANNEL_4_ASIO_CACHE_SIZE;
	for ( int i = 0; i < DK_PD_CHANNEL_MAX; ++i ) {
		pChannel = dk_pd_channel_start_4_asio( pInstNew, DK_PD_CHANNEL_4_ASIO_CACHE_SIZE );
		if ( pChannel == NULL )
			goto err;
	    pInstNew->m_channels[ i ] = pChannel;
	}
	//
	pPDRoot->m_module.m_pPD4ASIO = pInstNew;
	//
	pInstNew->m_server4ACP.m_eventAccepted.bind( _on_accepted_4_acp, pInstNew );
	pInstNew->m_server4ACP.m_eventDisConnected.bind( _on_disconnected_4_acp, pInstNew );
	pInstNew->m_server4ACP.m_eventRecvData.bind( _on_recv_4_acp, pInstNew );
	//
	pInstNew->m_server4Render.m_eventAccepted.bind( _on_accepted_4_render, pInstNew );
	pInstNew->m_server4Render.m_eventDisConnected.bind( _on_disconnected_4_render, pInstNew );
	pInstNew->m_server4Render.m_eventRecvData.bind( _on_recv_4_render, pInstNew );
	//
	pInstNew->m_server4Capture.m_eventAccepted.bind( _on_accepted_4_capture, pInstNew );
	pInstNew->m_server4Capture.m_eventDisConnected.bind( _on_disconnected_4_capture, pInstNew );
	pInstNew->m_server4Capture.m_eventRecvData.bind( _on_recv_4_capture, pInstNew );
	//
	pInstNew->m_server4ACP.start( "", (short)LAUDIO_PORT_PD_4_ACP );
	pInstNew->m_server4Render.start( "", (short)LAUDIO_PORT_PD_4_RENDER );
	pInstNew->m_server4Capture.start( "", (short)LAUDIO_PORT_PD_4_CAPTURE );
	//
	pInstNew->m_hThread4Render = (HANDLE)_beginthreadex( NULL, 0, _pd_thread_4_render, pInstNew, 0, NULL );
	if ( !pInstNew->m_hThread4Render )
		goto err;
	
	// for debug
	pInstNew->m_hPCMChannel0 = ::CreateFile( _T("channel_0.pcm"), GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, NULL, NULL );
	pInstNew->m_hPCMChannel1 = ::CreateFile( _T("channel_1.pcm"), GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, NULL, NULL ); 

	return pInstNew;

err:
	if ( pInstNew ) {
	    dk_pd_4_asio_destroy( pInstNew );
		pInstNew = NULL;
	}

	return NULL;
}

void dk_pd_4_asio_stop(struct dk_pd_root* pPDRoot) {
	if ( !pPDRoot )
		return;
	struct dk_pd_4_asio* pInst = pPDRoot->m_module.m_pPD4ASIO;
	if ( !pInst || !pInst->m_hEventExit )
		return;
	::SetEvent( pInst->m_hEventExit );
	::Sleep( 100 );
	//
	dk_pd_4_asio_destroy( pInst );
}

void dk_pd_4_asio_destroy( struct dk_pd_4_asio* pInst ) {
    if ( !pInst )
		return;
	dk_pd_channel_4_asio* pChannel = NULL;
	for ( int i = 0; i < DK_PD_CHANNEL_MAX; ++i ) {
	    pChannel = pInst->m_channels[ i ];
		if ( pChannel ) {
		    dk_pd_channel_stop_4_asio( pChannel );
			pChannel = NULL;
		}
		pInst->m_channels[ i ] = NULL;
	}
	if ( pInst->m_pDecoder4ACP ) {
	    delete pInst->m_pDecoder4ACP;
		pInst->m_pDecoder4ACP = NULL;
	}
	if ( pInst->m_hEventAccess ) {
		::CloseHandle( pInst->m_hEventAccess );
		pInst->m_hEventAccess = NULL;
	}
	if ( pInst->m_hEventExit ) {
		::CloseHandle( pInst->m_hEventExit );
		pInst->m_hEventExit = NULL;
	}
	if ( pInst->m_hEventBuf0Ready ) {
		::CloseHandle( pInst->m_hEventBuf0Ready );
		pInst->m_hEventBuf0Ready = NULL;
	}
	if ( pInst->m_hEventBuf1Ready ) {
		::CloseHandle( pInst->m_hEventBuf1Ready );
		pInst->m_hEventBuf1Ready = NULL;
	}
	if ( pInst->m_hThread4Render ) {
		::CloseHandle( pInst->m_hThread4Render );
		pInst->m_hThread4Render = NULL;
	}
	if ( pInst->m_hEventNewInstance ) {
		::CloseHandle( pInst->m_hEventNewInstance );
		pInst->m_hEventNewInstance = NULL;
	}
	if ( pInst->m_hPCMChannel0 ) {
	    ::CloseHandle( pInst->m_hPCMChannel0 );
		pInst->m_hPCMChannel0 = NULL;
	}
	if ( pInst->m_hPCMChannel1 ) {
		::CloseHandle( pInst->m_hPCMChannel1 );
		pInst->m_hPCMChannel1 = NULL;
	}
    delete pInst;
	pInst = NULL;
}

void _on_recv_4_acp( SOCKET sConnect, unsigned char* pBuf, unsigned int uLenBuf, void* pParam ) {
	struct dk_pd_4_asio* pInst = (struct dk_pd_4_asio*)pParam;

	pInst->m_pDecoder4ACP->pushback( pBuf, uLenBuf );
}

void _on_accepted_4_acp( SOCKET sConnect, void* pParam ) {
	// need more code here.
}

void _on_disconnected_4_acp( SOCKET sConnect, void* pParam ) {

}

void _on_whole_package_4_acp( void* pParamUser, unsigned char* pBuf, unsigned int uLenBuf ) {
	struct dk_pd_4_asio* pInst = (struct dk_pd_4_asio*)pParamUser;
	struct laudio_header* pHeader = (struct laudio_header*)pBuf;

	switch ( pHeader->m_cmd )
	{
	case LAUDIO_ACP_CMD_FIND_PD:
	{
	    _on_cmd_LAUDIO_ACP_CMD_FIND_PD( pInst, pBuf, uLenBuf );
	}
		break;
	default:
		break;
	}
	
}


void _on_cmd_LAUDIO_ACP_CMD_FIND_PD( struct dk_pd_4_asio* pInst, unsigned char* pBuf, unsigned int uLen ) {
	if ( !pInst || !pBuf || uLen == 0 )
		return;
	unsigned char cmd_LAUDIO_ACP_CMD_FIND_PD_ACK[ LAUDIO_PACKHEADER_SIZE + PAYLOAD_LEN_4_CMD_FIND_PD_ACK ];
	unsigned char* pPosStart = NULL;
	
	// fill header.
	LAUDIO_PACKAGE_HEADER_INIT_4_ACP( cmd_LAUDIO_ACP_CMD_FIND_PD_ACK, LAUDIO_ACP_CMD_FIND_PD_ACK, PAYLOAD_LEN_4_CMD_FIND_PD_ACK );
	// fill payload.
	pPosStart = &cmd_LAUDIO_ACP_CMD_FIND_PD_ACK[ LAUDIO_PACKHEADER_SIZE ];
	memcpy_s( pPosStart, 39, pInst->m_pPDRoot->m_szPDID, 39 );
	//
	pPosStart = &cmd_LAUDIO_ACP_CMD_FIND_PD_ACK[ LAUDIO_PACKHEADER_SIZE + 39];
	*pPosStart = DKBIT_HITTEST( pInst->m_pPDRoot->m_uFlag, DKPD_FLAG_BIND );
	//
	pPosStart = &cmd_LAUDIO_ACP_CMD_FIND_PD_ACK[ LAUDIO_PACKHEADER_SIZE + 39 + 1];
	memcpy_s( pPosStart, 39, pInst->m_pPDRoot->m_szASID, 39 );
	//
	pPosStart = &cmd_LAUDIO_ACP_CMD_FIND_PD_ACK[ LAUDIO_PACKHEADER_SIZE + 39 + 1 + 39];
	*pPosStart = DK_PD_CHANNEL_MAX;
	//
	pPosStart = &cmd_LAUDIO_ACP_CMD_FIND_PD_ACK[ LAUDIO_PACKHEADER_SIZE + 39 + 1 + 39 + 1];
	*pPosStart = LAUDIO_SORT_ASIO;
	//
	pInst->m_server4ACP.send( cmd_LAUDIO_ACP_CMD_FIND_PD_ACK, sizeof(cmd_LAUDIO_ACP_CMD_FIND_PD_ACK) );
}

unsigned int __stdcall _pd_thread_4_render( void* pParam ) {
	struct dk_pd_4_asio* pInst = (struct dk_pd_4_asio*)pParam;
	HANDLE aryEvent[4];
	DWORD dwWaitRet = 0;

	//
	aryEvent[0] = pInst->m_hEventExit;
	aryEvent[1] = pInst->m_hEventBuf0Ready;
	aryEvent[2] = pInst->m_hEventBuf1Ready;
	aryEvent[3] = pInst->m_hEventNewInstance;
	//
	while ( true )
	{
		dwWaitRet = ::WaitForMultipleObjectsEx( 4, aryEvent, FALSE, INFINITE, FALSE );
		if ( dwWaitRet == WAIT_OBJECT_0 ) {
		    break;
		} else if ( dwWaitRet == WAIT_OBJECT_0 + 1 ) {
			_req_pcmdata( pInst, 0 ); // request buf1
		    _do_render( pInst, 1 ); // render buf0
		} else if ( dwWaitRet == WAIT_OBJECT_0 + 2 ) {
			_req_pcmdata( pInst, 1 ); // request buf0
		    _do_render( pInst, 0 ); // render buf1
		} else if ( dwWaitRet == WAIT_OBJECT_0 + 3 ) {
		    _req_pcmdata( pInst, 1 ); // request buf0
		}
	}

    return 0;
}