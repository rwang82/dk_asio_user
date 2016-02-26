#ifndef __DK_AS_MANAGER_H__
#define __DK_AS_MANAGER_H__
#include "dk_as_proxy.h"
#include "LLXTSHelper.h"
#include "laudio_pack_decoder.h"
#include "laudio_pack_4_acp.h"
#include "dk_socket_server.h"
#include "dk_acp_defs.h"
#include "hmcmn_event.h"
#include <map>
//
class dk_acp_root;
//
class dk_as_module {
	typedef std::map< SOCKET, dk_as_proxy* > as_map_type;
public:
	dk_as_module(dk_acp_root* pACPRoot);
	~dk_as_module();

public:
	void sendCmdASConnect2PD( const char* pszIPAddrPD );

private:
	void _init();
	void _unInit();
	void _on_accepted( SOCKET sConnect );
	void _on_disconnected( SOCKET sConnect );
	void _on_recv( SOCKET sConnect, unsigned char* pBuf, unsigned int uLenBuf );
	void _handle_package( void* pParamUser, unsigned char* pBuf, unsigned int uLenBuf );
	bool _sendCmd2AS( unsigned char* pBufCmd, unsigned int uLenBufCmd );
	void _setAS2PDStatus( ENUMAS2PDSTATUS eStatusNew );

private:
	void _on_LAUDIO_ACP_CMD_REGIST_AS2ACP_REQ( unsigned char* pBuf, unsigned int uLenBuf );
	void _on_LAUDIO_ACP_CMD_CONNECT_PD_4_RENDER_ACK( unsigned char* pBuf, unsigned int uLenBuf );
	void _on_LAUDIO_ACP_CMD_AS_DISCONNECT_WITH_PD_4_RENDER( unsigned char* pBuf, unsigned int uLenBuf );
	
public:
	hmcmn::hm_event<ENUMAS2PDSTATUS, ENUMAS2PDSTATUS> m_eventAS2PDStatusChange;
	hmcmn::hm_event<bool, const char*> m_eventASRegStatus;

private:
	ENUMAS2PDSTATUS m_eAS2PDStatus;
	dk_acp_root* m_pACPRoot;
	dk_socket_server m_server;
	LLXTSHelper m_tsAccess;
	//as_map_type m_mapASUnReg;
	//as_map_type m_mapASReg;
	laudio_pack_decoder m_packDecoder;
	unsigned char m_cmdRegAck[ LAUDIO_PACKHEADER_SIZE + PAYLOAD_LEN_4_CMD_REGIST_AS2ACP_REQ ];
};


#endif //__DK_AS_MANAGER_H__