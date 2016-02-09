
#include "bouncethis.h"

int allow (char *ip)
{
 int i = 0;

 if (server.conn_limit <= server.conn_count && server.conn_limit != 0)
  return -1;

 if (server.clone_limit)
 {
  BT_CLIENT *conn ;

  EnterCriticalSection(&server.clientcs);

  conn = server.first;

  while (conn)
  {
   if (!strcmp(ip,conn->ip)) i++;

   if ((unsigned)i >= server.clone_limit)
   {
	LeaveCriticalSection(&server.clientcs);
	return -2;
   }

   conn = conn->next;
  }

  LeaveCriticalSection(&server.clientcs);
 }

 for (i = 0; i < server.filter_count; i++)
  if (ipmatch(server.filter_data[i],ip))
   return server.filter_mode;

 return !server.filter_mode;
}

int ipmatch (char *rule, char *ip)
{
 while (*rule != 0)
  if (*rule++ != *ip++)
   return 0;

 return 1;
}