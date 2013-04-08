### Makefile --- 

## Author: liutos@liutos-laptop
## Version: $Id: Makefile,v 0.0 2013/03/13 14:18:00 liutos Exp $
## Keywords: 
## X-URL: 

CC=gcc
CFLAGS=-Wall -g -std=c99 -D_GNU_SOURCE
CPPFLAGS=-Iinclude

OBJS=\
assembler.o\
compiler.o\
eval.o\
init.o\
object.o\
proc.o\
read.o\
vm.o\
write.o

# Object files

assembler.o: assembler.c include/assembler.h include/object.h include/types.h include/write.h

eval.o: eval.c include/types.h include/object.h

init.o: init.c include/object.h include/read.h

read.o: read.c include/types.h include/object.h

write.o: write.c include/types.h

main.o: main.c include/write.h include/eval.h include/read.h include/object.h include/init.h

object.o: object.c include/types.h

proc.o: proc.c include/types.h include/object.h

compiler.o: compiler.c include/types.h include/object.h include/eval.h include/compiler.h

vm.o: vm.c include/assembler.h include/object.h include/types.h

# Tests

test-repl.o: test-repl.c include/write.h include/eval.h include/read.h include/object.h include/init.h

test-compiler.o: test-compiler.c include/types.h include/object.h include/compiler.h include/eval.h include/init.h

test-vm.o: test-vm.c include/types.h include/object.h include/compiler.h include/eval.h include/read.h include/init.h

test-asm.o: test-asm.c include/types.h include/object.h include/compiler.h include/eval.h include/read.h include/init.h

# Executables

liutscm: main.o $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

run-repl-test: test-repl.o $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

run-compiler-test: test-compiler.o $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

run-vm-test: test-vm.o $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

run-asm-test: test-asm.o $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

.PHONY: clean

clean:
	rm -f *.o
	if [ -f liutscm ]; then rm liutscm; fi
	if [ -f run-repl-test ]; then rm run-repl-test; fi
	if [ -f run-compiler-test ]; then rm run-compiler-test; fi
	if [ -f run-vm-test ]; then rm run-vm-test; fi
	if [ -f run-asm-test ]; then rm run-asm-test; fi

### Makefile ends here
