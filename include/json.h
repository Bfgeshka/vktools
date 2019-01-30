#ifndef VKJSON_H_
#define VKJSON_H_

/* Macros */
#include <jansson.h>

/* Typedef */


/* Protos */
long long js_get_int( json_t * src, char * key );
const char * js_get_str( json_t * src, char * key );

#endif
