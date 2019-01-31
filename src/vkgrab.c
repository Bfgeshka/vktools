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
	// Init
	C_init();
	R_set_token();

	// Flow
	A_args( argc, argv );
	string * dir = AC_make_dir();
	(void)dir;

	// Finish
	AC_free();
	C_finish();

	return EXIT_SUCCESS;
}
