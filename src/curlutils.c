/* Macro */
#include <curl/curl.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "curlutils.h"

/* Visible 1k definition, default is 1 kibibyte */
#define ONE_K 1024

/* 1L - verbose curl connection, 0L - silent */
#define CRL_VERBOSITY 0L

/* Where to store downloading file */
#define TMP_CURL_FILENAME "/tmp/vkgrab.tmp"

/* Proto */
static CURLcode C_fetch ( const char *, struct curl_arg * );
static int progress_func ( void *, double, double, double, double );
static size_t C_callback ( void *, size_t, size_t, void * );
static size_t write_file( void *, size_t, size_t, FILE * );
static int cp_file ( const char *, const char * );

/* Local scope */
static CURL * Curl;

static size_t
C_callback ( void * content, size_t wk_size, size_t wk_nmemb, void * upoint )
{
	size_t rsize = wk_nmemb * wk_size;
	struct curl_arg * p = (struct curl_arg *)upoint;

	/* allocation for new size */
	p->payload = realloc( p->payload, p->size + rsize + 1 );
	if ( p->payload == NULL )
	{
		fprintf( stderr, "Reallocation failed in C_callback()\n" );
		return -1;
	}

	/* making valid string */
	memcpy( &( p->payload[p->size] ), content, rsize );
	p->size += rsize;
	p->payload[p->size] = 0;
	return rsize;
}

static CURLcode
C_fetch ( const char * url, struct curl_arg * fetch_str )
{
	CURLcode code;

	if ( fetch_str->payload == NULL )
	{
		fprintf( stderr, "Allocation failed in crl_fetch()\n" );
		return CURLE_FAILED_INIT;
	}

	curl_easy_setopt( Curl, CURLOPT_URL, url );
	curl_easy_setopt( Curl, CURLOPT_WRITEFUNCTION, C_callback );
	curl_easy_setopt( Curl, CURLOPT_WRITEDATA, (void *)fetch_str );
	curl_easy_setopt( Curl, CURLOPT_USERAGENT, "libcurl-agent/1.0" );
	curl_easy_setopt( Curl, CURLOPT_VERBOSE, CRL_VERBOSITY );
	code = curl_easy_perform(Curl);
	return code;
}

static size_t
write_file ( void * ptr, size_t size, size_t nmemb, FILE * stream )
{
	size_t written = fwrite( ptr, size, nmemb, stream );
	return written;
}

static int
progress_func ( void * ptr, double todl_total, double dl_ed, double undef_a, double undef_b )
{
	char i = 0;
	double percentage = 0;
	(void)undef_a;
	(void)undef_b;
	(void)ptr;

	if ( todl_total != 0 )
		percentage = ( dl_ed / todl_total ) * 100;

	putchar(' ');
	putchar('<');

	for ( ; i < 100; i += 5 )
	{
		if ( percentage > i )
			printf("%s", "▓");
		else
			putchar(' ');
	}

	printf( "> [%03.2f%%] %.1f/%.1f KiB\r\b", percentage, dl_ed/ONE_K, todl_total/ONE_K );
	fflush(stdout);

	return 0;
}

static int
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

/* Global scope */
void
C_init ( void )
{
	Curl = curl_easy_init();
	if ( !Curl )
	{
		fprintf( stderr, "Curl initialisation error.\n" );
		exit(EXIT_FAILURE);
	}
}

void
C_finish ( void )
{
	curl_easy_cleanup(Curl);
}

void
C_get_request( const char * url, struct curl_arg * cf )
{
	CURLcode code;

	/* struct initialiisation */
	cf->size = 0;
	cf->payload = malloc(1);

	/* fetching an answer */
	curl_easy_reset(Curl);
	code = C_fetch( url, cf );

	/* checking result */
	if ( code != CURLE_OK || cf->size < 1 )
		fprintf( stderr, "GET error: %s\n", curl_easy_strerror(code) );
	if ( !cf->payload )
		fprintf( stderr, "%s", "Callback is empty, nothing to do here\n" );
}

size_t
C_get_file ( const char * url, const char * filepath )
{
	if ( !Curl )
		exit(EXIT_FAILURE);

	/* skip downloading if file exists */
	errno = 0;
	long long file_size = 0;
	int err;
	FILE * fr = fopen( filepath, "r" );
	err = errno;

	if ( fr != NULL )
	{
		fclose(fr);
		struct stat fst;
		if (  stat( filepath, &fst ) != -1 )
			file_size = (long long)fst.st_size;

		if ( file_size > 0 )
		{
			printf( "\r\b%s", filepath );
			printf( "%s", " \033[00;36m[SKIP]\033[00m\n" );

			return 0;
		}
	}

	if ( err == ENOENT || file_size == 0 )
	{
		fflush(stdout);
		FILE * fw = fopen( TMP_CURL_FILENAME, "w" );
		curl_easy_setopt( Curl, CURLOPT_URL, url );
		curl_easy_setopt( Curl, CURLOPT_WRITEFUNCTION, write_file );
		curl_easy_setopt( Curl, CURLOPT_WRITEDATA, fw );
		curl_easy_setopt( Curl, CURLOPT_VERBOSE, CRL_VERBOSITY );
		curl_easy_setopt( Curl, CURLOPT_FOLLOWLOCATION, 1 );
		curl_easy_setopt( Curl, CURLOPT_MAXREDIRS, 2 );
		curl_easy_setopt( Curl, CURLOPT_NOPROGRESS, 0 );
		curl_easy_setopt( Curl, CURLOPT_PROGRESSFUNCTION, progress_func );
		CURLcode code;
		code = curl_easy_perform(Curl);

		if ( code != CURLE_OK )
		{
			fprintf( stderr, "GET error: %s [%s]\n", curl_easy_strerror(code), url );
			return 1;
		}

		curl_easy_reset(Curl);
		fclose(fw);
		cp_file( filepath, TMP_CURL_FILENAME );

		printf( "\r\b%s", filepath );
		printf( "%s", " \033[01;32m[OK]\033[00m\n" );
	}

	return 0;
}
