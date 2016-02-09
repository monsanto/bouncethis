
#include "bouncethis.h"

BT_SERVER server = { 0 } ;

void proxythread (void * client_ptr)
{
 BT_CLIENT *client;
 BT_PROXY proxy = { 0 };
 BT_CLIENTINFO ci = { 0 };

 char buf[4096] = { 0 };

 int c = 0;

 // set up client information

 client = (BT_CLIENT*) client_ptr;

 // welcome user

 if (!client->use_pass) // anon server
 {
  logevent("Accepted %s:%i", client->ip, client->port);

  ci.pass_reg = 1;

  bt_send (client->sock,
   ":BounceThis NOTICE :You have connected to a BounceThis server. (version " BT_VERSION_NUMBER ")\r\n"
   ":BounceThis NOTICE :Please type \002/BOUNCE <server> [port] [password]\002 to connect to a server.\r\n"
   ":BounceThis NOTICE :For a list of commands, type \002/COMMANDS\002.\r\n");
 }

 /* parsing engine */


 while (1)
 {
  char *ptr, *bufptr, *msgptr ;
  char msgbuf[512] = { 0 };

  c = bt_recv (client->sock, buf, 4096);

  if (c == SOCKET_ERROR)
   goto abort ;

  buf[c] = 0;

  bufptr = buf;

  do
  {
   msgptr = msgbuf;

   // get first message (terminated with \r\n)

   while (*bufptr != '\n' && *bufptr != '\0')
   {
	if (*bufptr == 0) goto abort;
	*msgptr++ = *bufptr++;
   }

   *bufptr++;
   *msgptr = 0;

   ptr = strtok (msgbuf," ");

   if (!ptr)
	continue;

   if (!strcmpi (ptr,"PASS"))
   {
	if (ci.pass_reg)
	{
	 bt_send(client->sock, 
	  ":BounceThis NOTICE :Password not needed.\r\n");
	 continue;
	}
	else
	{
	 ptr = strtok(NULL," \r");

	 if (!ptr)
	  continue;

	 if (!strcmp (ptr, client->password))
	 {
	  logevent("Accepted %s:%i", client->ip, client->port);

	  ci.pass_reg = 1;

	  bt_send(client->sock,
	   ":BounceThis NOTICE :Password accepted.\r\n"
	   ":BounceThis NOTICE :You have connected to a BounceThis server. (version " BT_VERSION_NUMBER ")\r\n"
	   ":BounceThis NOTICE :Please type \002/BOUNCE <server> [port] [password]\002 to connect to a server.\r\n"
	   ":BounceThis NOTICE :For a list of commands, type \002/COMMANDS\002.\r\n");

	  continue;
	 }
	 else
	 {
	  logevent("Refused %s (incorrect password)", client->ip);
	  bt_send (client->sock, 
	   ":BounceThis NOTICE :Password incorrect.\r\n");
	  goto abort2;
	 }
	}
   }

   if (ci.pass_reg)
   {
	if (!strcmpi (ptr,"COMMANDS"))
	{
	 bt_send (client->sock,
	 ":BounceThis NOTICE :Commands listing:\r\n"
	 ":BounceThis NOTICE :\002/ABOUT\002 About BounceThis.\r\n"
	 ":BounceThis NOTICE :\002/ADMIN\002 Login as the admin.\r\n"
	 ":BounceThis NOTICE :\002/BOUNCE\002 Connects you to a server.\r\n"
	 ":BounceThis NOTICE :\002/COMMANDS\002 Shows this screen.\r\n"
	 ":BounceThis NOTICE :\002/INFO\002 Info about yourself and the server.\r\n"
	 ":BounceThis NOTICE :\002/NICK\002 Changes your nickname.\r\n"
	 ":BounceThis NOTICE :\002/QUIT\002 Disconnects you.\r\n");

	 if (ci.is_admin)
	  bt_send (client->sock,
	  ":BounceThis NOTICE :Admin commands listing:\r\n"
	 ":BounceThis NOTICE :\002/BROADCAST\002 Send a message to all users.\r\n"
	 ":BounceThis NOTICE :\002/KILL\002 Kill everyone matching an ip (BT style filters allowed)\r\n"
	 ":BounceThis NOTICE :\002/TEMPFILTER\002 Ban everyone matching an ip (BT style filters allowed) until server is stopped.\r\n"
	 ":BounceThis NOTICE :\002/LISTCONN\002 Lists all connections to the server.\r\n");

	}

	else if (!strcmpi (ptr,"ABOUT"))
	 bt_send(client->sock,
	 ":BounceThis NOTICE :About BounceThis:\r\n"
	 ":BounceThis NOTICE :BounceThis was created by Chris M. in 2002.\r\n"
	 ":BounceThis NOTICE :It is a full featured Win32 BNC server designed for IRC.\r\n"
	 ":BounceThis NOTICE :Check us out on the web at 12http://bouncethis.linear.cc\r\n");

	else if (!strcmpi (ptr,"INFO"))
	{ 
	 bt_sendf (client->sock,
	  ":BounceThis NOTICE :" BT_VERSION ", bound on IP %s on port %i\r\n"
	  ":BounceThis NOTICE :You are connecting as %s!%s@%s (%s)\r\n",
	  server.ip, client->port, real(ci.nick), ci.user, client->ip, real(ci.name));

	 if (server.use_identd)
	  bt_sendf (client->sock,
	  ":BounceThis NOTICE :Identd server enabled, using ident '%s'\r\n",
	  server.ident);
	}

	else if (!strcmpi (ptr,"NICK"))
	{
	 ptr = strtok(NULL," \r");

	 if (!ptr)
	  continue;

	 bt_cpy (ci.nick, ptr);

	 if (ci.nick_reg)
	  bt_sendf (client->sock, 
	  ":BounceThis NOTICE :Your nickname is now \002%s\002.\r\n",
	  real(ci.nick));
	 else
	  ci.nick_reg = 1;
	}
	else if (!strcmpi (ptr,"QUIT"))
	 goto abort; 
	else if (!strcmpi (ptr,"USER"))
	{
	 if (ci.user_reg)
	 {
	  bt_send (client->sock, 
	   ":BounceThis NOTICE :You cannot reregister.\r\n");
	  continue;
	 }

	 ptr = strtok(NULL," ");

	 if (!ptr)
	  continue;

	 bt_cpy (ci.user, ptr);

	 // skip next two fields. useless -_-;
	 strtok(NULL," ");
	 strtok(NULL," ");

	 ptr = strtok(NULL,"\r");

	 if (!ptr)
	  continue;

	 bt_cpy (ci.name, ptr);
	 ci.user_reg = 1;
	}
	else if (!strcmpi (ptr,"ADMIN"))
	{ 
	 char user[16] ; 
	 char pass[16] ;

	 ptr = strtok(NULL," \r");

	 if (!ptr)
	  continue;

	 bt_cpy(user,ptr) ;

	 ptr = strtok(NULL," \r");

	 if (!ptr)
	  continue;

	 bt_cpy(pass,ptr) ;

	 if (!strcmpi(user,server.admin_user) && !strcmp(pass,server.admin_pass))
	 {
	  bt_send(client->sock,":BounceThis NOTICE :Admin password accepted.\r\n");
	  ci.is_admin = 1 ;
	 }
	 else
	 {
	  logevent("Connection from %s:%i closed (invalid admin attempt)", client->ip,client->port);
	  bt_send (client->sock,
	   ":BounceThis NOTICE :Admin password incorrect. Disconnecting.\r\n");
	  goto abort2;
	 }
	}
	else if (!strcmpi (ptr,"BROADCAST"))
	{
	 if (!ci.is_admin) // :O!
	 {
	  bt_send(client->sock,":BounceThis NOTICE :You cannot perform this command.\r\n");
	  continue ;
	 }

	 ptr = strtok(NULL,"\r");

	 if (!ptr)
	  continue;

	 broadcast(ptr) ;
	}
	else if (!strcmpi (ptr,"KILL"))
	{
	 if (!ci.is_admin) // :O!
	 {
	  bt_send(client->sock,":BounceThis NOTICE :You cannot perform this command.\r\n");
	  continue ;
	 }

	 ptr = strtok(NULL,"\r");

	 if (!ptr)
	  continue;

	 bt_sendf(client->sock,":BounceThis NOTICE :Kill effected \002%i\002 users.\r\n",kill(ptr)) ;
	}
	else if (!strcmpi (ptr,"TEMPFILTER"))
	{
	 if (!ci.is_admin) // :O!
	 {
	  bt_send(client->sock,":BounceThis NOTICE :You cannot perform this command.\r\n");
	  continue ;
	 }

	 ptr = strtok(NULL,"\r");

	 if (!ptr)
	  continue;

	 bt_sendf(client->sock,":BounceThis NOTICE :Filter effected \002%i\002 users.\r\n",tempfilter(ptr)) ;
	}
	else if (!strcmpi (ptr,"LISTCONN"))
	{
	 if (!ci.is_admin) // :O!
	 {
	  bt_send(client->sock,":BounceThis NOTICE :You cannot perform this command.\r\n");
	  continue ;
	 }

	 list(client->sock);
	}
	else if (!strcmpi (ptr,"BOUNCE"))
	{
	 ptr = strtok(NULL," \r");

	 if (!ptr)
	  continue;

	 bt_cpy (proxy.host, ptr);

	 ptr = strtok(NULL," \r");

	 if (!ptr)
	  bt_cpy(proxy.port,"6667");
	 else
	 {
	  bt_cpy (proxy.port, ptr);
	  ptr = strtok(NULL," \r");

	  if (ptr)
	  {
	   bt_cpy (ci.pass,ptr);
	   ci.use_pass = 1;
	  }
	 }

	 proxy.sock = bt_connect (proxy.host, proxy.port);

	 if (!proxy.sock)
	  bt_sendf (client->sock,
	  ":BounceThis NOTICE :Error connecting to %s:%s.\r\n",
	  proxy.host,	proxy.port);
	 else
	 {
	  bt_sendf (client->sock,
	   ":BounceThis NOTICE :Connection established to %s:%s.\r\n",proxy.host,
	   proxy.port);

	  if (ci.use_pass)
	   bt_sendf (client->sock,":BounceThis NOTICE :Attempting to login with password '%s'\r\n",
	   ci.pass);
	  break;
	 }
	}
	else
	 bt_sendf (client->sock,
	 ":BounceThis NOTICE :\002/%s\002: Invalid command.\r\n",ptr);
   }
   else
   {
	logevent("Refused %s:%i (incorrect password)", client->ip,client->port);
	bt_send (client->sock,
	 ":BounceThis NOTICE :You must supply the password before performing any other commands.\r\n");
	goto abort2;
   }

  } while ((char)*bufptr != (char)0) ;

  if (proxy.sock) /* proxy connected, break from BT IRC interface */
   break;
 }

 logevent("Proxy connection to %s:%s from %s:%i", proxy.host,proxy.port,
  client->ip, client->port);

 // Register user.
 if (ci.use_pass)
  bt_sendf(proxy.sock,"PASS %s\r\n", ci.pass);

 bt_sendf(proxy.sock,"NICK %s\r\nUSER %s # # %s\r\n",ci.nick,ci.user,ci.name);

 while (1)
 {
  fd_set rset = { 0 };

  FD_SET (client->sock, &rset);
  FD_SET (proxy.sock, &rset);

  if (select (0, &rset, NULL, NULL, NULL) == -1)
   goto abort;

  if (FD_ISSET (client->sock, &rset))
  {
   c = bt_recv(client->sock, buf, 4096);

   if (c <= 0)
	goto abort;

   buf[c] = 0;

   client->sent += bt_send (proxy.sock, buf);
  }

  if (FD_ISSET (proxy.sock, &rset))
  {
   c = bt_recv (proxy.sock, buf, 4096);

   if (c <= 0)
	goto abort;

   buf[c] = 0;

   client->recv += bt_send (client->sock, buf);
  }

  updatesrtowin(client);
 }

abort:
 logevent("Connection from %s:%i closed", client->ip,client->port);
abort2: // for skipping auto log.
 bt_close(client->sock);

 if (proxy.sock)
 {
  bt_close(proxy.sock);
 }

 delconn(client);

 _endthread();
}

