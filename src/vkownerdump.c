/* Macros */
#include <stdlib.h>
#include "curlutils.h"
#include "request_gen.h"
#include "application.h"
#include "account.h"
#include "content_messages.h"

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
	CT_get_conversations_history(Account);

	// Finish
	AC_free(Account);
	RQ_finish();
	C_finish();

	puts("All good, exitting...\n");
	return EXIT_SUCCESS;
}
