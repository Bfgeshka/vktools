/* Macros */
#include <stdio.h>
#include "curlutils.h"
#include "request_gen.h"
#include "../config.h"

#define REQ_HEAD "https://api.vk.com/method"
#define API_VER "5.62"

/* Local scope */
static json_t * make_request ( string * url, json_error_t * json_err );

static json_t *
make_request ( string * url, json_error_t * json_err )
{
	struct curl_arg cf;
	C_get_request( url->s, &cf );

	json_t * json;
	json = json_loads( cf.payload, 0, json_err );
	if ( cf.payload != NULL )
	{
		free(cf.payload);
		return json;
	}
	else
		return NULL;
}

/* Global scope */
string TOKEN;

json_t *
RQ_request ( string * api_method, int * err )
{
	string * url = construct_string(2048);
	stringset( url, "%s/%s%s&v=%s", REQ_HEAD, api_method->s, TOKEN.s, API_VER );

	json_error_t * json_err = NULL;
	json_t * json = make_request( url, json_err );
	free_string(url);
	if ( !json )
	{
		if ( json_err )
			fprintf( stderr, "%s parsing error.\n%d:%s\n", api_method->s, json_err->line, json_err->text );

		* err = -1;
		return NULL;
	}

	/* simplifying json */
	json_t * out_json = json_object_get( json, "response" );

	if ( !out_json )
	{
		fprintf( stderr, "No valid response found on %s\n", api_method->s );
		* err = -2;
		return NULL;
	}

	return out_json;
}

void
RQ_set_token ( void )
{
	/* Token */
	newstring( &TOKEN, 256 );
	stringset( &TOKEN, "%s", TOKEN_HEAD );

	string * CONSTTOKEN = construct_string(256);
	stringset( CONSTTOKEN, "%s", CONST_TOKEN );

	if ( TOKEN.length != CONSTTOKEN->length )
		stringset( &TOKEN, "%s", CONSTTOKEN->s );

	free_string(CONSTTOKEN);
}
