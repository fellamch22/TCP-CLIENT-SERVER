CC=gcc
CFLAGS=-Wall -g

ALL= serveur client1 client2
all: $(ALL)

serveur : serveur.o 
	$(CC) $^ $(LDLIBS) -o $@ -pthread
client1 : client1.o 
	$(CC) $^ $(LDLIBS) -o $@ -pthread
client2 : client2.o 
	$(CC) $^ $(LDLIBS) -o $@ -pthread

client2.o : client2.c
client1.o : client1.c 
serveur.o : serveur.c 

clean:
	rm -rf $(ALL)
