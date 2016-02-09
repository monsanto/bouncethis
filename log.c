
#include "bouncethis.h"
#include <limits.h>

/* stuff used by all the logging functions. might as well make 
it global. */

SYSTEMTIME time; 

const char *months[] = { "January", "Feburary", "March", "April", "May", "June",
"July", "August", "September", "October", "November", "December" };

char log_buf [128] = { 0 }; 

/* end of global stuff used by all functions. arent my comments nice? */

__inline void logscreen ()
{
 // uses current logging buffer.
 bt_cat(log_buf,"\r\n");
 SendMessage(server.log_screen, EM_SETSEL,0xffff,0xffff);
 SendMessage(server.log_screen, EM_REPLACESEL, 0, (LPARAM)log_buf);
}

void logstart ()
{
 // only one instance will be shown in the status screen
 // so no need to log this there.

 if (server.use_logging)
 {
  GetLocalTime(&time);

  fprintf (server.log_file,"*** %s %i, %i (%i:%i) Logging started.\n",
   months[time.wMonth-1], time.wDay, time.wYear, time.wHour, time.wMinute);
 }
}

void logend ()
{
 // we keep this format instead of the new kind designed
 // for status screen logging. since we exit here,
 // why bother logging to the status screen?

 if (server.use_logging)
 {
  GetLocalTime(&time);

  fprintf (server.log_file,"*** %s %i, %i (%i:%i) Logging ended.\n\n",
   months[time.wMonth-1], time.wDay, time.wYear, time.wHour, time.wMinute);

  fclose(server.log_file);
 }
}

void logerror (char *event, ...)
{
 char loc_buf [128] = { 0 };

 va_list l ;

 va_start (l, event);

 _vsnprintf (loc_buf, 128, event, l);

 bt_error (loc_buf);
 logevent (loc_buf);
}

void logevent (char *event, ...)
{
 char loc_buf [128] = { 0 };

 va_list l ;

 va_start (l, event);

 _vsnprintf (loc_buf, 128, event, l);

 GetLocalTime (&time);

 // no \n here. edit controls want \r\n, and log files dont. 
 // i'll manually add the return feed on both.
 wsprintf (log_buf, "(%i:%i) %s", time.wHour, time.wMinute, loc_buf);

 if (server.use_logging)
  fprintf (server.log_file,"%s\n",log_buf); 
 // log first. we mod log_buf with logscreen

 logscreen();
}