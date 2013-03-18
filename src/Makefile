### Makefile --- 

## Author: liutos@liutos-laptop
## Version: $Id: Makefile,v 0.0 2013/03/13 14:18:00 liutos Exp $
## Keywords: 
## X-URL: 

CC=cc
CFLAGS=-Wall -g -std=c99 -D_GNU_SOURCE
CPPFLAGS=-Iinclude

read.o: read.c include/types.h include/object.h

eval.o: eval.c include/types.h include/object.h

write.o: write.c include/types.h

main.o: main.c include/write.h include/eval.h include/read.h include/object.h

liutscm: main.o read.o eval.o write.o object.o proc.o
	$(CC) $(CFLAGS) $^ -o $@

test.o: test.c include/write.h include/eval.h include/read.h include/object.h

run-test: test.o read.o eval.o write.o object.o proc.o
	$(CC) $(CFLAGS) $^ -o $@

object.o: object.c include/types.h

proc.o: proc.c include/types.h include/object.h

compile.o: compile.c include/types.h include/object.h include/eval.h

test-compiler.o: test-compiler.c include/types.h include/object.h include/compile.h include/eval.h

run-compile-test: test-compiler.o object.o read.o write.o compile.o proc.o eval.o
	$(CC) $(CFLAGS) $^ -o $@

.PHONY: clean

clean:
	rm -f *.o
	if [ -f liutscm ]; then rm liutscm; fi
	if [ -f run-test ]; then rm run-test; fi

### Makefile ends here
