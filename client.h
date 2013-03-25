/*
 client.c
 client for boxdrop game.
 Team 2: EXIT_SUCCESS
 Nicole Hedley
 updated 03/05/13
*/

#ifndef CLIENT_H
#define CLIENT_H

#include "rbuff.h"
#include <sys/types.h>
#include <sys/socket.h>

// struct to include what is needed for the client
typedef struct client Client;
struct client {
    int sockfd; 
    struct sockaddr_in *servaddr;
    Rbuff *outstr;
    Rbuff *instr;
};

// connects to a server at a given address at a given port
Client * client_connect ( char * address, int port );

// disconnects client from server
void client_disconnect ( Client * c );

// writes string to server
void client_puts ( Client * c );//, char* message );

// gets string from server
void client_gets ( Client * c );

#endif