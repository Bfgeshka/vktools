/* Macro */
#include <stdio.h>
#include "json.h"
#include "stringutils.h"
#include "curlutils.h"

#define REQ_HEAD "https://api.vk.com/method"
#define API_VER "5.62"

/* Proto */

/* Local scope */
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

int
R_request ( string * api_method, json_t * out_json )
{
	string * url = construct_string(2048);
	stringset( url, "%s/%s%s&v=%s", REQ_HEAD, api_method, TOKEN.s, API_VER );

	json_error_t * json_err = NULL;
	out_json = make_request( url, json_err );
	free_string(url);
	if ( !out_json )
	{
		if ( json_err )
			fprintf( stderr, "%s parsing error.\n%d:%s\n", api_method->s, json_err->line, json_err->text );

		json_decref(out_json);
		return -1;
	}

	return 0;
}
