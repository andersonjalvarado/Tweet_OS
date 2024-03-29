#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <stdbool.h>
#include "Usuario.h"
#include "Tweet.h"
#include "Tweets.h"
#include "ExtraerArgumentos.h"

//Funciones
int Menu(int tipo);
int CrearPipeEscritura(char *pipe); 
int CrearPipeLectura(char *pipe);
void *mensajesTweet(void*);
void *OperacionCliente(void *a);

//variables globales
arg argumentos; ///< Estructura de los argumentos recibidos por consola
char mensaje[200];///< Mensaje del usuario que sigue
user cliente; ///< Estructura cliente
int tipo = 0; // Acoplado o Desacoplado
int fd,fd1;//file descriptor

//captura de señal
typedef void (*sighandler_t)(int);

sighandler_t signalHandler (void)
{
  if (tipo == 1)//Modo Desacoplado
    printf("\n \n");
    printf("\n    _________\n");
    printf("   %c_  ___   %c\n",47,92);
    printf("  %c@ %c%c@  %c   %c\n",47,92,47,92,92);
    printf("  %c__%c%c___%c   %c\n",92,47,92,47,47);
    printf("   %c_%c%c______%c\n",92,92,47,47);
    printf("   %c     %c%c%c%c%c%c    Un usuario que sigues\n",47,47,92,92,92,92,92);
    printf("  |     |%c%c%c%c%c%c   publico un nuevo Tweet \n",92,92,92,92,92,92);
    printf("   %c     %c%c%c%c%c%c\n",92,92,92,92,92,92,92);
    printf("    %c______%c%c%c%c%c\n",92,47,92,92,92,92);
  //  printf ("\nUn usuario que sigues publico un nuevo Tweet \n");
  if(tipo == 0)//Modo Acoplado
  {
    read(fd1, mensaje, 200);
    printf("\n \n");
    printf("\nNuevo Tweet:\n");
    printf("\n    _________\n");
    printf("   %c_  ___   %c\n",47,92);
    printf("  %c@ %c%c@  %c   %c\n",47,92,47,92,92);
    printf("  %c__%c%c___%c   %c\n",92,47,92,47,47);
    printf("   %c_%c%c______%c\n",92,92,47,47);
    printf("   %c     %c%c%c%c%c%c\n",47,47,92,92,92,92,92);
    printf("  |     |%c%c%c%c%c%c   %s \n",92,92,92,92,92,92,mensaje);
    printf("   %c     %c%c%c%c%c%c\n",92,92,92,92,92,92,92);
    printf("    %c______%c%c%c%c%c\n",92,47,92,92,92,92);
    close(fd1);
  }
}

