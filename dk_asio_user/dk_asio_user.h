#ifndef __DK_ASIO_USER_H__
#define __DK_ASIO_USER_H__
#include "iasiodrv.h"
#include "combase.h"
#include "winioctl.h"
#include "LLXTSHelper.h"

//enum
//{
//	kBlockFrames = 256,
//	kNumInputs = 16,
//	kNumOutputs = 16
//};


//#define     ASIO_BLOCK_FRAMES   256
#define     ASIO_BLOCK_FRAMES   1024
#define		ASIO_MAX_CHANNELS	20
#define     MAX_IN_CHANNELS     20
#define		MAX_OUT_CHANNELS	20
#define     ASIO_MAX_BUFFERS	2

// 定义一组IOCTL值。用户驱动通过这一组值，和内核驱动
// 进行通信，比如：建立同步，启动、停止ASIO数据流等
// 目前的驱动版本只实现了有限的功能，将来只要扩充这组
// IOCTL值，就可以进行功能扩充。
#define IOCTL_ASIO_START \
	CTL_CODE( FILE_DEVICE_UNKNOWN, 0x800, METHOD_NEITHER, FILE_ANY_ACCESS )

#define IOCTL_ASIO_STOP \
	CTL_CODE( FILE_DEVICE_UNKNOWN, 0x801, METHOD_NEITHER, FILE_ANY_ACCESS )

#define IOCTL_ASIO_EVENT \
	CTL_CODE( FILE_DEVICE_UNKNOWN, 0x802, METHOD_NEITHER, FILE_ANY_ACCESS )

#define IOCTL_ASIO_BUFFERS \
	CTL_CODE( FILE_DEVICE_UNKNOWN, 0x803, METHOD_NEITHER, FILE_ANY_ACCESS )

#define IOCTL_ASIO_BUFFER_READY \
	CTL_CODE( FILE_DEVICE_UNKNOWN, 0x804, METHOD_NEITHER, FILE_ANY_ACCESS )

#define IOCTL_ASIO_BUFFER_SIZE \
	CTL_CODE( FILE_DEVICE_UNKNOWN, 0x805, METHOD_NEITHER, FILE_ANY_ACCESS )



class DKASIOUser : public IASIO, public CUnknown {
public:
	DKASIOUser(LPUNKNOWN pUnk, HRESULT *phr);
	virtual ~DKASIOUser();

	//DECLARE_IUNKNOWN
	STDMETHODIMP QueryInterface(REFIID riid, void **ppv) {     
		return this->NonDelegatingQueryInterface( riid, ppv );
//        return GetOwner()->QueryInterface(riid,ppv);           
    };                                                          
    STDMETHODIMP_(ULONG) AddRef() {                             
		return this ->NonDelegatingAddRef();
//        return GetOwner()->AddRef();                            
    };                                                          
    STDMETHODIMP_(ULONG) Release() {        
		return this->NonDelegatingRelease();
//        return GetOwner()->Release();                           
    };


public:
	// Factory method
	static CUnknown *CreateInstance(LPUNKNOWN pUnk, HRESULT *phr);
	// IUnknown
	virtual HRESULT STDMETHODCALLTYPE
		NonDelegatingQueryInterface(REFIID riid, void **ppvObject);

public:
	virtual ASIOBool init(void *sysHandle);

	virtual void getDriverName(char *name);

	virtual long getDriverVersion();

	virtual void getErrorMessage(char *string);

	virtual ASIOError start();

	virtual ASIOError stop();

	virtual ASIOError getChannels(long *numInputChannels, long *numOutputChannels);

	virtual ASIOError getLatencies(long *inputLatency, long *outputLatency);

	virtual ASIOError getBufferSize(long *minSize, long *maxSize,

		long *preferredSize, long *granularity);

	virtual ASIOError canSampleRate(ASIOSampleRate sampleRate);

	virtual ASIOError getSampleRate(ASIOSampleRate *sampleRate);

	virtual ASIOError setSampleRate(ASIOSampleRate sampleRate);

	virtual ASIOError getClockSources(ASIOClockSource *clocks, long *numSources);

	virtual ASIOError setClockSource(long reference);

	virtual ASIOError getSamplePosition(ASIOSamples *sPos, ASIOTimeStamp *tStamp);

	virtual ASIOError getChannelInfo(ASIOChannelInfo *info);

	virtual ASIOError createBuffers(ASIOBufferInfo *bufferInfos, long numChannels,

		long bufferSize, ASIOCallbacks *callbacks);

	virtual ASIOError disposeBuffers();

	virtual ASIOError controlPanel();

	virtual ASIOError future(long selector, void *opt);

	virtual ASIOError outputReady();

public:
	void wantDataBuf0();
	void wantDataBuf1();
	unsigned int getPCMBlockSize();

private:
	static DWORD __stdcall DKASIOThreadProc( void* pParam );
	static DWORD __stdcall DKTriggleThreadProc( void* pParam );

private:
	void _init();
	void _unInit();
	void _clearAllBuf();
	void _clearBufArray( unsigned char* aryBuf[], unsigned int uNumArray );
	unsigned int _calcBlockSize( unsigned int uPCMBit, unsigned int uSampleCount );
	unsigned int _calcPCMBit( ASIOSampleType type );
	void _bufferSwitch( unsigned int uToggle );
	void _bufferSwitchTimeInfo( unsigned int uToggle );

private:
	unsigned int m_uFlag;
	ASIOCallbacks* m_callbacks;
	ASIOTime m_asioTime;
	ASIOTimeStamp m_sysTime;
	unsigned int m_uPosSample;
	char m_szErrMsg[128];
	ASIOSampleRate m_sampleRateCur;
	ASIOSampleType m_sampleType;
	unsigned int m_uBlockFrame;
	unsigned char* m_aryBufInput[ MAX_IN_CHANNELS ];
	unsigned char* m_aryBufOutput[ MAX_OUT_CHANNELS ];
	unsigned int m_inMap[ MAX_IN_CHANNELS ];
	unsigned int m_outMap[ MAX_OUT_CHANNELS ];
	unsigned int m_uActiveInputs;
	unsigned int m_uActiveOutputs;
	HANDLE m_hEventBuf0;
	HANDLE m_hEventBuf1;
	HANDLE m_hEventStop;
	HANDLE m_hThreadASIO;
	HANDLE m_hThreadTriggle;
};




#endif //__DK_ASIO_USER_H__