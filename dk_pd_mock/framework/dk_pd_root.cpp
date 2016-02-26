#include "dk_pd_root.h"
#include "dk_socket_server.h"
#include "dk_guid.h"
#include "laudio_port_defs.h"
#include "laudio_pack_defs.h"
#include "asio/dk_pd_4_asio.h"
//
struct dk_pd_root g_PDRoot;

bool dk_pd_start() {
	if ( ( g_PDRoot.m_uFlag & DKPD_FLAG_RUNNING ) == DKPD_FLAG_RUNNING )
		return false;
	std::string strGUID;

	if ( !createGUIDStr( strGUID ) )
		return false;
	strcpy_s( g_PDRoot.m_szPDID, strGUID.c_str() );
	memset( g_PDRoot.m_szASID, 0, sizeof(g_PDRoot.m_szASID) );
	//
	dk_pd_4_asio_start( &g_PDRoot );

	return true;
}
//
void dk_pd_stop() {
	switch ( g_PDRoot.m_eSort )
	{
	case LAUDIO_SORT_ASIO:
	{
	    dk_pd_4_asio_stop( &g_PDRoot );
	}
		break;
	default:
		break;
	}
}