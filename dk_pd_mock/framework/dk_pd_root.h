#ifndef __DK_PD_ROOT_H__
#define __DK_PD_ROOT_H__
#include "dk_socket_server.h"
//
#define DKPD_FLAG_NONE               (0x00000000)
#define DKPD_FLAG_RUNNING            (0x00000001)
#define DKPD_FLAG_BIND               (0X00000002)
//
struct dk_pd_4_asio;
//
struct dk_pd_root {
	unsigned int m_uFlag;
	char m_szPDID[39]; // play device id.
	char m_szASID[39]; // audio source id.
	unsigned int m_eSort; //as laudio_header::m_sort( LAUDIO_SORT_ASIO )
	union {
	   dk_pd_4_asio* m_pPD4ASIO; 
	} m_module;
};
//
extern struct dk_pd_root g_PDRoot;
//
bool dk_pd_start();
//
void dk_pd_stop();










#endif //__DK_PD_ROOT_H__