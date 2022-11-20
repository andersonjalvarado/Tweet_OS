#ifndef USUARIO_H
#define USUARIO_H

// Clase usuario, clientes que utilizan Tweeter
#include<stdio.h>
#include<stdlib.h>
#include <stdbool.h>
#include "Tweet.h"
typedef struct Usarios
{
    int id; ///< id usuario
    bool enLinea; ///< indica si el usuario esta en linea o no
    char pipe[50]; ///< Pipe por el cual se va a comunicar con el cliente
    bool seguidos[80]; ///< Arreglo de usuarios que sigue
    tw tweets[100]; ///< Arreglo de tweets que han publicado los usuarios que sigue
    int cantTweets; ///< Cantidad de tweets
} user;
#endif //USUARIO_H