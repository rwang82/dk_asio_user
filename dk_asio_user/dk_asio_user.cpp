#include "stdafx.h"
#include "dk_asio_user.h"
#include "dk_bit_helper.h"
#include "dk_asio_root.h"
#include "dk_iswow64.h"
#include <assert.h>
#include <MMSystem.h>

#define DKASIO_DEVNAME ("\\\\.\\DKASIO")

//CLSID IID_ASIO_DRIVER = { 0x188135e1, 0xd565, 0x11d2, { 0x86, 0x35, 0x0, 0xa0, 0xc9, 0x9f, 0x5d, 0x19 } };
static const GUID IID_DKASIOUSER_DRIVER = { 0x188135e1, 0xd565, 0x11d2, { 0x86, 0x35, 0x0, 0xa0, 0xc9, 0x9f, 0x5d, 0x19 } };
//{ 0x986d5d9c, 0x7377, 0x4b22, { 0xa7, 0xc3, 0xa4, 0x74, 0xd1, 0x51, 0xa6, 0x70 } };
// {986D5D9C - 7377 - 4B22 - A7C3 - A474D151A670}

CFactoryTemplate g_templates[1] =
{
	{ L"DKASIOUser", &IID_DKASIOUSER_DRIVER, DKASIOUser::CreateInstance }
};
//
extern dk_asio_root g_asioRoot;
//
#ifndef SIZEOF_ARRAY
#define SIZEOF_ARRAY(ar)        (sizeof(ar)/sizeof((ar)[0]))
#endif // !defined(SIZEOF_ARRAY)
int g_templateCount = SIZEOF_ARRAY(g_templates);

static const double twoRaisedTo32 = 4294967296.;
static const double twoRaisedTo32Reciprocal = 1. / twoRaisedTo32;

#if (DBG)
void UDBG(char *msg, ...)
{
	static char errstr[1024];

	va_list argp;

	va_start(argp, msg);
	StringCchVPrintfA(errstr, sizeof(errstr), msg, argp);
	va_end(argp);

	OutputDebugStringA("ASIODrv:");
	OutputDebugStringA(errstr);
	OutputDebugStringA("\r\n");
}
#else
void UDBG(char *msg, ...){};
#endif


extern LONG RegisterAsioDriver(CLSID, char *, char *, char *, char *);
extern LONG UnregisterAsioDriver(CLSID, char *, char *);
static void getMMETimeStamp (ASIOTimeStamp* ts);

//
// Server registration, called on REGSVR32.EXE "the dllname.dll"
//
HRESULT _stdcall DllRegisterServer()
{
	LONG	rc;
	char	errstr[128];

	rc = RegisterAsioDriver(
		IID_DKASIOUSER_DRIVER,
		"dk_asio_user.dll",
		"DK ASIO Driver",
		"DK ASIO",
		"Apartment");

	if (rc)
	{
		memset(errstr, 0, 128);
		sprintf_s(errstr, "Register Server failed ! (%d)", rc);
		::OutputDebugStringA(errstr);
		//MessageBox(0, (LPCTSTR)errstr, (LPCTSTR)"Error", MB_OK);
		return -1;
	}

	return S_OK;
}

//
// Server un-registration
//
HRESULT _stdcall DllUnregisterServer()
{
	LONG	rc;
	char	errstr[128];

	rc = UnregisterAsioDriver(
		IID_DKASIOUSER_DRIVER,
		"dk_asio_user.dll",
		"DK ASIO Driver");

	if (rc)
	{
		memset(errstr, 0, 128);
		sprintf_s(errstr, "Unregister Server failed ! (%d)", rc);
		::OutputDebugStringA(errstr);
		//MessageBox(0, (LPCTSTR)errstr, (LPCTSTR)"Error", MB_OK);
		return -1;
	}

	return S_OK;
}


#define DKASIOUSER_FLAG_NONE (0x00000000)
#define DKASIOUSER_FLAG_ACTIVE (0x00000001)
#define DKASIOUSER_FLAG_STARTED (0x00000002)
#define DKASIOUSER_FLAG_TIMEINFOMODE (0x00000004)
#define DKASIOUSER_FLAG_TCREAD (0x00000008)

