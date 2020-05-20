CC = clang
# CC = tcc

#tls: main.c show.o *.h
#	$(CC) -o $@ main.c show.o

#show.o: *.h

tls: new.c config.h
	$(CC) -o $@ $<


.PHONY: clean install

clean:
	rm -f *.o

install: tls
	mkdir -p  /usr/local/bin/
	cp -f tls /usr/local/bin/
	chmod 755 /usr/local/bin/tls
