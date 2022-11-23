#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include "ExtraerArgumentos.h"
#include "Usuario.h"
#include "Tweet.h"
#include "Tweets.h"
#define MAX_Usuarios 80

//Variables compartidas
arg argumentos; ///< Estructura de los argumentos recibidos por consola
int usuariosConectados = 0;
int tweetsEnviados = 0;
int tweetsRecibidos = 0;
user clientes[MAX_Usuarios];///< Arreglo de los clientes conectados
int PIDs [MAX_Usuarios]; //Arreglo de los pids de los clientes conectados
int fd, fd1, p;//Variables para el pipe

//captura de Signals
typedef void (*sighandler_t)(int);
sighandler_t signalHandler(void)
{
    printf("\n ESTADISTICAS \n");
    printf("Usuarios conectados: %d\n", usuariosConectados);
    printf("Tweets enviados: %d\n", tweetsEnviados); 
    printf("Tweets recibidos: %d\n", tweetsRecibidos);
    alarm(argumentos.time);
}
typedef void (*sighandler_t)(int);
sighandler_t signalHandlerKill(void)
{
    printf("\n GESTOR FINALIZADO \n");
    unlink(argumentos.pipeNom); // eliminar pipe
    int i;
    
    for (i = 0; i < argumentos.num; i++)
    {
        unlink(clientes[i].pipe);// eliminar pipe de cada cliente
        if(PIDs[i]!=-1)
            kill (PIDs[i], SIGINT);// enviar la señal para elminitR cada cliente conectado
    }
    exit(1);
}
//Funciones
int CrearPipeLectura(char *pipe);
int CrearPipeEscritura(char *pipe);
void *OperacionGestor(void*);
void *Estadisticas(int*); 
void cargarRelaciones(char *file);
void imprimirRelaciones();

