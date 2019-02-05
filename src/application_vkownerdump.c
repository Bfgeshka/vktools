/* Macros */
#include <stdio.h>
#include "application.h"
#include "account.h"
#include "content_processing.h"
#include "../config.h"

/* Global scope */
account * Account = NULL;

void
A_help ( void )
{
	puts( "Usage: vkownerdump [OPTIONS] <USER>" );
	puts( "" );
	puts( "Options:" );
	puts( "  -T                   generate link for getting a token" );
	puts( "  -t TOKEN             give a valid token without header \"&access_token=\". If TOKEN is zero then anonymous access given" );
	puts( "  -u USER              ignore group with same screenname" );
	puts( "  -g GROUP             ignore user with same screenname" );

	exit(EXIT_SUCCESS);
}

void
A_args ( int argc, char ** argv )
{
	extern string TOKEN;

	CT_default();

	if ( argc == 1 )
		goto get_id_print_help;

	if ( argc == 2 )
	{
		if ( argv[1][0] == '-' )
			switch( argv[1][1] )
			{
				case 'h':
					goto get_id_print_help;
				case 'T':
					goto get_id_token_request;
				default:
					goto get_id_invalid_arg;
			}

		Account = AC_get_user(argv[1]);

		goto get_id_after_switch;
	}

	for ( int t = 0; t < argc; ++t )
	{
		if ( argv[t][0] == '-' )
			switch( argv[t][1] )
			{
				case 'u':
				{
					Account = AC_get_user(argv[t+1]);
					break;
				}

				case 't':
				{
					if ( argv[t+1] != NULL )
					{
						if ( atoi(argv[t+1]) != 0 )
							stringset( &TOKEN, "%s%s", TOKEN_HEAD, argv[t+1] );
						else
							stringset( &TOKEN, "%c", '\0' );
					}
					else
						goto get_id_invalid_arg;

					break;
				}

				default:
					goto get_id_invalid_arg;
			}

		if ( ( t == argc - 1 ) && ( Account == NULL || Account->type == e_null ) )
		{
			Account = AC_get_user(argv[t]);
		}
	}

	get_id_after_switch:


	/* Info out */
	AC_info(Account);
	CT_print_types();
	return;

	/* Print halp and exit */
	get_id_print_help:
	A_help();
	exit(EXIT_SUCCESS);

	/* Message about invalid argument and exit */
	get_id_invalid_arg:
	puts("Invalid argument.");
	exit(EXIT_FAILURE);

	/* Prink token request link and exit */
	get_id_token_request:
	printf( "https://oauth.vk.com/authorize?client_id=%d&scope=%s&display=page&response_type=token\n", APPLICATION_ID, PERMISSIONS );
	exit(EXIT_SUCCESS);
}
