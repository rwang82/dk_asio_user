#include "stdafx.h"
#include "dk_acp_root.h"

dk_acp_root::dk_acp_root()
: m_packageHandler( this )
, m_asManager( this ) {
	_init();
}

dk_acp_root::~dk_acp_root() {
	_unInit();
}

void dk_acp_root::_init() {
}

void dk_acp_root::_unInit() {
	
}
