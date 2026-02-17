
CFLAGS+=-g2

all: bin/cfg-test bin/macro-test bin/glob-test

bin/macro-test: tests/macro-test.c macro-tools.h
	mkdir -p bin
	gcc $(CFLAGS) -o $@ $<

bin/cfg-test: tests/cfg-test.c cfgparser.h
	mkdir -p bin
	gcc $(CFLAGS) -o $@ $<

bin/glob-test: tests/glob-test.c glob.h
	mkdir -p bin
	gcc $(CFLAGS) -o $@ $<

clean:
	rm -rf bin
	
.PHONY: clean all