/*

Application entry point

Bsic Win32 graphical application creating a window, some basic menus and defining a modal dialog box
for connection with a remote server

The code located here only handles the Win32 API calls needed to create and handle the GUI; 
application specific functionality is implemented in the AppCore class, which has its methods
specifically built to be called from here.

*/

#include <winsock2.h>
#include <windows.h>
#include "Resources.h"
#include "AppCore.h"

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int nCmdShow);
LRESULT CALLBACK WndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK ConnectDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam);


int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int nCmdShow)
{
	//Create window class
	WNDCLASSEX  wcex;							
	//Specify window class
	wcex.cbSize        = sizeof (wcex);				
	wcex.style         = CS_HREDRAW | CS_VREDRAW;		
	wcex.lpfnWndProc   = WndProc;						
	wcex.cbClsExtra    = 0;								
	wcex.cbWndExtra    = 0;								
	wcex.hInstance     = hInstance;						
	wcex.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
	wcex.hCursor       = LoadCursor (NULL, IDC_ARROW);														
	wcex.hbrBackground = (HBRUSH) (COLOR_WINDOW+1);
	wcex.lpszMenuName  = MAKEINTRESOURCE(IDR_MENU1);							
	wcex.lpszClassName = TEXT("WinClass");
	wcex.hIconSm       = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
	//Register window class
	RegisterClassEx (&wcex);		
	//Create main window
	HWND hwnd = CreateWindow (TEXT("WinClass"), TEXT("Network Simulation"), WS_CAPTION | WS_SYSMENU, CW_USEDEFAULT, CW_USEDEFAULT, 806, 644, NULL, NULL, hInstance, NULL);								
	if (!hwnd)
	{
		MessageBox(NULL, TEXT("Unable to create window"), NULL, MB_ICONERROR | MB_OK);
		return FALSE;
	}

	//Create Connect dialog window
	HWND hdgwnd = CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_CONNECTDIAG), hwnd, ConnectDlgProc);
	if (!hdgwnd)
	{
		MessageBox(NULL, TEXT("Unable to create connect window"), NULL, MB_ICONERROR | MB_OK);
		return FALSE;
	}
	ShowWindow(hdgwnd, SW_HIDE);


	//Winsock initialization
	WSADATA wsadata;
	int wsastarted = WSAStartup (0x0202,&wsadata);
	if(wsastarted != 0)
	{
		MessageBox(NULL, TEXT("Unable to start winsock"), NULL, MB_ICONERROR | MB_OK);
		return FALSE;
	}
	if (wsadata.wVersion != 0x0202)
	{
		MessageBox(NULL, TEXT("Winsock version prior to 2.2 detected! Seriously, get a newer version of Windows!"), NULL, MB_ICONERROR | MB_OK);
		WSACleanup();
		return FALSE;
	}

	//Get Application Core up and running
	AppCore& ACInst = AppCore::GetInstance();
	ACInst.InitResources(hInstance, hwnd, hdgwnd);

	//Display main window
	ShowWindow (hwnd, nCmdShow);						
	UpdateWindow (hwnd);

	MSG msg;

	while (TRUE)					
	{							
		if (PeekMessage(&msg,NULL,0,0,PM_REMOVE))
		{
			if(!IsDialogMessage(hdgwnd, &msg))
			{
				if(msg.message==WM_QUIT)
				{
					break;
				}
				TranslateMessage (&msg);							
				DispatchMessage (&msg);
			}
		}

		else
		{		
			ACInst.MainLoop();
		}
	}
	
	DestroyWindow(hdgwnd);
	WSACleanup();
	ACInst.FreeResources();
	return int(msg.wParam) ;										
}

LRESULT CALLBACK WndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)											
	{
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case ID_FILE_CONNECT:
			ShowWindow(AppCore::GetInstance().GetConnectWindowHandle(), SW_SHOW);
			break;
		case ID_FILE_HOST:
			AppCore::GetInstance().StartHosting();
			break;
		case ID_FILE_QUIT:
			PostQuitMessage(0);
			break;
		}

	case WM_SOCKET:
		AppCore::GetInstance().NetInput();
		break;

	case WM_CREATE:	
		break;

	case WM_MOVE:
		AppCore::GetInstance().MainLoop();
		break;

	case WM_MOVING:
		AppCore::GetInstance().MainLoop();
		break;	

	case WM_KEYDOWN:
		AppCore::GetInstance().KeyDown(UINT(wParam));
		break;

	case WM_KEYUP:
		AppCore::GetInstance().KeyUp(UINT(wParam));
		break;

	case WM_DESTROY:
		PostQuitMessage(0);					
		break;				
	}													

	return DefWindowProc (hwnd, message, wParam, lParam);	
}

BOOL CALLBACK ConnectDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	switch(Message)
	{
	case WM_INITDIALOG:

		return TRUE;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:
			{
				TCHAR Buf[20];
				sockaddr_in Addr;
				int Addrlen = sizeof(Addr);
				GetDlgItemText(hwnd,IDC_EDIT1,Buf,20);
				if(WSAStringToAddress(Buf,AF_INET,NULL,(sockaddr*)&Addr,&Addrlen) == 0)
				{
					Addr.sin_family = AF_INET;
					Addr.sin_port = htons(NetDefs::cServerPort);
					memset(&(Addr.sin_zero), 0x0, 8);
					AppCore::GetInstance().ConnectToRemote(Addr);
					ShowWindow(hwnd, SW_HIDE);
				}
				break;
			}
		case IDCANCEL:
			ShowWindow(hwnd, SW_HIDE);
			break;
		}
		break;
	default:
		return FALSE;
	}
	return TRUE;
}