int main(int argc, char **argv)
{
    //verificar argumentos
    if(argc!=5) 
    {
        printf("Argumentos incorrectos, formato correcto: $./Cliente -i ID -p pipeNom");
        exit(1);
    }
    else
      argumentos = extraerArgumentos(argc, argv);

    signal (SIGUSR1,(sighandler_t) signalHandler);//señal 
     //variables
    int salir = 0;
    int peticion = 0;
    int opcion;
   //variables pipes
    //int  fd, fd1;
    mode_t fifo_mode = S_IRUSR | S_IWUSR;

    fd = CrearPipeEscritura(argumentos.pipeNom);
    cliente.id = argumentos.id;
    cliente.enLinea = 1;
    sprintf(cliente.pipe, "pipe%d", cliente.id);
    unlink(cliente.pipe);

    //creacion del pipe
    if (mkfifo (cliente.pipe, fifo_mode) == -1) 
    {
      perror("Client  mkfifo");
      exit(1);
    }

    int w = write(fd, &peticion, sizeof(int));
    if (w == -1)
    {
      perror("proceso escritor:");
      exit(1);
    }
    int gPid = getpid();// pid del proceso
    char pid[30];
    //  printf("\n\n pid: %d\n",pid);
    //enviar el PID
    sprintf(pid, "pid%d", gPid);
    w = write(fd, pid, 30);
    if (w == -1)
    {
      perror("proceso escritor:");
      exit(1);
    }
  //enviar la estructura cliente
    w = write(fd, &cliente, sizeof(cliente));
    if (w == -1)
    {
      perror("proceso escritor:");
      exit(1);
    }
    //leer datos enviados por el gestor
    tws tweetsAlmacenados;
    int respuesta; 
    fd1 = CrearPipeLectura(cliente.pipe);
    
    int r = read(fd1, &tweetsAlmacenados, sizeof(tweetsAlmacenados));
    if (r == -1)
    {
      perror("proceso lector");
      exit(1);
    }
    r = read(fd1, &respuesta, sizeof(respuesta));
    if (r == -1)
    {
      perror("proceso lector");
      exit(1);
    }
    r = read(fd1, &tipo, sizeof(tipo));
    if (r == -1)
    {
      perror("proceso lector:");
      exit(1);
    }
    // Se lee un mensaje por el segundo pipe.
    printf("Respuesta: %d, Tipo: %d \n", respuesta, tipo);
    char salida[20];//Mensaje de Salida enviado por gestor
    char solicitud[10];
    int i; //iterador
    if(respuesta)
    {     
    printf("\nCantidad de Tweets: %d \n",tweetsAlmacenados.cantTweets);   //mensajes recuperados mientras estaba desconectado
      if(tweetsAlmacenados.cantTweets > 0)
      {
          printf("\nTweets Recuperados: \n");
          for (i = 0; i < tweetsAlmacenados.cantTweets; i++)
          {
            
            
            printf("id %d tweet: %s\n",  tweetsAlmacenados.tweets[i].id, tweetsAlmacenados.tweets[i].mensaje);

            
          }
      }
      else
        printf("\n\n Tus seguidores no han enviado Tweets mientras estabas desconectado\n");
      
      do{
        opcion = Menu(tipo);
        if(tipo==0 && opcion==5)
          opcion=-1;
        if(tipo==0 && opcion==4)
          opcion=5;
          
        switch(opcion)
        {              
          case 1: int follow;
                  printf("\n\n Digite id del usuario que desea seguir\n");
                  scanf("%d",&follow);
                  sprintf(solicitud, "%d,%d", cliente.id, follow);
                  write(fd,&opcion,sizeof(int));//Envio de la opcion "Opcion 1(seguir)"
                  write(fd, solicitud, 10);//Envio de la solicitud

                  fd1 = CrearPipeLectura(cliente.pipe);//Abrimos un pipe lectura con la respuesta
                  r = read(fd1,salida,20);//Se lee el mensaje de salida
                  if (r == -1)
                  {
                      perror("proceso lector:");
                      exit(1);
                  }
                  printf("\n %s",salida);//Puede ser mensaje exitoso o fallido
                  close(fd1);        
                  break;
          case 2: int unfollow;
                  printf("\n\n Digite id del usuario que desea dejar de seguir\n");
                  scanf("%d",&unfollow);
                  sprintf(solicitud, "%d,%d", cliente.id, unfollow);
                  write(fd,&opcion,sizeof(int));//Envio de la opcion "Opcion 2(dejar de seguir)"
                  write(fd, solicitud, 10);//Envio de la solicitud

                  fd1 = CrearPipeLectura(cliente.pipe);//pipe lectura con la respuesta
                  r = read(fd1,salida,20);//Se lee el mensaje de salida
                  if (r == -1)
                  {
                      perror("proceso lector:");
                      exit(1);
                  }
                  printf("\n %s",salida);//Puede ser mensaje exitoso o fallido
                  close(fd1); 
                  break;
          case 3:	tw tweet;
                  printf("\n\n Digite el tweet que desea publicar: \n");
                  fgetc(stdin);
                  if(fgets(tweet.mensaje,200,stdin)==NULL)
                  {
                    break;
                  }
                  printf("mensaje: %s", tweet.mensaje);
                  tweet.id = cliente.id;
                  write(fd,&opcion,sizeof(int));//Envio de la opcion "Opcion 3(Publicar tweet)"
                  write(fd,&tweet,sizeof(tweet));// enviar tweet                 
                  break;
          case 4:	char recuperar[6];
                  user clienteRecuperado;          
                  sprintf(recuperar, "id,%d", cliente.id);
                  write(fd,&opcion,sizeof(int));//Envio de la opcion "Opcion 4(Recuperar tweet)"
                  write(fd, recuperar, 6);//Envio de la solicitud
                  fd1 = CrearPipeLectura(cliente.pipe);//pipe lectura con la respuesta
                  printf("\n\n Recuperando Tweets... \n");
                  r = read(fd1,&clienteRecuperado,sizeof(clienteRecuperado));
                  if (r == -1)
                  {
                      perror("proceso lector:");
                      exit(1);
                  }
                  printf("\n");
                  if(clienteRecuperado.cantTweets > 0)
                      for (i = 0; i < clienteRecuperado.cantTweets; i++)
                        printf("\nid %d tweet: %s\n",clienteRecuperado.tweets[i].id, clienteRecuperado.tweets[i].mensaje);
                  else
                       printf("\nNo hay mensajes\n");
                  
                  close(fd1);
                  break;
          case 5: char buffer3[3];
                  sprintf(buffer3, "%d", cliente.id);
                  write(fd,&opcion,sizeof(int));//Envio de la opcion "Opcion 5(Salir)"
                  write(fd, buffer3, 3);
                  printf("\n\n\t VUELVA PRONTO!\n\n");
                   return 0;
                  break;
          default:printf("\n\n\t OPCION INVALIDA! Intente de nuevo\n\n");
                  break;
        }
      }while(!salir);
    }
    else
      printf("Proceso NO exitoso\n");
    
    return 0;
}



