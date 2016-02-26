#include "stdafx.h"
#include "dk_as_module.h"
#include "laudio_pack_4_acp.h"
#include "laudio_port_defs.h"
#include "dk_guid.h"
#include "dk_acp_root.h"
#include "Objbase.h"

dk_as_module::dk_as_module(dk_acp_root* pACPRoot)
: m_eAS2PDStatus( EAS2PD_STATUS_DISCONNECTED )
, m_pACPRoot( pACPRoot )
, m_packDecoder( NULL, std::bind( &dk_as_module::_handle_package, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3 ) ) {
	_init();
}

dk_as_module::~dk_as_module() {
	_unInit();
}

void dk_as_module::_init() {
	m_server.start( "127.0.0.1", (short)LAUDIO_PORT_ACP_4_AS );
	m_server.m_eventAccepted.bind( this, &dk_as_module::_on_accepted );
	m_server.m_eventDisConnected.bind( this, &dk_as_module::_on_disconnected );
	m_server.m_eventRecvData.bind( this, &dk_as_module::_on_recv );
}

void dk_as_module::_unInit() {

}

void dk_as_module::_on_accepted( SOCKET sConnect ) {
	if ( !m_tsAccess.safeEnterFunc() )
		return;

	//m_mapASUnReg[ sConnect ] = new dk_as_proxy();

	m_tsAccess.safeExitFunc();
}

void dk_as_module::_on_disconnected( SOCKET sConnect ) {
	if ( !m_tsAccess.safeEnterFunc() )
		return;
	as_map_type::iterator itAS;

	////
	//itAS = m_mapASUnReg.find( sConnect );
	//if ( itAS != m_mapASUnReg.end() ) {
	//    delete itAS->second;
	//	itAS->second = NULL;
	//	m_mapASUnReg.erase( itAS );
	//}

	////
	//itAS = m_mapASReg.find( sConnect );
	//if ( itAS != m_mapASReg.end() ) {
	//    delete itAS->second;
	//	itAS->second = NULL;
	//	m_mapASReg.erase( itAS );
	//}

	m_tsAccess.safeExitFunc();

	//
	m_eventASRegStatus.invoke( 0, "" );
}

void dk_as_module::_on_recv( SOCKET sConnect, unsigned char* pBuf, unsigned int uLenBuf ) {
	if ( !m_tsAccess.safeEnterFunc() )
		return;

	m_packDecoder.pushback( pBuf, uLenBuf );

	m_tsAccess.safeExitFunc();
}

void dk_as_module::_handle_package( void* pParamUser, unsigned char* pBuf, unsigned int uLenBuf ) {
	laudio_header* pHeader = (laudio_header*)pBuf;

	if ( pHeader->m_sort != LAUDIO_SORT_ACP ) 
		return;

	switch ( pHeader->m_cmd )
	{
	case LAUDIO_ACP_CMD_REGIST_AS2ACP_REQ:
	{
	    _on_LAUDIO_ACP_CMD_REGIST_AS2ACP_REQ( pBuf, uLenBuf );
	}
		break;
	case LAUDIO_ACP_CMD_AS_DISCONNECT_WITH_PD_4_RENDER:
	{
	    _on_LAUDIO_ACP_CMD_AS_DISCONNECT_WITH_PD_4_RENDER( pBuf, uLenBuf );
	}
		break;
	case LAUDIO_ACP_CMD_CONNECT_PD_4_RENDER_ACK:
	{
	    _on_LAUDIO_ACP_CMD_CONNECT_PD_4_RENDER_ACK( pBuf, uLenBuf );
	}

	default:
		break;
	}
}

void dk_as_module::_setAS2PDStatus( ENUMAS2PDSTATUS eStatusNew ) {
    if ( m_eAS2PDStatus == eStatusNew )
		return;
	ENUMAS2PDSTATUS eStatusOld = m_eAS2PDStatus;
	m_eAS2PDStatus = eStatusNew;
	// notify change connect status.
	m_eventAS2PDStatusChange.invoke( eStatusOld, m_eAS2PDStatus );
}

