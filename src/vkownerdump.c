/* Macros */
#include <stdlib.h>
#include "curlutils.h"
#include "request_gen.h"
#include "application.h"
#include "account.h"
#include "content_processing.h"

int
main ( int argc, char ** argv )
{
	extern account * Account;
	// Init
	C_init();
	RQ_set_token();

	// Flow
	A_args( argc, argv );
	AC_make_dir(Account);

	CT_get_stars(Account);

	// Finish
	AC_free(Account);
	C_finish();

	return EXIT_SUCCESS;
}
