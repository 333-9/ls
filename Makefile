#CC = myclang

.PHONY: all
all: ls tree


ls: main.c config.h
	$(CC) -o $@ main.c

tree: tree.c config.h
	$(CC) -o $@ tree.c


#---------------------------------------

.PHONY: clean install

clean:
	rm -f *.o
	rm -f ls
	rm -f tree

install:
	mkdir -p   /usr/local/bin/
	[ -e ls   ] && cp -f ls   /usr/local/bin/
	[ -e tree ] && cp -f tree /usr/local/bin/

