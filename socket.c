
#include "bouncethis.h"



// no logging in this function. people are idiots.
// why waste space/interaction with host for their
// faults?

SOCKET bt_connect (const char *host, const char *port)
{
	SOCKET s;
	const ADDRINFO hints = { 0, AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, 0, 0 };
	ADDRINFO * res;

	s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (s < 0)
		return 0;

	if (getaddrinfo(host, port, &hints, &res) != 0) 
		return 0;

	if (connect(s, res->ai_addr, (int)res->ai_addrlen) == 0)
	{
		freeaddrinfo(res);
		return s;
	}
	else
	{
		closesocket(s);
		freeaddrinfo(res);
		return 0;
	}

}

SOCKET bt_listen (short port)
{
	SOCKET s;
	SOCKADDR_IN addr;

	addr.sin_family = AF_INET;
	addr.sin_port = htons (port);
	addr.sin_addr = server.addr;

	s = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (s < 0)
	{
		logerror ("Could not allocate new socket");
		return 0;
	}

	if (bind (s, (SOCKADDR*) &addr, sizeof(addr)) == 0)
	{
		if (listen (s, 1024) != 0)
		{
			logerror ("Error listening on %i",  port);
			goto socket_error;
		}

		return s;
	}
	else
	{
		logerror ((server.use_vhost) 
			? "Could not bind to virtual host %s on %i"
			: "Could not bind to local address %s on %i" 
			,server.ip,port);

		goto socket_error;
	}

socket_error:
	closesocket(s);
	return 0;
}

int bt_sendf (SOCKET s, char *data, ...)
{
	char buffer [4096] = { 0 };
	int len ;

	va_list l ;
	va_start (l, data);

	len = _vsnprintf (buffer, 4096, data, l);

	return send (s, buffer, len, 0);
}

