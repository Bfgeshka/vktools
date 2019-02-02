/* Macros */
#include "content_processing.h"
#include "content_download.h"
#include "stringutils.h"
#include "json.h"
#include "request_gen.h"
#include "os.h"
#include <stdio.h>
#include <string.h>

#define DEFAULT_CONTENT_DOCS 1
#define DEFAULT_CONTENT_PICS 1
#define DEFAULT_CONTENT_VIDS 0
#define DEFAULT_CONTENT_COMS 1

#define FILENAME_POSTS "wall.txt"
#define FILENAME_GROUPS "communities.txt"
#define FILENAME_FRIENDS "friends.txt"
#define DIRNAME_WALL "alb_attachments"
#define DIRNAME_DOCS "alb_docs"
#define DIRNAME_ALB_PROF "alb_profile"
#define DIRNAME_ALB_WALL "alb_wall"
#define DIRNAME_ALB_SAVD "alb_saved"
#define LOG_POSTS_DIVIDER "-~-~-~-~-~-~\n~-~-~-~-~-~-\n\n"

// Limitation for number of wall posts per request. Current is 100
#define LIMIT_W 100

// Limitation for number of comments per request. Current is 100
#define LIMIT_C 100

// Limitation for number of photos per request. Current is 1000
#define LIMIT_A 1000

#define CONVERT_TO_READABLE_DATE 1

/* Local scope */
static void CT_parse_attachments ( account *, json_t * input_json, FILE * logfile, long long post_id, long long comm_id );
static void CT_get_comments ( account * acc, FILE * logfile, long long post_id );

static void
CT_get_comments ( account * acc, FILE * logfile, long long post_id )
{
	string * apimeth = construct_string(512);

	long long offset = 0;
	long long posts_count = 0;

	do
	{
		stringset( apimeth, "wall.getComments?owner_id=%lld&extended=0&post_id=%lld&count=%d&offset=%lld",
		    acc->id,
		    post_id,
		    LIMIT_C,
		    offset );

		int err_ret = 0;
		json_t * json = RQ_request( apimeth, &err_ret );

		switch ( err_ret )
		{
			case -1:
				return;
			case -3:
				content.comments = 0;
				return;
			default:
				break;
		}

		if ( err_ret != 0 )
			return;

		/* Getting comments count */
		if ( offset == 0 )
			posts_count = js_get_int( json, "count" );

		/* Iterations in array */
		size_t index;
		json_t * el;
		json_t * items = json_object_get( json, "items" );
		json_array_foreach( items, index, el )
		{
			long long c_id = js_get_int( el, "id" );
			long long epoch = js_get_int( el, "date" );

			fprintf( logfile, "COMMENT %lld: EPOCH: %lld ", c_id, epoch  );
			if ( CONVERT_TO_READABLE_DATE )
				if ( OS_readable_date( epoch, logfile ) != 0 )
					fprintf( stderr, "%s", "Failed to find 'date' utility." );

			fprintf( logfile, "COMMENT %lld: TEXT: %s\n-~-~-~-~-~-~\n", c_id, js_get_str( el, "text" ) );

			/* Searching for attachments */
			json_t * att_json = json_object_get( el, "attachments" );
			if ( att_json )
				CT_parse_attachments( acc, att_json, logfile, post_id, c_id );
		}

		json_decref(el);
		offset += LIMIT_C;
	}
	while ( posts_count - offset > 0 );

	free_string(apimeth);
}

