/*---------------------------------*
 |  tree - list files recursively  |
 *---------------------------------*/

#include <dirent.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <grp.h>
#include <pwd.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <sys/types.h>


#define HALF_YEAR (183 * 86400)


/*
 * definitions required for config tables
 */

enum {
    Nol = 0, Dir = 1,
    Chr = 2, Blk = 3,
    Reg = 4, Fif = 5,
    Lnk = 6, Soc = 7,
};

enum {
    False = 0,
    Read  = 1,
    Write = 2,
    Exec  = 3,
};

enum {
    Archive   = 000,  Audio     = 001,
    Config    = 002,  Data      = 003,
    Document  = 004,  Font      = 005,
    Headers   = 006,  Object    = 007,
    Patch     = 010,  Picture   = 011,
    RichText  = 012,  Signature = 013,
    Software  = 014,  Source    = 015,
    Video     = 016,  Other     = 017,
};

struct color_match {
    char s[14];
    char c;
};


#include "config.h"




enum {
	Indent  = 0,
	Ascii   = 1,
	Unicode = 2,
	Yaml    = 3,
	Json    = 4,
};

struct {
	unsigned int   format     : 4;
	unsigned int   all        : 1;
	unsigned int   dirs_only  : 1;
	unsigned int   full_path  : 1;
	unsigned int   size       : 1;
	unsigned int   owner      : 1;
	unsigned int   group      : 1;
	unsigned int   time       : 1;
	unsigned int   color      : 1;
	//
} flags = {0};

unsigned maxdepth = 0x400;
unsigned depth = 0;
const char *indent = "  ";


struct file {
	char  * name;
	time_t  time;
	off_t   size;
	mode_t  mode;
	uid_t   user;
	gid_t   group;
};


struct padding {
	int size;
	int user;
	int group;
};




char name_buf[0x400];
const size_t ext_sz   = sizeof(ext)   / sizeof(struct color_match);
const size_t fname_sz = sizeof(fname) / sizeof(struct color_match);




static int   my_alphasort(const struct dirent **, const struct dirent **);
static int   filter(const struct dirent *);
static int   greater(int, int);
static int   ilen(unsigned long);
static int   match(const char *, const char *);
// ---
static int   show_dir (char *);
static void  list(char **, size_t, char *);
static void  print_id(long long id, char ** (*)());
static void  print_name(mode_t, const char *);
static void  print_path(const char *, const char *);
static void  print_time(time_t);
static void  show_file (char *);




static int
my_alphasort(
    const struct dirent **a,
    const struct dirent **b)
{
	return strcmp((*a)->d_name, (*b)->d_name);
}


static int
filter(const struct dirent *ent)
{
	return (flags.all || ent->d_name[0] != '.')
	    && strcmp(ent->d_name, ".")
	    && strcmp(ent->d_name, "..");
}


static int greater(int a, int b) {
	return (a > b) ? a : b;
};

static void print(const char *s) {
	fputs(s, stdout);
};


static char *
cat_dir(const char *a, const char *b)
{
	if (snprintf(name_buf, sizeof(name_buf), "%s/%s", a, b)
	    > sizeof(name_buf))
	{
		fputs("file path too long\n", stderr);
		exit(2);
	};
	return name_buf;
}


int
show_dir(char *path)
{
	int i, sz;
	struct dirent **ent;
	// ---
	sz = scandir(path, &ent, &filter, &my_alphasort);
	if (sz <= 0) return 0;
	for (i = 0; i < sz; i++) {
		memmove(ent[i], ent[i]->d_name, strlen(ent[i]->d_name) +1);
	};
	list((char **) ent, sz, path);
	//
	while (sz--) free(ent[sz]);
	free(ent);
	return 1;
}


//-------------------------------------------------------------------


void
list(char **name, size_t sz, char *path)
{
	size_t i, j;
	struct stat s;
	char *dname;
	for (i = 0; i < sz; i++) {
		dname = cat_dir(path, name[i]);
		if (lstat(cat_dir(path, name[i]), &s) < 0)
			return ;
		if (flags.size)   printf("%8lu ", (long) s.st_size);
		if (flags.owner)  print_id(s.st_uid,  (char **(*)()) &getpwuid);
		if (flags.group)  print_id(s.st_gid, (char **(*)()) &getgrgid);
		if (flags.time)   print_time(s.st_mtim.tv_sec);
		for (j = 0; j <= depth; j++)
			print(indent);
		if (flags.color)  print_name(s.st_mode, name[i]);
		else              print(name[i]);
		putchar('\n');
		if (S_ISDIR(s.st_mode)) {
			dname = strdup(dname);
			depth += 1;
			show_dir(dname);
			depth -= 1;
		};
	};
}


