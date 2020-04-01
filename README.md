tls - tiny-ls file listing

## options
	- 1  force one file per line
	- C  print columns
	- a  show all files
	- A  filter `.` and `..`
	- G  show color, indicators, and links
	- f  no sort
	- r  reverse sort
	- V  sort argv
	- l  long listing
	- n  * show numerical IDs
	- g  * show group
	- o  * show owner
	- t  * show mod time
> `*` options only effect long listing


## default behavior

the -C flag is set if output is a terminal.

links in the curent directory are expandet into
(name -> link) pair, this is turned off
after the first error (link is not in current directory)
