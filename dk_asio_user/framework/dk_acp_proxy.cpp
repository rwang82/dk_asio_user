#include "stdafx.h"
#include "dk_acp_proxy.h"
#include "laudio_pack_defs.h"
#include "laudio_pack_4_acp.h"
#include "laudio_port_defs.h"
#include "dk_asio_root.h"

dk_acp_proxy::dk_acp_proxy(dk_asio_root* pASIORoot)
: m_pASIORoot( pASIORoot )
, m_client( false )
, m_packDecoder( NULL, std::bind( &dk_acp_proxy::_handle_package, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3 ) )  {
	_init();
}

dk_acp_proxy::~dk_acp_proxy() {
	_unInit();
}

void dk_acp_proxy::_init() {
	//
	m_pASIORoot->m_eventConnect2PD4RenderSuccess.bind( this, &dk_acp_proxy::_on_event_Connect2PD4RenderSuccess );
	m_pASIORoot->m_eventDisConnect2PD4Render.bind( this, &dk_acp_proxy::_on_event_DisConnect2PD4Render );
	//
	m_client.m_eventConnected.bind( this, &dk_acp_proxy::_on_connected );
	m_client.m_eventDisConnect.bind( this, &dk_acp_proxy::_on_disconnected );
	m_client.m_eventRecvData.bind( this, &dk_acp_proxy::_on_recv );
	//
	m_client.connect( "127.0.0.1", (short)LAUDIO_PORT_ACP_4_AS );
}

void dk_acp_proxy::_unInit() {

}

void dk_acp_proxy::_on_connected() {
	//
	LAUDIO_PACKAGE_HEADER_INIT_4_ACP( m_bufCmdReg, LAUDIO_ACP_CMD_REGIST_AS2ACP_REQ, PAYLOAD_LEN_4_CMD_REGIST_AS2ACP_REQ );
	*((unsigned char*)(m_bufCmdReg + LAUDIO_PACKHEADER_SIZE)) = LAUDIO_SORT_ASIO;
	*((unsigned int*)(m_bufCmdReg + LAUDIO_PACKHEADER_SIZE) + 1) = ::GetCurrentProcessId();
	::GetModuleFileName( NULL, (TCHAR*)( m_bufCmdReg+LAUDIO_PACKHEADER_SIZE + 1 + 4 ), 256 );
	m_client.send( (unsigned char*)m_bufCmdReg, sizeof(m_bufCmdReg) );
}

void dk_acp_proxy::_on_disconnected() {

}

void dk_acp_proxy::_on_recv( unsigned char* pBuf, unsigned int uLenBuf ) {
	m_packDecoder.pushback( pBuf, uLenBuf );

}

void dk_acp_proxy::_handle_package( void* pParamUser, unsigned char* pBuf, unsigned int uLenBuf ) {
	laudio_header* pHeader = (laudio_header*)pBuf;

	if ( pHeader->m_sort != LAUDIO_SORT_ACP ) {
	    // if need output some error log.
		return;
	}

	switch ( pHeader->m_cmd )
	{
	case LAUDIO_ACP_CMD_REGIST_AS2ACP_ACK:
	{
		unsigned char bSuccess = *(pBuf + LAUDIO_PACKHEADER_SIZE);
	    char* pGUIDAS = (char*)( pBuf + LAUDIO_PACKHEADER_SIZE + 1 );
		if ( bSuccess ) {

		} else {

		}
	}
		break;
	case LAUDIO_ACP_CMD_CONNECT_PD_4_RENDER:
	{
	    _on_LAUDIO_ACP_CMD_CONNECT_PD_4_RENDER( pBuf, uLenBuf );
	}
		break;
	default:
		break;
	}
}

void dk_acp_proxy::_on_LAUDIO_ACP_CMD_CONNECT_PD_4_RENDER( unsigned char* pBuf, unsigned int uLenBuf ) {
	if ( !pBuf || ( uLenBuf != LAUDIO_PACKHEADER_SIZE + PAYLOAD_LEN_4_CMD_CONNECT_PD_4_RENDER ) )
		return;
    char* pszIPAddrPD = (char*)( pBuf + LAUDIO_PACKHEADER_SIZE );
	short sPort = *((short*)(pBuf + LAUDIO_PACKHEADER_SIZE + 17 ));
	
    m_pASIORoot->m_eventConnect2PDReq.invoke( pszIPAddrPD, sPort );
}

void dk_acp_proxy::_on_event_Connect2PD4RenderSuccess( const char* pszIPAddrPD ) {
	unsigned char cmdBuf[ LAUDIO_PACKHEADER_SIZE + PAYLOAD_LEN_4_CMD_CONNECT_PD_4_RENDER_ACK ];

	LAUDIO_PACKAGE_HEADER_INIT_4_ACP( cmdBuf, LAUDIO_ACP_CMD_CONNECT_PD_4_RENDER_ACK, PAYLOAD_LEN_4_CMD_CONNECT_PD_4_RENDER_ACK );
	strcpy_s( (char*)&cmdBuf[ LAUDIO_PACKHEADER_SIZE ], PAYLOAD_LEN_4_CMD_CONNECT_PD_4_RENDER_ACK, pszIPAddrPD );
	cmdBuf[ LAUDIO_PACKHEADER_SIZE + 17 ] = 1; // connect success.
	m_client.send( cmdBuf, LAUDIO_PACKHEADER_SIZE + PAYLOAD_LEN_4_CMD_CONNECT_PD_4_RENDER_ACK );
}

void dk_acp_proxy::_on_event_DisConnect2PD4Render( const char* pszIPAddrPD ) {
	unsigned char cmdBuf[ LAUDIO_PACKHEADER_SIZE + PAYLOAD_LEN_4_CMD_AS_DISCONNECT_WITH_PD_4_RENDER ];

	LAUDIO_PACKAGE_HEADER_INIT_4_ACP( cmdBuf, LAUDIO_ACP_CMD_AS_DISCONNECT_WITH_PD_4_RENDER, PAYLOAD_LEN_4_CMD_AS_DISCONNECT_WITH_PD_4_RENDER );
	strcpy_s( (char*)&cmdBuf[ LAUDIO_PACKHEADER_SIZE ], PAYLOAD_LEN_4_CMD_AS_DISCONNECT_WITH_PD_4_RENDER, pszIPAddrPD );
	m_client.send( cmdBuf, LAUDIO_PACKHEADER_SIZE + PAYLOAD_LEN_4_CMD_AS_DISCONNECT_WITH_PD_4_RENDER );

}







