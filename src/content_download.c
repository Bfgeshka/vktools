#include "content_download.h"
#include "stringutils.h"
#include "curlutils.h"

void
DL_doc ( account * acc, json_t * el, FILE * log, long long post_id, long long comm_id )
{
	string * docfile = construct_string(2048);
	stringset( docfile, "%s/%lld", acc->directory->s, acc->id );

	long long did = js_get_int( el, "id" );
	if ( post_id > 0 )
	{
		if ( comm_id > 0 )
		{
			if ( log != NULL )
				fprintf( log, "COMMENT %lld: ATTACH: DOCUMENT %lld (\"%s\")\n", comm_id, did, js_get_str( el, "title" ) );

			stringcat( docfile, "_%lld:%lld_%lld.%s", post_id, comm_id, did, js_get_str( el, "title" ) );
		}
		else
		{
			if ( log != NULL )
				fprintf( log, "ATTACH: DOCUMENT FOR %lld: %lld (\"%s\")\n", post_id, did, js_get_str( el, "title" ) );

			stringcat( docfile, "_%lld_%lld.%s", post_id, did, js_get_str( el, "ext" ) );
		}
	}
	else
		stringcat( docfile, "_%lld.%s", did, js_get_str( el, "ext" ) );

	C_get_file( js_get_str( el, "url" ), docfile->s );
}
