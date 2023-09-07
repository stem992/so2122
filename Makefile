CC = -gcc
CFLAGS := -std=c89 -pedantic -g -D_DEBUG=0 -D_GNU_SOURCE -fno-stack-protector

all: master node user

master: master.o sem.o
	gcc master.o sem.o -o master

node: node.o sem.o
	gcc node.o sem.o -o node

user: user.o sem.o
	gcc user.o sem.o -o user

master.o: src/master.c sem.o
	gcc -c $(CFLAGS) $(OPTIONS) src/master.c 
	
node.o: src/node.c include/node.h sem.o
	gcc -c $(CFLAGS) src/node.c

user.o: src/user.c include/user.h sem.o
	gcc -c $(CFLAGS) src/user.c

sem.o: src/sem.c include/sem.h
	gcc -c $(CFLAGS) src/sem.c

clean:
	rm -f *.o
	rm master
	rm node
	rm user