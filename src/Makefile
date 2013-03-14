### Makefile --- 

## Author: liutos@liutos-laptop
## Version: $Id: Makefile,v 0.0 2013/03/13 14:18:00 liutos Exp $
## Keywords: 
## X-URL: 

CC=cc
CFLAGS=-Wall -g -std=c99 -D_GNU_SOURCE
CPPFLAGS=-Iinclude

read.o: read.c include/types.h

eval.o: eval.c include/types.h

write.o: write.c include/types.h

main.o: main.c include/write.h include/eval.h include/read.h

liutscm: main.o read.o eval.o write.o
	$(CC) $(CFLAGS) $^ -o $@

test.o: test.c include/write.h include/eval.h include/read.h

run-test: test.o read.o eval.o write.o
	$(CC) $(CFLAGS) $^ -o $@

.PHONY: clean

clean:
	rm -f *.o
	rm liutscm
	rm run-test

### Makefile ends here
