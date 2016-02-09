
#include "bouncethis.h"

HINSTANCE hInst ;

BOOL CALLBACK CWProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
 switch (msg)
 {
 case WM_INITDIALOG:
  return TRUE;

 case WM_CLOSE:
  ShowWindow(hwnd,SW_HIDE);
  return TRUE;
 }

 return FALSE;
}

LRESULT CALLBACK WndProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
 static NOTIFYICONDATA nid;
 static HMENU menu, popup;
 static HWND hStatusScreen;

 switch (msg)
 {
 case WM_CREATE:
  {
   hStatusScreen = CreateDialog(hInst,"#1", hwnd, (DLGPROC)CWProc);
   server.log_screen = GetDlgItem(hStatusScreen,1);

   if (!server.log_screen)
	return -1;

   menu = LoadMenu (hInst, "#1");
   popup = GetSubMenu(menu,0);

   if (!popup)
	return -1;

   nid.hWnd = hwnd;
   nid.uID = 1;
   nid.hIcon = LoadImage (hInst, "#1", IMAGE_ICON, 16,16,0);
   nid.cbSize = sizeof(NOTIFYICONDATA);
   nid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
   nid.uCallbackMessage = WM_USER+1;
   strcpy(nid.szTip,"BounceThis running");

   if (!Shell_NotifyIcon (NIM_ADD,&nid))
	return -1;

   return 0;
  }

 case WM_USER+1:
  if (lParam == WM_RBUTTONUP)
  {
   POINT pt;

   GetCursorPos (&pt);
   SetForegroundWindow(hwnd);
   TrackPopupMenu (popup,0,pt.x,pt.y,0,hwnd,0);
   PostMessage(hwnd, WM_NULL, 0, 0);
  }

  return 0;

 case WM_COMMAND:
  switch (LOWORD(wParam))
  {
  case 3:
   {
	if (server.conn_open)
	 SetFocus(server.conn_screen) ;
	else
	{
	 HWND conn ;
	 conn = CreateDialog(hInst,"#2", hwnd, (DLGPROC)ConnProc);
	 connstart(conn);
	 ShowWindow (conn, SW_SHOW);
	}
	break;
   }
  case 2:
   ShowWindow (hStatusScreen, SW_SHOW);
   break;
  case 1:
   DestroyWindow (hwnd);
   break;
  }
  return 0;

 case WM_DESTROY:


  DeleteCriticalSection(&server.clientcs);

  Shell_NotifyIcon (NIM_DELETE,&nid); 

  DestroyMenu(menu);
  DestroyIcon(nid.hIcon);
  DestroyWindow(hStatusScreen);

  logend();

  WSACleanup(); 

  PostQuitMessage(0);

  return 0;
 }

 return DefWindowProc(hwnd,msg,wParam,lParam);
}

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
					PSTR szCmdLine, int iCmdShow)
{
 HWND         hwnd ;
 MSG          msg ;
 WNDCLASS     wc ;

 if (!start())
  goto startup_was_not_finished;

 hInst = hInstance ;

 wc.style         = 0 ;
 wc.lpfnWndProc   = WndProc ;
 wc.cbClsExtra    = 0 ;
 wc.cbWndExtra    = 0 ;
 wc.hInstance     = hInst ;
 wc.hIcon         = LoadIcon (hInstance, "#1") ;
 wc.hCursor       = LoadCursor (NULL, IDC_ARROW) ;
 wc.hbrBackground = (HBRUSH) (COLOR_BTNFACE + 1) ;
 wc.lpszMenuName  = 0 ;
 wc.lpszClassName = "BounceThisTray" ;

 if (!RegisterClass (&wc))
 {
  bt_error("Error registering window class.");
  goto startup_was_not_finished;
 }

 hwnd = CreateWindow ("BounceThisTray","BounceThis",0,0,0,0,0,NULL,0,hInstance,0);

 if (!hwnd)
 {
  bt_error ("Couldn't create tray icon.");
  goto startup_was_not_finished;
 }

 while (GetMessage (&msg, NULL, 0, 0))
 {
  TranslateMessage (&msg) ;
  DispatchMessage (&msg) ;
 }

 return (int)msg.wParam ;

startup_was_not_finished:

 DeleteCriticalSection(&server.clientcs);
 if (server.use_logging)
 {
  logevent("Logging could not commense because of a critical error.");
  logend();
 }
 return 0;

}