/* Macros */
#include <stdio.h>
#include "application.h"

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
}

void
A_args ( int argc, char ** argv )
{
	;
}