/// @brief Funcion para Mostrar el menu de opciones para el usuario
/// @param tipo modo acoplado o desacoplado
/// @param return opcion elegida por el usuario 
int Menu(int tipo)
{
  int opc;
	printf("\n\n\t\t-----------------------------------");
	printf("\n\t\t\tMENU PRINCIPAL");
	printf("\n\t\t-----------------------------------");
	printf("\n\t\t 1.Follow");
	printf("\n\t\t 2.Unfollow");
	printf("\n\t\t 3.Publicar un Tweet");
  if(tipo==0)
    printf("\n\t\t 4.Desconectarse");
  if(tipo==1)
  {
    printf("\n\t\t 4.Recuperar Tweets");
    printf("\n\t\t 5.Desconectarse");
  }
	printf("\n\n\t\t-----------------------------------");
	printf("\n\n\n\t\tDigite la opcion: ");
	scanf("%d", &opc);
	return opc;
}

/// @brief Funcion para Abrir y escribir en el pipe
/// @param pipe Nombre del pipe
/// @param return file descriptor 
int CrearPipeEscritura(char *pipe) {

  int i, creado, fd;
  do 
  { 
    fd = open(pipe, O_WRONLY);
    if (fd == -1) 
    {
      perror("pipe");
      printf(" Se volvera a intentar despues\n");
      sleep(10);       
    } else creado = 1;
  } while (creado == 0);

  printf ("Abrio el pipe\n");

  return fd;
}
/// @brief Funcion para Abrir y leer en el pipe
/// @param pipe Nombre del pipe
/// @param return file descriptor 
int CrearPipeLectura(char *pipe) {

  int i, creado, fd;
  do 
  { 
    fd = open(pipe, O_RDONLY);
    if (fd == -1) 
    {
      perror("pipe");
      printf(" Se volvera a intentar despues\n");
    } else creado = 1;
  } while (creado == 0);

  printf ("Abrio el pipe\n");
  return fd;
}