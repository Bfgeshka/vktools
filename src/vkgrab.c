/* Macro */
#include <stdio.h>
#include <stdlib.h>
#include "curlutils.h"
#include "request_gen.h"
#include "application.h"

int
main ( int argc, char ** argv )
{
	C_init();
	R_set_token();

	A_args( argc, argv );

	C_finish();

	return EXIT_SUCCESS;
}
