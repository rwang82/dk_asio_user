#ifndef __DK_PCM_ENGINE_H__
#define __DK_PCM_ENGINE_H__
#include "LLXTSHelper.h"
//
class dk_asio_root;
class DKASIOUser;
//
class dk_pcm_engine {
public:
	dk_pcm_engine(dk_asio_root* pASIORoot);
	~dk_pcm_engine();

public:


private:
	void _on_event_IntfChange( DKASIOUser* pIntfOld, DKASIOUser* pIntfNew );
	void _on_event_WantDataBuf0( const char* pszPDID );
	void _on_event_WantDataBuf1( const char* pszPDID );


private:
	dk_asio_root* m_pASIORoot;
	DKASIOUser* m_pIntfCur;
	LLXTSHelper m_tsHelper;
};


#endif //__DK_PCM_ENGINE_H__