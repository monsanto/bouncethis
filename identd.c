
#include "bouncethis.h"

void responsethread (void * ptr)
{
 BT_IDENTD client = *(BT_IDENTD*) ptr;
 char buf [128] = { 0 };
 int len;

 len = bt_recv (client.sock, buf, 128);

 buf[len] = 0;
 buf[len-1] = 0;

 logevent ("Ident requested from %s, sending ident %s",client.ip, server.ident);

 bt_sendf (client.sock, "%s:USERID:UNIX:%s\r\n", buf, server.ident);

 bt_close(client.sock);

 free (ptr);

 _endthread();
}

void identdthread (void * nothing)
{
 SOCKET s;
 BT_IDENTD *client;

 s = bt_listen (113);

 if (!s)
  _endthread();

 while (1)
 {
  client = malloc(sizeof(BT_IDENTD));
  client->addrlen = sizeof(SOCKADDR_IN);

  client->sock = accept (s, (SOCKADDR*) &client->addr, &client->addrlen);

  if (client->sock == -1)
  {
   free (client);
   continue ;
  }

  bt_cpy (client->ip, inet_ntoa(client->addr.sin_addr));

  if (_beginthread (responsethread,0,&client) == -1)
  {
   logevent ("Could not create ident response thread.");
   bt_close(client->sock);
   _endthread();
  }
 }

 _endthread();
}