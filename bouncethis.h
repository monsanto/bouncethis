
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <commctrl.h>
#include <shellapi.h>

#define BT_VERSION_NUMBER "1.3a"
#define BT_VERSION "BounceThis v" BT_VERSION_NUMBER

typedef struct _port
{
	short port ;
	
	BYTE use_pass ;
	char password [48] ;

} BT_PORT ;

typedef struct _client
{
	char ip [16] ;
	short port;

	SOCKET sock;

	BYTE use_pass ;
	char password [48];

	unsigned long sent;
	unsigned long recv;

	struct _client *next; 
} BT_CLIENT ;

typedef struct _identd 
{

	char ip[16]; 

	SOCKET sock;
	
	SOCKADDR_IN addr ;
	socklen_t addrlen;
	
} BT_IDENTD ;

typedef struct _server
{
	char ip [16] ;
	BYTE use_vhost ;

	IN_ADDR addr;

	BT_PORT port_data [24] ; // allowed to listen on 24 ports
	int port_count ;

	BYTE filter_mode ;
	char filter_data[128][16] ;
	int filter_count ;
	
	BYTE use_identd;
	char ident [32] ;

	HWND log_screen; // this is always used, regardless of logging settings.
	
	BYTE use_logging;
	char log_data [256];
	FILE * log_file ;

	HWND conn_screen;
	HWND conn_screencount;

	BYTE conn_open ; 

	unsigned int conn_limit;
	unsigned int conn_count;
	unsigned int clone_limit;

    BT_CLIENT *first ;
	BT_CLIENT *last ; // performance reasons.

	CRITICAL_SECTION clientcs ; // for accessing the connection data list.

	char admin_user[16] ;
	char admin_pass[64] ;

} BT_SERVER ;

typedef struct _proxy 
{

	char host [256] ;
	char port [6] ;

	SOCKET sock ;

} BT_PROXY ;

typedef struct _clientinfo
{
	char nick[48] ;
	BYTE nick_reg;

	char user[32] ;
	char name[64] ;
	BYTE user_reg;

	BYTE use_pass;
	char pass[48] ;
	BYTE pass_reg ;

	BYTE is_admin ;
} BT_CLIENTINFO ;


// macros
#define bt_cpy(a,b) \
	strncpy(a,b,sizeof(a))
#define bt_cat(a,b) \
	strncat(a,b,sizeof(a))
#define real(a) \
	((*a == ':') ? a+1 : a)
#define bt_send(a,b) send (a, b, (int)strlen(b), 0)
#define bt_recv(a,b,c) recv(a,b,c,0)
#define bt_close(a) shutdown(a,SD_BOTH); closesocket(a)
#define bt_error(a) MessageBox(NULL,a,"Error!",MB_ICONERROR|MB_OK);

// main.c

extern BT_SERVER server;

// server.c
int getserverinfo ();
void portthread (void*);

// filter.c
int ipmatch (char *rule, char *ip);
int allow (char *ip);

// init.c
int start ();

// config.c
int parse (FILE * file);

// socket.c
SOCKET bt_connect (const char *host, const char *port);
SOCKET bt_listen (short port);
int bt_sendf (SOCKET s, char *data, ...);

// identd.c
void identdthread (void * nothing);

// log.c
void logstart ();
void logevent (char *event, ...);
void logerror (char *event, ...);
void logend ();

// monitor.c

BT_CLIENT *addconn (char *ip, short port);
void delconn (BT_CLIENT *client);

// connwin.c
BOOL CALLBACK ConnProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
void connstart (HWND hwnd);
void addconntowin (BT_CLIENT *client,int x);
void delconnfromwin(BT_CLIENT*client);
void updatesrtowin (BT_CLIENT *client);

void broadcast(char * msg) ;
int kill(char * ip) ;
int tempfilter (char * ip);
void list (SOCKET s);
