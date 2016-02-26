#include "stdafx.h"
#include <winsock2.h>
#include <Windows.h>
#include "dk_socket_server.h"
#pragma comment(lib, "ws2_32.lib")
//
#define DKSS_FLAG_NONE (0x00000000)
#define DKSS_FLAG_RUNNING (0x00000001)
#define DKSS_FLAG_INIT_SUCCESS (0x00000002)
#define DKSS_SERVER_LEN_RECVBUF (1024*1024)
//
#define FLAG_HITTEST( flag, bit ) ( ( flag & bit ) == bit )
//
dk_socket_server::dk_socket_server()
: m_uFlag( DKSS_FLAG_NONE )
, m_hEventNeedExit( NULL )
, m_hThreadRecv( NULL )
, m_hThreadAccept( NULL )
, m_socketListen( NULL )
, m_sockConn( NULL )
, m_pBufRecv( NULL ){
	_init();
}

dk_socket_server::~dk_socket_server() {
	_unInit();
}

bool dk_socket_server::start(const char* pszAddressListen, short sPortListen) {
	if ( (m_uFlag & DKSS_FLAG_INIT_SUCCESS) != DKSS_FLAG_INIT_SUCCESS ) {
	    _init();
	}
	if ( FLAG_HITTEST(m_uFlag, DKSS_FLAG_RUNNING) )
		return false;
	struct sockaddr_in addrListen;
	SOCKET sockListen = NULL;

	sockListen = socket(AF_INET, SOCK_STREAM, 0);
	if (sockListen == INVALID_SOCKET) {
	    DWORD dwErr = ::WSAGetLastError();
		return false;
	}
	//
	addrListen.sin_family = AF_INET;
	if (!pszAddressListen || strlen(pszAddressListen) == 0) {
		addrListen.sin_addr.s_addr = INADDR_ANY;
	} else {
		addrListen.sin_addr.s_addr = inet_addr((char*)pszAddressListen);
	}
	addrListen.sin_port = htons(sPortListen);
	memset(addrListen.sin_zero, 0x00, 8);
	if (SOCKET_ERROR == ::bind(sockListen, (SOCKADDR *)&addrListen, sizeof(addrListen))) 
		goto error;
	//
	if (0 != ::listen(sockListen, SOMAXCONN))
		goto error;
	//
	m_socketListen = sockListen;
	m_hThreadAccept = ::CreateThread( NULL, 0, &dk_socket_server::AcceptThreadProc, this, 0, NULL );
	m_hThreadRecv = ::CreateThread( NULL, 0, &dk_socket_server::ReceiveThreadProc, this, 0, NULL );

	m_uFlag |= DKSS_FLAG_RUNNING;
	return true;

error:
	if ( sockListen ) {
		::closesocket(sockListen);
		sockListen = NULL;
	}

	return false;
}

void dk_socket_server::_init() {
	WSADATA Ws;
	//Init Windows Socket
	if (WSAStartup(MAKEWORD(2, 2), &Ws) != 0) {
		// need more code here. to notify error happened.
		DWORD dwErr = WSAGetLastError();
		TCHAR szMsg[50];
		_stprintf_s( szMsg, _T("Call WSAStartup failed. Err:%d"), dwErr );
		::MessageBox( 0, szMsg, 0, 0 );
	}

	if (m_pBufRecv) {
		delete[]m_pBufRecv;
		m_pBufRecv = NULL;
	}
	m_pBufRecv = new unsigned char[DKSS_SERVER_LEN_RECVBUF];
	m_hEventNeedExit = ::CreateEvent(NULL, TRUE, FALSE, NULL);
	m_uFlag |= DKSS_FLAG_INIT_SUCCESS;
}

