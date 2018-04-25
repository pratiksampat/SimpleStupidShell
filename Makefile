CC=gcc
CFLAGS=-I.
DEPS = shell.h

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

shell_main: shell_main.o shell.o 
	gcc -o shell shell_main.o shell.o -I.
	gcc -o host host.c -lpthread
	gcc -o client client.c

clean: 
	rm *.o