DKASIOUser::DKASIOUser(LPUNKNOWN pUnk, HRESULT *phr)
: CUnknown( _T("DKASIOUserCls"), pUnk, phr)
, m_callbacks( NULL )
, m_uPosSample( 0 )
, m_uFlag(DKASIOUSER_FLAG_NONE)
, m_sampleRateCur(44100)
, m_sampleType( ASIOSTInt16LSB )
, m_uBlockFrame( ASIO_BLOCK_FRAMES )
, m_uActiveInputs( 0 )
, m_uActiveOutputs( 0 )
, m_hEventBuf0( NULL )
, m_hEventBuf1( NULL )
, m_hEventStop( NULL ) 
, m_hThreadASIO( NULL )
, m_hThreadTriggle( NULL )
{
	::OutputDebugString( __FUNCTION__ );
	//::MessageBox( 0,0,0,0 );
	_init();
	g_asioRoot.setASIOInterface( this );
}

DKASIOUser::~DKASIOUser() {
	::OutputDebugString( __FUNCTION__ );
	
	g_asioRoot.setASIOInterface( NULL );
	
	::SetEvent( m_hEventStop );
	//
	::Sleep( 100 );
	//
	_unInit();
}

void DKASIOUser::_init() {
	memset(m_szErrMsg, 0, sizeof(m_szErrMsg));
    //
	m_hEventBuf0 = ::CreateEvent( NULL, FALSE, FALSE, NULL );
    m_hEventBuf1 = ::CreateEvent( NULL, FALSE, FALSE, NULL );
	m_hEventStop = ::CreateEvent( NULL, TRUE, FALSE, NULL );


}

void DKASIOUser::_unInit() {
	if ( m_hThreadASIO ) {
		::WaitForSingleObject( m_hThreadASIO, 1000 );
		m_hThreadASIO = NULL;
	}

	if ( m_hThreadTriggle ) {
		::WaitForSingleObject( m_hThreadTriggle, 1000 );
		m_hThreadTriggle = NULL;
	}

	if ( m_hEventBuf0 ) {
		::CloseHandle( m_hEventBuf0 );
		m_hEventBuf0 = NULL;
	}
	if ( m_hEventBuf1 ) {
		::CloseHandle( m_hEventBuf1 );
		m_hEventBuf1 = NULL;
	}
	if ( m_hEventStop ) {
		::CloseHandle( m_hEventStop );
		m_hEventStop = NULL;
	}
}

CUnknown* DKASIOUser::CreateInstance(LPUNKNOWN pUnk, HRESULT *phr) {
	UDBG( "enter %s\n", __FUNCTION__ );
	return (CUnknown*)new DKASIOUser(pUnk, phr);
}

HRESULT STDMETHODCALLTYPE DKASIOUser::NonDelegatingQueryInterface(REFIID riid, void **ppvObject) {
	if (riid == IID_DKASIOUSER_DRIVER)
	{
		return GetInterface(this, ppvObject);
	}

	return CUnknown::NonDelegatingQueryInterface(riid, ppvObject);
}

ASIOBool DKASIOUser::init(void *sysHandle) {
    UDBG( "enter %s\n", __FUNCTION__ );

	if ( DKBIT_HITTEST( m_uFlag, DKASIOUSER_FLAG_ACTIVE ) )
		return true;
	DKBIT_ADD( m_uFlag, DKASIOUSER_FLAG_ACTIVE );

	return true;
}

void DKASIOUser::getDriverName(char *name) {
	strcpy_s(name, 12, "DKASIO User");
}

long DKASIOUser::getDriverVersion() {
	return 0x00000001;
}

void DKASIOUser::getErrorMessage(char *string) {
	strcpy_s(string, sizeof(m_szErrMsg)-1, m_szErrMsg);
}


