#ifndef REQUEST_GEN_H_
#define REQUEST_GEN_H_

/* Macros */
#include "json.h"
#include "stringutils.h"

/* Protos */
void R_set_token ( void );
json_t * R_request ( string * api_method, int * out_json );

#endif
