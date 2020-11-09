# Target library
lib := libuthread.a

all: $(lib)

## TODO: Phase 1
$(lib): queue.o uthread.o
	ar rcs $(lib) queue.o uthread.o

queue.o: queue.c 
	gcc  -Wall -Werror -c queue.c -o queue.o

uthread.o: uthread.c
	gcc -Wall -Werror -c uthread.c -o uthread.o






