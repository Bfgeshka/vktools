#ifndef CURLUTILS_H_
#define CURLUTILS_H_

/* Macro */

/* Typedef */
struct curl_arg
{
	char * payload;
	size_t size;
};

/* Proto */
void C_init ( void );
void C_get_request( const char * url, struct curl_arg * cf );
size_t C_get_file ( const char * url, const char * filepath );

#endif
