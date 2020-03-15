/*********************************************************************
*
* File      : format_number_with_commas.c
*
* Author    : Barry Kimelman
*
* Created   : September 23, 2019
*
* Purpose   : Format an integer with commas
*
*********************************************************************/

#include	<stdio.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

/*********************************************************************
*
* Function  : format_number_with_commas
*
* Purpose   : Format an integer with commas
*
* Inputs    : int number - numeric value to be formatted
*             char *buffer - buffer to received formatted string
*
* Output    : (none)
*
* Returns   : nothing
*
* Example   : format_number_with_commas(size,buffer);
*
* Notes     : (none)
*
*********************************************************************/

void format_number_with_commas(int number, char *buffer)
{
	char	str_number[100] , *ptr1 , *ptr2 , *str;
	int	num_digits , num_groups , output_len , count , index;

	sprintf(str_number,"%d",number);
	num_digits = strlen(str_number);
	if ( num_digits <= 3 ) {
		strcpy(buffer,str_number);
	} /* IF */
	else {
		strcpy(buffer,str_number);
		num_groups = (num_digits + 2) / 3;
		output_len = (num_groups - 1) + num_digits;
		memset(buffer, 0, output_len+1);
		ptr2 = &str_number[num_digits - 1];
		ptr1 = &str_number[num_digits - 3];
		str = &buffer[output_len - 3];
		for ( index = 1 ; index <= num_groups ; ++index ) {
			count = (num_digits >= 3) ? 3 : num_digits;
			memmove(str, ptr1, count);
			num_digits -= count;
			if ( index < num_groups ) {
				*--str = ',';
			} /* IF */
			count = (num_digits >= 3) ? 3 : num_digits;
			ptr1 -= count;
			ptr2 -= count;
			str -= count;
		} /* FOR */
	} /* ELSE */

	return;
} /* end of format_number_with_commas */