void portthread (void * port_ptr)
{
 SOCKET s;
 BT_PORT port_info ;

 port_info = *(BT_PORT*) port_ptr ;

 s = bt_listen (port_info.port);

 if (!s)
  _endthread();

 while (1)
 {
  socklen_t addrlen = sizeof(SOCKADDR_IN);
  SOCKADDR_IN addr ; 
  SOCKET client_sock;
  BT_CLIENT * client ;
  char reason, *ip;

  client_sock = accept (s, (SOCKADDR*) &addr, &addrlen);

  if (client_sock == -1)
   continue;

  ip = inet_ntoa (addr.sin_addr);

  if ((reason = allow(ip)) <= 0)
  {
   switch (reason)
   {
   case 0:
	logevent("Refused %s:%i (filter)", 
	 ip,port_info.port);
	break;
   case -1:
	logevent("Refused %s:%i (connection limit)", 
	 ip,port_info.port);
	break;
   case -2:
	logevent("Refused %s:%i (clone limit)", 
	 ip,port_info.port);
	break;
   }
   bt_close(client_sock);
   continue;
  }

  client = addconn(ip,port_info.port);
  client->sock = client_sock;
  client->use_pass = port_info.use_pass ;

  if (client->use_pass)
   bt_cpy (client->password, port_info.password);

  if (_beginthread (proxythread,0,client) == -1)
  {
   logevent ("Could not create proxy thread for %s:%i",
	client->ip,client->port);

   bt_close(client->sock);
   delconn(client);

   _endthread();
  }
 }

 _endthread();
}