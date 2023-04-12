CFLAGS=-std=c99 -Wall -pedantic
all: utf8check
utf8check: utf8check.c utf8check.h
	$(CC) utf8check.c -o $@
clean:
	rm -f utf8check