static void
CT_parse_attachments ( account * acc, json_t * input_json, FILE * logfile, long long post_id, long long comm_id )
{
	(void)post_id;
	(void)comm_id;
	(void)acc;

	size_t att_index;
	json_t * att_elem;
	char data_type[5][6] = { "photo", "link", "doc", "video" };

	json_array_foreach( input_json, att_index, att_elem )
	{
		const char * att_type = js_get_str( att_elem, "type" );
		json_t * output_json;

		/* If photo: 0 */
		if ( content.pictures == 1 && strcmp( att_type, data_type[0] ) == 0 )
		{
			output_json = json_object_get( att_elem, data_type[0] );
			DL_photo( acc, output_json, logfile, post_id, comm_id );
		}

		/* If link: 1 */
		if ( strcmp( att_type, data_type[1] ) == 0 )
		{
			output_json = json_object_get( att_elem, data_type[1] );
			fprintf( logfile, "ATTACH: LINK_URL: %s\nATTACH: LINK_DSC: %s\n",
			    js_get_str( output_json, "url" ), js_get_str( output_json, "description" ) );
		}

		/* If doc: 2 */
		if ( content.documents == 1 && strcmp( att_type, data_type[2] ) == 0 )
		{
			output_json = json_object_get( att_elem, data_type[2] );
			DL_doc( acc, output_json, logfile, post_id, comm_id );
		}

		/* If video: 3 */
//		if ( content.videos == 1 && strcmp( att_type, data_type[3] ) == 0 )
//		{
//			output_json = json_object_get( att_elem, data_type[3] );
//			dl_video( dirpath, filepath, output_json, logfile, post_id, comm_id );
//		}
	}
}

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
		acc->albums_count = counter;
		json_t * al_items = json_object_get( json, "items" );
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
	FILE * wallfp;

	stringset( acc->currentdir, "%s/%s", acc->directory->s, DIRNAME_WALL );
	OS_new_directory(acc->currentdir->s);

	string * wallfilepath = construct_string(2048);
	stringset( wallfilepath, "%s/%s", acc->directory->s, FILENAME_POSTS );
	wallfp = fopen( wallfilepath->s, "w" );
	free_string(wallfilepath);

	long long offset = 0;
	long long posts_count = 0;
	do
	{
		stringset( apimeth, "wall.get?owner_id=%lld&extended=0&count=%d&offset=%lld", acc->id, LIMIT_W, offset );
		int err_ret = 0;
		json_t * json = RQ_request( apimeth, &err_ret );
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
			json_t * att_json = json_object_get( el, "attachments" );
			if ( att_json )
				CT_parse_attachments( acc, att_json, wallfp, p_id, -1 );

			/* Searching for comments */
			json_t * comments = json_object_get( el, "comments" );
			if ( comments )
			{
				long long comm_count = js_get_int( comments, "count" );
				if ( comm_count > 0 )
				{
					fprintf( wallfp, "COMMENTS: %lld\n", comm_count );
					if ( content.comments )
						CT_get_comments( acc, wallfp, p_id );
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
					json_t * rep_att_json = json_object_get( rep_elem, "attachments" );
					if ( rep_att_json )
						CT_parse_attachments( acc, rep_att_json, wallfp, p_id, -1 );
				}
			}

			fprintf( wallfp, "%s", LOG_POSTS_DIVIDER );
		}

		offset += LIMIT_W;
		json_decref(el);
	}
	while( posts_count - offset > 0 );

	CT_get_wall_cleanup:
	free_string(apimeth);
	fclose(wallfp);
}

void
CT_get_groups ( account * acc )
{
	string * apimeth = construct_string(256);
	stringset( apimeth, "groups.get?user_id=%lld&extended=1", acc->id );
	int err_ret = 0;
	json_t * json = RQ_request( apimeth, &err_ret );
	if ( err_ret < 0 )
		return;

	string * groupsfilepath = construct_string(2048);
	stringset( groupsfilepath, "%s/%s", acc->directory->s, FILENAME_GROUPS );
	FILE * groupsfp = fopen( groupsfilepath->s, "w" );
	free_string(groupsfilepath);

	printf( "Comminities: %lld.\n", js_get_int( json, "count" ) );

	/* iterations in array */
	size_t index;
	json_t * el;
	json_t * items = json_object_get( json, "items" );
	json_array_foreach( items, index, el )
	{
		if ( index != 0 )
			fprintf( groupsfp, "%s\n", js_get_str( el, "screen_name" ) );
	}

	json_decref(el);
	fclose(groupsfp);
}

