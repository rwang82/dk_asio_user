// dk_acp.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "dk_acp.h"
#include "dk_acp_root.h"
#include "laudio_port_defs.h"
#include <atlconv.h>

#define MAX_LOADSTRING 100
#define CTRLID_IPADDR (258)
#define CTRLID_BTN_CONNECT (259)
#define CTRLID_STATIC_CONN_STATUS (260)
#define CTRLID_STATIC_AS_REG_STATUS (261)

// Global Variables:
dk_acp_root g_acpRoot;
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name
//
static void onBtnConnect(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
static void onAS2PDStatusChange( ENUMAS2PDSTATUS eStatusOld, ENUMAS2PDSTATUS eStatusNew, void* pParam );
static void onASRegStatusChange( bool bReg, const char* pszASID, void* pParam );
//
// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_DK_ACP, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_DK_ACP));

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}


//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_DK_ACP));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_DK_ACP);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;
   HWND hWndIPAddr;
   HWND hBtnConnect;
   HWND hWndStatus;
   HWND hWndASRegStatus;

   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      100, 100, 600, 500, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }
   // title.
   ::CreateWindow( _T("Static"), _T("Play Device IP:"), WS_CHILD|WS_VISIBLE, 20, 30, 150, 30, hWnd, (HMENU)NULL, hInstance, NULL );
   // ip address edit
   hWndIPAddr = ::CreateWindowEx( WS_EX_LEFT|WS_EX_LTRREADING|WS_EX_RIGHTSCROLLBAR|WS_EX_NOPARENTNOTIFY|WS_EX_CLIENTEDGE
	   , _T("SysIPAddress32")
	   , NULL
	   , WS_CHILDWINDOW|WS_VISIBLE|WS_TABSTOP
	   , 150, 30, 200, 30, hWnd, (HMENU)CTRLID_IPADDR, hInstance, NULL);
   if ( !hWndIPAddr )
	   return FALSE;
   // connect button.
   hBtnConnect = ::CreateWindow( _T("Button"), _T("Connect"), WS_CHILD|WS_VISIBLE, 360, 30, 60, 30, hWnd, (HMENU)CTRLID_BTN_CONNECT, hInstance, NULL );
   if ( !hBtnConnect )
	   return FALSE;
   // status.
   hWndStatus = ::CreateWindow( _T("Static"), _T("DisConnected"), WS_CHILD|WS_VISIBLE, 430, 30, 100, 30, hWnd, (HMENU)CTRLID_STATIC_CONN_STATUS, hInstance, NULL );
    g_acpRoot.m_asManager.m_eventAS2PDStatusChange.bind( onAS2PDStatusChange, hWndStatus );

	// audio source regist status.
	hWndASRegStatus = ::CreateWindow( _T("Static"), _T("No AS Regist."), WS_CHILD|WS_VISIBLE, 20, 80, 300, 30, hWnd, (HMENU)CTRLID_STATIC_AS_REG_STATUS, hInstance, NULL );
	g_acpRoot.m_asManager.m_eventASRegStatus.bind( onASRegStatusChange, hWndASRegStatus );


   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case CTRLID_BTN_CONNECT:
		{
		    onBtnConnect(hWnd, message, wParam, lParam);
		}
			break;
  		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code here...
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

void onBtnConnect(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	TCHAR szIPAddr[ 17 ];
	HWND hBtnIPAddr = NULL;
	std::string strIPAddr;
	USES_CONVERSION;

	hBtnIPAddr = ::GetDlgItem( hWnd, CTRLID_IPADDR );
	if ( !hBtnIPAddr )
		return;
	memset( szIPAddr, 0, sizeof( szIPAddr ) );
	::GetWindowText( hBtnIPAddr, szIPAddr, 17 );
	if ( _tcslen( szIPAddr ) == 0 ) {
	    MessageBox( NULL, _T("ip address ERROR."), 0, 0 );
	}
	strIPAddr = T2A(szIPAddr);
	//
	g_acpRoot.m_asManager.sendCmdASConnect2PD( strIPAddr.c_str() );
}

void onASRegStatusChange( bool bReg, const char* pszASID, void* pParam ) {
	HWND hWndStatus = (HWND)pParam;
	HWND hWndConnStatus = ::GetDlgItem( ::GetParent(hWndStatus), CTRLID_STATIC_CONN_STATUS );
	USES_CONVERSION;

	if ( bReg ) {
		SetWindowText( hWndStatus, A2T( pszASID ) );

	} else {
	    SetWindowText( hWndStatus, _T("No AS regist") );
		SetWindowText( hWndConnStatus, _T("DisConnected") );
	}
}

void onAS2PDStatusChange( ENUMAS2PDSTATUS eStatusOld, ENUMAS2PDSTATUS eStatusNew, void* pParam ) {
	HWND hWndStatus = (HWND)pParam;

	switch ( eStatusNew )
	{
	case EAS2PD_STATUS_CONNECTED:
	{
		SetWindowText( hWndStatus, _T("Connected") );
	}
		break;
	case EAS2PD_STATUS_CONNECTING:
	{
		SetWindowText( hWndStatus, _T("Connecting") );
	}
		break;
	case EAS2PD_STATUS_DISCONNECTED:
	{
		SetWindowText( hWndStatus, _T("DisConnected") );
	}
		break;
	default:
		break;
	}
}


