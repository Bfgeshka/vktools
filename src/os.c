
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include "stringutils.h"
#include "os.h"

int
cp_file ( const char * to, const char * from )
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
write_file ( void * ptr, size_t size, size_t nmemb, FILE * stream )
{
	size_t written = fwrite( ptr, size, nmemb, stream );
	return written;
}

int
readable_date( long long epoch, FILE * log )
{
	string * date_invoke = construct_string(512);
	string * date_result = construct_string(512);
	int retvalue = 0;

	stringset( date_invoke, "date --date='@%lld'", epoch );

	FILE * piped;
	piped = popen( date_invoke->s, "r" );
	if ( piped == NULL )
	{
		stringset( date_result, "%s", "date get failed" );
		retvalue = -1;
		goto readable_data_ret_mark;
	}

	fgets( date_result->s, date_result->bytes, piped );
	pclose(piped);

	fprintf( log, "DATE: %s", date_result->s );

	readable_data_ret_mark:
	free_string(date_invoke);
	free_string(date_result);
	return retvalue;
}
