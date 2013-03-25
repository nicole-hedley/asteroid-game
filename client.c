/* 
 client.c
 client for boxdrop game.
 Team 2: EXIT_SUCCESS
 Nicole Hedley 
 updated 03/05/13
*/

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "client.h"
#include "rbuff.h"

#define SERV_PORT 4123 /*port*/
#define MAXLINE 4096 /*max text line length*/

// static function declarations
static Client * client_alloc ();
static void client_destroy ( Client * c );

// helper send/receive functions
static int receiveall ( int fd, Client *client ) ;
static int sendall ( int s, char *buf, int *len );
static void format_outstr ( Rbuff * outstr ) ;


// allocates space for client, returns pointer
static Client * client_alloc () {
    
    Client * c = (Client *) malloc ( sizeof ( Client ) );
    
    c->sockfd = 0; // initializes socket file descriptor to 0
    
    // allocates space for address struct
    c->servaddr = (struct sockaddr_in *) malloc ( sizeof ( struct sockaddr_in ) ); 
    
    c->outstr = rbuff_new ();
    c->instr = rbuff_new ();
    
    return c;
}


// destroys Client data type
static void client_destroy ( Client * c ) {
    
    free ( c->servaddr );
    rbuff_destroy ( c->outstr );
    rbuff_destroy ( c->instr );
    free ( c );
    
    return;
}


// connects to a server at a given address at a given port, returns a client if successful, null if not
Client * client_connect ( char* address, int port ) {
    
    Client * c = client_alloc (); // create a new client
    
    // Create a socket for the client, if sockfd < 0 there was an error in the creation of the socket
    if ( ( c->sockfd = socket ( AF_INET, SOCK_STREAM, 0 ) ) < 0 ) {
        perror ( "Problem in creating the socket" );
        client_destroy ( c );
        return NULL;
    }
    
    // Creation of the socket
    memset ( ( c->servaddr ), 0, sizeof ( struct sockaddr_in ) );
    c->servaddr->sin_family = AF_INET; 
    c->servaddr->sin_addr.s_addr = inet_addr ( address );
    c->servaddr->sin_port =  htons ( port ); //convert to big-endian order
    
    // Connection of the client to the socket
    if ( connect ( c->sockfd, (struct sockaddr *) (c->servaddr),
                sizeof ( struct sockaddr_in ) ) < 0 ) {
        perror ( "Problem in connecting to the server" );
        client_destroy ( c );
        return NULL;
    }
    
    return c; // returns if nothing went wrong
}

// closes the connection and destroys the client struct
void client_disconnect ( Client * c ) {
    
    close ( c->sockfd );
    client_destroy ( c );
}

// sends string to server
void client_puts ( Client * c ) {
    
   // printf ( "The outstr buff is: \n%s\n", c->outstr->buf );

    format_outstr ( c->outstr );
    
    if ( sendall ( c->sockfd, c->outstr->buf, &(c->outstr->ssize) ) == -1 ) {
        perror ( "send" );
    }
    return;
}


// sendall
// taken from beej's guide
// ensures that all of a string has been sent
static int sendall ( int s, char *buf, int *len ) {
    
    int total = 0;        // how many bytes we've sent
    int bytesleft = *len; // how many we have left to send
    int n;
    
    //printf ( "Buffer before sending is: \n%s\n", buf );
    
    while ( total < *len ) {
        n = send ( s, buf+total, bytesleft, 0 );
        if ( n == -1 ) { break; }
        total += n;
        bytesleft -= n;
    }
    
    *len = total; // return number actually sent here
    
    return n==-1?-1:0; // return -1 on failure, 0 on success
}


void client_gets ( Client * c ) {

    rbuff_clear ( c->instr );
    if ( receiveall ( c->sockfd, c ) <= 0 ) {
        printf ( "Error recieving data.\n" );
    }
}

// receiveall
// ensures that all of a string has been received
static int receiveall ( int fd, Client *client ) {
    
    int n = 0;
    int total = 0;
    char first_line[7];
    for (int i = 0; i < 7; i++) {
        first_line[i] = 0;
    }
    if ( ( n = recv ( fd, first_line, 7, 0 ) ) <= 0 ) {
        // got error or connection closed by client
        if ( n == 0 ) {
            // connection closed
            printf ( "selectserver: socket %d hung up\n", fd );
        } else perror ( "recv" );
        
        close ( fd ); // bye!
    }
    
    int len = atoi ( first_line ); // length of first line
    
    // make rbuff larger if necessary
    while ( client->instr->size < len ) rbuff_grow ( client->instr ); 
    
    int bytesleft = len;
    while ( total < len ) {
        if ( ( n = recv ( fd, client->instr->buf + total, bytesleft, 0 ) ) <= 0 ) {
            // got error or connection closed by client
            if ( client->instr->ssize == 0 ) {
                // connection closed
                printf ( "selectserver: socket %d hung up\n", fd );
            } else {
                perror ( "recv" );
            }
            close ( fd ); // bye!
            
        }
        if ( n == -1 ) { break; }
        total += n;
        bytesleft -= n;
    }
    
    client->instr->ssize = len;
    
    return n;
}


// formats outsring to be valid.
// adds header info to rbuff to be ready to send out
static void format_outstr ( Rbuff * outstr ) {
    
    int i = 0;
    // loops through buffer to see where string ends
    while( outstr->buf[i] != 0 ){
        i++;
    }
    
    // grows the outstring if necessary
    int int_str_len = i + 1;
    while ( outstr->size < int_str_len + 7 ) {
        rbuff_grow(outstr);
    }
    
    // runs through out str advancing chars 7 spaces
    for ( i = int_str_len + 7; i >= 7; i-- ) {
        outstr->buf[i] = outstr->buf[i - 7];
    }
    
    // replaces first 7 spaces with information about message
    char line[7]; // will hold first line
    for ( int j = 0; j < 7; j++ ) {
        line[j] = 0;
    }
    sprintf ( line, "%d", int_str_len );
    
    int num_size = 0; // holds size of num
    while ( line[num_size] !=0 ) num_size++;
    
    // fill first
    for ( int j = 0; j < 6 - num_size; j++ ) {
        outstr->buf[j] = '0';
    }
    
    i = 0;
    for ( int j = 6 - num_size; j < 6; j++ ) {
        outstr->buf[j] = line[i];
        i++;
    }
    
    outstr->buf[6] = '\n';
    outstr->ssize = int_str_len +7;
}


// test code for the client
#ifdef TESTCLI

int main ( int argc, char **argv ) {
    
    Client * c = client_connect ( argv[1], SERV_PORT ); // connect to the server 
    
    if ( c == NULL ) {
        puts ( "Did not connect to server.\n" );
        return 1;
    }
        
    rbuff_clear ( c->outstr ); // clear the outstring 
    char * message = "REQ {\nbox\n1.0\n5.00\n2.0\n6.05\n0.00\n0.5\n0.5\n0.5\n0.7\n0.8\n}\n"; // create a request
    rbuff_scat ( c->outstr, message, strlen ( message ) ); // concatonate the message to the outstring  
    
    client_puts ( c ); // send it to the server 
    
    for ( int i = 0; i < 20; i++ ) {
        client_gets ( c ); // receive from the server 
    
        printf ( "Server reply:\n" );
        fputs ( c->instr->buf, stdout ); // print the server's response 
    }
    
    client_disconnect ( c );
    
    exit ( 0 );
}

#endif
