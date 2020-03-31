#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


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
