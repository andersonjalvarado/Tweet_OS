#ifndef EXTRAER_ARGUMENTOS_H
#define EXTRAER_ARGUMENTOS_H

// Clase que se encarga de extraer los datos de la consola
// Entrada por consola cliente $ cliente -i ID -p pipeNom
// Entrada por consola gestor $ gestor -n Num -r Relaciones -m modo -t time -p pipeNom

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct argumentos {
    int id; ///< Identificador del cliente.
    char *pipeNom; ///< Nombre del pipe nominal.
    int num; ///< Es el número máximo de usuarios que se pueden registrar en el sistema. 
    char *relaciones; ///< Es un archivo de texto que describe las relaciones actuales entre los usuarios del sistema. 
    char modo; ///< indica si el modo en el que trabajará el Gestor es acoplado o desacoplado.
    int time; ///< Es un valor en segundos, que utilizará el Gestor para imprimir las estadisticas 
}arg;

/// @brief Función que se encarga de extraer los datos de la consola
/// @param argv Argumentos
/// @param argc Cantidad de argumentos
/// @return Estructura con los datos extraidos
struct argumentos extraerArgumentos(int argc, char **argv)
{
    arg args;
    args.id = 0; 
    args.pipeNom = NULL;
    args.num = 0; 
    args.relaciones = NULL; 
    args.modo = ' '; 
    args.time = 0;

    int i;
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-i") == 0) { 
            args.id = atoi(argv[i + 1]); // Extraer el id
        } else if (strcmp(argv[i], "-p") == 0) { 
            args.pipeNom = argv[i + 1]; // Extraer el pipeNom
        } else if (strcmp(argv[i], "-n") == 0) { 
            args.num = atoi(argv[i + 1]); // Extraer el num
        } else if (strcmp(argv[i], "-r") == 0) { 
            args.relaciones = argv[i + 1]; // Extraer el relaciones
        } else if (strcmp(argv[i], "-m") == 0) { 
            args.modo = argv[i + 1][0]; // Extraer el modo
        } else if (strcmp(argv[i], "-t") == 0) {
            args.time = atoi(argv[i + 1]); // Extraer el time
        }
    }
    return args; // Retornar los argumentos
}

#endif //EXTRAER_ARGUMENTOS_H