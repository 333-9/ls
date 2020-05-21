/*
 *
 */

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




struct lsflags {
    unsigned int   columns       : 2; /* 1 on, 2 auto */
    unsigned int   all           : 1; /* 1 -a, 2 -A */
    unsigned int   dot_dot       : 1;
    unsigned int   long_list     : 1;
    unsigned int   no_sort       : 1;
    unsigned int   reverse_sort  : 1;
    unsigned int   color         : 1;
    unsigned int   num_ids       : 1;
    unsigned int   show_owner    : 1;
    unsigned int   show_group    : 1;
    unsigned int   show_time     : 1;
    unsigned int   sort_argv     : 1;
};


struct file {
	char  * name;
	time_t  tim;
	off_t   size;
	size_t  blocks;
	mode_t  mode;
	uid_t   user;
	gid_t   group;
};


struct padding {
	int blocks;
	int size;
	int user;
	int group;
};




struct lsflags flags = { 0 };
char name_buf[0x400];
const size_t ext_sz   = sizeof(ext)   / sizeof(struct color_match);
const size_t fname_sz = sizeof(fname) / sizeof(struct color_match);
int ignore_links = 0;




static int   filter(const struct dirent *);
static int   greater(int, int);
static int   ilen(int );
static int   match(const char *, const char *);
// ---
static int   show_dir (char *);
static void  long_ls  (char **, size_t, size_t, char *);
static void  print_id(long long id, char ** (*)());
static void  print_line(struct file *, struct padding *);
static void  print_link(const char *);
static void  print_name(mode_t, const char *);
static void  print_path(const char *, const char *);
static void  print_perms(mode_t);
static void  print_time(time_t);
static void  print_type(mode_t);
static void  short_ls (char **, size_t, size_t, char *);
static void  show_file (char *);




static int
no_sort(
    const struct dirent **a,
    const struct dirent **b)
{
	return 0;
}

static int
my_alphasort(
    const struct dirent **a,
    const struct dirent **b)
{
	return strcmp((*a)->d_name, (*b)->d_name);
}

static int
sort_reverse(
    const struct dirent **a,
    const struct dirent **b)
{
	return -strcmp((*a)->d_name, (*b)->d_name);
}

typeof (&alphasort)  sort_func = my_alphasort;




static int
filter(const struct dirent *ent)
{
	return flags.all || ent->d_name[0] != '.';
}

static int
filter_dot(const struct dirent *ent)
{
	return filter(ent)
	    && strcmp(ent->d_name, ".")
	    && strcmp(ent->d_name, "..");
}


typeof (&filter)  filter_func = filter;




static int greater(int a, int b) {
	return (a > b) ? a : b;
};


static void print(const char *s) {
	fputs(s, stdout);
};


char *
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
	size_t sz;
	struct dirent **ent;
	// ---
	if (flags.dot_dot)
		filter_func = filter_dot;
	//
	if (flags.no_sort)
		sort_func = no_sort;
	else if (flags.reverse_sort)
		sort_func = sort_reverse;
	//
	sz = scandir(path, &ent, filter_func, sort_func);
	if (sz == 0) return 0;
	if (flags.long_list) {
		long_ls(
		    (char **) ent,
		    offsetof(struct dirent, d_name),
		    sz, path
		);
	} else {
		short_ls(
		    (char **) ent,
		    offsetof(struct dirent, d_name),
		    sz, path
		);
	};
	while (sz--) free(ent[sz]);
	free(ent);
	return 1;
}


void
show_file(char *path)
{
	struct stat s;
	struct file f;
	struct padding  pad = { 4, 8, 4, 4 };
	// ---
	if (lstat(path, &s) < 0) return ;
	f.mode = s.st_mode;
	if (S_ISDIR(f.mode)) {
		putchar('\n');
		if (flags.color) print_path(path, type_color[Dir]);
		else print(path);
		puts(" :");
		if (!show_dir(path) && flags.color)
			puts("\e[38;5;237mnone\e[0m");
		putchar('\n');
	} else if (flags.long_list) {
		f.name   = path;
		f.tim    = s.st_mtim.tv_sec; /*s.st_atim.tv_sec*/
		f.user   = s.st_uid;
		f.group  = s.st_gid;
		f.blocks = s.st_blocks;
		if (S_ISCHR(s.st_mode) || S_ISBLK(s.st_mode))
			f.size = s.st_rdev;
		else
			f.size  = s.st_size;
		print_line(&f, &pad);
	} else {
		if (flags.color) {
			print_name(f.mode, path);
		} else {
			print(path);
		};
		putchar('\n');
	};
}