DWORD __stdcall DKASIOUser::DKTriggleThreadProc( void* pParam ) {
	DKASIOUser* pInst = (DKASIOUser*)pParam;
	unsigned int uInterval = (unsigned int)((double)(1000*pInst->m_uBlockFrame)/pInst->m_sampleRateCur)/2;
	HANDLE hEventStop = pInst->m_hEventStop;
	bool bActiveBuf0 = true;
	unsigned int uTag1 = 0;
	unsigned int uTag2 = 0;
	unsigned int uTime = 0;
	unsigned int n4Less = 0;
	unsigned int n5To7 = 0;
	unsigned int n8To14 = 0;
	unsigned int n15To17 = 0;
	unsigned int n18To32 = 0;
	unsigned int n33More = 0;
	unsigned int uCount = 0;
	TCHAR szMsg[ 200 ];

	if (uInterval <= 0) {
	    uInterval = 1; //
	}

	uTag1 = ::timeGetTime();
	while( WAIT_TIMEOUT == ::WaitForSingleObject( hEventStop, uInterval ) ) {
		uTag2 = ::timeGetTime();

		if ( bActiveBuf0 ) {
			::SetEvent( pInst->m_hEventBuf0 );
		} else {
			::SetEvent( pInst->m_hEventBuf1 );
		}

		// calc delay.
		uTime = uTag2 - uTag1;
		if ( uTime <= 4 ) {
		    ++n4Less;
		} else if ( uTime <= 7 ) {
		    ++n5To7;
		} else if ( uTime <= 14 ) {
		    ++n8To14;
		} else if ( uTime <= 17 ) {
		    ++n15To17;
		} else if ( uTime <= 32 ) {
		    ++n18To32;
		} else {
		    ++n33More;
		}
		++uCount;

		if ( uCount >= 100 ) {
			
		    _stprintf_s( szMsg, _T("count:%d, lt4:%d, 5~7:%d, 8~14:%d, 15~17:%d, 18~32:%d, 33More:%d\n"), uCount, n4Less, n5To7, n8To14, n15To17, n18To32, n33More );
			::OutputDebugString( szMsg );
			uCount = 0;
			n4Less = 0;
			n5To7 = 0;
			n8To14 = 0;
			n15To17 = 0;
			n18To32 = 0;
			n33More = 0;
		}

		//
		bActiveBuf0 = !bActiveBuf0;
		uInterval = (unsigned int)((double)(1000*pInst->m_uBlockFrame)/pInst->m_sampleRateCur)/2;
		uTag1 = ::timeGetTime();
	}

	return 0;
}

DWORD __stdcall DKASIOUser::DKASIOThreadProc( void* pParam ) {
	DKASIOUser* pInst = (DKASIOUser*)pParam;
	HANDLE aryEvent[ 3 ];
	DWORD dwWaitRet = 0;

	if ( !pInst )
		return 0;
	aryEvent[ 0 ] = pInst->m_hEventStop;
	aryEvent[ 1 ] = pInst->m_hEventBuf0;
	aryEvent[ 2 ] = pInst->m_hEventBuf1;

	DKBIT_ADD( pInst->m_uFlag, DKASIOUSER_FLAG_STARTED );
	while ( 1 )
	{
		dwWaitRet = ::WaitForMultipleObjects( 3, aryEvent, FALSE, INFINITE );
		if ( dwWaitRet == WAIT_OBJECT_0) {
			break;
		} else if ( dwWaitRet == WAIT_OBJECT_0 + 1 ) {
		    pInst->_bufferSwitch( 0 );
		} else if ( dwWaitRet == WAIT_OBJECT_0 + 2 ) {
		    pInst->_bufferSwitch( 1 );
		} else {
		    break;
		}
	}

	DKBIT_CLEAR( pInst->m_uFlag, DKASIOUSER_FLAG_STARTED );
	return 0;
}

void DKASIOUser::_bufferSwitch( unsigned int uToggle ) {
	if ( DKBIT_HITTEST( m_uFlag, DKASIOUSER_FLAG_STARTED ) && m_callbacks ) {
	    getMMETimeStamp( &m_sysTime );
		m_uPosSample += m_uBlockFrame;

		if ( DKBIT_HITTEST( m_uFlag, DKASIOUSER_FLAG_TIMEINFOMODE ) ) {
		    // need more code to support this mode.
			_bufferSwitchTimeInfo( uToggle );
		} else {
		    m_callbacks->bufferSwitch( uToggle, ASIOFalse );
		}
		
		unsigned int ubufCacheSize = getPCMBlockSize();
		//this->m_aryBufInput
		for ( unsigned char uIdx = 0; uIdx < m_uActiveOutputs; ++uIdx ) {
		    g_asioRoot.m_proxyPDRender.sendPCMData( uToggle, uIdx, m_uActiveOutputs, ( m_aryBufOutput[ uIdx ] + uToggle*ubufCacheSize ), ubufCacheSize );
		}
	}
}

