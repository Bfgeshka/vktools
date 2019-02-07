/* Macros */
#include "content.h"
#include "content_messages.h"
#include "json.h"
#include "request_gen.h"
#include "os.h"
#include <stdio.h>
#include <unistd.h>

#define FILENAME_STARS "stars.txt"
#define DIRNAME_STARS "alb_stars"
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

static void S_CT_get_conversators ( json_t * json );
static void S_CT_get_conversations ( json_t * json );
static void S_CT_free_conversators ( void );
static void S_CT_free_conversations ( void );
static conversator * S_CT_find_conversator ( long long id );
static conversation * S_CT_find_conversation ( long long id );
static void S_CT_single_star ( account * acc, json_t * el, FILE * log, int nested );
static void S_CT_remove_star ( long long id );

static void
S_CT_remove_star ( long long id )
{
	string * apimeth = construct_string(256);
	stringset( apimeth, "messages.markAsImportant?important=0&message_ids=%lld", id );

	int err_ret = 0;
	json_t * json = RQ_request( apimeth, &err_ret );
	free_string(apimeth);
	if ( err_ret < 0 )
		return;

	json_decref(json);

	printf( "Message with id=%lld was marked as not important.\n", id );
}

static void
S_CT_single_star ( account * acc, json_t * el, FILE * log, int nested )
{
	long long id = js_get_int( el, "id" );

	fprintf( log, "ID: %lld\n", id );

	conversator * author = S_CT_find_conversator( js_get_int(el, "from_id" ));
	if ( author != NULL )
		fprintf( log, "FROM: %s %s (%lld)\n", author->fname->s, author->lname->s, author->id );
	else
		fputs( "FROM: unknown source\n", log );

	conversation * conv = S_CT_find_conversation( js_get_int(el, "peer_id" ));
	if ( conv != NULL )
		fprintf( log, "IN: %s (%lld)\n", conv->name->s, conv->id );
	else
		fputs( "IN: unknown source\n", log );

	long long epoch = js_get_int( el, "date" );
	if ( CONVERT_TO_READABLE_DATE )
		OS_readable_date( epoch, log );

	fprintf( log, "EPOCH: %lld\n", epoch );
	fprintf( log, "TEXT:\n%s\n", js_get_str( el, "text" ) );

	json_t * reposted = json_object_get( el, "fwd_messages" );
	if ( reposted != NULL && json_array_size(reposted) > 0 )
	{
		for ( size_t i = 0; i < json_array_size(reposted); ++i )
		{
			fputs( "Reposted, original post below:\n", log );
			json_t * subelement = json_array_get( reposted, i );
			S_CT_single_star( acc, subelement, log, 1 );
		}
	}

	CT_parse_attachments( acc, json_object_get( el, "attachments" ), log, id, -1 );

	if ( nested == 0 && content.clear_stars )
		S_CT_remove_star(id);

	fprintf( log, "END OF ID: %lld\n", id );
	fprintf( log, "%s", LOG_POSTS_DIVIDER );
}

static conversation *
S_CT_find_conversation ( long long id )
{
	for ( size_t i = 0; i < Conversations_count; ++i )
		if ( id == Conversations[i].id )
			return &Conversations[i];

	return NULL;
}

static conversator *
S_CT_find_conversator ( long long id )
{
	for ( size_t i = 0; i < Conversators_count; ++i )
		if ( id == Conversators[i].id )
			return &Conversators[i];

	return NULL;
}

static void
S_CT_free_conversations ( void )
{
	for ( size_t i = 0; i < Conversations_count; ++i )
		free_string(Conversations[i].name);

	free(Conversations);
}

static void
S_CT_free_conversators ( void )
{
	for ( size_t i = 0; i < Conversators_count; ++i )
	{
		free_string(Conversators[i].fname);
		free_string(Conversators[i].lname);
	}

	free(Conversators);
}

static void
S_CT_get_conversations ( json_t * json )
{
	Conversations_count = json_array_size(json);
	Conversations = malloc( sizeof(conversation) * Conversations_count );

	for ( size_t i = 0; i < Conversations_count; ++i )
	{
		json_t * el = json_array_get( json, i );
		json_t * peer = json_object_get( el, "peer" );
		json_t * settings = json_object_get( el, "chat_settings" );

		Conversations[i].id = js_get_int( peer, "id" );
		Conversations[i].localid = js_get_int( peer, "local_id" );

		Conversations[i].name = construct_string(128);
		stringset( Conversations[i].name, "%s", js_get_str( settings, "title" ) );
	}
}

static void
S_CT_get_conversators ( json_t * json )
{
	Conversators_count = json_array_size(json);
	Conversators = malloc( sizeof(conversator) * Conversators_count );

	for ( size_t i = 0; i < Conversators_count; ++i )
	{
		json_t * el = json_array_get( json, i );
		Conversators[i].id = js_get_int( el, "id" );

		Conversators[i].fname = construct_string(128);
		stringset( Conversators[i].fname, "%s", js_get_str( el, "first_name" ) );
		Conversators[i].lname = construct_string(128);
		stringset( Conversators[i].lname, "%s", js_get_str( el, "last_name" ) );
	}
}

/* Global scope */
void
CT_get_conversations_history ( account * acc )
{
	(void)acc;

	string * apimeth = construct_string(128);

	long long offset = 0;
	long long posts_count = 0;
	do
	{
		stringset( apimeth, "messages.getConversations?count=%d&offset=%lld&extended=1", LIMIT_M, offset );
		int err_ret = 0;
		json_t * json = RQ_request( apimeth, &err_ret );
		if ( err_ret < 0 )
			goto CT_get_conversations_history_cleanup;

		if ( offset == 0 )
		{
			posts_count = js_get_int( json, "count" );
			printf( "Conversations: %lld.\n", posts_count );
		}

		offset += LIMIT_M;
		json_decref(json);
	}
	while( posts_count - offset > 0 );

	CT_get_conversations_history_cleanup:
	free_string(apimeth);
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

		S_CT_get_conversators(json_object_get( json, "profiles"));
		S_CT_get_conversations(json_object_get( json, "conversations" ));

		json_t * js_messages_container = json_object_get( json, "messages" );
		json_t * js_messages = json_object_get( js_messages_container, "items" );
		size_t messages_num = json_array_size(js_messages);

		if ( offset == 0 )
		{
			posts_count = js_get_int( js_messages_container, "count" );
			printf( "Starred posts: %lld.\n", posts_count );
		}

		printf( "Iteration %lld-%lld, people: %zu, dialogs: %zu, messages: %zu\n", offset, offset + LIMIT_M, Conversators_count, Conversations_count, messages_num );

		sleep(3);

		for ( size_t i = 0; i < messages_num; ++i )
			S_CT_single_star( acc, json_array_get( js_messages, i ), starsfp, 0 );

		// Finishing iteration
		S_CT_free_conversations();
		S_CT_free_conversators();

		json_decref(json);

		offset += LIMIT_M;
	}
	while( posts_count - offset > 0 );

	CT_get_stars_cleanup:
	free_string(apimeth);
	fclose(starsfp);
}
