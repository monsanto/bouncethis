
#include "bouncethis.h"

int start ()
{
 WSADATA wsa;
 FILE * conf_file;
 int i ;

 if (WSAStartup(MAKEWORD(1,1),&wsa) != 0)
 {
  bt_error("Couldn't init Winsock 1.1 support.");
  return 0;
 }

 conf_file = fopen("bt.conf","r");

 if (!conf_file)
 {
  bt_error("Invalid or nonexisting bt.conf file.");
  return 0;
 }

 if (!parse(conf_file))
 {
  fclose(conf_file); // Q:why not just close conf_file in parse()?
  // A:too much work.
  return 0;
 }

 fclose(conf_file);

 if (server.use_vhost)
 {
  unsigned long addr = inet_addr (server.ip);

  if (addr == INADDR_NONE)
  {
   bt_error("Invalid VHost.");
   return 0;
  }

  memcpy (&server.addr, &addr, sizeof(IN_ADDR));
 }
 else
 {
  char addr[256] ;

  if (gethostname(addr,256) == 0)
  {
   HOSTENT * hp ;

   hp = gethostbyname (addr);

   if (!hp)
   {
	bt_error("Couldn't resolve your local hostname.");
	return 0;
   }

   memcpy (&server.addr, hp->h_addr, hp->h_length);

   bt_cpy(server.ip, inet_ntoa (server.addr));
  }
  else
  {
   bt_error("Couldn't get your local hostname.");
   return 0;
  }
 }

 if (server.port_count == 0)
 {
  bt_error("You haven't defined any ports.");
  return 0;
 }

 if (server.use_logging)
 {
  server.log_file = fopen(server.log_data,"a");

  if (!server.log_file)
  {
   bt_error("Couldn't open log file.");
   return 0;
  }

  logstart();
 }

 InitializeCriticalSection (&server.clientcs);

 if (server.use_identd)
 {
  if (_beginthread (identdthread, 0, 0) == -1)
  {
   bt_error("Could not create identd thread.");
   return 0;
  }
 }

 for (i = 0; i < server.port_count; i++)
 {
  if (_beginthread(portthread, 0, &server.port_data[i]) == -1)
  {
   bt_error("Could not create one of the port threads.");
   return 0;
  }
 }

 return 1;
}

