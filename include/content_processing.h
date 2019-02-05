#ifndef CONTENT_PROCESSING_H_
#define CONTENT_PROCESSING_H_

/* Macros */
#include "account.h"

/* Typedef */
struct content
{
	unsigned documents   : 1;
	unsigned pictures    : 1;
	unsigned videos      : 1;
	unsigned comments    : 1;
	unsigned clear_stars : 1;
} content;

/* Protos */
void CT_default ( void );
void CT_print_types ( void );
void CT_get_albums ( account * acc );
void CT_get_wall ( account * acc );
void CT_get_groups ( account * acc );
void CT_get_friends( account * acc );
void CT_get_docs ( account * acc );
void CT_get_albums_files ( account * acc );
void CT_get_stars ( account * acc );

#endif
