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


enum {
	Indent  = 0,
	Unicode = 1,
	Ascii   = 2,
	Yaml    = 3,
	Json    = 4,
};

struct {
	unsigned  format     : 4;
	unsigned  all        : 1;
	unsigned  dirs_only  : 1;
	unsigned  full_path  : 1;
	unsigned  color      : 1;
	union {
		char info;
		struct {
			char  size   : 1;
			char  owner  : 1;
			char  group  : 1;
			char  time   : 1;
		};
	};
	//
} flags = {0};

unsigned maxdepth = 0x200;
unsigned depth = 0;
const char *indent = "  ";
const char *gpath = "";

char name_buf[0x400];
char leaf_flags[0x200] = {0};

const size_t ext_sz    = sizeof(ext)   / sizeof(struct color_match);
const size_t fname_sz  = sizeof(fname) / sizeof(struct color_match);



static int   my_alphasort(const struct dirent **, const struct dirent **);
static int   filter(const struct dirent *);
static int   match(const char *, const char *);
static void  soft_tab(int);
static void  print(const char *);
static char *cat_dir(const char *, const char *);
//
static void  parse_flags(const char *);
static int   parse_argv(unsigned, const char **);
static void  root(const char *);
static void  recurse(const char *, int);
static void  list(const char *);
// format:
static void print_info_yaml (const struct stat *, int);
static void print_info_json (const struct stat *, int);
static void print_ascii     (const struct stat *, const char *, int);
static void print_unicode   (const struct stat *, const char *, int);
static void print_indent    (const struct stat *, const char *, int);
static void print_entry     (const struct stat *, const char *, int );
static void print_contains  (void);
// print:
static void print_info(const struct stat *);
static void print_time(time_t);
static void print_id(long, char ** (*)());
static void print_name(mode_t, const char *);
static void print_path(const char *, const char *);


static void
print_info_yaml(const struct stat *s, int indent)
{
	if (flags.size)   {
		soft_tab(indent);
		printf("size: %lu", (long) s->st_size);
		putchar('\n');
	};
	if (flags.owner)  {
		soft_tab(indent);
		print("owner: \"");
		print_id(s->st_uid,  (char **(*)()) &getpwuid);
		print("\"\n");
	};
	if (flags.group)  {
		soft_tab(indent);
		print("group: \"");
		print_id(s->st_gid,  (char **(*)()) &getgrgid);
		print("\"\n");
	};
	if (flags.time)   {
		soft_tab(indent);
		print("modified: \"");
		print_time(s->st_mtim.tv_sec);
		print("\"\n");
	};
}

static void
print_info_json(const struct stat *s, int indent)
{
	int bits = flags.info;
	if (flags.size)   {
		soft_tab(indent);
		printf("\"size\": %lu", (long) s->st_size);
		if (bits -= flags.size) putchar(',');
		putchar('\n');
	};
	if (flags.owner)  {
		soft_tab(indent);
		print("\"owner\": \"");
		print_id(s->st_uid,  (char **(*)()) &getpwuid);
		putchar('"');
		if (bits -= flags.owner) putchar(',');
		putchar('\n');
	};
	if (flags.group)  {
		soft_tab(indent);
		print("\"group\": \"");
		print_id(s->st_gid,  (char **(*)()) &getgrgid);
		putchar('"');
		if (bits -= flags.group) putchar(',');
		putchar('\n');
	};
	if (flags.time)   {
		soft_tab(indent);
		print("\"modified\": \"");
		print_time(s->st_mtim.tv_sec);
		putchar('"');
		if (S_ISDIR(s->st_mode)) putchar(',');
		putchar('\n');
	};
}

static void
print_info(const struct stat *s)
{
	if (flags.size)   printf("%8lu ", (long) s->st_size);
	if (flags.owner)  { print_id(s->st_uid,  (char **(*)()) &getpwuid); putchar(' '); };
	if (flags.group)  { print_id(s->st_gid,  (char **(*)()) &getgrgid); putchar(' '); };
	if (flags.time)   { putchar(' '); print_time(s->st_mtim.tv_sec); putchar(' '); };
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
		strftime(str, 20, "%b %d  %Y", t);
	else
		strftime(str, 20, "%b %d %H:%M", t);
	print(str);
}


