#ifndef __DK_PD_CHANNEL_4_ASIO_H__
#define __DK_PD_CHANNEL_4_ASIO_H__
#include <Windows.h>
//
struct dk_pd_4_asio;
//
struct dk_pd_channel_4_asio {
	unsigned int m_uIndexInAS; // index of audio source channel.
	unsigned char* m_pBuf0;
	unsigned int m_uLenBuf0Valid;
	unsigned char* m_pBuf1;
	unsigned int m_uLenBuf1Valid;
	struct dk_pd_4_asio* m_pPD4ASIO;
};

struct dk_pd_channel_4_asio* dk_pd_channel_start_4_asio( struct dk_pd_4_asio* pPD4ASIO, unsigned int uSizeCache );

void dk_pd_channel_stop_4_asio( struct dk_pd_channel_4_asio* pPDChannel4ASIO );



#endif //__DK_PD_CHANNEL_4_ASIO_H__