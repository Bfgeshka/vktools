/* Macro */
#include <stdlib.h>
#include "curlutils.h"
#include "request_gen.h"
#include "application.h"
#include "account.h"
#include "content.h"

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

	if ( Account->type == e_user )
	{
		CT_get_groups(Account);
		CT_get_friends(Account);
	}

	CT_get_docs(Account);
	CT_get_wall(Account);
	CT_get_albums(Account);
	CT_get_albums_files(Account);

	// Finish
	AC_free(Account);
	RQ_finish();
	C_finish();

	puts("All good, exitting...\n");
	return EXIT_SUCCESS;
}
