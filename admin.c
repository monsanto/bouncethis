
#include "bouncethis.h"

void broadcast (char * msg)
{
 BT_CLIENT *conn ;

 EnterCriticalSection(&server.clientcs);

 conn = server.first;

 while (conn)
 {
  bt_sendf(conn->sock,":BounceThis NOTICE :\002Broadcast:\002 %s\r\n", msg);
  conn = conn->next;
 }

 LeaveCriticalSection(&server.clientcs);
}

int kill (char * ip)
{
 BT_CLIENT *conn ;
 int i = 0;

 EnterCriticalSection(&server.clientcs);

 conn = server.first;

 while (conn)
 {
  if (ipmatch(ip,conn->ip))
  {
   bt_send(conn->sock,":BounceThis NOTICE :You have been killed from this server.\r\n") ;
   bt_close(conn->sock) ;
   i++;
  }
  conn = conn->next;
 }

 LeaveCriticalSection(&server.clientcs);

 return i ;
}

int tempfilter (char * ip)
{
 BT_CLIENT *conn ;
 int i = 0;

 EnterCriticalSection(&server.clientcs);

 conn = server.first;

 while (conn)
 {
  if (ipmatch(ip,conn->ip))
  {
   bt_send(conn->sock,":BounceThis NOTICE :You have been banned from this server.\r\n") ;
   bt_close(conn->sock) ;
   i++;
  }
  conn = conn->next;
 }

 LeaveCriticalSection(&server.clientcs);

 strcpy(server.filter_data[server.filter_count++],ip);

 return i ;
}

void list (SOCKET s)
{
 BT_CLIENT *conn ;

 bt_send(s,":BounceThis NOTICE :Listing all connections:\r\n");

 EnterCriticalSection(&server.clientcs);

 conn = server.first;

 while (conn)
 {
  bt_sendf(s, ":BounceThis NOTICE :%s:%i- Bytes sent: %i, recieved: %i\r\n",conn->ip,conn->port, conn->sent,conn->recv) ;
  conn = conn->next;
 }

 LeaveCriticalSection(&server.clientcs);
}