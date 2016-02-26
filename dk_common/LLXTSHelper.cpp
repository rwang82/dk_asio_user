#include "stdafx.h"
#include "LLXTSHelper.h"
#include <assert.h>
//
#define HITTEST_BIT( val, bit ) ( ( val & bit ) == bit )
#define LLXTSHelper_FLAG_NONE (0x00000000)
#define LLXTSHelper_FLAG_EXIT (0x00000001)

//
LLXTSHelper::LLXTSHelper()
: m_hTSEventAccessSafe( NULL )
, m_hTSEventExit( NULL )
, m_dwTSFlag( LLXTSHelper_FLAG_NONE ) {
    m_hTSEventAccessSafe = ::CreateEvent( NULL, FALSE, TRUE, NULL );
    m_hTSEventExit = ::CreateEvent( NULL, TRUE, FALSE, NULL );
}

LLXTSHelper::~LLXTSHelper() {
    cancelAllAccess();
    ::Sleep( 1 );
    ::CloseHandle( m_hTSEventAccessSafe );
    ::CloseHandle( m_hTSEventExit );
    m_hTSEventAccessSafe = NULL;
    m_hTSEventExit = NULL;
}

bool LLXTSHelper::safeEnterFunc() const {
    DWORD dwWaitRet = 0;
    HANDLE aryEvent[ 2 ];

    if ( HITTEST_BIT( m_dwTSFlag, LLXTSHelper_FLAG_EXIT ) ) {
        return false;
    }
    aryEvent[ 0 ] = m_hTSEventExit;
    aryEvent[ 1 ] = m_hTSEventAccessSafe;
    dwWaitRet = ::WaitForMultipleObjects( 2, aryEvent, FALSE, INFINITE );
    if ( dwWaitRet == WAIT_OBJECT_0 ) {
        return false; // m_hTSEventExit
    } else if ( dwWaitRet == WAIT_OBJECT_0 + 1 ) {
        return true; // m_hTSEventAccessSafe
    }

    assert( false );
    return false;
}

void LLXTSHelper::safeExitFunc() const {
    assert( m_hTSEventAccessSafe );
    ::SetEvent( m_hTSEventAccessSafe );
}

void LLXTSHelper::cancelAllAccess() const {
    m_dwTSFlag |= LLXTSHelper_FLAG_EXIT;
    ::SetEvent( m_hTSEventExit );
}
