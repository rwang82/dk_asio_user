#include "stdafx.h"
#include "dk_pd_4_capture.h"
#include "dk_pd_4_asio.h"

void _on_accepted_4_capture( SOCKET sConnect, void* pParam ) {

}

void _on_disconnected_4_capture( SOCKET sConnect, void* pParam ) {

}

void _on_recv_4_capture( SOCKET sConnect, unsigned char* pBuf, unsigned int uLenBuf, void* pParam ) {
	struct dk_pd_4_asio* pInst = (struct dk_pd_4_asio*)pParam;

	pInst->m_pDecoder4Capture->pushback( pBuf, uLenBuf );
}

void _on_whole_package_4_capture( void* pParamUser, unsigned char* pBuf, unsigned int uLenBuf ) {

}












