/* Macro */
#include <stdio.h>
#include <stdlib.h>
#include "curlutils.h"
#include "request_gen.h"

int
/*main ( int argc, char ** argv )*/
main ( void )
{
	C_init();
	R_set_token();

	puts("oi");

	C_finish();
	return EXIT_SUCCESS;
}
