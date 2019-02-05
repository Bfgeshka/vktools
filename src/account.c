/* Macros */
#include <stdio.h>
#include "account.h"
#include "json.h"
#include "request_gen.h"
#include "os.h"

#define FILENAME_IDNAME "description.txt"

/* Global scope */
account *
AC_init ( void )
{
	account * acc = malloc(sizeof(account));
	acc->type = e_null;
	acc->screenname = construct_string(128);
	acc->usr_fname = construct_string(128);
	acc->usr_lname = construct_string(128);
	acc->grp_name = construct_string(128);
	acc->grp_type = construct_string(64);
	acc->currentdir = construct_string(2048);

	return acc;
}

void
AC_free ( account * acc )
{
	free_string(acc->screenname);
	free_string(acc->usr_fname);
	free_string(acc->usr_lname);
	free_string(acc->grp_name);
	free_string(acc->grp_type);
	free_string(acc->directory);
	free_string(acc->currentdir);

	if (acc->albums != NULL)
	{
		for ( size_t i = 0; i < acc->albums_count; ++i )
			free_string(acc->albums[i].title);

		free(acc->albums);
	}

	free(acc);
}

void
AC_info ( account * acc )
{
	switch ( acc->type )
	{
		case e_group:
		{
			printf( "Group: %s (%s).\nGroup ID: %lld.\nType: %s.\n\n",
			    acc->grp_name->s,
			    acc->screenname->s,
			    acc->id,
			    acc->grp_type->s );

			return;
		}

		case e_user:
		{
			printf( "User: %s %s (%s).\nUser ID: %lld.\n\n",
			    acc->usr_fname->s,
			    acc->usr_lname->s,
			    acc->screenname->s,
			    acc->id );

			return;
		}

		default:
		{
			puts("No such account.");

			exit(EXIT_FAILURE);
		}
	}
}

account *
AC_get_user ( char * str )
{
	int err_ret = 0;
	string * apimeth = construct_string(256);
	stringset( apimeth, "users.get?user_ids=%s", str );
	json_t * json = RQ_request( apimeth, &err_ret );
	free_string(apimeth);
	if ( err_ret < 0 )
		return NULL;

	json_t * el = json_array_get( json, 0 );

	/* filling struct */
	account * acc = AC_init();

	acc->type = e_user;
	acc->id = js_get_int( el, "id" );
	stringset( acc->screenname, "%s", str );
	stringset( acc->usr_fname, "%s", js_get_str( el, "first_name" ) );
	stringset( acc->usr_lname, "%s", js_get_str( el, "last_name" ) );

	json_decref(json);

	return acc;
}

account *
AC_get_group ( char * str )
{
	int err_ret = 0;
	string * apimeth = construct_string(256);
	stringset( apimeth, "groups.getById?group_id=%s", str );
	json_t * json = RQ_request( apimeth, &err_ret );
	free_string(apimeth);
	if ( err_ret < 0 )
		return NULL;

	json_t * el = json_array_get( json, 0 );

	/* filling struct */
	account * acc = AC_init();

	acc->type = e_group;
	acc->id = - js_get_int( el, "id" );
	stringset( acc->screenname, "%s", str );
	stringset( acc->grp_name, "%s", js_get_str( el, "name" ) );
	stringset( acc->grp_type, "%s", js_get_str( el, "type" ) );

	json_decref(json);

	return acc;
}

void
AC_make_dir ( account * acc )
{
	/* Naming file metadata */
	string * name_descript = construct_string(2048);
	string * dirname = construct_string(2048);

	switch ( acc->type )
	{
		case e_group:
		{
			stringset( dirname, "c_%lld", acc->id );
			stringset( name_descript, "%lld: %s: %s\n",
			    acc->id,
			    acc->screenname->s,
			    acc->grp_name->s );

			break;
		}

		case e_user:
		{
			stringset( dirname, "u_%lld", acc->id );
			stringset( name_descript, "%lld: %s: %s %s\n",
			    acc->id,
			    acc->screenname->s,
			    acc->usr_fname->s,
			    acc->usr_lname->s );

			break;
		}

		default:
		{
			fprintf( stderr, "Screenname is invalid.\n" );
			exit(EXIT_FAILURE);
		}
	}

	OS_new_directory(dirname->s);

	string * name_dsc_path = construct_string(BUFSIZ);
	stringset( name_dsc_path, "%s/%s", dirname->s, FILENAME_IDNAME );

	FILE * u_name = fopen( name_dsc_path->s, "w" );
	fprintf( u_name, "%s", name_descript->s );
	fclose(u_name);

	free_string(name_dsc_path);
	free_string(name_descript);

	acc->directory = dirname;
}
