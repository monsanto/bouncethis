
#include "bouncethis.h"

BOOL CALLBACK ConnProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
 switch (msg)
 {
 case WM_INITDIALOG:
  server.conn_open = 1;
  return TRUE;
 case WM_COMMAND:
  if (HIWORD(wParam) == BN_CLICKED)
  {
   switch (LOWORD(wParam))
   {
   case 4:
	{
	 int curitem ;
	 LVITEM lvi = { 0 };

	 curitem = (int)SendDlgItemMessage(hwnd,1,LVM_GETSELECTIONMARK,0,0);

	 if (curitem == -1)
	 {
	  bt_error ("You must select a connection to kill") ;
	  break;
	 }

	 lvi.mask = LVIF_PARAM ;
	 lvi.iItem = curitem ;

	 SendDlgItemMessage(hwnd,1,LVM_GETITEM,0,(LPARAM)&lvi) ;

	 kill(((BT_CLIENT*)lvi.lParam)->ip) ;

	 break;
	}

   case 5:
	{
	 int curitem ;
	 LVITEM lvi = { 0 };

	 curitem = (int)SendDlgItemMessage(hwnd,1,LVM_GETSELECTIONMARK,0,0);

	 if (curitem == -1)
	 {
	  bt_error ("You must select a connection to filter") ;
	  break;
	 }

	 lvi.mask = LVIF_PARAM ;
	 lvi.iItem = curitem ;

	 SendDlgItemMessage(hwnd,1,LVM_GETITEM,0,(LPARAM)&lvi) ;

	 tempfilter(((BT_CLIENT*)lvi.lParam)->ip) ;

	 break;
	}

   case 7:
	{
	 char ip[16] ;
	 if (!GetDlgItemText(hwnd,6,ip,16))
	 {
	  bt_error ("You must enter an IP/BT-style filter.") ;
	  break;
	 }

	 kill(ip) ;
	 break;
	}

   case 8:
	{
	 char ip[16] ;
	 if (!GetDlgItemText(hwnd,6,ip,16))
	 {
	  bt_error ("You must enter an IP/BT-style filter.") ;
	  break;
	 }

	 tempfilter(ip) ;
	 break;
	}

   case 9:
	{
	 char msg[128] ;
	 if (!GetDlgItemText(hwnd,6,msg,128))
	 {
	  bt_error ("You must enter some text to broadcast.") ;
	  break;
	 }

	 broadcast(msg) ;
	 break;
	}
   }
  }
  return TRUE;

 case WM_CLOSE:
  server.conn_open = 0;
  DestroyWindow(hwnd);
  return TRUE;
 }

 return FALSE;
}

void connstart (HWND hwnd)
{
 BT_CLIENT *conn;
 LVCOLUMN lvc = { 0 };
 int i = 0 ;

 server.conn_screen = GetDlgItem(hwnd,1);
 server.conn_screencount = GetDlgItem(hwnd,2);

 if (server.conn_limit)
  SetDlgItemInt(hwnd,3,server.conn_limit,FALSE);
 else
  SetDlgItemText(hwnd,3,"N/A");

 SendMessage(server.conn_screen,
  LVM_SETEXTENDEDLISTVIEWSTYLE,0,LVS_EX_FULLROWSELECT );

 lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM; 
 lvc.iSubItem = 0;
 lvc.pszText = "IP";	
 lvc.fmt = LVCFMT_LEFT;
 lvc.cx = 120;

 SendMessage (server.conn_screen,LVM_INSERTCOLUMN,0, (LPARAM)&lvc);

 lvc.iSubItem = 1;
 lvc.pszText = "Port";
 lvc.cx = 60;

 SendMessage (server.conn_screen,LVM_INSERTCOLUMN,1, (LPARAM)&lvc);

 lvc.iSubItem = 2;
 lvc.pszText = "Bytes/Sent";
 lvc.cx = 100;

 SendMessage (server.conn_screen,LVM_INSERTCOLUMN,2, (LPARAM)&lvc);

 lvc.iSubItem = 3;
 lvc.pszText = "Bytes/Recv";
 lvc.cx = 100;

 SendMessage (server.conn_screen,LVM_INSERTCOLUMN,3, (LPARAM)&lvc);

 EnterCriticalSection(&server.clientcs);

 conn = server.first;

 while (conn)
 {
  addconntowin(conn,i++);
  conn = conn->next;
 }

 LeaveCriticalSection(&server.clientcs);
} 

void updatesrtowin (BT_CLIENT *client)
{
 char buf[16] = { 0 };
 LVFINDINFO lvfi = { 0 };
 LVITEM lvi = { 0 };
 int curitem;

 if (!server.conn_open)
  return;

 lvfi.lParam = (LPARAM) client;
 lvfi.flags = LVFI_PARAM;

 curitem = (int)SendMessage(server.conn_screen,LVM_FINDITEM,-1,(LPARAM)&lvfi);

 lvi.mask = LVIF_TEXT;
 lvi.iItem = curitem;
 lvi.iSubItem = 2;
 ultoa(client->sent,buf,10);
 lvi.pszText = buf;

 SendMessage (server.conn_screen,LVM_SETITEMTEXT,curitem,(LPARAM)&lvi);

 lvi.iSubItem = 3;
 ultoa(client->recv,buf,10);
 lvi.pszText = buf;

 SendMessage (server.conn_screen,LVM_SETITEMTEXT,curitem,(LPARAM)&lvi);
}

void addconntowin (BT_CLIENT *client,int x)
{
 LVITEM lvi;
 char buf[16] = { 0 };

 if (!server.conn_open)
  return;

 lvi.mask = LVIF_TEXT|LVIF_PARAM; 
 lvi.iItem = x;
 lvi.iSubItem = 0;
 lvi.pszText = client->ip;
 lvi.lParam = (LPARAM) client;

 SendMessage (server.conn_screen,LVM_INSERTITEM,x,(LPARAM)&lvi);

 lvi.iSubItem = 1;
 ultoa(client->port,buf,10);
 lvi.pszText = buf;

 SendMessage (server.conn_screen,LVM_SETITEMTEXT,x,(LPARAM)&lvi);

 lvi.iSubItem = 2;
 ultoa(client->sent,buf,10);
 lvi.pszText = buf;

 SendMessage (server.conn_screen,LVM_SETITEMTEXT,x,(LPARAM)&lvi);

 lvi.iSubItem = 3;
 ultoa(client->recv,buf,10);
 lvi.pszText = buf;

 SendMessage (server.conn_screen,LVM_SETITEMTEXT,x,(LPARAM)&lvi);

 ultoa(server.conn_count,buf,10);
 SetWindowText(server.conn_screencount,buf);
}

void delconnfromwin(BT_CLIENT*client)
{
 char buf[16] = { 0 };
 LVFINDINFO lvfi = { 0 };

 if (!server.conn_open)
  return;

 lvfi.lParam = (LPARAM) client;
 lvfi.flags = LVFI_PARAM;

 SendMessage(server.conn_screen,LVM_DELETEITEM,
  (WPARAM)SendMessage(server.conn_screen,LVM_FINDITEM,-1,(LPARAM)&lvfi),0);

 ultoa(server.conn_count,buf,10);
 SetWindowText(server.conn_screencount,buf);
}