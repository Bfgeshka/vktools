/* Typedef */
struct content
{
	unsigned documents : 1;
	unsigned pictures  : 1;
	unsigned videos    : 1;
	unsigned comments  : 1;
} content;

/* Protos */
void CT_default ( void );
void CT_print_types ( void );
