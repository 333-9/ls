#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/sysmacros.h>
#include "defs.h"
#include "show.h"

#include "config.h"

extern struct lsflags flags;




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




char file_buf[0x400];
const size_t ext_sz   = sizeof(ext)   / sizeof(struct color_match);
const size_t fname_sz = sizeof(fname) / sizeof(struct color_match);
int ignore_links = 0;




static void short_ls (char **, size_t, size_t, char *);
static void long_ls  (char **, size_t, size_t, char *);

static void  print_line(struct file *, struct padding *);
static void  print_path(const char *, const char *);
static void  print_link(const char *);
static int   print_name(mode_t, const char *);
static int   match(const char *, const char *);
static int   filter(const struct dirent *);
static int   greater(int, int);
static int   ilen(int );
static void  print_time(time_t);
static char  get_extra_char(char *);
static char *get_id(long long id, void *(*func)());
static void  print_type(mode_t);
static void  print_perms(mode_t);




static int
no_sort(
    const struct dirent **a,
    const struct dirent **b) {
	return 0;
}

static int
my_alphasort(
    const struct dirent **a,
    const struct dirent **b) {
	return strcmp((*a)->d_name, (*b)->d_name);
}

static int
sort_reverse(
    const struct dirent **a,
    const struct dirent **b) {
	return -strcmp((*a)->d_name, (*b)->d_name);
}

typeof(&alphasort)  sort_func = my_alphasort;




static int
filter(const struct dirent *ent) {
	return flags.all || ent->d_name[0] != '.';
}

static int
filter_dot(const struct dirent *ent) {
	return filter(ent)
	    && strcmp(ent->d_name, ".")
	    && strcmp(ent->d_name, "..");
}

typeof(&filter)  filter_func = filter;




static int greater(int a, int b) {
	return (a > b) ? a : b;
};


static void print(const char *s) {
	fputs(s, stdout);
};


int
print_name(mode_t mode, const char *name)
{
	int i;
	const char *p;
	if (name == NULL) return 0;
	switch (mode & S_IFMT) {
	case S_IFCHR:  fputs(CHR,  stdout); break;
	case S_IFBLK:  fputs(BLK,  stdout); break;
	case S_IFIFO:  fputs(FIFO, stdout); break;
	case S_IFSOCK: fputs(SOCK, stdout); break;
	case S_IFDIR:
		printf(DIR_ "%s" NORMAL " >", name);
		return strlen(name) + 2;
	case S_IFLNK:
		i = printf(LINK "%s" NORMAL " ->%.*s",
		        name, (int) (4 - strlen(name)%4), "    ");
		print_link(name);
		return (i - strlen(LINK NORMAL));
	case S_IFREG: /* fallthrow */
		if (mode & (S_IXUSR|S_IXGRP|S_IXOTH)) {
			printf(EXEC "%s" NORMAL " *", name);
			return strlen(name) + 2;
		};
	default:
		if ((p = strrchr(name, '.')) != NULL) {
			for (++p, i = 0; i < ext_sz; i++) {
				if (!match(ext[i].r, p)) continue;
				printf("%s%s" NORMAL, ext[i].c, name);
				return strlen(name);
			};
		};
		for (i = 0; i < fname_sz; i++) {
			if (!match(fname[i].r, name)) continue;
			printf("%s%s" NORMAL, fname[i].c, name);
			return strlen(name);
		};
	};
	i = printf("%s", name);
	fputs(NORMAL, stdout);
	return i;
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
		print_path(name, LINK);
	};

}


void
print_path(const char *str, const char *color)
{
	fputs(color, stdout);
	for (;*str;str++) {
		if (*str != '/') putchar(*str);
		else printf(NORMAL "/%s", color);
	};
	fputs(NORMAL, stdout);
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


char *
cat_dir(const char *a, const char *b)
{
	if (snprintf(file_buf, sizeof(file_buf), "%s/%s", a, b)
	    > sizeof(file_buf))
	{
		fputs("file path too long\n", stderr);
		exit(2);
	};
	return file_buf;
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
	while (sz--)
		free(ent[sz]);
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
		if (flags.colour) print_path(path, DIR_);
		else print(path);
		puts(" :");
		if (!show_dir(path) && flags.colour)
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
		if (flags.colour) {
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
		if (lstat(cat_dir(path, f[i].name), &s) < 0) return ;
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
	if (flags.show_owner) printf("%-*i ", 3, (int) f->user);
	if (flags.show_group) printf("%-*i ", 3, (int) f->group);
	if (S_ISCHR(f->mode) || S_ISBLK(f->mode))
		printf(" %*u, %*u ", 3, major(f->size), 3, minor(f->size));
	else
		printf("%*u ", (int) pad->size, (unsigned) f->size);
	if (flags.show_time) print_time(f->tim);
	if (flags.colour) print_name(f->mode, f->name);
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
	/* time is called only once */
	static time_t now = 0;
	struct tm *t;
	char str[16];
	if (!now) now = time(0);
	t = localtime(&sec);
	if ((now - sec) > HALF_YEAR)
		strftime(str, 16, "%b %d  %Y ", t);
	else
		strftime(str, 16, "%b %d %H:%M ", t);
	print(str);
}


char *
get_id(long long id, void *(*func)())
{
	char *str;
	if (!flags.num_ids && (str = func(id)) != NULL) {
		str = strdup(str);
		if (str == NULL) exit(1);
		return str;
	};
	str = malloc((sizeof(long long) * 3) + 2);
	if (str == NULL) exit(1);
	sprintf(str, "%lli", id);
	return str;
}


static void
print_type(mode_t mode)
{
#ifdef TYPE_PRINT
	if (flags.colour)
	    switch (mode & S_IFMT) {
		case S_IFDIR:  print(TYPE_DIR);  break;
		case S_IFCHR:  print(TYPE_CHR);  break;
		case S_IFBLK:  print(TYPE_BLK);  break;
		case S_IFREG:  print(TYPE_REG);  break;
		case S_IFIFO:  print(TYPE_FIFO); break;
		case S_IFLNK:  print(TYPE_LINK); break;
		case S_IFSOCK: print(TYPE_SOCK); break;
		default:       print(TYPE_NONE); break;
	}
	else
#endif
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
	if (!flags.colour) {
		print(mode);
	} else {
		for (i = 0; mode[i]; i++) {
			perm_color(mode[i], i);
			putchar(mode[i]);
		};
		print(NORMAL);
	};
	putchar(' ');
}




static inline size_t
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
		max += flags.colour ? 3 : 0;
		for (c = 0; sz--; str++) {
			if (flags.colour) {
				if (lstat(cat_dir(path, *str+off), &s) < 0)
					exit(3);
				i = print_name(s.st_mode, *str+off);
				for (; i < max; i++)  putchar(' ');
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
			if (flags.colour) {
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
