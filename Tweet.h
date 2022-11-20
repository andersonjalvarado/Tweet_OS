#ifndef TWEET_H
#define TWEET_H

// Clase Tweet
#define Max 200
typedef struct Tweet
{
    int id; ///< Identificador del cliente que envio el tweet.
    char mensaje[Max]; ///< Mensaje del tweet
} tw;
#endif //TWEET_H