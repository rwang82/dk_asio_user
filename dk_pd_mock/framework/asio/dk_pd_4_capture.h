#ifndef __DK_PD_4_CAPTURE_H__
#define __DK_PD_4_CAPTURE_H__

void _on_accepted_4_capture( SOCKET sConnect, void* pParam );
void _on_disconnected_4_capture( SOCKET sConnect, void* pParam );
void _on_recv_4_capture( SOCKET sConnect, unsigned char* pBuf, unsigned int uLenBuf, void* pParam );
void _on_whole_package_4_capture( void* pParamUser, unsigned char* pBuf, unsigned int uLenBuf );



#endif //__DK_PD_4_CAPTURE_H__