static void
print_id(long id, char ** (*tostr)())
{
	char **str;
	str = (char **) tostr(id);
	if (str == NULL) return ;
	printf("%s", *str);
}


static void
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
	if (flags.full_path) {
		print_path(gpath, color);
		printf("/%s%s\e[0m%s", color, name, mark);
	} else {
		printf("%s%s\e[0m%s", color, name, mark);
	};
}


static void
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

static void
print_ascii(const struct stat *s, const char *name, int last)
{
	int i;
	for (i = 0; i < depth; i++) {
		if (leaf_flags[i]) print("   ");
		else print("|  ");
		//print(indent);
	};
	if (last) print("`- ");
	else      print("|- ");
	if (flags.color) print_name(s->st_mode, name);
	else             print(name);
	//
	if (!flags.info) {
		putchar('\n');
		return ;
	};
	soft_tab(32 - (strlen(name) + depth * 3));
	putchar('\t');
	print_info(s);
	putchar('\n');
}

static void
print_unicode(const struct stat *s, const char *name, int last)
{
	int i;
	for (i = 0; i < depth; i++) {
		if (leaf_flags[i]) print("  ");
		else print("│ ");
	};
	print(last ? "└╴" : "├╴");
	if (flags.color)  print_name(s->st_mode, name);
	else  print(name);
	//
	if (!flags.info) {
		putchar('\n');
		return ;
	};
	soft_tab(24 - (strlen(name) + depth * 2));
	putchar('\t');
	print_info(s);
	putchar('\n');
}

static void
print_indent(const struct stat *s, const char *name, int last)
{
	int i;
	for (i = 0; i <= depth; i++) {
		print(indent);
	};
	if (flags.color) print_name(s->st_mode, name);
	else             print(name);
	//
	if (!flags.info) {
		putchar('\n');
		return ;
	};
	soft_tab(16 - (strlen(name) + depth * 2));
	putchar('\t');
	print_info(s);
	putchar('\n');
}

static void
print_entry(const struct stat *s, const char *name, int last)
{
	switch (flags.format) {
	case Indent:   print_indent  (s, name, last); break;
	case Ascii:    print_ascii   (s, name, last); break;
	case Unicode:  print_unicode (s, name, last); break;
	case Yaml:
		soft_tab(depth * 4 + 2);
		printf("- name: \"%s%s%s\"%s\n",
		    flags.full_path ? gpath : "",
		    flags.full_path ? "/" : "",
		    name,
		    flags.info ? "," : "");
		print_info_yaml(s, depth * 4 + 4);
		break;
	case Json:
		soft_tab(depth * 4 + 2);  puts("{");
		soft_tab(depth * 4 + 4);
		printf("\"name\": \"%s%s%s\"%s\n",
		    flags.full_path ? gpath : "",
		    flags.full_path ? "/" : "",
		    name,
		    flags.info ? "," : "");
		print_info_json(s, depth * 4 + 4);
		if (!S_ISDIR(s->st_mode)) {
			soft_tab(depth * 4 + 2);
			printf("}%s\n", last ? "" : ",");
		};
		break;
	default:
		break;
	};
}

static void
print_contains()
{
	soft_tab(depth * 4 + 4);
	if (flags.format == Yaml) {
		print("contains:\n");
	} else if (flags.format == Json) {
		print("\"contains\": [\n");
	};
}

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
	struct stat s;
	if (flags.dirs_only) {
		stat(cat_dir(gpath, ent->d_name), &s);
		return (S_ISDIR(s.st_mode))
		    && (flags.all || ent->d_name[0] != '.')
		    && strcmp(ent->d_name, ".")
		    && strcmp(ent->d_name, "..");
	};
	return (flags.all || ent->d_name[0] != '.')
	    && strcmp(ent->d_name, ".")
	    && strcmp(ent->d_name, "..");
}

static void print(const char *s) {
	fputs(s, stdout);
};

static char *
cat_dir(const char *a, const char *b)
{
	if (snprintf(name_buf, sizeof(name_buf), "%s/%s", a, b)
	    > sizeof(name_buf))
	{
		fputs("tree: file path too long\n", stderr);
		exit(2);
	};
	return name_buf;
}

