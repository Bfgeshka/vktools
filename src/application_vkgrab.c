/* Macros */
#include <stdio.h>
#include "application.h"
#include "account.h"
#include "content_processing.h"
#include "../config.h"

/* Global scope */
void
A_help ( void )
{
	puts( "Usage: vkgrab [OPTIONS] <USER|GROUP>" );
	puts( "" );
	puts( "Options:" );
	puts( "  -T                   generate link for getting a token" );
	puts( "  -t TOKEN             give a valid token without header \"&access_token=\". If TOKEN is zero then anonymous access given" );
	puts( "  -u USER              ignore group with same screenname" );
	puts( "  -g GROUP             ignore user with same screenname" );
	puts( "  -yv, -yd, -yp        allows downloading of video, documents or pictures" );
	puts( "  -nv, -nd, -np        forbids downloading of video, documents or pictures\n" );
	puts( "Notice: if both USER and GROUP do exist, group id proceeds" );

	exit(EXIT_SUCCESS);
}

void
A_args ( int argc, char ** argv )
{
	extern string TOKEN;

	CT_default();

	switch ( argc )
	{
		case 1:
			goto get_id_print_help;

		case 2:
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
			else
			{
				AC_get_user(argv[1]);
				AC_get_group(argv[1]);
			}

			break;
		}

		default:
		{
			for ( int t = 0; t < argc; ++t )
			{
				if ( argv[t][0] == '-' )
					switch( argv[t][1] )
					{
						case 'u':
						{
							AC_get_user(argv[t+1]);
							break;
						}

						case 'g':
						{
							AC_get_group(argv[t+1]);
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

						case 'n':
						case 'y':
						{
							int value = ( argv[t][1] == 'n' ) ? 0 : 1;
							switch( argv[t][2] )
							{
								case 'p':
								{
									content.pictures = value;
									break;
								}

								case 'd':
								{
									content.documents = value;
									break;
								}

								case 'v':
								{
									content.videos = value;
									break;
								}

								case 'c':
								{
									content.comments = value;
									break;
								}

								default:
									goto get_id_print_help;
							}

							break;
						}

						default:
							goto get_id_invalid_arg;
					}
				if ( ( t == argc - 1 ) && account.type == e_null )
				{
					AC_get_user(argv[t]);
					AC_get_group(argv[t]);
				}
			}

			break;
		}
	}

	/* Info out */
	AC_info();
	CT_print_types();
	return;

	/* Print halp and exit */
	get_id_print_help:
	A_help();
	return;

	/* Message about invalid argument and exit */
	get_id_invalid_arg:
	puts("Invalid argument.");
	return;

	/* Prink token request link and exit */
	get_id_token_request:
	printf( "https://oauth.vk.com/authorize?client_id=%d&scope=%s&display=page&response_type=token\n", APPLICATION_ID, PERMISSIONS );
	return;
}
