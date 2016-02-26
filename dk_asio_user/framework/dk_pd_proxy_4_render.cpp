#include "stdafx.h"
#include "dk_pd_proxy_4_render.h"
#include "dk_asio_root.h"
#include "laudio_pack_defs.h"
#include "laudio_pack_4_asio.h"

dk_pd_proxy_4_render::dk_pd_proxy_4_render(dk_asio_root* pASIORoot)
: m_pASIORoot( pASIORoot )
, m_client( false )
, m_packDecoder( this, std::bind( &dk_pd_proxy_4_render::_handle_package, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3 ) )
, m_pBufCmd( NULL )
, m_uLenCmdBuf( 0 ) {
	_init();
}

dk_pd_proxy_4_render::~dk_pd_proxy_4_render() {
	_unInit();
}

void dk_pd_proxy_4_render::_doConnect2PD( const char* pszIPAddr, short sPort ) {
	if ( !pszIPAddr )
		return;
	
	m_client.connect( pszIPAddr, sPort );
}

void dk_pd_proxy_4_render::_init() {
	//
	m_pASIORoot->m_eventConnect2PDReq.bind( this, &dk_pd_proxy_4_render::_on_event_Connect2PDReq );
	//
	m_client.m_eventConnected.bind( this, &dk_pd_proxy_4_render::_on_connected );
	m_client.m_eventDisConnect.bind( this, &dk_pd_proxy_4_render::_on_disconnected );
	m_client.m_eventRecvData.bind( this, &dk_pd_proxy_4_render::_on_recv );

}

void dk_pd_proxy_4_render::_unInit() {
	if ( m_pBufCmd ) {
	    delete []m_pBufCmd;
		m_pBufCmd = NULL;
		m_uLenCmdBuf = 0;
	}
}

void dk_pd_proxy_4_render::_on_connected() {
	m_pASIORoot->m_eventConnect2PD4RenderSuccess.invoke( m_client.getDestIPAddr().c_str() );
	//
	if ( m_pASIORoot->m_pIntfCur ) {
	    m_pASIORoot->m_proxyPDRender.sendNewInstance();
	}
}

void dk_pd_proxy_4_render::_on_disconnected() {
	m_pASIORoot->m_eventDisConnect2PD4Render.invoke( m_client.getDestIPAddr().c_str() );
}

void dk_pd_proxy_4_render::_on_recv( unsigned char* pBuf, unsigned int uLenBuf ) {
	if ( !pBuf || ( uLenBuf == 0 ) )
		return;
	m_packDecoder.pushback( pBuf, uLenBuf );
}

void dk_pd_proxy_4_render::_handle_package( void* pParamUser, unsigned char* pBuf, unsigned int uLenBuf ) {
	struct laudio_header* pHeader = (struct laudio_header*)pBuf;
	dk_pd_proxy_4_render* pInst = (dk_pd_proxy_4_render*)pParamUser;

	switch ( pHeader->m_cmd ) {
	case LAUDIO_ASIO_CMD_WANTDATA_BUF0:
	{
	    _onLAUDIO_ASIO_CMD_WANTDATA_BUF0( pBuf, uLenBuf );
	}
		break;
	case LAUDIO_ASIO_CMD_WANTDATA_BUF1:
	{
        char* pszPDID = (char*)(pBuf + LAUDIO_PACKHEADER_SIZE);
		*(pszPDID + 38) = 0;
		_onLAUDIO_ASIO_CMD_WANTDATA_BUF1( pBuf, uLenBuf );
	}
		break;
	}
}

void dk_pd_proxy_4_render::_on_event_Connect2PDReq( char* pszIPAddrPD, short sPortPD ) {
	_doConnect2PD( pszIPAddrPD, sPortPD );
}

void dk_pd_proxy_4_render::_onLAUDIO_ASIO_CMD_WANTDATA_BUF0( unsigned char* pBuf, unsigned int uLenBuf ) {
    char* pszPDID = (char*)(pBuf + LAUDIO_PACKHEADER_SIZE);
	*(pszPDID + 38) = 0;

	
	m_pASIORoot->m_eventWantDataBuf0.invoke( pszPDID );
}

void dk_pd_proxy_4_render::_onLAUDIO_ASIO_CMD_WANTDATA_BUF1( unsigned char* pBuf, unsigned int uLenBuf ) {
	char* pszPDID = (char*)(pBuf + LAUDIO_PACKHEADER_SIZE);
	*(pszPDID + 38) = 0;

	m_pASIORoot->m_eventWantDataBuf1.invoke( pszPDID );
}

void dk_pd_proxy_4_render::sendPCMData( unsigned int uToggle, unsigned char uChannel, unsigned char uChannelCount, unsigned char* pBufPCM, unsigned int uLenPCM ) {
	unsigned int uLenCmdNeed = LAUDIO_PACKHEADER_SIZE + 1 + 1 + 2 + uLenPCM;
	unsigned char* pBufPayload = NULL;
	if ( uLenCmdNeed > m_uLenCmdBuf ) {
		if ( m_pBufCmd ) {
		    delete []m_pBufCmd;
			m_pBufCmd = NULL;
		}
		m_pBufCmd = new unsigned char[ uLenCmdNeed ];
		m_uLenCmdBuf = uLenCmdNeed;
	}
	//
	LAUDIO_PACKAGE_HEADER_INIT_4_ASIO( m_pBufCmd
		, (uToggle == 0 ) ? LAUDIO_ASIO_CMD_PCMDATA_BUF0 : LAUDIO_ASIO_CMD_PCMDATA_BUF1
		, (uLenCmdNeed-LAUDIO_PACKHEADER_SIZE) );
	pBufPayload = m_pBufCmd + LAUDIO_PACKHEADER_SIZE;
	pBufPayload[0] = uChannel;
	pBufPayload[1] = uChannelCount;
	*((short*)(pBufPayload+2)) = uLenPCM;
	memcpy( (pBufPayload + 4), pBufPCM, uLenPCM );
    m_client.send( m_pBufCmd, uLenCmdNeed );
}

void dk_pd_proxy_4_render::sendNewInstance() {
    unsigned char bufCmd[ LAUDIO_PACKHEADER_SIZE ];

	LAUDIO_PACKAGE_HEADER_INIT_4_ASIO( bufCmd, LAUDIO_ASIO_CMD_NEW_INSTANCE, 0 );
	m_client.send( bufCmd, LAUDIO_PACKHEADER_SIZE );
}
