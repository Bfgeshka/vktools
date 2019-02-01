/* Macros */
#include "content_processing.h"
#include "stringutils.h"
#include "json.h"
#include "request_gen.h"
#include <stdio.h>

#define DEFAULT_CONTENT_DOCS 1
#define DEFAULT_CONTENT_PICS 1
#define DEFAULT_CONTENT_VIDS 0
#define DEFAULT_CONTENT_COMS 1

/* Global scope */
void
CT_default ( void )
{
	content.comments = DEFAULT_CONTENT_COMS;
	content.documents = DEFAULT_CONTENT_DOCS;
	content.pictures = DEFAULT_CONTENT_PICS;
	content.videos = DEFAULT_CONTENT_VIDS;
}

void
CT_print_types ( void )
{
	string * str = construct_string(1024);

	stringset( str, "Will try to get:" );

	if ( content.comments )
		stringcat( str, " comments" );

	if ( content.documents )
		stringcat( str, " documents" );

	if ( content.pictures )
		stringcat( str, " pictures" );

	if ( content.videos )
		stringcat( str, " videos" );

	stringcat( str, ".\n");

	puts(str->s);
	free_string(str);
}

void
CT_get_albums ( account * acc )
{
	string * apimeth = construct_string(1024);
	stringset( apimeth, "photos.getAlbums?owner_id=%lld&need_system=1", acc->id );

	if ( acc->id < 0 )
		stringcat( apimeth, "%s", "&album_ids=-7" );

	int err_ret = 0;
	json_t * json = RQ_request( apimeth, &err_ret );
	free_string(apimeth);
	if ( err_ret < 0 )
		return;

	long long counter = js_get_int( json, "count" );

	if ( counter > 0 )
	{
		acc->albums = malloc( counter * sizeof(album) );
		json_t * al_items;
		al_items = json_object_get( json, "items" );
//		albums = malloc( counter * sizeof(struct data_album) );
		printf( "\nAlbums: %lld.\n", counter );

		json_t * el;
		size_t index;
		json_array_foreach( al_items, index, el )
		{
			acc->albums[index].id = js_get_int( el, "id" );
			acc->albums[index].size = js_get_int( el, "size" );
			acc->albums[index].title = construct_string(1024);
			stringset( acc->albums[index].title, "%s", js_get_str( el, "title" ) );
//			photos_count += acc->albums[index].size;
		}
	}
	else
		puts( "No albums found." );

	json_decref(json);
}
