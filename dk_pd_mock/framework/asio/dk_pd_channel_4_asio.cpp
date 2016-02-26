#include "stdafx.h"
#include "dk_pd_channel_4_asio.h"
#include "dk_pd_4_asio.h"
//
#define DK_PD_PCMBUF_4_ASIO_MAX (1024*1024)
//
static void dk_pd_channel_destroy( struct dk_pd_channel_4_asio* pChannel );
//
struct dk_pd_channel_4_asio* dk_pd_channel_start_4_asio( struct dk_pd_4_asio* pPD4ASIO, unsigned int uSizeCache ) {
	DWORD dwThreadId;
	struct dk_pd_channel_4_asio* pChannel = (struct dk_pd_channel_4_asio*)malloc( sizeof(struct dk_pd_channel_4_asio) );
    if ( !pChannel )
		return NULL;
	memset( pChannel, 0, sizeof( struct dk_pd_channel_4_asio ) );
	//
	pChannel->m_pPD4ASIO = pPD4ASIO;
	pChannel->m_pBuf0 = (unsigned char*)malloc( uSizeCache );
	memset( pChannel->m_pBuf0, 0, uSizeCache );
	pChannel->m_uLenBuf0Valid = 0;
	pChannel->m_pBuf1 = (unsigned char*)malloc( uSizeCache );
	memset( pChannel->m_pBuf1, 0, uSizeCache );
	pChannel->m_uLenBuf1Valid = 0;
	if ( !pChannel->m_pBuf0 || !pChannel->m_pBuf1 )
		goto err;

    return pChannel;

err:
	if ( pChannel ) {
	    dk_pd_channel_destroy( pChannel );
		pChannel = NULL;
	}
	return NULL;
}

void dk_pd_channel_stop_4_asio( struct dk_pd_channel_4_asio* pPDChannel4ASIO ) {
	if ( !pPDChannel4ASIO 
		|| pPDChannel4ASIO->m_pPD4ASIO 
		|| !pPDChannel4ASIO->m_pPD4ASIO->m_hEventExit )
		return;
	//
	dk_pd_channel_destroy( pPDChannel4ASIO );
}

void dk_pd_channel_destroy( struct dk_pd_channel_4_asio* pChannel ) {
    if ( !pChannel )
		return;
	if ( pChannel->m_pBuf0 ) {
	    free( pChannel->m_pBuf0 );
		pChannel->m_pBuf0 = NULL;
	}
	if ( pChannel->m_pBuf1 ) {
	    free( pChannel->m_pBuf1 );
		pChannel->m_pBuf1 = NULL;
	}
	pChannel->m_pPD4ASIO = NULL;
	
	free( pChannel );
	pChannel = NULL;
}