#ifndef ACCOUNT_H_
#define ACCOUNT_H_

/* Macros */
#include "stringutils.h"

/* Typedef */
typedef enum account_type
{
	e_null = 0,
	e_group,
	e_user
} acc_type;

typedef struct data_album
{
	long long id;
	long long size;
	string * title;
} album;

typedef struct data_account
{
	acc_type type;
	long long id;
	long long albums_count;
	album * albums;
	string * screenname;
	string * usr_fname;
	string * usr_lname;
	string * grp_name;
	string * grp_type;

	string * directory;
	string * currentdir;
} account;


/* Protos */
account * AC_init ( void );
void AC_free ( account * );
void AC_info ( account * );
account * AC_get_user ( char * str );
account * AC_get_group ( char * str );
void AC_make_dir ( account * );

#endif