void
long_ls(char **name, size_t off,  size_t sz, char *path)
{
	size_t i;
	struct stat s;
	struct file *f;
	struct padding  pad = { 1, 1, 1, 1 };
	f = malloc(sizeof(struct file) * sz);
	if (f == NULL) exit(1);
	for (i = 0; i < sz; i++) {
		f[i].name = name[i] + off;
		if (lstat(cat_dir(path, name[i] + off), &s) < 0) return ;
		f[i].tim    = s.st_mtim.tv_sec; // s.st_atim.tv_sec
		f[i].user   = s.st_uid;
		f[i].group  = s.st_gid;
		f[i].blocks = s.st_blocks;
		f[i].mode   = s.st_mode;
		if (S_ISCHR(s.st_mode) || S_ISBLK(s.st_mode)) {
			f[i].size = s.st_rdev;
			pad.size = greater(8, pad.size);
		} else {
			f[i].size  = s.st_size;
			pad.size = greater(ilen(f[i].size), pad.size);
		};
		pad.blocks = greater(ilen(f[i].blocks),  pad.blocks);
	};
	pad.size++;
	for (i = 0; i < sz; i++) {
		print_line(f + i, &pad);
	};
	free(f);
}


void
print_line(struct file *f, struct padding *pad)
{
	print_type(f->mode);
	print_perms(f->mode);
	printf("%*u ", (int) pad->blocks, (unsigned) f->blocks);
	if (S_ISCHR(f->mode) || S_ISBLK(f->mode))
		printf(" %*u, %*u ", 3, major(f->size), 3, minor(f->size));
	else
		printf("%*u ", (int) pad->size, (unsigned) f->size);
	if (flags.show_owner) print_id(f->user,  (char **(*)()) &getpwuid);
	if (flags.show_group) print_id(f->group, (char **(*)()) &getgrgid);
	if (flags.show_time) print_time(f->tim);
	if (flags.color) print_name(f->mode, f->name);
	else print(f->name);
	putchar('\n');
}


static inline int
ilen(int a) {
	int i;
	for (i = 0; a; i++)
		a >>= 3; //a /= 10;
	return i;
}


static void
print_time(time_t sec)
{
#define HALF_YEAR (183 * 86400)
	/* time() is called only once */
	static time_t now = 0;
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
	if (!flags.num_ids) {
		str = (char **) tostr(id);
		if (str == NULL) return ;
		printf("%s ", *str);
	} else {
		printf("%*u ", 3, (unsigned) id);
	};
}


static void
print_type(mode_t mode)
{
	if (flags.color)
	    switch (mode & S_IFMT) {
		case S_IFDIR:  print(type_char[Dir]);  break;
		case S_IFCHR:  print(type_char[Chr]);  break;
		case S_IFBLK:  print(type_char[Blk]);  break;
		case S_IFREG:  print(type_char[Reg]);  break;
		case S_IFIFO:  print(type_char[Fif]); break;
		case S_IFLNK:  print(type_char[Lnk]); break;
		case S_IFSOCK: print(type_char[Soc]); break;
		default:       print(type_char[Nol]); break;
	}
	else
	    switch (mode & S_IFMT) {
		case S_IFDIR:  putchar('d'); break;
		case S_IFCHR:  putchar('c'); break;
		case S_IFBLK:  putchar('b'); break;
		case S_IFREG:  putchar('-'); break;
		case S_IFIFO:  putchar('p'); break;
		case S_IFLNK:  putchar('l'); break;
		case S_IFSOCK: putchar('s'); break;
		default:       putchar('?'); break;
	};
}


static void
print_perms(mode_t m)
{
	int i;
	char mode[] = {
	    // owner
	    [0] = m & S_IRUSR ? 'r' : '-',
	    [1] = m & S_IWUSR ? 'w' : '-',
	    [2] = m & S_IXUSR ? 'x' : '-',
	    // group
	    [3] = m & S_IRGRP ? 'r' : '-',
	    [4] = m & S_IWGRP ? 'w' : '-',
	    [5] = m & S_IXGRP ? 'x' : '-',
	    // other
	    [6] = m & S_IROTH ? 'r' : '-',
	    [7] = m & S_IWOTH ? 'w' : '-',
	    [8] = m & S_IXOTH ? 'x' : '-',
	    //
	    0,
	};
	if (!flags.color) {
		print(mode);
	} else {
		for (i = 0; mode[i]; i++) {
			print(perm_char[mode[i] == '-'? False : 1 + i%3]);
			putchar(mode[i]);
		};
		print("\e[0m");
	};
	putchar(' ');
}