void
CT_get_friends ( account * acc )
{
	string * apimeth = construct_string(256);
	stringset( apimeth, "friends.get?user_id=%lld&order=domain&fields=domain", acc->id );
	int err_ret = 0;
	json_t * json = RQ_request( apimeth, &err_ret );
	if ( err_ret < 0 )
		return;

	string * friendsfilepath = construct_string(2048);
	stringset( friendsfilepath, "%s/%s", acc->directory->s, FILENAME_FRIENDS );
	FILE * friendsfp = fopen( friendsfilepath->s, "w" );
	free_string(friendsfilepath);

	printf( "Friends: %lld.\n", js_get_int( json, "count" ) );

	/* iterations in array */
	size_t index;
	json_t * el;
	json_t * items = json_object_get( json, "items" );
	json_array_foreach( items, index, el )
	{
		if ( index != 0 )
			fprintf( friendsfp, "%s\n", js_get_str( el, "domain" ) );
	}

	json_decref(el);
	fclose(friendsfp);
}

void
CT_get_docs ( account * acc )
{
	if ( content.documents == 0 )
		return;

	string * apimeth = construct_string(256);
	stringset( apimeth, "docs.get?owner_id=%lld", acc->id );
	int err_ret = 0;
	json_t * json = RQ_request( apimeth, &err_ret );
	if ( err_ret < 0 )
		return;

	stringset( acc->currentdir, "%s/%s", acc->directory->s, DIRNAME_DOCS );
	OS_new_directory(acc->currentdir->s);

	printf("Documents: %lld.\n", js_get_int( json, "count" ) );

	/* Loop init */
	size_t index;
	json_t * el;
	json_t * items = json_object_get( json, "items" );
	json_array_foreach( items, index, el )
	{
		if ( index != 0 )
			DL_doc( acc, el, NULL, -1, -1 );
	}

	json_decref(el);
}

void
CT_get_albums_files ( account * acc )
{
	string * apimeth = construct_string(1024);
	string * albumdir = construct_string(2048);
	json_t * json;

	for ( size_t i = 0; i < acc->albums_count; ++i )
	{
		printf( "\nAlbum %zu/%zu, id: %lld \"%s\" contains %lld photos.\n",
		    i + 1,
		    acc->albums_count,
		    acc->albums[i].id,
		    acc->albums[i].title->s,
		    acc->albums[i].size );

		if ( acc->albums[i].size == 0 )
			goto CT_get_albums_files_loop_end;

		stringset( albumdir, "%s/", acc->directory->s );
		switch ( acc->albums[i].id )
		{
			case -6:
				stringcat( albumdir, "%s", DIRNAME_ALB_PROF );
				break;
			case -7:
				stringcat( albumdir, "%s", DIRNAME_ALB_WALL );
				break;
			case -15:
				stringcat( albumdir, "%s", DIRNAME_ALB_SAVD );
				break;
			default:
				stringcat( albumdir, "alb_%lld_(%zu:%zu)_(%lld:p)_(%s)",
				    acc->albums[i].id,
				    i + 1,
				    acc->albums_count,
				    acc->albums[i].size,
				    acc->albums[i].title );
				break;
		}

		OS_new_directory(albumdir->s);
		stringset( acc->currentdir, "%s", albumdir->s );

		int times = acc->albums[i].size / LIMIT_A;
		for ( int offset = 0; offset <= times; ++offset )
		{
			stringset( apimeth, "photos.get?owner_id=%lld&album_id=%lld&photo_sizes=0&offset=%d",
			    acc->id,
			    acc->albums[i].id,
			    offset & LIMIT_A );

			int err_ret = 0;
			json = RQ_request( apimeth, &err_ret );
			if ( err_ret < 0 )
				goto CT_get_albums_files_loop_end;

			size_t index;
			json_t * el;
			json_t * items = json_object_get( json, "items" );
			json_array_foreach( items, index, el )
			{
				DL_photo( acc, el, NULL, (long long)index + offset * LIMIT_A + 1, -1 );
			}
		}

		CT_get_albums_files_loop_end:;
	}

	json_decref(json);
	free_string(apimeth);
	free_string(albumdir);
}
