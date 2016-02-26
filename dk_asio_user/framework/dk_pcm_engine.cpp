#include "stdafx.h"
#include "dk_pcm_engine.h"
#include "dk_asio_root.h"

dk_pcm_engine::dk_pcm_engine(dk_asio_root* pASIORoot)
: m_pASIORoot( pASIORoot )
, m_pIntfCur( NULL ) {
	m_pASIORoot->m_eventIntfChange.bind( this, &dk_pcm_engine::_on_event_IntfChange );
	m_pASIORoot->m_eventWantDataBuf0.bind( this, &dk_pcm_engine::_on_event_WantDataBuf0 );
	m_pASIORoot->m_eventWantDataBuf1.bind( this, &dk_pcm_engine::_on_event_WantDataBuf1 );
}

dk_pcm_engine::~dk_pcm_engine() {

}

void dk_pcm_engine::_on_event_IntfChange( DKASIOUser* pIntfOld, DKASIOUser* pIntfNew ) {
	if ( !m_tsHelper.safeEnterFunc() )
		return;
	 assert( pIntfOld == m_pIntfCur );
	m_pIntfCur = pIntfNew;
	if ( m_pIntfCur ) {
	    m_pASIORoot->m_proxyPDRender.sendNewInstance();
	}
	m_tsHelper.safeExitFunc();
}	

void dk_pcm_engine::_on_event_WantDataBuf0( const char* pszPDID ) {
	if ( !m_tsHelper.safeEnterFunc() )
		return;
	if ( m_pIntfCur ) {
	    m_pIntfCur->wantDataBuf0();
	}
	m_tsHelper.safeExitFunc();
}

void dk_pcm_engine::_on_event_WantDataBuf1( const char* pszPDID ) {
	if ( !m_tsHelper.safeEnterFunc() )
		return;
	if ( m_pIntfCur ) {
	    m_pIntfCur->wantDataBuf1();
	}
	m_tsHelper.safeExitFunc();

}
