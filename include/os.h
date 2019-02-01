#ifndef OS_H_
#define OS_H_

/* Macros */
#include <stdlib.h>
#include <stdio.h>

/* Protos */
int OS_cp_file ( const char * to, const char * from );
int OS_readable_date ( long long epoch, FILE * log );
void OS_new_directory ( char * str );
size_t OS_write_file ( void * ptr, size_t size, size_t nmemb, FILE * stream );
void OS_fix_filename ( char * dirty );

#endif
