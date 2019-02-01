#ifndef REQUEST_GEN_H_
#define REQUEST_GEN_H_

/* Macros */
#include "json.h"
#include "stringutils.h"

/* Protos */
void RQ_set_token ( void );
json_t * RQ_request ( string * api_method, int * out_json );

#endif
