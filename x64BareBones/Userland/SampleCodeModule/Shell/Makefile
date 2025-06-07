include ../Makefile.inc

MODULE=0002-shell.bin
SOURCES=$(wildcard [^_]*.c)

all: $(MODULE) shell.o

# Regla para compilar shell.c a shell.o, que SampleCodeModule puede enlazar
shell.o: shell.c ../include/shell.h
	$(GCC) $(GCCFLAGS) -I../include -c shell.c -o shell.o

$(MODULE): $(SOURCES)
	$(GCC) $(GCCFLAGS) -I../include -T shell.ld $(SOURCES) -o ../$(MODULE)

clean:
	rm -rf *.o shell.o

.PHONY: all clean