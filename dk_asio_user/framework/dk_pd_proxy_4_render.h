#ifndef __DK_PD_PROXY_H__
#define __DK_PD_PROXY_H__
#include "dk_socket_client.h"
#include "laudio_pack_decoder.h"
#include "laudio_pack_4_asio.h"
//
class dk_asio_root;
//
class dk_pd_proxy_4_render {
public:
    dk_pd_proxy_4_render(dk_asio_root* pASIORoot);
    ~dk_pd_proxy_4_render();

public:
	void sendPCMData( unsigned int uToggle, unsigned char uChannel, unsigned char uChannelCount, unsigned char* pBufPCM, unsigned int uLenPCM );
	void sendNewInstance();

private:
	void _init();
	void _unInit();
	void _on_connected();
	void _on_disconnected();
	void _on_recv( unsigned char* pBuf, unsigned int uLenBuf );
	void _handle_package( void* pParamUser, unsigned char* pBuf, unsigned int uLenBuf );
	void _doConnect2PD( const char* pszIPAddr, short sPort );
//
private:
	void _onLAUDIO_ASIO_CMD_WANTDATA_BUF0( unsigned char* pBuf, unsigned int uLenBuf );
	void _onLAUDIO_ASIO_CMD_WANTDATA_BUF1( unsigned char* pBuf, unsigned int uLenBuf );

public:
	void _on_event_Connect2PDReq( char* pszIPAddrPD, short sPortPD );

private:
	dk_asio_root* m_pASIORoot;
	dk_socket_client m_client;
	laudio_pack_decoder m_packDecoder;
	char m_szPDID[ 39 ];
	unsigned char* m_pBufCmd;
	unsigned int m_uLenCmdBuf;
};









#endif //__DK_PD_PROXY_H__