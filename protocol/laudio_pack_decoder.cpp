#include "stdafx.h"
#include "laudio_pack_decoder.h"
#include "laudio_pack_defs.h"
#include "dk_bit_helper.h"
#include <assert.h>
#include <string.h>
//
#define LPD_FLAG_NONE (0x00000000)
//
#ifndef min
#define min( a, b ) ( (a) < (b) ? (a) : (b) )
#endif //min
//
#ifndef max
#define max( a, b ) ( (a) > (b) ? (a) : (b) )
#endif //max

laudio_pack_decoder::laudio_pack_decoder(void* pParamUser, func_recv_type funcObjRecv )
	: m_uFlag( 0 )
	, m_pParamUser( pParamUser )
	, m_pBufPackageCache( 0 )
	, m_uLenValid( LPD_FLAG_NONE )
	, m_funcObjRecv( funcObjRecv ) {
	_init();
}



laudio_pack_decoder::~laudio_pack_decoder() {
	_unInit();
}

void laudio_pack_decoder::pushback(unsigned char* pBufIn, unsigned int uLenBufIn) {
	unsigned int uLenEat = 0;

	while (uLenBufIn > 0 && (uLenEat = _pushback_1_package(pBufIn, uLenBufIn))) {
		pBufIn += uLenEat;
		uLenBufIn -= uLenEat;
	}

	return;
}

bool laudio_pack_decoder::_isHeaderValid() {
	if (m_uLenValid < LAUDIO_PACKHEADER_SIZE)
		return false;
	laudio_header* pHeader = (laudio_header*)m_pBufPackageCache;
	if (pHeader->m_len > LAUDIO_PACKAGE_LEN_MAX) {
	    // if need output some log.
	}
	return (pHeader->m_start_flag == LAUDIO_START_FLAG)
		&& (pHeader->m_end_flag == LAUDIO_END_FLAG)
		&& (pHeader->m_crc8 == LAUDIO_CALC_CRC8( pHeader))
		&& (pHeader->m_len <= LAUDIO_PACKAGE_LEN_MAX);
}

unsigned int laudio_pack_decoder::_pushback_1_package(unsigned char* pBufIn, unsigned int uLenBufIn) {
	if (!pBufIn || uLenBufIn == 0)
		return 0;
	unsigned int uLenNeedCopy = 0;
	unsigned int uLenEat = 0;

	// fill header.
	if (m_uLenValid < LAUDIO_PACKHEADER_SIZE) {
		uLenEat = _fillHeader(pBufIn, uLenBufIn);
	}
	if (m_uLenValid < LAUDIO_PACKHEADER_SIZE) {
		assert(uLenEat == uLenBufIn);
		return uLenEat;
	}
	// check header valid.
	if (!_isHeaderValid()) {
		_reFindHeaderStart();
		return uLenEat;
	}
	// find payload.
	uLenEat += _fillPayload( pBufIn + uLenEat, uLenBufIn - uLenEat );
	//
	if (_isPackageReady()) {
		_notifyPackageReady();
		_resetPackageCache();
	}

	return uLenEat;
}

void laudio_pack_decoder::_notifyPackageReady() {
	if (m_uLenValid < LAUDIO_PACKHEADER_SIZE)
		return;
	laudio_header* pHeader = (laudio_header*)m_pBufPackageCache;

	if (m_uLenValid != pHeader->m_len) {
		assert( false );
		return;
	}

	//if (m_pfnHandleCmdPackage) {
	//	m_pfnHandleCmdPackage(m_pParamUser, m_pBufPackageCache, m_uLenValid);
	//}
	m_funcObjRecv( m_pParamUser, m_pBufPackageCache, m_uLenValid );
}

bool laudio_pack_decoder::_isPackageReady() {
	if (m_uLenValid < LAUDIO_PACKHEADER_SIZE) return false;
	laudio_header* pHeader = (laudio_header*)m_pBufPackageCache;
	if (m_uLenValid > pHeader->m_len) {
		//DKTunLogOutput( "m_uLenValid > pHeader->m_len" );
	}
	return m_uLenValid >= pHeader->m_len;
}

void laudio_pack_decoder::_reFindHeaderStart() {
	unsigned char* pPosHeaderStart = NULL;
	unsigned int uLenNeedMove = 0;

	assert(m_uLenValid == LAUDIO_PACKHEADER_SIZE);
	pPosHeaderStart = _findHeaderStart(m_pBufPackageCache, LAUDIO_PACKHEADER_SIZE);
	if (pPosHeaderStart) {
		uLenNeedMove = (m_pBufPackageCache + LAUDIO_PACKHEADER_SIZE) - pPosHeaderStart;
		memmove(m_pBufPackageCache, pPosHeaderStart, uLenNeedMove);
		m_uLenValid = uLenNeedMove;
	} else {
		m_uLenValid = 0;
	}
}

unsigned int laudio_pack_decoder::_fillPayload(unsigned char* pBufIn, unsigned int uLenBufIn) {
	assert(m_uLenValid >= LAUDIO_PACKHEADER_SIZE);
	unsigned int uLenEat = 0;
	unsigned int uLenNeedCopy = 0;
	laudio_header* pHeader = (laudio_header*)m_pBufPackageCache;

	uLenNeedCopy = min(pHeader->m_len - m_uLenValid, uLenBufIn);
	memcpy(m_pBufPackageCache + m_uLenValid, pBufIn, uLenNeedCopy);
	m_uLenValid += uLenNeedCopy;
	uLenEat = uLenNeedCopy;
	return uLenEat;
}

unsigned int laudio_pack_decoder::_fillHeader(unsigned char* pBufIn, unsigned int uLenBufIn) {
	unsigned char* pPosHeaderStart = NULL;
	unsigned int uLenEat = 0;
	unsigned int uLenNeedCopy = 0;

	if (m_uLenValid == 0) {
		pPosHeaderStart = _findHeaderStart(pBufIn, uLenBufIn);
		if (!pPosHeaderStart) {
			uLenEat = uLenBufIn;
			return uLenEat;
		}
		uLenEat += (pPosHeaderStart - pBufIn);
		uLenBufIn -= (pPosHeaderStart - pBufIn);
		pBufIn = pPosHeaderStart;
	}
	assert(m_uLenValid<LAUDIO_PACKHEADER_SIZE);
	uLenNeedCopy = min(LAUDIO_PACKHEADER_SIZE - m_uLenValid, uLenBufIn);
	memcpy(m_pBufPackageCache + m_uLenValid, pBufIn, uLenNeedCopy);
	m_uLenValid += uLenNeedCopy;
	uLenEat += uLenNeedCopy;

	return uLenEat;
}

unsigned char* laudio_pack_decoder::_findHeaderStart(unsigned char* pBuf, unsigned int uLenBuf) {
	unsigned char* pHeaderStart = (unsigned char*)memchr(pBuf, LAUDIO_START_FLAG, uLenBuf);
	return pHeaderStart;
}

void laudio_pack_decoder::_init() {
	m_pBufPackageCache = new unsigned char[LAUDIO_PACKAGE_LEN_MAX];
}

void laudio_pack_decoder::_unInit() {
	if (m_pBufPackageCache) {
		delete[]m_pBufPackageCache;
		m_pBufPackageCache = NULL;
	}
}
