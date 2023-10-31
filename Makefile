.phony all:
all: mts

mts: mts.c
	gcc -g mts.c -lhistory -Wall -pthread -o mts linkedlist.c

.PHONY clean:
clean: 
	-rm -rf *.o *.exe