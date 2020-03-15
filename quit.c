#include	<stdio.h>
#include	<stdarg.h>
#include	<string.h>
#include	<stdlib.h>
#include	<errno.h>

/*********************************************************************
*
* Function  : quit
*
* Purpose   : Issue a system error message and then terminate
*             program execution.
*
* Inputs    : int exit_code - program exit code
*             ... - arguments for vfprintf()
*
* Output    : error message
*
* Returns   : (nothing)
*
* Example   : quit(1,"open failed for file [%s]",filename);
*
* Notes     : (none)
*
*********************************************************************/

void quit(int exit_code,char *format,...)
{
	va_list	ap;
	int		errnum;

	errnum = errno;
	va_start(ap,format);
	vfprintf(stderr,format,ap);
	va_end(ap);
	fprintf(stderr," : %s\n",strerror(errnum));
	fflush(stderr);
	errno = errnum;
	exit(exit_code);
} /* end of quit */
