
#include "bouncethis.h"

int parse(FILE * file) // new parser. 8D
{
 char line[256] = { 0 };
 char *word, *param ;

 while (fgets(line,256,file))
 {
  word = strtok(line," \t\n");
  param = strtok(NULL, " \t\n");

  if (!word || !param)
   continue ;

  if (!strcmpi(word,"port"))
  {
   if (server.port_count >= 24)
   {
	bt_error("Too many ports defined; 24 max.");
	return 0;
   }

   server.port_data[server.port_count].port = (short) atoi(param);

   if (server.port_data[server.port_count].port <= 0 ||
	server.port_data[server.port_count].port >= 65535)
   {
	bt_error("Port number may be a number between 1 and 65535.");
	return 0;
   }

   server.port_count++;
  }
  else if (!strcmpi(word,"filtermode"))
  {
   if (!strcmpi(param,"accept"))
	server.filter_mode = 1;
   else if (!strcmpi(param,"deny"))
	server.filter_mode = 0;
   else // not critical, won't return here.
	bt_error("FilterMode can either be \"Accept\" or \"Deny.\"\n"
	"Defaulting to \"Deny\".");
  }
  else if (!strcmpi(word,"filter"))
  {
   if (server.filter_count >= 128)
   {
	bt_error("Too many filters defined; 128 max.");
	return 0;
   }

   bt_cpy (server.filter_data[server.filter_count],param);

   server.filter_count++;
  }
  else if (!strcmpi(word,"password"))
  {
   if (server.port_count == 0)
   {
	bt_error("No port to set the password for.");
	return 0;
   }

   server.port_data[server.port_count-1].use_pass = 1;
   bt_cpy (server.port_data[server.port_count-1].password, param);
  }
  else if (!strcmpi(word,"ident"))
  {
   server.use_identd = 1;
   bt_cpy (server.ident, param);
  }
  else if (!strcmpi(word,"vhost"))
  {
   server.use_vhost = 1;
   bt_cpy (server.ip, param);
  }
  else if (!strcmpi(word,"logfile"))
  {
   server.use_logging = 1;
   bt_cpy (server.log_data, param);
  }
  else if (!strcmpi(word,"limitclones"))
  {
   server.clone_limit = atoi(param);

   if (server.clone_limit <= 0 ||
	server.clone_limit >= 4294967295 )
   {
	bt_error("Clones limit may be a number between 1 and 4294967295.");
	return 0;
   }
  }
  else if (!strcmpi(word,"limitconn"))
  {
   server.conn_limit  = atoi(param);

   if (server.conn_limit <= 0 ||
	server.conn_limit >= 4294967295 )
   {
	bt_error("Connection limit may be a number between 1 and 4294967295.");
	return 0;
   }
  }
  else if (!strcmpi(word,"admin"))
  {
   char * param2 = strtok(NULL," \t\n") ;

   if (!param2)
	continue ;

   bt_cpy(server.admin_user,param) ;
   bt_cpy(server.admin_pass,param2) ;
  }
  else
   continue;
 }

 return 1;
}