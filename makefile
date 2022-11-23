all: Cliente Gestor

server: gestor.o usuario.h ExtraerArgumentos.h Tweet.h Tweets.h
	gcc -o gestor gestor.o -lpthread

server.o: Gestor.c usuario.h ExtraerArgumentos.h Tweet.h Tweets.h
	gcc -c Gestor.c

Client: cliente.o usuario.h ExtraerArgumentos.h Tweet.h Tweets.h
	gcc -o cliente cliente.o -lpthread

Client.o: cliente.c usuario.h ExtraerArgumentos.h Tweet.h Tweets.h
	gcc -c client.c
