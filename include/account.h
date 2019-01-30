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
	string * screenname;
	string * usr_fname;
	string * usr_lname;
	string * grp_name;
	string * grp_type;

	enum account_type type;
} account;


#endif
