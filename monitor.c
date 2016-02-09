
#include "bouncethis.h"

BT_CLIENT *addconn (char *ip, short port)
{
 BT_CLIENT *client ;

 client = (BT_CLIENT*)malloc(sizeof(BT_CLIENT));

 // is this our first allocation?

 EnterCriticalSection(&server.clientcs);

 if (server.first == 0)
 {
  // seems so.
  server.first = client;
  server.last = client;
 }
 else if (server.last) // guess not
 {
  server.last->next = client;
  server.last = client;
 }
 else // what the fuck? should never get here.
 {
  logerror("BT connection list error. Please contact me about this.");
  server.last = client;
 }

 bt_cpy(client->ip, ip);
 client->port = port;
 client->next = NULL;
 client->recv = 0;
 client->sent = 0;

 server.conn_count++;

 addconntowin(client,server.conn_count-1);

 LeaveCriticalSection(&server.clientcs);

 return client;	
}

void delconn (BT_CLIENT *client)
{
 BT_CLIENT *conn ;

 EnterCriticalSection(&server.clientcs);
 conn = server.first ;

 if (conn == client) // getting rid of the last connection
  server.first = conn->next;
 else
  while (conn)
  {
   if (conn->next == client)
   {
	if (server.last == client)
	 server.last = conn;

	conn->next = client->next;

	conn = NULL;
   }
   else
	conn = conn->next;
  }

  server.conn_count--;

  delconnfromwin(client);

  LeaveCriticalSection(&server.clientcs);

  free(client);
}