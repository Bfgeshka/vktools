/* Macros */
#include "content_processing.h"
#include "stringutils.h"
#include <stdio.h>

#define DEFAULT_CONTENT_DOCS 1
#define DEFAULT_CONTENT_PICS 1
#define DEFAULT_CONTENT_VIDS 0
#define DEFAULT_CONTENT_COMS 1

/* Global scope */
void
CT_default ( void )
{
	content.comments = DEFAULT_CONTENT_COMS;
	content.documents = DEFAULT_CONTENT_DOCS;
	content.pictures = DEFAULT_CONTENT_PICS;
	content.videos = DEFAULT_CONTENT_VIDS;
}

void
CT_print_types ( void )
{
	string * str = construct_string(1024);

	stringset( str, "Will try to get:" );

	if ( content.comments )
		stringcat( str, " comments" );

	if ( content.documents )
		stringcat( str, " documents" );

	if ( content.pictures )
		stringcat( str, " pictures" );

	if ( content.videos )
		stringcat( str, " videos" );

	stringcat( str, ".\n");

	puts(str->s);
	free_string(str);
}
