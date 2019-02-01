#ifndef ACCOUNT_H_
#define ACCOUNT_H_

/* Macros */
#include "stringutils.h"

/* Typedef */
enum account_type
{
	e_null = 0,
	e_group,
	e_user
} account_type;

struct account
{
	long long id;
	string * screenname;
	string * usr_fname;
	string * usr_lname;
	string * grp_name;
	string * grp_type;
	string * directory;

	enum account_type type;
} account;

/* Protos */
void AC_init ( void );
void AC_free ( void );
void AC_info ( void );
void AC_get_user ( char * str );
void AC_get_group ( char * str );
void AC_make_dir ( void );

#endif