void DKASIOUser::_bufferSwitchTimeInfo( unsigned int uToggle ) {
	
	getSamplePosition( &m_asioTime.timeInfo.samplePosition, &m_asioTime.timeInfo.systemTime );
	if ( DKBIT_HITTEST( m_uFlag, DKASIOUSER_FLAG_TCREAD ) ) {
	    m_asioTime.timeCode.timeCodeSamples.lo = m_asioTime.timeInfo.samplePosition.lo + 600.0 * m_sampleRateCur;
		m_asioTime.timeCode.timeCodeSamples.hi = 0;
	}
	//
	if ( m_callbacks && m_callbacks->bufferSwitchTimeInfo ) {
	    m_callbacks->bufferSwitchTimeInfo( &m_asioTime, uToggle, ASIOFalse );
	}
	m_asioTime.timeInfo.flags &= ~(kSampleRateChanged | kClockSourceChanged);
}

ASIOError DKASIOUser::start() {
	if ( DKBIT_HITTEST( m_uFlag, DKASIOUSER_FLAG_STARTED ) )
		return ASE_OK;
	DWORD dwThreadId;

	::ResetEvent( m_hEventStop );
	m_sysTime.hi = m_sysTime.lo = 0;
	m_uPosSample = 0;
	m_hThreadASIO = ::CreateThread( NULL, 0, &DKASIOThreadProc, this, 0, &dwThreadId );
	if ( !m_hThreadASIO )
		return ASE_OK;
    SetThreadPriority( m_hThreadASIO, THREAD_PRIORITY_TIME_CRITICAL );

	//
	//m_hThreadTriggle = ::CreateThread( NULL, 0, &DKTriggleThreadProc, this, 0, &dwThreadId );
	//if ( !m_hThreadTriggle )
	//	return ASE_OK;

	return ASE_OK;
}

ASIOError DKASIOUser::stop() {
	::SetEvent( m_hEventStop );

	if ( m_hThreadASIO ) {
		WaitForSingleObject(m_hThreadASIO, 1000);
		::CloseHandle( m_hThreadASIO );
		m_hThreadASIO = NULL;
	}

	if ( m_hThreadTriggle ) {
		WaitForSingleObject(m_hThreadTriggle, 1000);
		::CloseHandle( m_hThreadTriggle );
		m_hThreadTriggle = NULL;
	}

	return ASE_OK;
}

ASIOError DKASIOUser::getChannels(long *numInputChannels, long *numOutputChannels) {
	*numInputChannels = MAX_IN_CHANNELS;
	*numOutputChannels = MAX_OUT_CHANNELS;
	return ASE_OK;
}

ASIOError DKASIOUser::getLatencies(long *inputLatency, long *outputLatency) {
	*inputLatency = ASIO_BLOCK_FRAMES;
	*outputLatency = ASIO_BLOCK_FRAMES*2;
	return ASE_OK;
}

ASIOError DKASIOUser::getBufferSize(long *minSize, long *maxSize, long *preferredSize, long *granularity) {
	*minSize = *maxSize = *preferredSize = ASIO_BLOCK_FRAMES;
	*granularity = 0;
	return ASE_OK;
}

ASIOError DKASIOUser::canSampleRate(ASIOSampleRate sampleRate) {
	if ( sampleRate == 44100 || sampleRate == 48000 )
		return ASE_OK;
	return ASE_NoClock;
}

ASIOError DKASIOUser::getSampleRate(ASIOSampleRate *sampleRate) {
	*sampleRate = m_sampleRateCur;
	return ASE_OK;
}