int main(int argc,char **argv)
{    
   //Verificar argumentos
    if(argc!=11) 
    {
        printf("Error en la cantidad de argumentos, Formato: $./Gestor -n Num -r Relaciones -m modo -t time -p pipeNom");
        exit(1);
    }
    else
      argumentos = extraerArgumentos(argc, argv);

    int i,j;
    //inicializar pids
    for ( i = 0; i < argumentos.num; i++)
    {
        PIDs[i] = -1;
    }

    printf("Gestor Iniciado\n");  

    //inicializar clientes
    
    for (i = 0; i < argumentos.num; ++i) {
        clientes[i].id = i+1; // Inicializar el id
        clientes[i].enLinea = false; // Inicializar enLinea en false
        clientes[i].cantTweets = 0; // Inicializar la cantidad de tweets en 0
        sprintf(clientes[i].pipe, "pipe%d", clientes[i].id);
        //clientes[i].seguidos = malloc(sizeof(int) * argumentos.num); // Inicializar suscripciones
    }

    cargarRelaciones(argumentos.relaciones);
    //imprimirRelaciones();

    //Uso de Hilos y Signals
    pthread_t thread1, thread2;
    signal (SIGALRM, (sighandler_t)signalHandler);
    signal (SIGINT, (sighandler_t)signalHandlerKill);
    
    if (pthread_create(&thread1, NULL, (void*)OperacionGestor,NULL)) 
    {
         perror("Error creando hilo 1");
         exit(-1);
    }
    if (pthread_create(&thread2, NULL, (void*)Estadisticas,(void*)&argumentos.time)) 
    {
         perror("Error creando hilo 2");
         exit(-1);
    }
    pthread_join(thread1,NULL);
    pthread_join(thread2,NULL);


    return 0;
}
/// @brief Función que se encarga de realizar todas las operaciones del gestor
void *OperacionGestor(void *a)
{
    int i,j;// iterador
    user cliente;
    tw tweet;
    mode_t fifo_mode = S_IRUSR|S_IWUSR;
    int tipo;// acoplado o desacoplado
    if(argumentos.modo == 'A') tipo = 0;
    else tipo = 1;

    remove(argumentos.pipeNom); // Borrar contenido basura del pipe
    //Crear el primer pipe nominal de nombre argumentos.pipeNom y abrirlo en modo escritura. Se usara para asignar un identificador y un pipe a cada usuario que se conecte al sistema.
    unlink(argumentos.pipeNom); // Eliminar el pipe si existe
    //creacion del pipe
    if (mkfifo(argumentos.pipeNom, fifo_mode) == -1)
    {
        perror("Server mkfifo");
        exit(1);
    }
    fd = open(argumentos.pipeNom,O_RDONLY);
    
    int bn; //bandera
    int respuesta; 
    char r[4]; // respuesta a enviar
    int opcion;// opcion recibida
    char pid[30]; //pid del proceso cliente
    char salida[20];
    char buffer[10]; ///< Buffer para almacenar lo que se lee del pipe
    while(1)
    {
        p = read (fd, &opcion, sizeof(int));
        if (p == -1)
        {
            perror("proceso lector:");
            exit(1);
        }
        else{
            printf("Opcion %d\n", opcion);
        }
        
        switch(opcion)
        {              
            case 0: p = read (fd, pid, 30);
                    if (p == -1)
                    {
                        perror("proceso lector:");
                        exit(1);
                    }        
                    char *token = strtok(pid,"d");
                    token = strtok(NULL, "d");  
                    
                    p = read(fd, &cliente, sizeof(cliente));
                    if (p == -1) { 
                        perror("Error al leer el pipe");
                        exit(EXIT_FAILURE);
                    }
                    
                    bn = 0;
                    for(i=0; i<argumentos.num; i++)
                    {
                        //verificar que el usuario exista y no este en linea
                        if(clientes[i].id == cliente.id && clientes[i].enLinea == 0)
                        {
                            clientes[i].enLinea = cliente.enLinea;
                            strcpy(clientes[i].pipe, cliente.pipe);
                            bn = 1;//cambiar el valor de la bandera
                            usuariosConectados++;
                            printf("Usuario %d conectado\n", cliente.id);
                            break;
                        }
                    }
                    int id = cliente.id;
                   

                    fd1 = CrearPipeEscritura(cliente.pipe);
                    if(bn)
                    {
                        PIDs[cliente.id-1] = atoi(token); //guardar el PID del cliente
                    printf("id: %d, pos: %d, pid : %d ", cliente.id, cliente.id -1, PIDs[cliente.id-1]);
                     //for ( i = 0; i < argumentos.num; i++)
                   // {
                     //   printf("pid : %d ", PIDs[i]);
                   // }
                        tws tweetsAlmacenados;
                        tweetsAlmacenados.cantTweets = clientes[id].cantTweets;
                        for(i = 0; i < clientes[id].cantTweets; i++)
                            tweetsAlmacenados.tweets[i] = clientes[id].tweets[i];    
                        
                        respuesta = 1;
                        write (fd1, &tweetsAlmacenados, sizeof(tweetsAlmacenados));
                        write (fd1, &respuesta, sizeof(respuesta));
                        write(fd1, &tipo, sizeof(int));
                       if(clientes[id].cantTweets > 0)
                        {
                            tweetsEnviados+=clientes[id].cantTweets;// incrementar los tweets enviados
                            clientes[id].cantTweets=0;//reiniciar la cantidad de tweets almacenados
                            printf("%d Tweets pendientes:\n", tweetsAlmacenados.cantTweets);
                            printf("Cantidad:\n");
                            for ( j = 0; j < tweetsAlmacenados.cantTweets; j++)
                                printf("id %d tweet: %s\n",  tweetsAlmacenados.tweets[j].id, tweetsAlmacenados.tweets[j].mensaje);
                        }                         
                    }
                    else
                    {
                        respuesta = 0;
                        write (fd1, &respuesta, sizeof(respuesta));
                    }
                    close(fd1);
                    break;

            case 1:p = read(fd, buffer, 10);
                    if (p == -1) { 
                        perror("Error al leer el pipe");
                        exit(EXIT_FAILURE);
                    }
                    char *token1 = strtok(buffer,",");
                    int idCliente = atoi(token1); // id del cliente que envio la solicitud
                    token1 = strtok(NULL, ","); 
                    int idFollow = atoi(token1);    //id del cliente a seguir
                    
                	printf("\nCliente %d quiere seguir a %d: \n", idCliente,idFollow);
                    if(idCliente == idFollow){ //comprobar si no es el mismo id
                        strcpy(salida, "Solicitud Errada");
                    }
                    else if(clientes[idCliente-1].seguidos[idFollow-1] == 0){ //comprobar si lo sigue
                        clientes[idCliente-1].seguidos[idFollow-1] = 1; //seguirlo
                        strcpy(salida, "Solicitud Exitosa");
                    }else
                        strcpy(salida, "Solicitud Errada");

                    fd1 = CrearPipeEscritura(clientes[idCliente-1].pipe);
                    write (fd1, salida, 20);
                    printf("\nNuevas Relaciones:\n");
                    imprimirRelaciones();
                    strcpy(buffer, "");
                    close(fd1);
                    break;
            case 2:	p = read(fd, buffer, 10);
                    if (p == -1) { 
                        perror("Error al leer el pipe");
                        exit(EXIT_FAILURE);
                    }
                    char *token2 = strtok(buffer,",");
                    int idCliente2 = atoi(token2); // id del cliente que envio la solicitud
                    token2 = strtok(NULL, ","); 
                    int idUnfollow = atoi(token2);    //id del cliente a seguir
                    
                	printf("\nCliente %d quiere dejar de seguir a %d: \n", idCliente2,idUnfollow);
                    
                    if(idCliente2 == idUnfollow){ //comprobar si no es el mismo id
                        strcpy(salida, "Solicitud Errada");
                    }
                    else if(clientes[idCliente2-1].seguidos[idUnfollow-1] == 1){ //comprobar si lo sigue
                        clientes[idCliente2-1].seguidos[idUnfollow-1] = 0; //dejar de seguirlo
                        strcpy(salida, "Solicitud Exitosa");
                    }else
                        strcpy(salida, "Solicitud Errada");

                    fd1 = CrearPipeEscritura(clientes[idCliente2-1].pipe);
                    write (fd1, salida, 20);
                    printf("\nNuevas Relaciones:\n");
                    imprimirRelaciones();
                    close(fd1);
                    strcpy(buffer, "");
                    break;
            case 3: p = read(fd, &tweet,sizeof(tweet)); 
                    if (p == -1) { 
                        perror("Error al leer el pipe");
                        exit(EXIT_FAILURE);
                    }
                    printf("%d envia el siguiente mensaje: %s\n", tweet.id, tweet.mensaje);
                    tweetsRecibidos++;
                    
                    if (tipo == 1)// modo Desacoplado
                    {
                        for (i = 0; i < argumentos.num; i++)
                        {
                           //printf("%d pid : %d \n", i, PIDs[i]);
                           if(clientes[i].seguidos[tweet.id-1] == 1){ //seguidores del cliente que envio el tweet;
                                //printf("id seguidor: %d",clientes[i].id);
                                printf("id %d, se guarda el tweet %s\n", clientes[i].id, tweet.mensaje);
                                clientes[i].tweets[clientes[i].cantTweets] = tweet;// agregar el tweet al arreglo 
                                clientes[i].cantTweets++; // incrementar la cantidad de tweets
                                if(clientes[i].enLinea && PIDs[i] != -1)// verificar que está en linea
                                { 
                                   // printf("pid %d\n", PIDs[i]);
                                    kill (PIDs[i], SIGUSR1);// enviar la señal para notificar que tienen un nuevo tweet
                                }
                            }
                        }
                    }    
                    else if (tipo == 0) //modo Acoplado
                    {
                        char mensaje[200];
                        strcpy(mensaje, tweet.mensaje);
                        for (i = 0; i < argumentos.num; i++)
                        {    
                            //printf("%d pid : %d \n", i, PIDs[i]);
                            if(clientes[i].seguidos[tweet.id-1] == 1){ //seguidores del cliente que envio el tweet;
                                //printf("id seguidor: %d",clientes[i].id);
                                
                                if(clientes[i].enLinea) // cliente en linea
                                {
                                    fd1 = CrearPipeEscritura(clientes[i].pipe);// crear el pipe de cada cliente
                                    write (fd1, mensaje, 200);//enviar el mensaje
                                    printf("id %d - pid %d, se le envia el tweet: %s\n", clientes[i].id, PIDs[i], mensaje);
                                    kill (PIDs[i], SIGUSR1);// enviar la señal para notificar que tienen un nuevo tweet
                                    tweetsEnviados++;// incrementar los tweets enviados
                                    close(fd1);         
                                }         
                                if(!clientes[i].enLinea)// si no está en linea agregar el tweet al arreglo
                                { 
                                    clientes[i].tweets[clientes[i].cantTweets] = tweet;// agregar el tweet al arreglo
                                    printf("id %d, se le guarda el tweet %s\n", clientes[i].id, clientes[i].tweets[clientes[i].cantTweets].mensaje);
                                    clientes[i].cantTweets++; // incrementar la cantidad de tweets                                   
                                }
                            }
                        }
                    }   
                    break;
            case 4: char buffer2[6];
                    p = read(fd, buffer2, 6);
                    if (p == -1) { 
                        perror("Error al leer el pipe");
                        exit(EXIT_FAILURE);
                    }
                    char *token3 = strtok(buffer2,",");
                    token3 = strtok(NULL, ","); 
                    int idRecuperar = atoi(token3); //id del cliente que envio la solicitud
                    
                    user clientRecuperar = clientes[idRecuperar-1]; 
                    printf("id:%d",clientRecuperar.id);
                    fd1 = CrearPipeEscritura(clientRecuperar.pipe);
                    write (fd1, &clientRecuperar, sizeof(clientRecuperar));
                    
                    if (clientes[idRecuperar-1].cantTweets > 0){
                        clientes[idRecuperar-1].cantTweets = 0;
                        tweetsEnviados++;// incrementar los tweets enviados 
                    } 
                    close(fd1);
                    break;
            case 5: printf("\nDesconectando...\n");
                    char buffer3[3];
                    p = read(fd, buffer3, 3);
                    if (p == -1) { 
                        perror("Error al leer el pipe");
                        exit(EXIT_FAILURE);
                    }
                    
                    int idSalir = atoi(buffer3);
                    unlink(clientes[idSalir].pipe);//eliminar el pipe del cliente
                    clientes[idSalir-1].enLinea = 0;//poner fuera de linea
                    printf("id %d Desconectado\n",idSalir);
                    usuariosConectados--;
                  //  kill (PIDs[idSalir-1], SIGINT);//terminar el proceso con una señal
                    break;

        }
    }
}

