/* Macros */
#include "content_processing.h"
#include "stringutils.h"
#include "json.h"
#include "request_gen.h"
#include "os.h"
#include <stdio.h>

#define DEFAULT_CONTENT_DOCS 1
#define DEFAULT_CONTENT_PICS 1
#define DEFAULT_CONTENT_VIDS 0
#define DEFAULT_CONTENT_COMS 1

#define FILENAME_POSTS "wall.txt"
#define DIRNAME_WALL "alb_attachments"
#define LOG_POSTS_DIVIDER "-~-~-~-~-~-~\n~-~-~-~-~-~-\n\n"

// Limitation for number of wall posts per request. Current is 100
#define LIMIT_W 100

#define CONVERT_TO_READABLE_DATE 1

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
		printf( "Albums: %lld.\n", counter );

		json_t * el;
		size_t index;
		json_array_foreach( al_items, index, el )
		{
			acc->albums[index].id = js_get_int( el, "id" );
			acc->albums[index].size = js_get_int( el, "size" );

			acc->albums[index].title = construct_string(1024);
			stringset( acc->albums[index].title, "%s", js_get_str( el, "title" ) );
			OS_fix_filename(acc->albums[index].title->s);
		}
	}
	else
		puts( "No albums found." );

	json_decref(json);
}

void
CT_get_wall ( account * acc )
{
	string * apimeth = construct_string(128);
//	string * walldir = construct_string(2048);
	FILE * wallfp;

	stringset( acc->currentdir, "%s/%s", acc->directory->s, DIRNAME_WALL );
	OS_new_directory(acc->currentdir->s);

	string * wallfilepath = construct_string(2048);
	stringset( wallfilepath, "%s/%s", acc->directory->s, FILENAME_POSTS );
	wallfp = fopen( wallfilepath->s, "w" );
	free_string(wallfilepath);

	long long offset = 0;
	long long posts_count = 0;

//	/* Char allocation */
//	sstring * url = construct_string(2048);
//	sstring * attach_path = construct_string(2048);
//	sstring * curpath = construct_string(2048);

//	sstring * posts_path = construct_string(2048);
//	stringset( posts_path, "%s/%s", idpath, FILNAME_POSTS );
//	FILE * posts = fopen( posts_path->c, "w" );
//	free_string(posts_path);

//	stringset( curpath, "%s/%s", idpath, DIRNAME_WALL );
//	if ( mkdir( curpath->c, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH ) != 0 )
//		if ( errno != EEXIST )
//			fprintf( stderr, "mkdir() error (%d).\n", errno );

	/* Loop start */
//	long long offset = 0;
//	long long posts_count = 0;
	do
	{
//		stringset( url, "%s/wall.get?owner_id=%lld&extended=0&count=%d&offset=%lld%s&v=%s",
//		         REQ_HEAD, acc.id, LIMIT_W, offset, TOKEN.c, API_VER );
//
//		json_error_t * json_err = NULL;
//		json_t * json = make_request( url, json_err );
//		if ( !json )
//			if ( json_err )
//				fprintf( stderr, "JSON wall.get parsing error.\n%d:%s\n", json_err->line, json_err->text );
//
//		/* Finding response */
//		json_t * rsp;
//		rsp = json_object_get( json, "response" );
//		if ( !rsp )
//		{
//			fprintf( stderr, "Wall error.\n" );
//			rsp = json_object_get( json, "error" );
//			fprintf( stderr, "%s\n", js_get_str( rsp, "error_msg" ) );
//		}

		stringset( apimeth, "wall.get?owner_id=%lld&extended=0&count=%d&offset=%lld", acc->id, LIMIT_W, offset );
		int err_ret = 0;
		json_t * json = RQ_request( apimeth, &err_ret);
		if ( err_ret < 0 )
			goto CT_get_wall_cleanup;

		/* Getting posts count */
		if ( offset == 0 )
		{
			posts_count = js_get_int( json, "count" );
			printf( "Wall posts: %lld.\n", posts_count );
		}

		/* Iterations in array */
		size_t index;
		json_t * el;
		json_t * items = json_object_get( json, "items" );
		json_array_foreach( items, index, el )
		{
			long long p_id = js_get_int( el, "id" );
			long long epoch = js_get_int( el, "date" );

			fprintf( wallfp, "ID: %lld\n", p_id );
			if ( CONVERT_TO_READABLE_DATE )
				if ( OS_readable_date( epoch, wallfp ) != 0 )
					fprintf( stderr, "%s", "Failed to find 'date' utility." );

			fprintf( wallfp, "EPOCH: %lld\nTEXT: %s\n", epoch, js_get_str( el, "text" ) );

			/* Searching for attachments */
//			json_t * att_json = json_object_get( el, "attachments" );
//			if ( att_json )
//				parse_attachments( curpath, attach_path, att_json, wallfp, p_id, -1 );

			/* Searching for comments */
			json_t * comments = json_object_get( el, "comments" );
			if ( comments )
			{
				long long comm_count = js_get_int( comments, "count" );
				if ( comm_count > 0 )
				{
					fprintf( wallfp, "COMMENTS: %lld\n", comm_count );
//					if ( content.comments )
//						get_comments( curpath, attach_path, wallfp, p_id );
				}
			}

			/* Checking if repost */
			json_t * repost_json = json_object_get( el, "copy_history" );
			if ( repost_json )
			{
				json_t * rep_elem = json_array_get( repost_json, 0 );
				if ( rep_elem )
				{
					fprintf( wallfp, "REPOST FROM: %lld\nTEXT: %s\n",
					         js_get_int( rep_elem, "from_id" ), js_get_str( rep_elem, "text" ) );
//					json_t * rep_att_json = json_object_get( rep_elem, "attachments" );
//					if ( rep_att_json )
//						parse_attachments( curpath, attach_path, rep_att_json, wallfp, p_id, -1 );
				}
			}

			fprintf( wallfp, "%s", LOG_POSTS_DIVIDER );
		}

		offset += LIMIT_W;
		json_decref(json);
	}
	while( posts_count - offset > 0 );

	CT_get_wall_cleanup:
	free_string(apimeth);
//	free_string(attach_path);
//	free_string(walldir);

	fclose(wallfp);
}
