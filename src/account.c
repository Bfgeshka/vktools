/* Macros */
#include <stdio.h>
#include "account.h"
#include "json.h"
#include "request_gen.h"

/* Global scope */
void
AC_init ( void )
{
	account.type = e_null;
	account.screenname = construct_string(128);
	account.usr_fname = construct_string(128);
	account.usr_lname = construct_string(128);
	account.grp_name = construct_string(128);
	account.grp_type = construct_string(64);

}

void
AC_free ( void )
{
	free_string(account.screenname);
	free_string(account.usr_fname);
	free_string(account.usr_lname);
	free_string(account.grp_name);
	free_string(account.grp_type);
}

void
AC_info ( void )
{
	switch ( account.type )
	{
		case e_group:
		{
			printf( "Group: %s (%s).\nGroup ID: %lld.\nType: %s.\n\n",
			    account.grp_name->s,
			    account.screenname->s,
			    account.id,
			    account.grp_type->s );

			return;
		}

		case e_user:
		{
			printf( "User: %s %s (%s).\nUser ID: %lld.\n\n",
			    account.usr_fname->s,
			    account.usr_lname->s,
			    account.screenname->s,
			    account.id );

			return;
		}

		default:
		{
			puts("No such account.");

			return;
		}
	}
}

void
AC_get_user ( char * str )
{
	if ( account.type != e_null )
		return;

	json_t * json = NULL;
	string * apimeth = construct_string(256);
	stringset( apimeth, "users.get?user_ids=%s", str );
	if ( !R_request( apimeth, json ) )
		return;

	json_t * el;
	el = json_array_get( json, 0 );
	json_decref(json);

	/* filling struct */
	account.type = e_user;
	account.id = js_get_int( el, "id" );
	stringset( account.screenname, "%s", str );
	stringset( account.usr_fname, "%s", js_get_str( el, "first_name" ) );
	stringset( account.usr_lname, "%s", js_get_str( el, "last_name" ) );
}

void
AC_get_group ( char * str )
{
	if ( account.type != e_null )
		return;

	json_t * json = NULL;
	string * apimeth = construct_string(256);
	stringset( apimeth, "groups.getById?group_id=%s", str );
	if ( !R_request( apimeth, json ) )
		return;

	json_t * el;
	el = json_array_get( json, 0 );
	json_decref(json);

	/* filling struct */
	account.type = e_group;
	account.id = - js_get_int( el, "id" );
	stringset( account.screenname, "%s", str );
	stringset( account.grp_name, "%s", js_get_str( el, "name" ) );
	stringset( account.grp_type, "%s", js_get_str( el, "type" ) );
}