static void
print_indent()
{
	// something
}


static void
print_time(time_t sec)
{
	static time_t now = 0; // time travel not supported
	struct tm *t;
	char str[20];
	// ---
	if (!now) now = time(0);
	t = localtime(&sec);
	if ((now - sec) > HALF_YEAR)
		strftime(str, 20, " %b %d  %Y ", t);
	else
		strftime(str, 20, " %b %d %H:%M ", t);
	print(str);
}


static void
print_id(long long id, char ** (*tostr)())
{
	char **str;
	str = (char **) tostr(id);
	if (str == NULL) return ;
	printf("%s ", *str);
}


void
print_name(mode_t mode, const char *name)
{
	int i;
	const char *color = "", *mark = "";
	const char *p;
	if (name == NULL) return;
	switch (mode & S_IFMT) {
	case S_IFCHR:  color = type_color[Chr]; mark = marker[Chr]; break;
	case S_IFBLK:  color = type_color[Blk]; mark = marker[Blk]; break;
	case S_IFIFO:  color = type_color[Fif]; mark = marker[Fif]; break;
	case S_IFSOCK: color = type_color[Soc]; mark = marker[Soc]; break;
	case S_IFDIR:  color = type_color[Dir]; mark = marker[Dir]; break;
	case S_IFLNK:  color = type_color[Lnk]; mark = marker[Lnk]; break;
	case S_IFREG:
	default:
		if (mode & (S_IXUSR|S_IXGRP|S_IXOTH)) {
			color = type_color[Reg];
			mark  = marker[Reg];
			goto print;
		};
		p = strrchr(name, '.');
		if (p != NULL) {
			p += 1;
			for (i = 0; i < ext_sz; i++) {
				if (match(ext[i].s, p)) {
					color = color_codes[ext[i].c];
					goto print;
				};
			};
		};
		for (i = 0; i < fname_sz; i++) {
			if (!match(fname[i].s, name)) {
				color = color_codes[fname[i].c];
				goto print;
			};
		};
		color = color_codes[Other];
	};
print:
	printf("%s%s\e[0m%s", color, name, mark);
}


void
print_path(const char *str, const char *color)
{
	fputs(color, stdout);
	for (;*str;str++) {
		if (*str != '/') putchar(*str);
		else printf("\e[0m/%s", color);
	};
	fputs("\e[0m", stdout);
	return ;
}


static int
match(const char *r, const char *s)
{
	int m;
	for (;*r; r++, s++) switch (*r) {
		case '*':
			for (++r;*s;)
				if (match(r, s++)) return 1;
			return !*r;
		case '?':
			break;
		case '[':
			for (m = 0; *r && *r != ']';)
				m += (*s == *++r);
			if (!m) return 0;
			break;
		default:
			if (*s != *r) return 0;
	};
	return !*s;
};


static void parse_flags(const char *);
static void parse_argv(unsigned, const char **);




static void
parse_argv(unsigned n, const char **arg)
{
	for (; n; n--, arg++) {
		if (!strcmp(*arg, "-n")) {
			arg++;
			n--;
			maxdepth = strtol(*arg, NULL, 0);
			if (maxdepth <= 0)
				maxdepth = 0x400;
			continue;
		} else if (**arg == '-') {
			parse_flags(*arg + 1);
		};
	};
}


static void
parse_flags(const char *arg)
{
	for (; *arg; arg++) {
		switch (*arg)
		{
		case 'A': flags.format  = Ascii;   break;
		case 'U': flags.format  = Unicode; break;
		case 'Y': flags.format  = Yaml;    break;
		case 'J': flags.format  = Json;    break;
		//
		case 'a': flags.all        = 1; break;
		case 'd': flags.dirs_only  = 1; break;
		case 'p': flags.full_path  = 1; break;
		case 's': flags.size       = 1; break;
		case 'o': flags.owner      = 1; break;
		case 'g': flags.group      = 1; break;
		case 't': flags.time       = 1; break;
		case 'G': flags.color      = 1; break;
		//
		case '-':
			/* long options, if any */
		default:
			printf("tls: illegal option:  %c\n", *arg);
		case 'h':
			puts("usage: tls [-1CAaGlgonfrV] files");
			exit(1);
		};
	};
}


int
main(int argc, char *argv[])
{
	int i, noarg = 1;
	parse_argv(argc -1, argv +1);
	if (argc >= 2) {
		for (i = 1; i < argc; i++) {
			if (*argv[i] == '-') continue;
			// show file
			noarg = 0;
		};
	};
	if (noarg) show_dir(".");
	return 0;
}
