/* Macros */
#include <stdio.h>
#include "account.h"
#include "json.h"
#include "request_gen.h"
#include "os.h"

#define FILENAME_IDNAME "description.txt"

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

	int err_ret = 0;
	string * apimeth = construct_string(256);
	stringset( apimeth, "users.get?user_ids=%s", str );
	json_t * json = RQ_request( apimeth, &err_ret );
	if ( err_ret < 0 )
		return;

	json_t * el = json_array_get( json, 0 );

	/* filling struct */
	account.type = e_user;
	account.id = js_get_int( el, "id" );
	stringset( account.screenname, "%s", str );
	stringset( account.usr_fname, "%s", js_get_str( el, "first_name" ) );
	stringset( account.usr_lname, "%s", js_get_str( el, "last_name" ) );

	json_decref(el);
}

void
AC_get_group ( char * str )
{
	if ( account.type != e_null )
		return;

	int err_ret = 0;
	string * apimeth = construct_string(256);
	stringset( apimeth, "groups.getById?group_id=%s", str );
	json_t * json = RQ_request( apimeth, &err_ret );
	if ( err_ret < 0 )
		return;

	json_t * el = json_array_get( json, 0 );

	/* filling struct */
	account.type = e_group;
	account.id = - js_get_int( el, "id" );
	stringset( account.screenname, "%s", str );
	stringset( account.grp_name, "%s", js_get_str( el, "name" ) );
	stringset( account.grp_type, "%s", js_get_str( el, "type" ) );

	json_decref(el);
}

void
AC_make_dir ( void )
{
	/* Naming file metadata */
	string * name_descript = construct_string(2048);
	string * dirname = construct_string(2048);

	switch ( account.type )
	{
		case e_group:
		{
			stringset( dirname, "c_%lld", account.id );
			stringset( name_descript, "%lld: %s: %s\n",
			    account.id,
			    account.screenname->s,
			    account.grp_name->s );

			break;
		}

		case e_user:
		{
			stringset( dirname, "u_%lld", account.id );
			stringset( name_descript, "%lld: %s: %s %s\n",
			    account.id,
			    account.screenname->s,
			    account.usr_fname->s,
			    account.usr_lname->s );

			break;
		}

		default:
		{
			fprintf( stderr, "Screenname is invalid.\n" );
			exit(EXIT_FAILURE);
		}
	}

	new_directory(dirname->s);

	string * name_dsc_path = construct_string(BUFSIZ);
	stringset( name_dsc_path, "%s/%s", dirname->s, FILENAME_IDNAME );

	FILE * u_name = fopen( name_dsc_path->s, "w" );
	fprintf( u_name, "%s", name_descript->s );
	fclose(u_name);

	free_string(name_dsc_path);
	free_string(name_descript);

	account.directory = dirname;
}