ASIOError DKASIOUser::setSampleRate(ASIOSampleRate sampleRate) {
	if ( sampleRate != 44100 && sampleRate != 48000 )
		return ASE_NoClock;
	if ( sampleRate != m_sampleRateCur ) {
	    m_sampleRateCur = sampleRate;
		m_asioTime.timeInfo.sampleRate = sampleRate;
		m_asioTime.timeInfo.flags |= kSampleRateChanged;
	
		if ( m_callbacks && m_callbacks->sampleRateDidChange ) {
		    m_callbacks->sampleRateDidChange( m_sampleRateCur );
		}
	}

	return ASE_OK;
}

ASIOError DKASIOUser::getClockSources(ASIOClockSource *clocks, long *numSources) {
	// internal
	clocks->index = 0;
	clocks->associatedChannel = -1;
	clocks->associatedGroup = -1;
	clocks->isCurrentSource = ASIOTrue;
	strcpy_s(clocks->name, 9, "Internal");
	*numSources = 1;
	return ASE_OK;
}

ASIOError DKASIOUser::setClockSource(long reference) {
	if (!reference)
	{
		m_asioTime.timeInfo.flags |= kClockSourceChanged;
		return ASE_OK;
	}
	return ASE_NotPresent;
}

ASIOError DKASIOUser::getSamplePosition(ASIOSamples *sPos, ASIOTimeStamp *tStamp) {
	tStamp->hi = m_sysTime.hi;
	tStamp->lo = m_sysTime.lo;
	if ( m_uPosSample > twoRaisedTo32 ) {
	    sPos->hi = (unsigned long )( m_uPosSample * twoRaisedTo32Reciprocal );
		sPos->lo = (unsigned long )( m_uPosSample - (sPos->hi*twoRaisedTo32Reciprocal) );
	} else {
	    sPos->hi = 0;
		sPos->lo = (unsigned long )m_uPosSample;
	}

	return ASE_OK;
}

ASIOError DKASIOUser::getChannelInfo(ASIOChannelInfo *info) {
	if (info->channel < 0 || 
		(info->isInput ? 
		info->channel >= MAX_IN_CHANNELS : 
		info->channel >= MAX_OUT_CHANNELS))
	{
		return ASE_InvalidParameter;
	}

	info->type = m_sampleType;
	info->channelGroup = 0;
	info->isActive = ASIOFalse;
	unsigned int i;

	if (info->isInput)
	{
		for (i = 0; i < m_uActiveInputs; i++)
		{
			if (m_inMap[i] == info->channel)
			{
				info->isActive = ASIOTrue;
				break;
			}
		}
	}
	else
	{
		for (i = 0; i < m_uActiveOutputs; i++)
		{
			if (m_outMap[i] == info->channel)
			{
				info->isActive = ASIOTrue;
				break;
			}
		}
	}
	strcpy_s(info->name, "Sample ");
	return ASE_OK;
}

void DKASIOUser::_clearBufArray( unsigned char* aryBuf[], unsigned int uNumArray ) {
	if ( !aryBuf ) return;
	unsigned char* pBufTmp = NULL;
	for ( unsigned int uIdx = 0; uIdx < uNumArray; ++uIdx ) {
	    pBufTmp = aryBuf[ uIdx ];
		if ( pBufTmp ) {
		    delete []pBufTmp;
			pBufTmp = NULL;
		}
		aryBuf[ uIdx ] = NULL;
	}
}

void DKASIOUser::_clearAllBuf() {
	unsigned char* pBuf = NULL;

	// for input.
	if ( m_aryBufInput ) {
		_clearBufArray( m_aryBufInput, m_uActiveInputs );
		memset( &m_inMap, 0, sizeof(m_inMap) );
		m_uActiveInputs = 0;
	}

    // for output
	if ( m_aryBufOutput ) {
		_clearBufArray( m_aryBufOutput, m_uActiveOutputs );
		memset( &m_outMap, 0, sizeof(m_outMap) );
		m_uActiveOutputs = 0;
	}

}

unsigned int DKASIOUser::_calcBlockSize( unsigned int uPCMBit, unsigned int uSampleCount ) {
	return ( uPCMBit / 8 ) * uSampleCount;
}

