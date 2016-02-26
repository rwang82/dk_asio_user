#ifndef __DK_PD_4_RENDER_H__
#define __DK_PD_4_RENDER_H__
//
struct dk_pd_4_asio;
//
void _on_accepted_4_render( SOCKET sConnect, void* pParam );
void _on_disconnected_4_render( SOCKET sConnect, void* pParam );
void _on_recv_4_render( SOCKET sConnect, unsigned char* pBuf, unsigned int uLenBuf, void* pParam );
void _on_whole_package_4_render( void* pParamUser, unsigned char* pBuf, unsigned int uLenBuf );
//
void _do_render( struct dk_pd_4_asio* pInst, unsigned int bBuf0 );
void _req_pcmdata( struct dk_pd_4_asio* pInst, unsigned int bBuf0 );



#endif //__DK_PD_4_RENDER_H__