/* file type colors */
const char *type_color[] = {
    [Dir] = "\e[38;5;70m",
    [Chr] = "\e[38;5;88m",
    [Blk] = "\e[38;5;88m",
    [Reg] = "\e[38;5;46m",  // executable
    [Fif] = "\e[4;33m",
    [Lnk] = "\e[36m",
    [Soc] = "\e[4;33m",
    [Nol] = "\e[0m",
};

/* file type markers */
const char marker[][8] = {
    [Dir] = " >",
    [Chr] = "'",
    [Blk] = " #",
    [Reg] = " *", // executable
    [Fif] = " |",
    [Lnk] = " ->",
    [Soc] = " |",
    [Nol] = "",
};

/* file type char */
const char *type_char[] = {
    [Dir] = "\e[32m" "d",
    [Chr] = "\e[31m" "c",
    [Blk] = "\e[31m" "b",
    [Reg] = " ",
    [Fif] = "p",
    [Lnk] = "l",
    [Soc] = "s",
    [Nol] = "?",
};

/* permission bits colors */
const char *perm_char[] = {
    [Read]  = "\e[31m",
    [Write] = "\e[32m",
    [Exec]  = "\e[32m",
    [False] = "\e[38;5;235m",
};


/* file extension match colors */
char *color_codes[020] =
{
    [Archive]   = "\e[1;31m",
    [Audio]     = "\e[1;36m",
    [Config]    = "\e[38;5;42m",
    [Data]      = "\e[38;5;221m",
    [Document]  = "\e[38;5;29m",
    [Font]      = "\e[38;5;170m",
    [Headers]   = "\e[38;5;130m",
    [Object]    = "\e[2m",
    [Patch]     = "\e[38;5;197m",
    [Picture]   = "\e[38;5;199m",
    [RichText]  = "\e[1;38;5;58m",
    [Signature] = "\e[31m",
    [Software]  = "\e[38;5;32m",
    [Source]    = "\e[38;5;208m",
    [Video]     = "\e[1;33m",
    [Other]     = "",
};


/*
 * Warning: pattern string can't exceed 13 characters !!!
 *
 * pattern matching:
 *     ? - any character
 *     * - any number of any characters
 *     [ ] - matches any character in between the brackets
 *         - '^' does not have a special meaning
 */

/* matches the last file extension  */
const struct color_match  ext[] =
{
    { "[cfr]",      Source },
    { "go",         Source },
    { "asm",        Source },
    { "py",         Source },
    { "hs",         Source },
    { "vim",        Source },
    { "pl",         Source },
    { "swift",      Source },
    { "c[p+][p+]",  Source },
    { "sql",        Source },
    { "js",         Source },
    { "perl",       Source },
    { "php",        Source },
    { "cs",         Source },
    { "src",        Source },
    { "bf",         Source },
    { "lua",        Source },
    { "sasf",       Source },
    { "[yl]",       Source },
    { "*mk",        Source },
    { "s",          Headers },
    { "[hp]",       Headers },
    { "asi",        Headers },
    { "hpp",        Headers },
    { "css",        Headers },
    { "def",        Headers },
    { "diff",       Patch },
    { "path",       Patch },
    { "*html*",     Document },
    { "xml",        Document },
    { "ps",         Document },
    { "md",         Document },
    { "tex",        Document },
    { "wiki",       Document },
    { "[12345678]", Document },
    { "man",        Document },
    { "info",       Document },
    { "o",          Object },
    { "so",         Object },
    { "pyc",        Object },
    { "dll",        Object },
    { "luac",       Object },
    { "t*mp",       Object },
    { "swp",        Object },
    { "part",       Object },
    { "bak",        Object },
    { "out",        Object },
    { "log",        Object },
    { "bin",        Object },
    { "dmg",        Object },
    { "iso",        Object },
    { "[ct]sv",     Data },
    { "json",       Data },
    { "yaml",       Data },
    { "yml",        Data },
    { "qry",        Data },
    { "png",        Picture },
    { "jpg",        Picture },
    { "gif",        Picture },
    { "ff",         Picture },
    { "bmp",        Picture },
    { "tif*",       Picture },
    { "svg*",       Picture },
    { "ogm",        Picture },
    { "tga",        Picture },
    { "ico",        Picture },
    { "xcf",        Picture },
    { "sig",        Signature },
    { "cer",        Signature },
    { "a",          Archive },
    { "tar",        Archive },
    { "tgz",        Archive },
    { "deb",        Archive },
    { "zip",        Archive },
    { "?ar",        Archive },
    { "ar*",        Archive },
    { "taz",        Archive },
    { "z",          Archive },
    { "[gb]z*",     Archive },
    { "tbz*",       Archive },
    { "[xtl]z",     Archive },
    { "fon",        Font },
    { "[to]tf",     Font },
    { "bdf",        Font },
    { "mp[23]",     Audio },
    { "mid*",       Audio },
    { "wav",        Audio },
    { "ogg",        Audio },
    { "mpa",        Audio },
    { "aif",        Audio },
    { "flac",       Audio },
    { "mp[eg4]*",   Video },
    { "avi",        Video },
    { "mov",        Video },
    { "nuv",        Video },
    { "asf",        Video },
    { "rm",         Video },
    { "flc",        Video },
    { "divx",       Video },
    { "vob",        Video },
    { "webm",       Video },
    { "do[ct]*",    RichText },
    { "o[dt]*",     RichText },
    { "pdf",        RichText },
    { "xls",        RichText },
    { "epub",       RichText },
    { "sc",         RichText },
};


/* matches full name */
const struct color_match  fname[] =
{
    { "tags",     Data },
    { "*rc",      Config },
    { "*conf*",   Config },
    { "Makefile", Software },
    { "README*",  Software },
    { "LICENSE",  Software },
    { "COPYRIGHT",Software },
    { "AUTHOR*",  Software },
    { "BUGS",     Software },
    { "TODO",     Software },
    { "VERSION",  Software },
    { "FAQ",      Software },
    { "INSTALL",  Software },
    { "CHANGE*",  Software },
};
