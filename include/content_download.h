#ifndef CONTENT_DOWNLOADING_H_
#define CONTENT_DOWNLOADING_H_

/* Macros */
#include "account.h"
#include "json.h"
#include <stdio.h>

/* Protos */
void DL_doc ( account * acc, json_t * el, FILE * log, long long post_id, long long comm_id );
void DL_photo ( account * acc, json_t * el, FILE * log, long long post_id, long long comm_id );

#endif
