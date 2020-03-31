/*
 * if you uncomment TYPE_PRINT bls
 * will print char before file permissions in color
 */
#define TYPE_PRINT 1
#define TYPE_DIR  DIR_ "d" NORMAL
#define TYPE_CHR  CHR  "c" NORMAL
#define TYPE_BLK  BLK  "b" NORMAL
#define TYPE_REG  "\e[38;5;235m" "-" NORMAL
#define TYPE_FIFO "p"
#define TYPE_LINK "l"
#define TYPE_SOCK "s"
#define TYPE_NONE "?"

/*
 * should only print the color escape, not character itself
 * `col` is in range 0-8
 */
static void perm_color(char c, int col) {
	switch(c) {
	case 'r': print("\e[36m"); break;
	case 'w': // -->
	case 'x': print("\e[32m"); break;
	case '-': print("\e[38;5;235m"); break;
	};
}


#define NORMAL  "\e[0m"

/* file type colors */
#define CHR   "\e[33m"
#define BLK   "\e[33m"
#define FIFO  "\e[4;33m"
#define SOCK  "\e[4;35m"
#define DIR_  "\e[38;5;70m"
#define LINK  "\e[36m"
#define EXEC  "\e[38;5;46m" 

/* file extension match colors */
#define F_SOURCE     "\e[38;5;208m"
#define F_HEADER     "\e[38;5;130m"
#define F_DIFF       "\e[38;5;197m"
#define F_DOCUMENT   "\e[38;5;29m"
#define F_OBJECT     "\e[2m"
#define F_SCRIPT     "\e[9;32m"
#define F_DATA       "\e[38;5;221m"
#define F_PICTURE    "\e[38;5;199m"
#define F_SIGNATURE  "\e[31m"
#define F_ARCHIVE    "\e[1;31m"
#define F_BINARY     "\e[38;5;32m"
#define F_FONT       "\e[38;5;170m"
#define F_AUDIO      "\e[1;36m"
#define F_VIDEO      "\e[1;33m"
#define F_RICH_TEXT  "\e[1;38;5;58m"

/* file name match colors */
#define F_CONFIG     "\e[38;5;48m"
#define F_SOFTWARE   "\e[38;5;48m"




struct color_match {
	char *r; /* regex */
	char *c; /* color */
};

/*
 * the pattern matching only allows:
 *     ? - any character
 *     * - any number of any characters
 *     [ ] - matches any character in between the brackets
 *         - '^' doesn't have a special meaning
 *
 * ext   - matches an extension after the last '.'
 */
