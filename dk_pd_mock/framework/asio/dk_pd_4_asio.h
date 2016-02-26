#ifndef __DK_PD_4_ASIO_H__
#define __DK_PD_4_ASIO_H__
#include "dk_pd_envcfg.h"
#include "dk_pd_channel_4_asio.h"
#include "dk_socket_server.h"
#include "laudio_pack_decoder.h"
#include <Windows.h>
//
struct dk_pd_root;
//
struct dk_pd_4_asio {
	dk_pd_root* m_pPDRoot;
    dk_pd_channel_4_asio* m_channels[ DK_PD_CHANNEL_MAX ];
	unsigned int m_uFlag;
	HANDLE m_hEventAccess;
	HANDLE m_hEventExit;
	HANDLE m_hEventBuf0Ready;
	HANDLE m_hEventBuf1Ready;
	HANDLE m_hEventNewInstance;
	HANDLE m_hThread4Render;
	dk_socket_server m_server4ACP;
	dk_socket_server m_server4Render;
	dk_socket_server m_server4Capture;
	laudio_pack_decoder* m_pDecoder4ACP;
	laudio_pack_decoder* m_pDecoder4Render;
	laudio_pack_decoder* m_pDecoder4Capture;
	unsigned int m_uSizePCMBuf;
	// for debug.
	HANDLE m_hPCMChannel0;
	HANDLE m_hPCMChannel1;
};

struct dk_pd_4_asio* dk_pd_4_asio_start( struct dk_pd_root* pPDRoot);

void dk_pd_4_asio_stop( struct dk_pd_root* pPDRoot );

#endif //__DK_PD_4_ASIO_H__