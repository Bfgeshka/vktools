/* Macros */
#include "content.h"
#include "content_messages.h"
#include "json.h"
#include "request_gen.h"
#include "os.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define FILENAME_STARS "stars.txt"
#define FILENAME_CONV_INDEX "conversations.txt"
#define FILENAME_CONV_HISTORY "log.txt"
#define DIRNAME_STARS "alb_stars"
// Limitation for number of messages per request. Current is 200
#define LIMIT_M 200

/* Typedef */
typedef enum conversation_type
{
	e_ct_null = 0,
	e_ct_chat,
	e_ct_user,
	e_ct_group
} conversation_type;

typedef struct conversation_user
{
	enum conversation_type type;
	long long id;
	string * name;
} conversator;

typedef struct conversation_data
{
	enum conversation_type type;
	long long id;
	long long localid;
	string * name;
} conversation;

/* Local scope */
static conversator * Conversators = NULL;
static size_t Conversators_count = 0;
static conversation * Conversations = NULL;
static size_t Conversations_count = 0;

static void S_CT_get_conversators ( json_t * profiles, json_t * groups );
static void S_CT_get_conversations ( json_t * json );
static void S_CT_free_conversators ( void );
static void S_CT_free_conversations ( void );
static conversator * S_CT_find_conversator ( long long id );
static conversation * S_CT_find_conversation ( long long id );
static void S_CT_single_star ( account * acc, json_t * el, FILE * log, int nested );
static void S_CT_remove_star ( long long id );
static void S_CT_single_conversation ( account * acc, conversation * conv, FILE * log );

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
		fprintf( log, "FROM: %s (%lld)\n", author->name->s, author->id );
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
		free_string(Conversators[i].name);

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
S_CT_get_conversators ( json_t * profiles, json_t * groups )
{
	size_t profiles_count = 0;
	size_t groups_count = 0;
	size_t iter = 0;

	if ( profiles != NULL )
		profiles_count = json_array_size(profiles);

	if ( groups != NULL )
		groups_count = json_array_size(groups);

//	printf( "pfs: %zu, grps: %zu\n", profiles_count, groups_count );

	Conversators_count = profiles_count + groups_count;
	Conversators = malloc( sizeof(conversator) * Conversators_count );

	if ( profiles != NULL )
	{
		for ( size_t pi = 0; pi < profiles_count; ++pi )
		{
			Conversators[iter].type = e_ct_user;

			json_t * el = json_array_get( profiles, pi );
			Conversators[iter].id = js_get_int( el, "id" );

			Conversators[iter].name = construct_string(128);
			stringset( Conversators[iter].name, "%s %s", js_get_str( el, "first_name" ), js_get_str( el, "last_name" ) );

			iter++;
		}
	}

	if ( groups != NULL )
	{
		for ( size_t gi = 0; gi < groups_count; ++gi )
		{
			Conversators[iter].type = e_ct_group;

			json_t * el = json_array_get( groups, gi );
			Conversators[iter].id = js_get_int( el, "id" );

			Conversators[iter].name = construct_string(128);
			stringset( Conversators[iter].name, "%s", js_get_str( el, "name" ) );

			iter++;
		}
	}
}

