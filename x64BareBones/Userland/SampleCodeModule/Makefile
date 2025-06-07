include ../Makefile.inc

MODULE=0000-sampleCodeModule.bin
SOURCES=$(wildcard [^_]*.c) Shell/shell.c
ASM_SRCS=$(wildcard *.asm)
ASM_OBJS=$(ASM_SRCS:.asm=.o)

all: $(MODULE)

%.o: %.asm
	$(ASM) $(ASMFLAGS) -f elf64 $< -o $@          # ← [1] Para compilar syscalls.asm

$(MODULE): $(SOURCES) $(ASM_OBJS)
	$(GCC) $(GCCFLAGS) -T sampleCodeModule.ld _loader.c $(SOURCES) $(ASM_OBJS) -o ../$(MODULE)

clean:
	rm -rf *.o
	rm -f ../$(MODULE)

.PHONY: all clean print
