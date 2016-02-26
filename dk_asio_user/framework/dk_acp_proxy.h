#ifndef __DK_ACP_PROXY_H__
#define __DK_ACP_PROXY_H__
#include "dk_socket_client.h"
#include "laudio_pack_decoder.h"
#include "laudio_pack_4_acp.h"
//
class dk_asio_root;
//
class dk_acp_proxy {
public:
	dk_acp_proxy(dk_asio_root* pASIORoot);
	~dk_acp_proxy();

private:
	void _init();
	void _unInit();
	void _on_connected();
	void _on_disconnected();
	void _on_recv( unsigned char* pBuf, unsigned int uLenBuf );
	void _handle_package( void* pParamUser, unsigned char* pBuf, unsigned int uLenBuf );

private:
	void _on_event_Connect2PD4RenderSuccess( const char* pszIPAddrPD );
	void _on_event_DisConnect2PD4Render( const char* pszIPAddrPD );

private:
	void _on_LAUDIO_ACP_CMD_CONNECT_PD_4_RENDER( unsigned char* pBuf, unsigned int uLenBuf );

private:
	dk_asio_root* m_pASIORoot;
	dk_socket_client m_client;
	laudio_pack_decoder m_packDecoder;
	unsigned char m_bufCmdReg[ LAUDIO_PACKHEADER_SIZE + PAYLOAD_LEN_4_CMD_REGIST_AS2ACP_REQ ];
};



#endif //__DK_ACP_PROXY_H__