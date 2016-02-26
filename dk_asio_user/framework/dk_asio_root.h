#ifndef __DK_ASIO_ROOT_H__
#define __DK_ASIO_ROOT_H__
#include "dk_acp_proxy.h"
#include "dk_pd_proxy_4_render.h"
#include "hmcmn_event.h"
#include "dk_asio_user.h"
#include "dk_pcm_engine.h"

class dk_asio_root {
public:
	dk_asio_root();
	~dk_asio_root();
	

public:
    void setASIOInterface( DKASIOUser* pIntfNew );
	
private:
	void _init();
	void _unInit();

public:
	hmcmn::hm_event< char*, short > m_eventConnect2PDReq;
	hmcmn::hm_event< const char* > m_eventConnect2PD4RenderSuccess;
	hmcmn::hm_event< const char* > m_eventDisConnect2PD4Render;
	hmcmn::hm_event< DKASIOUser*, DKASIOUser* > m_eventIntfChange;
	hmcmn::hm_event< const char* > m_eventWantDataBuf0;
	hmcmn::hm_event< const char* > m_eventWantDataBuf1;

public:
	DKASIOUser* m_pIntfCur;
	dk_acp_proxy m_proxyACP;
	dk_pd_proxy_4_render m_proxyPDRender;
	char m_szASID[39];
	dk_pcm_engine m_pcmEngine;
};


#endif //__DK_ASIO_ROOT_H__