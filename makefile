all: Cliente Gestor

server: gestor.o usuario.h ExtraerArgumentos.h Tweet.h
	gcc -o gestor gestor.o -lpthread

server.o: Gestor.c usuario.h ExtraerArgumentos.h Tweet.h
	gcc -c Gestor.c

client: cliente.o usuario.h ExtraerArgumentos.h Tweet.h
	gcc -o cliente cliente.o

client.o: Cliente.c usuario.h ExtraerArgumentos.h Tweet.h
	gcc -c Client.c
