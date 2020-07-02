#CC = clang
CC = tcc


tls: main.c config.h
	$(CC) -o $@ main.c

tree: tree.c config.h
	$(CC) -o $@ tree.c


.PHONY: clean install

clean:
	rm -f *.o
	rm -f tls
	rm -f tree

install: tls tree
	mkdir -p   /usr/local/bin/
	cp -f tls  /usr/local/bin/
	if [ -x tree ]; then cp -f tree /usr/local/bin/; fi
	chmod 755  /usr/local/bin/tls
	chmod 755  /usr/local/bin/tree
