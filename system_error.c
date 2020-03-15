#include <stdio.h>
#include <stdarg.h>
#include <errno.h>
#include	<string.h>

/*********************************************************************
*
* Function  : system_error
*
* Purpose   : Issue a system error message.
*
* Inputs    : ... - arguments for vfprintf()
*
* Output    : error message
*
* Returns   : (nothing)
*
* Example   : system_error("open failed for file [%s]",filename);
*
* Notes     : (none)
*
*********************************************************************/

void system_error(char *format,...)
{
      va_list args;
      int	errnum;

      va_start(args,format);
      errnum = errno;
      vfprintf(stderr,format,args);
      fprintf(stderr," : %s\n",strerror(errnum));
      fflush(stderr);
      errno = errnum;

      va_end(args);
} /* end of system_error */
