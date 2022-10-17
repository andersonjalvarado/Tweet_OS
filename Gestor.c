#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include "Usuario.h"
#define MAX_NUM 80 // maximo numero de usuarios

//Variables compartidas
int usuarios[MAX_NUM][MAX_NUM];
user users[MAX_NUM];
int t, contUsuarios = 0;
typedef struct Datos //estructuras con el nombre del pipe principal y el numero de usuarios conectados
{
    int num;
    char nomPipe[20]; 
} data;

//captura de Signal
typedef void (*sighandler_t)(int);

sighandler_t signalHandler(void)
{
  printf ("Cantidad de usuarios: %d\n", contUsuarios);
  alarm(t);
}

//Funciones
int CrearPipeLectura(char *pipe);
int CrearPipeEscritura(char *pipe);
void *OperacionGestor(data *d);
void *Estadisticas(int*); 

int main(int argc,char **argv)
{    
    //Verificar argumentos
    if(argc!=11) 
    {
        printf("Formato debe ser $./Gestor -n Num -r Relaciones -m modo -t time -p pipeNom");
        exit(1);
    }

    // limitar la invocación del gestor a solo dos modos
    if ((strcmp(argv[6],"A")==1) || (strcmp(argv[6],"D")==1))
    {
        printf("modo: 'A' Para acoplado o 'D' para desacoplado\n");
        exit(1);
    }

    //variables para leer archivo
    int num = atoi(argv[2]);  //numero de usuarios
    t = atoi(argv[8]); // time para estadisticas
    data d;
    d.num = num;
    strcpy(d.nomPipe,argv[10]);
    //int usuarios[num][num];
    int i=0,j;

    FILE *relaciones = fopen(argv[4],"r");
    
    if (relaciones == NULL)
    {
        printf("El archivo no pudo abrirse, vuelva a Intentarlo");
        exit(1);
    }
    while (!feof(relaciones))
    {    
        for(j=0; j<num; j++)
        {
            fscanf(relaciones,"%i ",&usuarios[i][j]);
        }
        j++; 
        fscanf(relaciones,"\n");            
    }
    fclose(relaciones);
    
    //user users[num];

    for(i=0; i<num; i++)
    {
        users[i].id = i+1;
        users[i].enLinea = 0;
        sprintf(users[i].pipe, "pipe%d", users[i].id);
    }

    //utilizar Hilos y Signals
    pthread_t thread1, thread2;
    signal (SIGALRM, (sighandler_t)signalHandler);
    
    if (pthread_create(&thread1, NULL, (void*)OperacionGestor,(void*)&d)) 
    {
         perror("Error creando hilo 1");
         exit(-1);
    }
    if (pthread_create(&thread2, NULL, (void*)Estadisticas,(void*)&t)) 
    {
         perror("Error creando hilo 2");
         exit(-1);
    }
    pthread_join(thread1,NULL);
    pthread_join(thread2,NULL);



    /*//Variables para el pipe
    int fd, fd1, p;
    user u;
    mode_t fifo_mode = S_IRUSR|S_IWUSR;
    
    //creacion del pipe
    if (mkfifo(argv[10], fifo_mode) == -1)
    {
        perror("Server mkfifo");
        exit(1);
    }
    fd = open(argv[10],O_RDONLY);
    
    int bn;
    int respuesta;
    while(1)
    {
      p = read (fd, &u, sizeof(u));
      if (p == -1)
      {
          perror("proceso lector:");
          exit(1);
      }
      else{
          printf("Funciono %d", u.id);
      }
      //  printf("Funciono, pipe desde gestor: %s\n",u.pipe );
      //verificar usuario que llega por el pipe

      bn = 0;
      for(i=0; i<num; i++)
      {
          //verificar que el usuario exista y no este en linea
          if(users[i].id == u.id && users[i].enLinea == 0)
          {
              users[i].enLinea = u.enLinea;
              strcpy(users[i].pipe, u.pipe);
              bn = 1;
              break;
          }
      }

      //pipe de respuesta
      
      fd1 = CrearPipeEscritura(u.pipe);
      if(bn)
      {
          respuesta = 1;
          write (fd1, &respuesta, sizeof(respuesta));
      }
      else
      {
          respuesta = 0;
          write (fd1, &respuesta, sizeof(respuesta));
      }
      unlink(u.pipe);
    }*/
    return 0;
}
/*Nombre: OperacionGestor 
  Entradas: Estructuras de datos con el nombre del pipe principal y la cantidad de usuarios
  Objetivo: Realizar todas las operaciones del gestor
  Salida: 
*/
void *OperacionGestor(data *d)
{
  //Variables para el pipe
    int fd, fd1, p,i;
    user u;
    mode_t fifo_mode = S_IRUSR|S_IWUSR;
    
    //creacion del pipe
    if (mkfifo(d->nomPipe, fifo_mode) == -1)
    {
        perror("Server mkfifo");
        exit(1);
    }
    fd = open(d->nomPipe,O_RDONLY);
    
    int bn;
    int respuesta;
    while(1)
    {
      p = read (fd, &u, sizeof(u));
      if (p == -1)
      {
          perror("proceso lector:");
          exit(1);
      }
      else{
          printf("Funciono %d", u.id);
      }
      //  printf("Funciono, pipe desde gestor: %s\n",u.pipe );
      //verificar usuario que llega por el pipe

      bn = 0;
      for(i=0; i<d->num; i++)
      {
          //verificar que el usuario exista y no este en linea
          if(users[i].id == u.id && users[i].enLinea == 0)
          {
              users[i].enLinea = u.enLinea;
              strcpy(users[i].pipe, u.pipe);
              bn = 1;
              contUsuarios++;
              break;
          }
      }

      //pipe de respuesta
      
      fd1 = CrearPipeEscritura(u.pipe);
      if(bn)
      {
          respuesta = 1;
          write (fd1, &respuesta, sizeof(respuesta));
      }
      else
      {
          respuesta = 0;
          write (fd1, &respuesta, sizeof(respuesta));
      }
      unlink(u.pipe);
    }
}
/*Nombre: Estadisticas 
  Entradas: tiempo que espera para enviar la signal
  Objetivo: Enviar una signal para imprimir las estadisticas de usuarios conectastos y tweets enviados
  Salida: 
*/
void *Estadisticas(int *t)
{
  while(1)
  {
    alarm(*t);
    pause();
  }
}
/*Nombre: CrearPipe 
  Entradas: Nombre del pipe
  Objetivo: Abrir y leer el pipe 
  Salida: El fd del pipe
*/
int CrearPipeLectura(char *pipe) 
{
  int i, creado, fd;
  do 
  { 
    fd = open(pipe, O_RDONLY);
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
/*Nombre: CrearPipe 
  Entradas: Nombre del pipe
  Objetivo: Abrir y escribir en el pipe 
  Salida: El fd del pipe
*/
int CrearPipeEscritura(char *pipe) 
{
  int i, creado, fd;
  do 
  { 
    if ((fd = open(pipe, O_WRONLY))==-1){
      perror("Gestor abriendo Pipe Escritura");
      printf(" Se volvera a intentar despues\n");
      sleep(5);
    }else creado = 1;
  } while (creado == 0);

  printf ("Abrio el pipe\n");

  return fd;
}