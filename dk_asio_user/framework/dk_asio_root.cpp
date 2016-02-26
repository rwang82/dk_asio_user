#include "stdafx.h"
#include "dk_asio_root.h"
//
dk_asio_root g_asioRoot;
//
dk_asio_root::dk_asio_root()
: m_proxyACP( this )
, m_proxyPDRender( this )
, m_pIntfCur( NULL )
, m_pcmEngine( this ) {
	_init();
}

dk_asio_root::~dk_asio_root() {
	_unInit();
}

void dk_asio_root::_init() {
	m_eventConnect2PDReq.bind( &m_proxyPDRender, &dk_pd_proxy_4_render::_on_event_Connect2PDReq );

}

void dk_asio_root::_unInit() {

}

void dk_asio_root::setASIOInterface( DKASIOUser* pIntfNew ) {
    if ( m_pIntfCur == pIntfNew )
        return;

	//
	m_eventIntfChange.invoke( m_pIntfCur, pIntfNew );

	m_pIntfCur = pIntfNew;
}