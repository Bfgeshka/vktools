/* Macros */
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include "stringutils.h"
#include "os.h"

/* Global scope */
void
OS_fix_filename ( char * dirty )
{
	for ( size_t i = 0; dirty[i] != '\0'; ++i )
		if ( ( ( dirty[i] & 0xC0 ) != 0x80 ) && ( dirty[i] == '/' || dirty[i] == '\\' || dirty[i] == '|' ) )
			dirty[i] = '_';
}

int
OS_cp_file ( const char * to, const char * from )
{
	int fd_to, fd_from;
	char buf[BUFSIZ];
	ssize_t nread;

	fd_from = open( from, O_RDONLY );
	if ( fd_from < 0 )
		return -1;

	fd_to = open( to, O_WRONLY | O_CREAT | O_EXCL, 0666 );
	if ( fd_to < 0 )
		goto cp_func_out_error;

	while ( nread = read( fd_from, buf, sizeof buf ), nread > 0 )
	{
		char * out_ptr = buf;
		ssize_t nwritten;

		do
		{
			nwritten = write( fd_to, out_ptr, nread );

			if ( nwritten >= 0 )
			{
				nread -= nwritten;
				out_ptr += nwritten;
			}
			else if ( errno != EINTR )
				goto cp_func_out_error;
		}
		while ( nread > 0 );
	}

	if ( nread == 0 )
	{
		if ( close( fd_to ) < 0 )
		{
			fd_to = -1;
			goto cp_func_out_error;
		}

		close(fd_from);

		/* Success! */
		return 0;
	}

	/* Close descriptors */
	cp_func_out_error:
	close(fd_from);

	if ( fd_to >= 0 )
		close(fd_to);
	return -1;
}

size_t
OS_write_file ( void * ptr, size_t size, size_t nmemb, FILE * stream )
{
	return fwrite( ptr, size, nmemb, stream );
}

void
OS_readable_date ( long long epoch, FILE * log )
{
	time_t epochtime = epoch;
	fprintf( log, "DATE: %s", ctime(&epochtime));
}

void
OS_new_directory ( char * str )
{
	if ( mkdir( str, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH ) != 0 )
		if ( errno != EEXIST )
		{
			fprintf( stderr, "mkdir() error (%d).\n", errno );
			exit(EXIT_FAILURE);
		}
}
