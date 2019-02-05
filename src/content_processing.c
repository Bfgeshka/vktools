/* Macros */
#include "content_processing.h"
#include "content_download.h"
#include "stringutils.h"
#include "json.h"
#include "request_gen.h"
#include "os.h"
#include <stdio.h>
#include <string.h>

#define CONVERT_TO_READABLE_DATE 1
#define DEFAULT_CONTENT_DOCS 1
#define DEFAULT_CONTENT_PICS 1
#define DEFAULT_CONTENT_VIDS 0
#define DEFAULT_CONTENT_COMS 1
#define FILENAME_POSTS "wall.txt"
#define FILENAME_GROUPS "communities.txt"
#define FILENAME_FRIENDS "friends.txt"
#define FILENAME_STARS "stars.txt"
#define DIRNAME_WALL "alb_attachments"
#define DIRNAME_DOCS "alb_docs"
#define DIRNAME_ALB_PROF "alb_profile"
#define DIRNAME_ALB_WALL "alb_wall"
#define DIRNAME_ALB_SAVD "alb_saved"
#define DIRNAME_STARS "alb_stars"
#define LOG_POSTS_DIVIDER "-~-~-~-~-~-~\n~-~-~-~-~-~-\n\n"

// Limitation for number of wall posts per request. Current is 100
#define LIMIT_W 100

// Limitation for number of comments per request. Current is 100
#define LIMIT_C 100

// Limitation for number of photos per request. Current is 1000
#define LIMIT_A 1000

// Limitation for number of messages per request. Current is 200
#define LIMIT_M 200


/* Typedef */
typedef struct conversation_user
{
	long long id;
	string * fname;
	string * lname;
} conversator;

typedef struct conversation_data
{
	long long id;
	long long localid;
	string * name;
} conversation;


/* Local scope */
static conversator * Conversators = NULL;
static size_t Conversators_count = 0;
static conversation * Conversations = NULL;
static size_t Conversations_count = 0;

static void CT_parse_attachments ( account *, json_t * input_json, FILE * logfile, long long post_id, long long comm_id );
static void CT_get_comments ( account * acc, FILE * logfile, long long post_id );
static void CT_get_conversators ( json_t * json );
static void CT_get_conversations ( json_t * json );
static void free_conversator ( conversator * c );
static void free_conversation ( conversation * c );
static conversator * CT_find_conversator ( long long id );
static conversation * CT_find_conversation ( long long id );
static void CT_single_star ( account * acc, json_t * el, FILE * log );

static void CT_single_star ( account * acc, json_t * el, FILE * log )
{
	long long id = js_get_int( el, "id" );

	fprintf( log, "ID: %lld\n", id );

	conversator * author = CT_find_conversator( js_get_int(el, "from_id" ));
	if ( author != NULL )
		fprintf( log, "FROM: %s %s (%lld)\n", author->fname->s, author->lname->s, author->id );
	else
		fputs( "FROM: unknown source\n", log );

	conversation * conv = CT_find_conversation( js_get_int(el, "peer_id" ));
	if ( conv != NULL )
		fprintf( log, "IN: %s (%lld)\n", conv->name->s, conv->id );
	else
		fputs( "IN: unknown source\n", log );

	long long epoch = js_get_int( el, "date" );
	if ( CONVERT_TO_READABLE_DATE )
		OS_readable_date( epoch, log );

	fprintf( log, "EPOCH: %lld\n", epoch );
	fprintf( log, "TEXT:\n%s\n", js_get_str( el, "text" ) );

	CT_parse_attachments( acc, json_object_get( el, "attachments" ), log, id, -1 );

	json_t * reposted = json_object_get( el, "attachments" );
	if ( reposted != NULL )
	{
		for ( size_t i = 0; i < json_array_size(reposted); ++i )
		{
			fputs( "Reposted, original post below:\n", log );
			CT_single_star( acc, json_array_get( el, i ), log );
		}
	}

	fprintf( log, "%s", LOG_POSTS_DIVIDER );
}

static conversation * CT_find_conversation ( long long id )
{
	for ( size_t i = 0; i < Conversations_count; ++i )
		if ( id == Conversations[i].id )
			return &Conversations[i];

	return NULL;
}

static conversator * CT_find_conversator ( long long id )
{
	for ( size_t i = 0; i < Conversators_count; ++i )
		if ( id == Conversators[i].id )
			return &Conversators[i];

	return NULL;
}

static void free_conversation ( conversation * c )
{
	free(c->name);
}

static void free_conversator ( conversator * c )
{
	free(c->fname);
	free(c->lname);
}

static void CT_get_conversations ( json_t * json )
{
	Conversations_count = json_array_size(json);
	Conversations = malloc( sizeof(conversation) * Conversations_count );

	json_t * el;
	json_t * peer;
	json_t * settings;
	for ( size_t i = 0; i < Conversations_count; ++i )
	{
		el = json_array_get( json, i );
		peer = json_object_get( el, "peer" );
		settings = json_object_get( el, "chat_settings" );

		Conversations[i].id = js_get_int( peer, "id" );
		Conversations[i].localid = js_get_int( peer, "local_id" );

		Conversations[i].name = construct_string(128);
		stringset( Conversations[i].name, "%s", js_get_str( settings, "title" ) );
	}
}