unsigned int DKASIOUser::_calcPCMBit( ASIOSampleType type ) {

	switch ( type ) {
	case ASIOSTInt16MSB:
	case ASIOSTInt16LSB:
		return 16;
	case ASIOSTInt24MSB: // used for 20 bits as well
	case ASIOSTInt24LSB: // used for 20 bits as well
		return 24;
	case ASIOSTInt32MSB:
	case ASIOSTFloat32MSB: // IEEE 754 32 bit float
	// these are used for 32 bit data buffer, with different alignment of the data inside
	// 32 bit PCI bus systems can be more easily used with these
	case ASIOSTInt32MSB16: // 32 bit data with 18 bit alignment
	case ASIOSTInt32MSB18: // 32 bit data with 18 bit alignment
	case ASIOSTInt32MSB20: // 32 bit data with 20 bit alignment
	case ASIOSTInt32MSB24: // 32 bit data with 24 bit alignment
	case ASIOSTInt32LSB:
	case ASIOSTFloat32LSB: // IEEE 754 32 bit float, as found on Intel x86 architecture
	// these are used for 32 bit data buffer, with different alignment of the data inside
	// 32 bit PCI bus systems can more easily used with these
	case ASIOSTInt32LSB16: // 32 bit data with 18 bit alignment
	case ASIOSTInt32LSB18: // 32 bit data with 18 bit alignment
	case ASIOSTInt32LSB20: // 32 bit data with 20 bit alignment
	case ASIOSTInt32LSB24: // 32 bit data with 24 bit alignment
		return 32;
	case ASIOSTFloat64MSB: // IEEE 754 64 bit double float
	case ASIOSTFloat64LSB: // IEEE 754 64 bit double float, as found on Intel x86 architecture
		return 64;
	}
		
	assert( false );
	return 32;
}

ASIOError DKASIOUser::createBuffers(
	ASIOBufferInfo *bufferInfos, 
	long numChannels, 
	long bufferSize, 
	ASIOCallbacks *callbacks) 
{
	if ( NULL == callbacks || NULL == bufferInfos ) {
		return ASE_InvalidParameter;
	}
	unsigned int uIdx = 0;
	ASIOBufferInfo *info = NULL;
	unsigned int ubufCacheSize = 0;
	ASIOError ret = ASE_OK;
	bool bNoEnoughMem = false;

	m_callbacks = callbacks;
	if (m_callbacks->asioMessage(kAsioSupportsTimeInfo, 0, 0, 0)) {
		DKBIT_ADD( m_uFlag, DKASIOUSER_FLAG_TIMEINFOMODE );
		m_asioTime.timeInfo.speed = 1;
		m_asioTime.timeInfo.systemTime.hi = m_asioTime.timeInfo.systemTime.lo = 0;
		m_asioTime.timeInfo.samplePosition.hi = m_asioTime.timeInfo.samplePosition.lo = 0;
		m_asioTime.timeInfo.sampleRate = m_sampleRateCur;
		m_asioTime.timeInfo.flags = kSystemTimeValid | kSamplePositionValid | kSampleRateValid;
		//
		m_asioTime.timeCode.speed = 1;
		m_asioTime.timeCode.timeCodeSamples.lo = m_asioTime.timeCode.timeCodeSamples.hi = 0;
		m_asioTime.timeCode.flags = kTcValid | kTcRunning;
	} else {
		DKBIT_CLEAR( m_uFlag, DKASIOUSER_FLAG_TIMEINFOMODE );
	}

	// may be need more lock.
	_clearAllBuf();

	//
	__try
	{
		m_uBlockFrame = bufferSize;
		ubufCacheSize = getPCMBlockSize();
		assert( (m_uActiveInputs == 0) && (m_uActiveOutputs == 0) );
		for ( uIdx = 0, info = bufferInfos; uIdx < numChannels; ++uIdx, ++info ) {

			if ( info->isInput ) {
				::OutputDebugString( "createBuffers info->isInput" );
				if ( m_uActiveInputs >= MAX_IN_CHANNELS ) {
					ret = ASE_InvalidParameter;
					__leave;
				}
				if ( info->channelNum < 0 || info->channelNum >= MAX_IN_CHANNELS ) {
				   ret = ASE_InvalidParameter;
                   __leave;					   
				}
				//
				m_inMap[ m_uActiveInputs ] = info->channelNum;
				m_aryBufInput[ m_uActiveInputs ] = new unsigned char[ ubufCacheSize * 2 ];
				if ( m_aryBufInput[ m_uActiveInputs ] ) {
				    info->buffers[ 0 ] = m_aryBufInput[ m_uActiveInputs ];
					info->buffers[ 1 ] = m_aryBufInput[ m_uActiveInputs ] + ubufCacheSize;
				} else {
				    info->buffers[ 0 ] = info->buffers[ 1 ] = 0;
					bNoEnoughMem = true;
				}
				m_uActiveInputs++;
			} else { // output
				::OutputDebugString( "createBuffers info->isOutput" );
				if ( m_uActiveOutputs >= MAX_OUT_CHANNELS ) {
				    ret = ASE_InvalidParameter;
					__leave;
				}
				if ( info->channelNum < 0 || info->channelNum >= MAX_OUT_CHANNELS ) {
				   ret = ASE_InvalidParameter;
                   __leave;
				}
				//
				m_outMap[ m_uActiveOutputs ] = info->channelNum;
				m_aryBufOutput[ m_uActiveOutputs ] = new unsigned char[ ubufCacheSize * 2 ];
				if ( m_aryBufOutput[ m_uActiveOutputs ] ) {
				    info->buffers[ 0 ] = m_aryBufOutput[ m_uActiveOutputs ];
					info->buffers[ 1 ] = m_aryBufOutput[ m_uActiveOutputs ] + ubufCacheSize;
				} else {
				    info->buffers[ 0 ] = info->buffers[ 1 ] = 0;
					bNoEnoughMem = true;
				}
				m_uActiveOutputs++;
			}

			if ( bNoEnoughMem ) {
			    ret = ASE_NoMemory;
				__leave;
			}

		} 
	}
	__finally
	{
		if(ret != ASE_OK)
		{
			disposeBuffers();
		}
	}


	return ASE_OK;
}

