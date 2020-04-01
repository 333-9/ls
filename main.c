#include "defs.h"
#include "show.h"


static void parse_flags(char *);
static void parse_argv(unsigned, char **);


struct lsflags  flags = { 0 };




static int
sort_argv(const void *a, const void *b) {
	return flags.reverse_sort ?
		-strcmp(a, b) :
		 strcmp(a, b);
}


static void
parse_argv(unsigned argc, char **argv)
{
	for (; argc; argc--, argv++)
		if (**argv == '-') parse_flags(*argv + 1);
}


static void
parse_flags(char *astr)
{
	while (*astr)  switch (*astr++)
	{
		case '1': flags.columns      = 0; break;
		case 'C': flags.columns      = 1; break;
		case 'A': flags.dot_dot      = 1; // -->
		case 'a': flags.all          = 1; break;
		case 'f': flags.no_sort      = 1; break;
		case 'g': flags.show_group   = 1; break;
		case 'G': flags.color        = 1; break;
		case 'l': flags.long_list    = 1; break;
		case 'n': flags.num_ids      = 1; break;
		case 'o': flags.show_owner   = 1; break;
		case 'r': flags.reverse_sort = 1; break;
		case 't': flags.show_time    = 1; break;
		case 'V': flags.sort_argv    = 1; break;
		case '-':
			/* long options, if any */
		default:
			printf("tls: illegal option:  %c\n", astr[-1]);
			puts("usage: tls [-AGafglnor1] files");
			exit(1);
	};
}


int
main(unsigned argc, char *argv[])
{
	int i, noarg = 1;
	flags.columns = isatty(1);
	parse_argv(argc -1, argv +1);
	if (flags.sort_argv)
		qsort(argv +1, argc -1, sizeof(char *), &sort_argv);
	if (argc >= 2) {
		for (i = 1; i < argc; i++) {
			if (*argv[i] == '-') continue;
			show_file(argv[i]);
			noarg = 0;
		};
	};
	if (noarg) show_dir(".");
	return 0;
}