static void CT_get_conversators ( json_t * json )
{
	Conversators_count = json_array_size(json);
	Conversators = malloc( sizeof(conversator) * Conversators_count );

	json_t * el;
	for ( size_t i = 0; i < Conversators_count; ++i )
	{
		el = json_array_get( json, i );
		Conversators[i].id = js_get_int( el, "id" );

		Conversators[i].fname = construct_string(128);
		stringset( Conversators[i].fname, "%s", js_get_str( el, "first_name" ) );
		Conversators[i].lname = construct_string(128);
		stringset( Conversators[i].lname, "%s", js_get_str( el, "last_name" ) );
	}
}

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
				OS_readable_date( epoch, logfile );

			fprintf( logfile, "COMMENT %lld: TEXT: %s\n-~-~-~-~-~-~\n", c_id, js_get_str( el, "text" ) );

			/* Searching for attachments */
			CT_parse_attachments( acc, json_object_get( el, "attachments" ), logfile, post_id, c_id );
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
	if ( input_json == NULL )
		return;

	size_t att_index;
	json_t * att_elem;
	const char data_type[][6] = { "photo", "link", "doc", "video" };

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
CT_get_stars ( account * acc )
{
	string * apimeth = construct_string(128);
	FILE * starsfp;

	stringset( acc->currentdir, "%s/%s", acc->directory->s, DIRNAME_STARS );
	OS_new_directory(acc->currentdir->s);

	string * starfilepath = construct_string(2048);
	stringset( starfilepath, "%s/%s", acc->directory->s, FILENAME_STARS );
	starsfp = fopen( starfilepath->s, "w" );
	free_string(starfilepath);

	long long offset = 0;
	long long posts_count = 0;
	do
	{
		stringset( apimeth, "messages.getImportantMessages?count=%d&offset=%lld&extended=1", LIMIT_M, offset );
		int err_ret = 0;
		json_t * json = RQ_request( apimeth, &err_ret );
		if ( err_ret < 0 )
			goto CT_get_stars_cleanup;

		CT_get_conversators(json_object_get( json, "profiles"));
		CT_get_conversations(json_object_get( json, "conversations" ));

		json_t * js_messages_container = json_object_get( json, "messages" );
		json_t * js_messages = json_object_get( js_messages_container, "items" );
		size_t messages_num = json_array_size(js_messages);

		if ( offset == 0 )
		{
			posts_count = js_get_int( js_messages_container, "count" );
			printf( "Starred posts: %lld.\n", posts_count );
		}

		printf( "Iteration %lld-%lld, people: %zu, dialogs: %zu, messages: %zu\n", offset, offset + LIMIT_M, Conversators_count, Conversations_count, messages_num );
		for ( size_t i = 0; i < messages_num; ++i )
		{
			json_t * el = json_array_get( js_messages, i );
			CT_single_star( acc, el, starsfp );
//			long long id = js_get_int( el, "id" );
//
//			fprintf( starsfp, "ID: %lld\n", id );
//
//			conversator * author = CT_find_conversator( js_get_int( el, "from_id" ), people, conversators_count );
//			fprintf( starsfp, "FROM: %s %s (%lld)\n", author->fname->s, author->lname->s, author->id );
//
//			conversation * conv = CT_find_conversation( js_get_int( el, "peer_id" ), conversations, conversations_count );
//			fprintf( starsfp, "IN: %s (%lld)\n", conv->name->s, conv->id );
//
//			long long epoch = js_get_int( el, "date" );
//			if ( CONVERT_TO_READABLE_DATE )
//				OS_readable_date( epoch, starsfp );
//
//			fprintf( starsfp, "EPOCH: %lld\n", epoch );
//			fprintf( starsfp, "TEXT:\n%s\n", js_get_str( el, "text" ) );
//
//			CT_parse_attachments( acc, json_object_get( el, "attachments" ), starsfp, id, -1 );
//
//			fprintf( starsfp, "%s", LOG_POSTS_DIVIDER );
		}

		// Finishing iteration
		for ( size_t i = 0; i < Conversators_count; ++i )
			free_conversator(&Conversators[i]);
		free(Conversators);

		for ( size_t i = 0; i < Conversations_count; ++i )
			free_conversation(&Conversations[i]);
		free(Conversations);

		json_decref(json);

		offset += LIMIT_M;
	}
	while( posts_count - offset > 0 );

	CT_get_stars_cleanup:
	free(apimeth);
	fclose(starsfp);
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
				OS_readable_date( epoch, wallfp );

			fprintf( wallfp, "EPOCH: %lld\nTEXT: %s\n", epoch, js_get_str( el, "text" ) );

			/* Searching for attachments */
			CT_parse_attachments( acc, json_object_get( el, "attachments" ), wallfp, p_id, -1 );

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

					CT_parse_attachments( acc, json_object_get( rep_elem, "attachments" ), wallfp, p_id, -1 );
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