ASIOError DKASIOUser::disposeBuffers() {

	_clearAllBuf();

	return ASE_OK;
}

ASIOError DKASIOUser::controlPanel() {
	return ASE_NotPresent;
}

ASIOError DKASIOUser::future(long selector, void *opt) {

	ASIOTransportParameters* tp = (ASIOTransportParameters*)opt;
	switch (selector)
	{
	case kAsioEnableTimeCodeRead:	DKBIT_ADD( m_uFlag, DKASIOUSER_FLAG_TCREAD );	return ASE_SUCCESS;
	case kAsioDisableTimeCodeRead:	DKBIT_CLEAR(m_uFlag, DKASIOUSER_FLAG_TCREAD);	return ASE_SUCCESS;
	case kAsioSetInputMonitor:		return ASE_SUCCESS;	// for testing!!!
	case kAsioCanInputMonitor:		return ASE_SUCCESS;	// for testing!!!
	case kAsioCanTimeInfo:			return ASE_SUCCESS;
	case kAsioCanTimeCode:			return ASE_SUCCESS;
	}
	return ASE_NotPresent;
}

ASIOError DKASIOUser::outputReady() {
	return ASE_NotPresent;
}


void getMMETimeStamp (ASIOTimeStamp* ts)
{
	double nanoSeconds = (double)((unsigned long)timeGetTime ()) * 1000000.;
	ts->hi = (unsigned long)(nanoSeconds / twoRaisedTo32);
	ts->lo = (unsigned long)(nanoSeconds - (ts->hi * twoRaisedTo32));
}


void DKASIOUser::wantDataBuf0() {
	::SetEvent( m_hEventBuf0 );
}

void DKASIOUser::wantDataBuf1() {
	::SetEvent( m_hEventBuf1 );
}

unsigned int DKASIOUser::getPCMBlockSize() {
	unsigned int uPCMBit = _calcPCMBit( m_sampleType );
	unsigned int ubufCacheSize = _calcBlockSize( uPCMBit, m_uBlockFrame );

    return ubufCacheSize;
}