static void
S_CT_single_conversation ( account * acc, conversation * conv, FILE * log )
{
	stringset( acc->currentdir, "%s/conv_%lld", acc->directory->s, conv->id );
	OS_new_directory(acc->currentdir->s);

	string * historypath = construct_string(2048);
	stringset( historypath, "%s/%s", acc->currentdir->s, FILENAME_CONV_HISTORY );
	FILE * convlog = fopen( historypath->s, "w" );
	free_string(historypath);

	string * apimeth = construct_string(128);
	conv->name = construct_string(256);

	long long offset = 0;
	long long posts_count = 0;
	do
	{
		stringset( apimeth, "messages.getHistory?count=%d&offset=%lld&extended=1&peer_id=%lld", LIMIT_M, offset, conv->id );
		int err_ret = 0;
		json_t * json = RQ_request( apimeth, &err_ret );
		if ( err_ret < 0 )
			goto S_CT_single_conversation_cleanup;

		json_t * convjs_arr = json_object_get( json, "conversations" );
		S_CT_get_conversators( json_object_get( json, "profiles" ), json_object_get( json, "groups" ) );
//		json_t * profiles = json_object_get( json, "profiles" );
//		json_t * groups = json_object_get( json, "groups" );
		json_t * items = json_object_get( json, "items" );

		if ( offset == 0 )
		{
			posts_count = js_get_int( json, "count" );
			printf( "Messages in conversation: %lld.\n", posts_count );

			if ( conv->type == e_ct_null )
			{
				fprintf( stderr, "Error: null conversation.");
				return;
			}

			if ( conv->type == e_ct_chat )
			{
				json_t * conver_meta = json_array_get( convjs_arr, 0 );
				json_t * chat_settings = json_object_get( conver_meta, "chat_settings" );
				stringset( conv->name, "%s", js_get_str( chat_settings, "title" ) );
			}

			if ( conv->type == e_ct_user )
			{
				conversator * cvr = S_CT_find_conversator(conv->id);
				stringset( conv->name, "%s", cvr->name );
//				size_t profsize = json_array_size(profiles);
//				for ( size_t i = 0; i < profsize; ++i )
//				{
//					json_t * el = json_array_get( profiles, i );
//					if ( conv->id == js_get_int( el, "id" ) )
//						stringset( conv->name, "%s %s", js_get_str( el, "first_name" ), js_get_str( el, "last_name" ) );
//				}
			}

			if ( conv->type == e_ct_group )
			{
				conversator * cvr = S_CT_find_conversator(conv->localid);
				stringset( conv->name, "%s", cvr->name );
//				size_t groupsize = json_array_size(groups);
//				for ( size_t i = 0; i < groupsize; ++i )
//				{
//					json_t * el = json_array_get( groups, i );
//					if ( conv->localid == js_get_int( el, "id" ) )
//						stringset( conv->name, "%s", js_get_str( el, "name" ) );
//				}
			}

			printf( "Conversation with %s, id: %lld, localid: %lld, type: %d\n", conv->name->s, conv->id, conv->localid, conv->type );
			fprintf( log, "%lld: %s; count: %lld\n", conv->id, conv->name->s, posts_count );

			if ( posts_count == 0 )
			{
				json_decref(json);
				goto S_CT_single_conversation_cleanup;
			}
		}

		size_t items_size = json_array_size(items);
		for ( size_t i = 0; i < items_size; ++i )
		{
			json_t * el = json_array_get( items, i );

			long long epoch = js_get_int( el, "date" );
			long long from = js_get_int( el, "from_id" );
			long long mess_id = js_get_int( el, "id" );

			fprintf( convlog, "ID: %lld\n", mess_id );

			if ( CONVERT_TO_READABLE_DATE )
				OS_readable_date( epoch, convlog );

			fprintf( convlog, "EPOCH: %lld\n", epoch );
			conversator * cvr = S_CT_find_conversator(from);
			fprintf( convlog, "FROM: %s (%lld)\n", cvr->name->s, from );

			fprintf( convlog, "TEXT:\n%s\n", js_get_str( el, "text" ) );

			CT_parse_attachments( acc, json_object_get( el, "attachments" ), convlog, mess_id, -1 );

			json_t * reposted = json_object_get( el, "fwd_messages" );
			if ( reposted != NULL && json_array_size(reposted) > 0 )
			{
				for ( size_t j = 0; j < json_array_size(reposted); ++j )
				{
					fputs( "Quotation, original post below:\n", convlog );
					json_t * subelement = json_array_get( reposted, j );
					S_CT_single_star( acc, subelement, convlog, 1 );
				}
			}

			fprintf( convlog, "END OF ID: %lld\n", mess_id );
			fprintf( convlog, "%s", LOG_POSTS_DIVIDER );
		}

		offset += LIMIT_M;
		json_decref(json);
	}
	while( posts_count - offset > 0 );

	S_CT_single_conversation_cleanup:
	free_string(apimeth);
	fclose(convlog);
	free_string(conv->name);
	free(conv);
}

/* Global scope */
void
CT_get_conversations_history ( account * acc )
{
	string * apimeth = construct_string(128);

	string * indexpath = construct_string(2048);
	stringset( indexpath, "%s/%s", acc->directory->s, FILENAME_CONV_INDEX );
	FILE * convindex = fopen( indexpath->s, "w" );
	free_string(indexpath);

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

		json_t * items = json_object_get( json, "items" );
		size_t size = json_array_size(items);
		for ( size_t i = 0; i < size; ++i )
		{
			json_t * el_container = json_array_get( items, i );
			json_t * el = json_object_get( el_container, "conversation" );
			json_t * peer = json_object_get( el, "peer");
			conversation * conv = malloc(sizeof(conversation));
			conv->id = js_get_int( peer, "id" );
			conv->localid = js_get_int( peer, "local_id" );

			conv->type = e_ct_null;
			char * type = js_get_str( peer, "type" );
			if ( strncmp( type, "ch", 2 ) == 0 )
				conv->type = e_ct_chat;
			if ( strncmp( type, "us", 2 ) == 0 )
				conv->type = e_ct_user;
			if ( strncmp( type, "gr", 2 ) == 0 )
				conv->type = e_ct_group;

			S_CT_single_conversation( acc, conv, convindex );
		}

		offset += LIMIT_M;
		json_decref(json);
	}
	while( posts_count - offset > 0 );

	CT_get_conversations_history_cleanup:
	free_string(apimeth);
	fclose(convindex);
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

		S_CT_get_conversators( json_object_get( json, "profiles" ), NULL );
		S_CT_get_conversations(json_object_get( json, "conversations" ));

		json_t * js_messages_container = json_object_get( json, "messages" );
		json_t * js_messages = json_object_get( js_messages_container, "items" );
		size_t messages_num = json_array_size(js_messages);

		if ( offset == 0 )
		{
			posts_count = js_get_int( js_messages_container, "count" );
			printf( "Starred posts: %lld.\n", posts_count );

			if ( posts_count == 0 )
			{
				json_decref(json);
				S_CT_free_conversations();
				S_CT_free_conversators();
				goto CT_get_stars_cleanup;
			}
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