/// @brief Funcion para Enviar una signal para imprimir las estadisticas de usuarios conectastos y tweets enviados
/// @param t tiempo de espera
void *Estadisticas(int *t)
{
  while(1)
  {
    alarm(*t);
    pause();
  }
}
/// @brief Funcion para Abrir y leer en el pipe
/// @param pipe Nombre del pipe
/// @param return file descriptor 
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

/// @brief Funcion para Abrir y escribir en el pipe
/// @param pipe Nombre del pipe
/// @param return file descriptor 
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

/// @brief Funcion para cargar las relaciones del archivo
/// @param file Nombre del archivo
void cargarRelaciones(char *file){

    FILE *fp; ///< Archivo de relaciones
    char line[500]; ///< Linea del archivo
    
    fp = fopen(file, "r"); // Abrir archivo de relaciones
    if (fp == NULL){
        perror("Error al abrir el archivo de relaciones\n");
        exit(1);
    }
        
    int i = 0; // # de lineas
    while (fgets(line,500,fp)) { // Leer linea por linea
        char *token; ///< Token de la linea
        //printf("\nline:%s\n",line);
        token = strtok(line, "	"); // Extraer el primer token
        //printf("\n token:%s\n",token); 

        int j = 0; ///< # columnas
        while(token != NULL){
            
            //printf("\ntoken : %s\n",token); // El usuario i sigue al usuario j  
            if (atoi(token) == 1){ // Si el token es 1
                clientes[i].seguidos[j] = 1;
                 //printf("fila: %d columna %d \n",i, j); // El usuario i sigue al usuario j
            }
            else{ 
                clientes[i].seguidos[j] = 0; // El usuario i no sigue al usuario j
            }


            token = strtok(NULL, "	"); // Extraer el siguiente token
            j++; 
        }
        i++;
    }
    fclose(fp);
}
/// @brief Funcion para imprimir las relaciones de los usuarios
void imprimirRelaciones(){
    int i,j;
    for (i = 0; i < argumentos.num; ++i) {
        printf("\nUsuario %d\n", clientes[i].id); 
        printf("En linea %d\n", clientes[i].enLinea);  
        printf("Seguidos: ");  
        for (j = 0; j < argumentos.num; j++)
        {
            if(clientes[i].seguidos[j]==1)
                printf("%d ",j+1);
        }
        printf("\n");  
    }
}