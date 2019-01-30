#ifndef OS_H_
#define OS_H_

/* Macros */
#include <stdlib.h>
#include <stdio.h>

/* Protos */
int cp_file ( const char * to, const char * from );
size_t write_file ( void * ptr, size_t size, size_t nmemb, FILE * stream );
int readable_date( long long epoch, FILE * log );

#endif