const struct color_match ext[] =
{
#ifdef F_SOURCE
	{ "[cfr]", F_SOURCE },
	{ "go"   , F_SOURCE },
	{ "asm"  , F_SOURCE },
	{ "py"   , F_SOURCE },
	{ "hs"   , F_SOURCE },
	{ "vim"  , F_SOURCE },
	{ "pl"   , F_SOURCE },
	{ "swift", F_SOURCE },
	{ "cpp"  , F_SOURCE },
	{ "c++"  , F_SOURCE },
	{ "sql"  , F_SOURCE },
	{ "js"   , F_SOURCE },
	{ "perl" , F_SOURCE },
	{ "php"  , F_SOURCE },
	{ "cs"   , F_SOURCE },
	{ "src"  , F_SOURCE },
	{ "bf"   , F_SOURCE },
	{ "lua"  , F_SOURCE },
	{ "sasf" , F_SOURCE },
	{ "[yl]" , "0;38;5;203" },
	{ "*mk"  , "0;38;5;48" },
#endif
#ifdef F_HEADER
	{ "[hp]", F_HEADER },
	{ "asi" , F_HEADER },
	{ "hpp" , F_HEADER },
	{ "css" , F_HEADER },
#endif
#ifdef F_DIFF
	{ "diff", F_DIFF },
	{ "path", F_DIFF },
#endif
#ifdef F_DOCUMENT
	{ "*html*", F_DOCUMENT },
	{ "xml"   , F_DOCUMENT },
	{ "ps"    , F_DOCUMENT },
	{ "md"    , F_DOCUMENT },
	{ "tex"   , F_DOCUMENT },
	{ "wiki"  , F_DOCUMENT },
	{ "1"     , F_DOCUMENT },
	{ "man"   , F_DOCUMENT },
	{ "info"  , F_DOCUMENT },
#endif
#ifdef F_OBJECT
	{ "o"   , F_OBJECT },
	{ "so"  , F_OBJECT },
	{ "pyc" , F_OBJECT },
	{ "dll" , F_OBJECT },
	{ "luac", F_OBJECT },
	{ "t*mp", F_OBJECT },
	{ "swp" , F_OBJECT },
	{ "part", F_OBJECT },
	{ "bak" , F_OBJECT },
	{ "out" , F_OBJECT },
	{ "log" , F_OBJECT },
#endif
#ifdef F_SCRIPT
	{ "bash", F_SCRIPT },
	{ "?sh" , F_SCRIPT },
	{ "sh"  , F_SCRIPT },
#endif
#ifdef F_DATA
	{ "[ct]sv", F_DATA },
	{ "json"  , F_DATA },
	{ "qry"   , F_DATA },
#endif
#ifdef F_PICTURE
	{ "png",      F_PICTURE },
	{ "jpg",      F_PICTURE },
	{ "gif",      F_PICTURE },
	{ "ff",       F_PICTURE },
	{ "bmp",      F_PICTURE },
	{ "tif*",     F_PICTURE },
	{ "svg*",     F_PICTURE },
	{ "ogm",      F_PICTURE },
	{ "tga",      F_PICTURE },
	{ "ico",      F_PICTURE },
	{ "xcf",      F_PICTURE },
#endif
#ifdef F_SIGNATURE
	{ "sig", F_SIGNATURE },
	{ "cer", F_SIGNATURE },
#endif
#ifdef F_ARCHIVE
	{ "a"     , F_ARCHIVE },
	{ "tar"   , F_ARCHIVE },
	{ "tgz"   , F_ARCHIVE },
	{ "deb"   , F_ARCHIVE },
	{ "zip"   , F_ARCHIVE },
	{ "?ar"   , F_ARCHIVE },
	{ "ar*"   , F_ARCHIVE },
	{ "taz"   , F_ARCHIVE },
	{ "z"     , F_ARCHIVE },
	{ "[gb]z*", F_ARCHIVE },
	{ "tbz*"  , F_ARCHIVE },
	{ "[xtl]z", F_ARCHIVE },
#endif
#ifdef F_BINARY
	{ "bin", F_BINARY },
	{ "dmg", F_BINARY },
	{ "iso", F_BINARY },
#endif
#ifdef F_FONT
	{ "fon"   , F_FONT },
	{ "[to]tf", F_FONT },
	{ "bdf"   , F_FONT },
#endif
#ifdef F_AUDIO
	{ "mp[23]", F_AUDIO },
	{ "mid*"  , F_AUDIO },
	{ "wav"   , F_AUDIO },
	{ "ogg"   , F_AUDIO },
	{ "mpa"   , F_AUDIO },
	{ "aif"   , F_AUDIO },
	{ "flac"  , F_AUDIO },
#endif
#ifdef F_VIDEO
	{ "mp[eg4]*", F_VIDEO },
	{ "avi",      F_VIDEO },
	{ "mov",      F_VIDEO },
	{ "nuv",      F_VIDEO },
	{ "asf",      F_VIDEO },
	{ "rm",       F_VIDEO },
	{ "flc",      F_VIDEO },
	{ "divx",     F_VIDEO },
	{ "vob" ,     F_VIDEO },
	{ "webm",     F_VIDEO },
#endif
#ifdef F_RICH_TEXT
	{ "do[ct]*", F_RICH_TEXT },
	{ "o[dt]*" , F_RICH_TEXT },
	{ "pdf"    , F_RICH_TEXT },
	{ "xls"    , F_RICH_TEXT },
	{ "epub"   , F_RICH_TEXT },
	{ "sc"     , F_RICH_TEXT },
#endif
};




/* 
 * fname - matches full name
 */
const struct color_match fname[] =
{
#ifdef F_CONFIG
	{ "*rc"   , F_CONFIG },
	{ "*conf*", F_CONFIG },
#endif
#ifdef F_SOFTWARE
	{ "Makefile", F_SOFTWARE },
	{ "README*" , F_SOFTWARE },
	{ "LICENSE" , F_SOFTWARE },
	{ "AUTHOR*" , F_SOFTWARE },
	{ "BUGS"    , F_SOFTWARE },
	{ "TODO"    , F_SOFTWARE },
#endif
};
