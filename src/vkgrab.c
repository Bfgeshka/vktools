/* Macro */
#include <stdio.h>
#include <stdlib.h>
#include "curlutils.h"
#include "request_gen.h"
#include "application.h"
#include "account.h"

int
main ( int argc, char ** argv )
{
	C_init();
	R_set_token();

	A_args( argc, argv );

	AC_free();
	C_finish();

	return EXIT_SUCCESS;
}
