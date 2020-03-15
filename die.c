#include	<stdio.h>
#include	<stdarg.h>
#include	<stdlib.h>

/*********************************************************************
*
* Function  : die
*
* Purpose   : Issue an error message and then terminate program execution.
*
* Inputs    : int exit_code - program exit code
*             ... - arguments for vfprintf()
*
* Output    : error message
*
* Returns   : (nothing)
*
* Example   : die(1,"Unknown error : %d\n",status);
*
* Notes     : (none)
*
*********************************************************************/

void die(int exit_code,char *format,...)
{
	va_list	ap;

	va_start(ap,format);
	vfprintf(stderr,format,ap);
	va_end(ap);
	exit(exit_code);
} /* end of die */
