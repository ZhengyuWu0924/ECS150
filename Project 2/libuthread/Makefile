# Target library
lib := libuthread.a

all: $(lib)

## TODO: Phase 1
$(lib): queue.o uthread.o context.o private.o
	ar rcs $(lib) queue.o uthread.o context.o private.o

queue.o: queue.c 
	gcc  -Wall -Werror -c queue.c -o queue.o

uthread.o: uthread.c
	gcc -Wall -Werror -c uthread.c -o uthread.o
	
context.o: context.c
	gcc -Wall -Werror -c context.c -o context.o
private.o: private.h
	gcc -Wall -Werror -c private.h -o private.o




