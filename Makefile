CC ?= cc
CFLAGS ?= -g -Wall -O2

all:
	$(CC) $(CFLAGS) -o memfs memfs.c `pkg-config fuse --cflags --libs`

check: all
	$(MAKE) -C tests check
