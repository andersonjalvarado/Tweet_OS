/*Proyecto Primera Entrga SOP
Desarrollado por 
Anderson Jair Alvarado
Kevin
Santiago
Profesora: Mariela Curiel
*/

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "Usuario.h"

int Menu();
int CrearPipeEscritura(char *pipe); 
int CrearPipeLectura(char *pipe);

int main(int argc, char **argv)
{
  //variables
  int salir = 0;
  user users;

  //verificar argumentos
  if(argc!=5) 
    {
        printf("Argumentos incorrectos, formato correcto: $./Cliente -i ID -p pipeNom");
        exit(1);
    }

  //variables pipes
  int  fd, fd1;
  mode_t fifo_mode = S_IRUSR | S_IWUSR;

  fd = CrearPipeEscritura(argv[4]);
  users.id = atoi(argv[2]);
  users.enLinea = 1;
  sprintf(users.pipe, "pipe%d", users.id);
  unlink(users.pipe);

  //creacion del pipe
  //unlink(users.pipe); 
  if (mkfifo (users.pipe, fifo_mode) == -1) 
  {
    perror("Client  mkfifo");
    exit(1);
  }

  printf("pipe cliente %s\n",users.pipe );
  write(fd, &users, sizeof(users));

  int respuesta; 
  fd1 = CrearPipeLectura(users.pipe);
  int p = read(fd1, &respuesta, sizeof(respuesta));
  if (p == -1)
  {
    perror("proceso lector:");
    exit(1);
  }

  // Se lee un mensaje por el segundo pipe.
  //printf("El proceso cliente termina y lee %d \n", respuesta);

  if(respuesta)
  {
      do{
      switch(Menu())
      {              
        case 1: printf("\n\n Digite id del usuario que desea seguir\n");
                        
                break;
        case 2: printf("\n\n Digite id del usuario que desea dejar de seguir\n");
                
                break;
        case 3:	printf("\n\n Digite el tweet que desea publicar: \n");
                
                
                break;
        case 4: salir=1;
                printf("\n\n\t VUELVA PRONTO!\n\n");
                break;
        default:printf("\n\n\t OPCION INVALIDA! Intente de nuevo\n\n");
                break;
      }
    }while(!salir);
  }
  else
    printf("El proceso NO exitoso\n");
  
  return 0;
}

/*Nombre: Menu 
  Entradas: 
  Objetivo: Mostrar el menu de opciones para el usuario
  Salida: La opcion elegida por el usuario
*/
int Menu()
{
  int opc;
	printf("\n\n\t\t-----------------------------------");
	printf("\n\t\t\tMENU PRINCIPAL");
	printf("\n\t\t-----------------------------------");
	printf("\n\t\t 1.Follow");
	printf("\n\t\t 2.Unfollow");
	printf("\n\t\t 3.Publicar un Tweet");
	printf("\n\t\t 4.Desconectarse");
	printf("\n\n\t\t-----------------------------------");
	printf("\n\n\n\t\tDigite la opcion: ");
	scanf("%d", &opc);
	return opc;
}

/*Nombre: CrearPipe 
  Entradas: Nombre del pipe
  Objetivo: Abrir y escribir en el pipe 
  Salida: El fd del pipe
*/
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
/*Nombre: CrearPipe 
  Entradas: Nombre del pipe
  Objetivo: Abrir y leer el pipe 
  Salida: El fd del pipe
*/
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