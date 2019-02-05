/* Macros */
#include "content_download.h"
#include "stringutils.h"
#include "curlutils.h"

#define SIZES 10
static const char SIZES_PRIORITY[SIZES] = { 's', 'm', 'o', 'p', 'q', 'x', 'y', 'z', 'w' };

/* Local scope */
static const char * DL_get_photo_url ( json_t * el );

static const char *
DL_get_photo_url ( json_t * el )
{
	json_t * sizes = json_object_get( el, "sizes" );
	if ( sizes )
	{
		int max_size = 0;
		int max_arr_index = 0;

		int arrsize = json_array_size(sizes);
		for ( int i = 0; i < arrsize; ++i )
		{
			json_t * el = json_array_get( sizes, i );

			for ( int j = max_size; j < SIZES; ++j )
				if ( js_get_str( el, "type" )[0] == SIZES_PRIORITY[j] )
				{
					max_size = j;
					max_arr_index = i;
				}
		}

		return js_get_str( json_array_get( sizes, max_arr_index ), "url" );
	}
	else
	{
		json_t * biggest;

		if ( ( biggest = json_object_get( el, "photo_2560" ) ) != NULL )
			return json_string_value(biggest);

		if ( ( biggest = json_object_get( el, "photo_1280" ) ) != NULL )
			return json_string_value(biggest);

		if ( ( biggest = json_object_get( el, "photo_807" ) ) != NULL )
			return json_string_value(biggest);

		if ( ( biggest = json_object_get( el, "photo_604" ) ) != NULL )
			return json_string_value(biggest);

		if ( ( biggest = json_object_get( el, "photo_130" ) ) != NULL )
			return json_string_value(biggest);

		if ( ( biggest = json_object_get( el, "photo_75" ) ) != NULL )
			return json_string_value(biggest);
	}

	return NULL;
}

/* Global scope */
void
DL_doc ( account * acc, json_t * el, FILE * log, long long post_id, long long comm_id )
{
	string * docfile = construct_string(2048);
	stringset( docfile, "%s/%lld", acc->currentdir->s, acc->id );

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

	free_string(docfile);
}

void /* if no post_id, then set it to '-1', FILE * log replace with NULL */
DL_photo ( account * acc, json_t * el, FILE * log, long long post_id, long long comm_id )
{
	long long pid = js_get_int( el, "id" );

	if ( post_id > 0 && log != NULL )
	{
		if ( comm_id > 0 )
			fprintf( log, "COMMENT %lld: ATTACH: PHOTO %lld\n", comm_id, pid );
		else
			fprintf( log, "ATTACH: PHOTO FOR %lld: %lld\n", post_id, pid );
	}

	string * picfile = construct_string(2048);
	stringset( picfile, "%s/%lld", acc->currentdir->s, acc->id );

	if ( post_id > 0 )
		stringcat( picfile, "_%lld", post_id );

	if ( comm_id > 0 )
		stringcat( picfile, ":%lld", comm_id );

	stringcat( picfile, "_%lld.jpg", pid );

	C_get_file( DL_get_photo_url(el), picfile->s );

	free_string(picfile);
}
