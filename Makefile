CC = clang
# CC = tcc


tls: main.c config.h
	$(CC) -o $@ main.c


.PHONY: clean install

clean:
	rm -f *.o
	rm -f tls

install: tls
	mkdir -p  /usr/local/bin/
	cp -f tls /usr/local/bin/
	chmod 755 /usr/local/bin/tls