static void
soft_tab(int n)
{
	while (n-- > 0)
		putchar(' ');
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

static void
root(const char *name)
{
	struct stat s;
	if (lstat(name, &s) < 0) {
		fprintf(stderr, "tree: %s does not exist\n", name);
		exit(1);
	} else if (!S_ISDIR(s.st_mode)) {
		fprintf(stderr, "tree: %s not a directory\n", name);
		exit(1);
	};
	depth = (flags.format == Json) ? 1 : 0;
	if (flags.format < Yaml) {
		if (flags.color)  print_name(s.st_mode, name);
		else              print(name);
		if (flags.info) {
			soft_tab(16 + flags.format * 8
			    - (strlen(name) + depth * 2));
			putchar('\t');
			print_info(&s);
		};
		putchar('\n');
	} else if (flags.format == Yaml) {
		printf("name: \"%s\"\n", name);
		if (flags.info) print_info_yaml(&s, 0);
		print("contains:\n");
	} else if (flags.format == Json) {
		printf("  {\n    \"name\": \"%s\",\n", name);
		if (flags.info) print_info_json(&s, 4);
		soft_tab(4);
		print("\"contains\": [\n");
	};
	list(name);
	if (flags.format == Json)
		print("    ]\n  }");
}

static void
recurse(const char *dname, int last)
{
	leaf_flags[depth] = last;
	depth += 1;
	list(dname);
	depth -= 1;
}

static void
list(const char *path)
{
	int i, j, sz;
	struct dirent **ent;
	char **name;
	struct stat s;
	char *dname;
	//
	if (flags.dirs_only)
		gpath = path;
	sz = scandir(path, &ent, &filter, &my_alphasort);
	if (sz <= 0) return ;
	for (i = 0; i < sz; i++) {
		memmove(ent[i], ent[i]->d_name, strlen(ent[i]->d_name) +1);
	};
	name = (char **) ent;
	//
	for (i = 0; i < sz; i++) {
		dname = cat_dir(path, name[i]);
		if (lstat(dname, &s) < 0)
			return ;
		if (flags.dirs_only && !S_ISDIR(s.st_mode)) {
			continue;
		};
		// else
		if (flags.full_path) gpath = path;
		print_entry(&s, name[i], i == sz-1);
		if (S_ISDIR(s.st_mode) && depth < maxdepth) {
			if (flags.format >= Yaml)
				print_contains();
			//
			dname = strdup(dname);
			recurse(dname, i == sz-1);
			//
			if (flags.format == Json) {
				soft_tab(depth * 4 + 4);
				print("]\n");
				soft_tab(depth * 4 + 2);
				printf("}%s\n", i == sz-1 ? "" : ",");
			};
		};
	};
	//
	while (sz--) free(ent[sz]);
	free(ent);
}

static int
parse_argv(unsigned n, const char **arg)
{
	int notfirst;
	//
	for (; n; n--, arg++) {
		if (!strcmp(*arg, "-n")) {
			if (arg++ == NULL) break;
			if (n-- <= 0) break;
			maxdepth = strtol(*arg, NULL, 0);
			if (maxdepth < 0) maxdepth = 0x400;
		} else if (**arg == '-') {
			parse_flags(*arg + 1);
		} else {
			if (flags.format == Json)
				print(notfirst ? ",\n" : "[\n");
			else if (notfirst)
				putchar('\n');
			root(*arg);
			notfirst = 1;
		};
	};
	return notfirst;
}

static void
parse_flags(const char *arg)
{
	for (; *arg; arg++) {
		switch (*arg)
		{
		case 'A': flags.format  = Ascii;   break;
		case 'U': flags.format  = Unicode; break;
		case 'I': flags.format  = Indent;  break;
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
			printf("tree: illegal option:  %c\n", *arg);
		case 'h':
			puts("usage: tree [-adpsogtGAUIYJ] [file ...]");
			exit(1);
		};
	};
}

int
main(int argc, const char *argv[])
{
	int i, noarg = 1;
	if (!parse_argv(argc -1, argv +1)) {
		if (flags.format == Json) print("[\n");
		root(".");
	};
	if (flags.format == Json)
		print("\n]\n");
	return 0;
}