void dk_socket_server::_unInit() {
	if (m_socketListen) {
		::closesocket(m_socketListen);
		m_socketListen = NULL;
	}
	if (m_sockConn) {
		::closesocket(m_sockConn);
		m_sockConn = NULL;
	}
	if (m_hEventNeedExit) {
		::SetEvent(m_hEventNeedExit);
	}
	m_tsAccessSocketConn.cancelAllAccess();
	::Sleep(1);
	::CloseHandle(m_hEventNeedExit);
	
	if (m_hThreadRecv) {
		::WaitForSingleObject(m_hThreadRecv, INFINITE);
		m_hThreadRecv = NULL;
	}

	if (m_hThreadAccept) {
		::WaitForSingleObject(m_hThreadAccept, INFINITE);
		m_hThreadAccept = NULL;
	}

	if (m_pBufRecv) {
		delete[] m_pBufRecv;
		m_pBufRecv = NULL;
	}

	::WSACleanup();
}

unsigned int dk_socket_server::send( unsigned char* pBuf, unsigned int uLenBuf ) {
    if ( !pBuf || uLenBuf == 0 )
		return 0;
    SOCKET socketConn = NULL;

	if (!m_tsAccessSocketConn.safeEnterFunc())
		return 0;
	socketConn = m_sockConn;
	m_tsAccessSocketConn.safeExitFunc();
	if ( !socketConn )
		return 0;
	//
	return ::send( socketConn, (const char*)pBuf, (int)uLenBuf, 0 );
}

DWORD WINAPI dk_socket_server::ReceiveThreadProc(void* pParam) {
	dk_socket_server* pDKSS = (dk_socket_server*)pParam;
	SOCKET socketConn;
	int nRecvLen = 0;
	DWORD dwWaitRet = 0;

	while ( 1 ) {
		dwWaitRet = ::WaitForSingleObject(pDKSS->m_hEventNeedExit, 1);
		if (dwWaitRet != WAIT_TIMEOUT)
			break;

		if (!pDKSS->m_tsAccessSocketConn.safeEnterFunc())
			break;
		socketConn = pDKSS->m_sockConn;
		pDKSS->m_tsAccessSocketConn.safeExitFunc();
		if (socketConn == NULL) {
			::Sleep(1);
			continue;
		}
		//
		nRecvLen = recv(socketConn, (char*)pDKSS->m_pBufRecv, DKSS_SERVER_LEN_RECVBUF, 0);
		if (nRecvLen <= 0) {
			pDKSS->m_eventDisConnected.invoke( socketConn );
			pDKSS->m_tsAccessSocketConn.safeEnterFunc();
			if (socketConn == pDKSS->m_sockConn) {
				::closesocket(pDKSS->m_sockConn);
				pDKSS->m_sockConn = NULL;
			}
			pDKSS->m_tsAccessSocketConn.safeExitFunc();
			continue;
		}
		//
		pDKSS->m_eventRecvData.invoke(socketConn, pDKSS->m_pBufRecv, nRecvLen);
	}

	return 0;
}

DWORD WINAPI dk_socket_server::AcceptThreadProc(void* pParam) {
	dk_socket_server* pDKSS = (dk_socket_server*)pParam;
	sockaddr_in addrConnNew;
	SOCKET socketConn;
	int nLenAddr;
	//sockaddr_in addrClient;
	DWORD dwWaitRet = 0;

	while (1)
	{
		dwWaitRet = ::WaitForSingleObject(pDKSS->m_hEventNeedExit, 1);
		if (dwWaitRet != WAIT_TIMEOUT)
			break;

		nLenAddr = sizeof(addrConnNew);
		socketConn = ::accept(pDKSS->m_socketListen, (sockaddr*)&addrConnNew, &nLenAddr);
		if (INVALID_SOCKET == socketConn) {
			::Sleep(1);
			continue;
		}
		//
		//struct timeval timeout = { 3, 0 };//3s
		//int ret = setsockopt(socketConn, SOL_SOCKET, SO_SNDTIMEO, (const char*)&timeout, sizeof(timeout));
		//
		pDKSS->m_tsAccessSocketConn.safeEnterFunc();
		pDKSS->m_sockConn = socketConn;
		pDKSS->m_tsAccessSocketConn.safeExitFunc();
		//
		pDKSS->m_eventAccepted.invoke(socketConn);
		//char* pAddrClient = inet_ntoa(addrConnNew.sin_addr);
		//printf("recv connection. address:%s\n", pAddrClient ? pAddrClient : "none.");
		::Sleep(1);
	}

	return 0;
}
