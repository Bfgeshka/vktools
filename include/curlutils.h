#ifndef CURLUTILS_H_
#define CURLUTILS_H_

/* Typedef */
struct curl_arg
{
	char * payload;
	size_t size;
};

/* Protos */
void C_init ( void );
void C_finish ( void );
void C_get_request( const char * url, struct curl_arg * cf );
size_t C_get_file ( const char * url, const char * filepath );

#endif
