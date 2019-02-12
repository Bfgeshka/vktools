#ifndef CONTENT_PROCESSING_H_
#define CONTENT_PROCESSING_H_

/* Macros */
#include "account.h"
#include "json.h"
#include <stdio.h>
#define CONVERT_TO_READABLE_DATE 1
#define LOG_POSTS_DIVIDER "-~-~-~-~-~-~\n~-~-~-~-~-~-\n\n"

/* Typedef */
struct content
{
	unsigned documents   : 1;
	unsigned pictures    : 1;
	unsigned videos      : 1;
	unsigned audio       : 1;
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
void CT_parse_attachments ( account *, json_t * input_json, FILE * logfile, long long post_id, long long comm_id );

#endif
