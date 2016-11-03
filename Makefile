C=gcc
CFLAGS=-O -Wall -Wno-unused-result -static -static-libstdc++ -static-libgcc -pthread

all: dc32 dc64

dc32: dc.c pl32.c
	$(C) $(CFLAGS) -m32 $^ -o $@
	strip dc32

dc64: dc.c pl64.c
	$(C) $(CFLAGS) $^ -o $@
	strip dc64

clean:
	- rm -f dc32 dc64