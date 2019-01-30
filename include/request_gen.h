#ifndef REQUEST_GEN_H_
#define REQUEST_GEN_H_

/* Macros */
#include "json.h"
#include "stringutils.h"

/* Protos */
void R_set_token ( void );
int R_request ( string * api_method, json_t * out_json );

#endif