void dk_as_module::_on_LAUDIO_ACP_CMD_CONNECT_PD_4_RENDER_ACK( unsigned char* pBuf, unsigned int uLenBuf ) {
	char* pszIPAddrPD = (char*)(pBuf + LAUDIO_PACKHEADER_SIZE );
	unsigned char bSuccess = (pBuf[ LAUDIO_PACKHEADER_SIZE + 17 ] == 1);

    _setAS2PDStatus( bSuccess ? EAS2PD_STATUS_CONNECTED : EAS2PD_STATUS_DISCONNECTED );
}

void dk_as_module::_on_LAUDIO_ACP_CMD_AS_DISCONNECT_WITH_PD_4_RENDER( unsigned char* pBuf, unsigned int uLenBuf ) {
	
	_setAS2PDStatus( EAS2PD_STATUS_CONNECTING );
}

void dk_as_module::_on_LAUDIO_ACP_CMD_REGIST_AS2ACP_REQ( unsigned char* pBuf, unsigned int uLenBuf ) {
	unsigned int uPID = *((unsigned int*)(pBuf + LAUDIO_PACKHEADER_SIZE + 1));
	TCHAR* pProcessName = (TCHAR*)(pBuf + LAUDIO_PACKHEADER_SIZE + 1 + 4);
	std::string strGUID;
	if ( !createGUIDStr( strGUID ) )
		return;
	LAUDIO_PACKAGE_HEADER_INIT_4_ACP( m_cmdRegAck, LAUDIO_ACP_CMD_REGIST_AS2ACP_ACK, PAYLOAD_LEN_4_CMD_REGIST_AS2ACP_ACK );
    *(unsigned char*)(m_cmdRegAck + LAUDIO_PACKHEADER_SIZE) = 1;
	memcpy( (m_cmdRegAck + LAUDIO_PACKHEADER_SIZE + 1), strGUID.c_str(), min( 39, strGUID.length() + 1) );
	_sendCmd2AS( (unsigned char*)m_cmdRegAck, sizeof(m_cmdRegAck) );
	//
	m_eventASRegStatus.invoke( 1, strGUID.c_str() );
}

bool dk_as_module::_sendCmd2AS( unsigned char* pBufCmd, unsigned int uLenBufCmd ) {
	if ( !pBufCmd || ( uLenBufCmd == 0 ) )
		return false;
	if ( m_server.send( pBufCmd, uLenBufCmd ) <= 0 ) {
	    // notify send cmd failed.

		return false;
	}

	// notify send cmd success.
    return true;
}

void dk_as_module::sendCmdASConnect2PD( const char* pszIPAddrPD ) {
	if ( !pszIPAddrPD )
		return;
	unsigned char bufCmdConnectPD[ LAUDIO_PACKHEADER_SIZE + PAYLOAD_LEN_4_CMD_CONNECT_PD_4_RENDER ];
	
	// notify change connect status.
	_setAS2PDStatus( EAS2PD_STATUS_CONNECTING );
	//
	LAUDIO_PACKAGE_HEADER_INIT_4_ACP( bufCmdConnectPD, LAUDIO_ACP_CMD_CONNECT_PD_4_RENDER, PAYLOAD_LEN_4_CMD_CONNECT_PD_4_RENDER );
	memcpy_s( &bufCmdConnectPD[ LAUDIO_PACKHEADER_SIZE ], 17, pszIPAddrPD, strlen( pszIPAddrPD )+1 );
	*((short*)&bufCmdConnectPD[ LAUDIO_PACKHEADER_SIZE + 17 ]) = LAUDIO_PORT_PD_4_RENDER;
    _sendCmd2AS( bufCmdConnectPD, sizeof(bufCmdConnectPD) );
}