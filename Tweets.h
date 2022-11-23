#ifndef TWEETS_H
#define TWEETS_H

// Clase Tweets, para almacenar los twees pendientes de cada usuario desconectado
#include<stdio.h>
#include<stdlib.h>
#include <stdbool.h>
#include "Tweet.h"
typedef struct Tweets
{
    tw tweets[100]; ///< Arreglo de tweets que han publicado los usuarios que sigue
    int cantTweets; ///< Cantidad Tweets
} tws;
#endif //USUARIO_HTWEETS_H