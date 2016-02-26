#ifndef __LLXTSHELPER_H__
#define __LLXTSHELPER_H__
#include <Windows.h>

class LLXTSHelper {
public:
    LLXTSHelper();
    virtual ~LLXTSHelper();

public:
    bool safeEnterFunc() const;
    void safeExitFunc() const;
    void cancelAllAccess() const;
    HANDLE getExitEvent() const { return m_hTSEventExit; }

private:
    HANDLE m_hTSEventAccessSafe;
    HANDLE m_hTSEventExit;
    mutable DWORD m_dwTSFlag;
};

#endif //__LLXTSHELPER_H__





















