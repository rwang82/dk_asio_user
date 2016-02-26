#ifndef __LAUDIO_PACK_DECODER_H__
#define __LAUDIO_PACK_DECODER_H__
#include "laudio_pack_defs.h"
#include <functional>
//
class laudio_pack_decoder
{
public:
	typedef std::function< void( void* pParamUser, unsigned char* pBuf, unsigned int uLenBuf ) > func_recv_type;
public:
	laudio_pack_decoder(void* pParamUser, func_recv_type funcObjRecv );
	~laudio_pack_decoder();

public:
	void pushback(unsigned char* pBufIn, unsigned int uLenBufIn);
	
private:
	void _init();
	void _unInit();
	unsigned int _pushback_1_package(unsigned char* pBufIn, unsigned int uLenBufIn);
	/////////////////////////////////////
	// find the header start position.
	// if success, return the header start flag position, if failed, return NULL.
	unsigned char* _findHeaderStart(unsigned char* pBufIn, unsigned int uLenBufIn);
	bool _isHeaderValid();
	unsigned int _fillHeader( unsigned char* pBufIn, unsigned int uLenBufIn );
	unsigned int _fillPayload(unsigned char* pBufIn, unsigned int uLenBufIn);
	void _reFindHeaderStart();
	bool _isPackageReady();
	void _notifyPackageReady();
	void _resetPackageCache() { m_uLenValid = 0; }

private:
	unsigned int m_uFlag;
	unsigned char* m_pBufPackageCache;
	unsigned int m_uLenValid;
	void* m_pParamUser;
	func_recv_type m_funcObjRecv;
	
};


#endif //__LAUDIO_PACK_DECODER_H__