static size_t
get_tw()
{
	struct winsize	w;
	ioctl(0, TIOCGWINSZ, &w);
	return (w.ws_col);
}


void
short_ls(char **str, size_t off, size_t sz, char *path)
{
	int cols, c, i;
	int max, tmp;
	struct stat s = { 0 };
	// ---
	if (flags.columns) {
		if ((cols = get_tw()) < 0) cols = 80;
		for (max = 3, c = 0; c < sz; ++c) {
			tmp = strlen(str[c]+off);
			if (tmp > max) max = tmp;
		};
		if (((max +5) * 2) > cols) goto single_column;
		else  cols = cols / (max +5);
		max += flags.color ? 3 : 0;
		ignore_links = 1;
		for (c = 0; sz--; str++) {
			if (flags.color) {
				if (lstat(cat_dir(path, *str+off), &s) < 0)
					exit(3);
				print_name(s.st_mode, *str+off);
				i = strlen(*str+off);
				switch (s.st_mode & S_IFMT) {
				case S_IFCHR:  i += strlen(marker[Chr]); break;
				case S_IFBLK:  i += strlen(marker[Blk]); break;
				case S_IFIFO:  i += strlen(marker[Fif]); break;
				case S_IFSOCK: i += strlen(marker[Soc]); break;
				case S_IFDIR:  i += strlen(marker[Dir]); break;
				case S_IFLNK:  i += strlen(marker[Lnk])
				                  + 4 - i%4; break;
				case S_IFREG:
				default:
					if (s.st_mode & (S_IXUSR|S_IXGRP|S_IXOTH))
						i += strlen(marker[Reg]);
				};
				for (; i < max; i++) putc(' ', stdout);
			} else {
				printf("%-*s", max, *str+off);
			};
			if (++c == cols)
				c = 0, putchar('\n');
			else
				putchar(' ');
		};
		if (c) putchar('\n');
	} else {
single_column:
		for (; sz--; str++) {
			if (flags.color) {
				if (lstat(cat_dir(path, *str+off), &s) < 0)
					exit(3);
				print_name(s.st_mode, *str+off);
			} else {
				print(*str+off);
			};
			putchar('\n');
		};
	};
}


void
print_name(mode_t mode, const char *name)
{
	int i;
	const char *p;
	if (name == NULL) return;
	switch (mode & S_IFMT) {
	case S_IFCHR:  i = Chr; break;
	case S_IFBLK:  i = Blk; break;
	case S_IFIFO:  i = Fif; break;
	case S_IFSOCK: i = Soc; break;
	case S_IFDIR:  i = Dir; break;
	case S_IFLNK:
		print(type_color[Lnk]);
		print(name);
		print("\e[0m");
		print(marker[Lnk]);
		printf("%.*s", (int) (4 - strlen(name)%4), "    ");
		print_link(name);
		return;
	case S_IFREG:
	default:
		if (mode & (S_IXUSR|S_IXGRP|S_IXOTH)) {
			printf("%s%s\e[0m%s", type_color[Reg], name, marker[Reg]);
			return;
		};
		p = strrchr(name, '.');
		if (p == NULL) goto no_extension;
		for (++p, i = 0; i < ext_sz; i++) {
			if (!match(ext[i].s, p)) continue;
			printf("%s%s\e[0m", color_codes[ext[i].c], name);
			return;
		};
no_extension:
		for (i = 0; i < fname_sz; i++) {
			if (!match(fname[i].s, name)) continue;
			printf("%s%s\e[0m", color_codes[fname[i].c], name);
			return;
		};
		print(color_codes[Other]);
		print(name);
		print("\e[0m");
		return;
	};
	print(type_color[i]);
	print(name);
	print("\e[0m");
	print(marker[i]);
}


static void
print_link(const char *lname)
{
	if (!ignore_links) {
		char name[0x400];
		long sz;
		sz = readlink(lname, name, sizeof(name));
		if (sz <= 0) {
			ignore_links = 1;
			return ;
		};
		name[sz] = '\0';
		print_path(name, type_color[Lnk]);
	};

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


static void parse_flags(char *);
static void parse_argv(unsigned, char **);




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
			puts("usage: tls [-1CAaGlgonfrV] files");
			exit(1);
	};
}


int
main(int argc, char *argv[])
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
