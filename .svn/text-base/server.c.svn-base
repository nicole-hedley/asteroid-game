/* 
 Server to play boxdrop on.
 Nicole Hedley 
 Brad Nelson
 updated 03/05/13
 */

#include "core.h"
#include <stdlib.h>
#include <stdio.h>
#include "rbuff.h"
#include <chipmunk.h>
#include <sys/time.h>
#include <time.h>

#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>

#define LISTENQ 8           // maximum number of client connections
#define MAXLINE 4096        // buffer size for reads and writes
#define SERVER_PORT 4123
#define FDL_SIZE_DEFAULT 4
#define DEFAULT_TIMESTEP 1.0/60.0 // default timestep for server's cpSpace
#define SPACE_BROADCAST_TIMESTEP 1.0/30.0 // timestep to broadcast_space
#define BODY_TIMEOUT 3.0 // time until user-created bodies die

#define ASIM 30

#define STDIN 0

typedef struct server Server;
struct server {
    
    int listenfd;
    int connfd;
    Rbuff *instr;
    Rbuff *outstr;
    fd_set * master;    // master file descriptor list
    fd_set * read_fds;  // temp file descriptor list for select()
    
    int fdmax;        // maximum file descriptor number
    struct timeval * tv;
    
    int num_conns;
    int message_from;
    int message_to;

    int num_planets;
    
    int name_table_size; // keeps track of name_table size
    char ** name_table; // keeps track of names associated with players
    cpSpace * space;
    int body_count;
};

// create and destroy a server 
static Server * server_new ();
static void server_destroy ( Server *server );
static void server_destroy_name_table ( Server * server );

// set upt the connection between server and client 
static void setup_listener ( const int port, Server *server );
static void accept_connection ( Server *server );
static int in_exceptions ( const int query, int * exceptions, const int num_exceptions );

// send and receive data to and from the client 
static void receive_data ( Server * server, const int fd );
static void broadcast_data ( Server * server, int * exceptions, const int num_exceptions );
static int sendall ( const int s, char *buf, int *len );
static int receiveall ( const int fd, Server * server );

// parsing functions
static void server_parser ( Server * server );
static int read_line ( char ** message, Rbuff * line );
static void format_outstr ( Rbuff * outstr );
static void parse_chat ( Server * server, char ** message, Rbuff * line );
static void parse_req ( Server * server, char ** message, Rbuff * line );
static void parse_name ( Server * server, char ** message, Rbuff * line );
static void parse_homebase ( Server * server, char ** message, Rbuff * line );

// create shape functions 
static void create_freestyle ( Server * server, char ** message, Rbuff * line );
static void create_shape ( Server * server, char ** message, Rbuff * line, const int type );
static void create_asteroid (Server * server);

// function for broadcasting state of cpSpace
static void server_space_update ( Server * server, const double time_step );
static void server_read_body ( cpBody * body, void * data );

// functions for sending space to new client
static void server_send_whole_space ( const int fd, Server * server );
static void server_send_whole_body ( cpBody * body, void * data );
static void send_shape ( BodyInfo * bi , Server * server );
static void send_freestyle_shape ( BodyInfo * bi, Server * server );

// shape timeout functions
static void server_remove_old_shapes ( Server * server, time_t now );
static void body_timeout_check ( cpBody *body, void *data );

// asteroid update
static double update_asteroid_timer ( double asteroid_timer_old );


// Initialize new server
static Server *server_new () {
    Server *server = ( Server * ) malloc ( sizeof ( Server ) );
    
    server->listenfd = 0;
    server->connfd = 0;
    
    server->instr = rbuff_new ();
    server->outstr = rbuff_new ();
    
    server->tv = (struct timeval *) malloc ( sizeof (struct timeval) );
    server->tv->tv_sec = 0;
    server->tv->tv_usec = 016000;
    
    server->master = (fd_set *) malloc ( sizeof (fd_set) );
    server->read_fds = (fd_set *) malloc ( sizeof (fd_set) );
    
    server->num_conns = 0;
    
    server->name_table_size = 10; // start off with room for 10 file descriptor names
    server->name_table = (char** ) malloc ( server->name_table_size * sizeof ( char* ) );
    
    // initialize all locations to null until read in
    for ( int i = 0; i < server->name_table_size; i++ ) {
        server->name_table[i] = NULL;
    }
    
    
    FD_ZERO ( server->master );    // clear the master and temp sets
    FD_ZERO ( server->read_fds );
    
    FD_SET ( STDIN, server->master ); // adds stdin to master fdset
    
    server->space = core_create_space ();
    server->body_count = 0;
    
    GameInfo * info = (GameInfo *) (server->space->data);
    info->multiplayermode = 1;
    info->server_body_count = &(server->body_count);
    
    server->num_planets = 0;
    
    return server;
    
}

// destroys server datatype
static void server_destroy ( Server *server ){
    
    // close connections
    close(server->listenfd);
    close(server->connfd);
    
    free (server->master);
    free (server->read_fds);
    free (server->tv);
    
    rbuff_destroy(server->instr);
    rbuff_destroy(server->outstr);
    
    core_destroy_space(server->space);
    
    server_destroy_name_table(server);
    
    free (server);
}

// frees memory associated with server name table
static void server_destroy_name_table(Server * server) {
    
    for(int i = 0; i < server->name_table_size; i++ ) {
        
        // free memory allocated for name if name exists
        if((server->name_table)[i] != NULL ) {
            free((server->name_table)[i]);
        }
        
    }
    free(server->name_table);
}

// creates the socket, binds it to a local address and port
// and converts it to a listening socket
static void setup_listener ( const int port, Server *server ){
    
    server->listenfd = socket ( AF_INET, SOCK_STREAM, 0 );
    
    struct sockaddr_in serveraddr;
    serveraddr.sin_family = AF_INET;
    
    serveraddr.sin_addr.s_addr = htonl ( INADDR_ANY );
    serveraddr.sin_port = htons ( port );
    
    if ( bind ( server->listenfd, ( struct sockaddr* ) &serveraddr, sizeof (serveraddr) ) == -1 ) {
        printf ( "Error binding to listening port.\n" );
    }
    
    listen ( server->listenfd, LISTENQ );
    
    FD_SET ( server->listenfd, server->master );
    server->fdmax = server->listenfd;
}

// creates a structure to store client address and accepts the connection
static void accept_connection ( Server *server ){
    
    struct sockaddr_in cliaddr;
    socklen_t clilen = sizeof ( cliaddr );
    
    printf ( "Waiting to accept a socket connection request.\n" );
    server->connfd = accept ( server->listenfd, (struct sockaddr *) &cliaddr, &clilen );
    
    printf ( "Received socket sonnection request.\n" );
    
    
    if ( server->connfd == -1 ) {
        perror ( "Accept" );
    } else {
        
        FD_SET ( server->connfd, server->master ); // add to master set
        if ( server->connfd > server->fdmax ) {    // keep track of the max
            server->fdmax = server->connfd;
        }
        printf ( "Selectserver: new connection.\n" );
        
        (server->num_conns)++; // now connection
        
        server_send_whole_space ( server->connfd, server );
    }
}

// receives data from file descriptor fd
// writes to server->instr
static void receive_data ( Server * server, const int fd ) {
    
    if ( receiveall ( fd, server ) <= 0 ) {
        printf ( "Error recieving data\n" );
    }
    
    
}

// receiveall
// ensures that all of a string has been received
static int receiveall ( const int fd, Server * server ) {
    
    int n;
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
        } else {
            perror( "recv" );
        }
        close ( fd ); // bye!
        FD_CLR ( fd, server->master ); // remove from master set
    }
    
    if ( fd == STDIN ) return 1;
    
    int len = atoi ( first_line ); // length of first line
    
    while ( server->instr->size < len ) rbuff_grow ( server->instr ); // make rbuff larger if necessary
    
    int bytesleft = len;
    while ( total < len ) {
        if ( ( n = recv ( fd, server->instr->buf + total, bytesleft, 0 ) ) <= 0 ) {
            // got error or connection closed by client
            if ( server->instr->ssize == 0 ) {
                // connection closed
                printf ( "selectserver: socket %d hung up\n", fd );
            } else {
                perror ( "recv" );
            }
            close( fd ); // bye!
            FD_CLR ( fd, server->master ); // remove from master set
            
        }
        if ( n == -1 ) { break; }
        total += n;
        bytesleft -= n;
    }
    
    server->instr->ssize = len;
    
    return n;
    
}

// broadcasts data to all clients, except for those indicated in exceptions
// reads from server->outstr
static void broadcast_data ( Server * server, int * exceptions, const int num_exceptions ) {
    
    format_outstr ( server->outstr );
    
    for ( int j = 0; j <= server->fdmax; j++ ) {
        // send to everyone except listener and exceptions
        if ( FD_ISSET ( j, server->master ) ) {
            
            if ( j != server->listenfd && j != STDIN && !( in_exceptions ( j,exceptions, num_exceptions ) ) ) {
                if ( sendall ( j, server->outstr->buf, &(server->outstr->ssize) ) == -1 ) {
                    perror ( "send" );
                }
            }
        }
    }
}


// sendall
// taken from beej's guide
// ensures that all of a string has been sent
static int sendall ( const int s, char *buf, int *len ) {

    int total = 0;        // how many bytes we've sent
    int bytesleft = *len; // how many we have left to send
    int n;
    
    while ( total < *len ) {
        n = send ( s, buf+total, bytesleft, 0 );
        if (n == -1) { break; }
        total += n;
        bytesleft -= n;
    }
    
    *len = total; // return number actually sent here
    
    return n==-1?-1:0; // return -1 on failure, 0 on success
}


// formats outsring to be valid.
// adds header info to rbuff to be ready to send out
static void format_outstr ( Rbuff * outstr ) {
    
    int i = 0;
    // loops through buffer to see where string ends
    while( outstr->buf[i] != 0 ){
        i++;
    }
    
    // grow outstring if necessary
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


// function that determines if a query matches exceptions
// returns 0 if false
// returns 1 if true
static int in_exceptions ( const int query, int * exceptions, const int num_exceptions ) {
    
    for ( int i = 0; i < num_exceptions; i++ ){
        if ( query == exceptions[i] ) return 1;
    }
    return 0;
}


// parses instr to see what needs to be done
static void server_parser ( Server * server ) {
    
    Rbuff *line = rbuff_new();
    
    char * message = server->instr->buf; // starts at beginning of message
    
    while( read_line ( &message, line ) == 1 ) {
        
        if( strncmp ( line->buf, "CHAT {", 6 ) == 0 ){
            parse_chat ( server, &message , line);
            
        } else if ( strncmp ( line->buf, "REQ {", 5 )== 0 ){
            // process request in master cpSpace
            parse_req ( server, &message, line );
            
        } else if ( strncmp ( line->buf, "NAME {", 6 ) == 0 ) {
            // add name to associate with a file descriptor
            parse_name ( server, &message, line );
            
        } else if ( strncmp ( line->buf, "HOMEBASE {", 10 ) == 0 ) {
            // add name to associate with a file descriptor
            parse_homebase ( server, &message, line );
        }
    }
    
    rbuff_destroy ( line );
    rbuff_clear ( server->instr );
}

// parse the homebase message
static void parse_homebase ( Server * server, char ** message, Rbuff * line ) {
    
    // clear server string for sending 
    rbuff_clear_str ( server->outstr ); 
    read_line ( message, line );
    double homebasex = atof ( line->buf );
    
    read_line ( message, line );
    double homebasey = atof ( line->buf );
    
    server->num_planets += 1;
    
    // create the homebase 
    core_make_homebase ( server->space, homebasex, homebasey, server->body_count +1, server->num_planets + 1 );

    char temp_string[200]; // string to hold index
    sprintf ( temp_string, "HOMEBASE {\n%f\n%f\n%d\n%d\n}\n", homebasex, homebasey, server->body_count + 1, server->num_planets + 1 );
    
    rbuff_scat ( server->outstr, temp_string, strlen ( temp_string ) );

    broadcast_data ( server, NULL, 0 );
    
    server->body_count = server->body_count +5;   
}

// parse name statment, associate with the file descriptor that the message came from
static void parse_name ( Server * server, char ** message, Rbuff * line ) {
    
    // if name table is not large enough, expand
    if ( server->name_table_size < server->message_from + 1 ) {
        
        // allocate more memory
        server->name_table = realloc ( server->name_table, ( server->message_from + 1 ) * sizeof(char*));
        
        // initialize new entries to NULL
        for ( int i = server->name_table_size; i < server->message_from - 1; i++ ) {
            server->name_table[i] = NULL;
        }
    }
    
    read_line ( message, line );
    
    // malloc memory for name string
    char * name = (char *) malloc ( line->ssize * sizeof(char));
    // copy name from line
    for ( int i = 0; i < line->ssize - 1; i++ ) {
        name[i] = (line->buf)[i];
    }
    
    // set name to server name table
    (server->name_table)[server->message_from] = name;
    
    return;    
}


// parse_chat parses chat statment, broadcasts chat statement to all clients
static void parse_chat ( Server * server, char ** message, Rbuff * line ) {
    
    rbuff_clear_str ( server->outstr ); // clear outstr for writing message
    
    // CHAT { is already on line
    rbuff_cat(server->outstr, line);
    
    // add preamble here
    if( (server->name_table)[server->message_from] == NULL ) {
        
        // no name provided, so just use file descriptor
        char temp_string[10]; // string to hold index
        for (int i = 0; i < 10; i++) {
            temp_string[i] = 0;
        }
        sprintf ( temp_string, "%d : ", server->message_from );
        rbuff_scat ( server->outstr, temp_string, strlen(temp_string) );
        
    } else { 
        // add name from name table
        rbuff_scat ( server->outstr, (server->name_table)[server->message_from], 
                   strlen ( (server->name_table)[server->message_from]) );
        rbuff_scat ( server->outstr, " : ", 3 );
    }
    
    
    // read chat message
    read_line ( message, line );
    rbuff_cat ( server->outstr, line );
    
    // add end brace
    read_line ( message, line );
    rbuff_cat ( server->outstr, line );
    
    
    broadcast_data ( server, NULL, 0 );
}


// parses a request to make a new shape
static void parse_req ( Server * server, char ** message, Rbuff * line ) {
    
    read_line(message, line); // read first line
    
    if ( ( strncmp ( line->buf, "box", 3 ) == 0 ) ) { // want to create a new box shape in server cpSpace
        create_shape ( server, message, line, BOX_TYPE );
        
    } else if ( strncmp ( line->buf, "circle", 6 ) == 0 ) { // want to create a new circle shape 
        create_shape ( server, message, line, CIRCLE_TYPE );
        
    } else if ( strncmp ( line->buf,"segment", 7 ) == 0 ) { // new segment
        create_shape ( server, message, line, SINGLE_SEGMENT_TYPE) ;
        
    } else if ( strncmp ( line->buf,"freestyle", 9 ) == 0 ) { // new freestyle shape
        create_freestyle ( server, message, line );
        
    }
}

// generic function that creates shape and launches it
// used for box, circle, and segment shapes
static void create_shape ( Server * server, char ** message, Rbuff * line, const int type ) {
    
    // variables
    double p1x, p1y, p2x, p2y;
    Color c;
    double ang = 0.0;
    double friction, density, elasticity;
    
    // cannon variables
    cpVect impulse = cpvzero;
    cpVect im_offset = cpvzero;
    int cannon = 0; // tells whether we should use impulse constructor
    
    rbuff_clear_str ( server->outstr );
    rbuff_scat ( server->outstr, "SHAPE {\n", 8 );
    
    // append message for type
    if ( type == CIRCLE_TYPE ) {
        rbuff_scat ( server->outstr, "circle\n", 7 );
    } else if ( type == BOX_TYPE ) {
        rbuff_scat ( server->outstr, "box\n", 4 );
    } else { // segment type
        rbuff_scat ( server->outstr, "segment\n", 8 ); 
    }
    
    
    // check to see if we are adding a projectile shape
    read_line ( message, line );
    if ( strncmp ( line->buf, "cannon", 6 ) == 0 ) {
        
        cannon = 1;
        
        read_line ( message, line );
        impulse.x = atof ( line->buf );
        
        read_line ( message, line );
        impulse.y = atof ( line->buf );
        
        read_line ( message, line );
        im_offset.x = atof ( line->buf );
        
        read_line ( message, line );
        im_offset.y = atof ( line->buf );
        
        read_line( message, line );
    }
    
    // run through the parameters to create new shape
    // prepare string to broadcast
    
    // x and y coordinates 
    p1x = atof ( line->buf );
    rbuff_cat ( server->outstr, line );
    read_line ( message, line );
    p1y = atof ( line->buf );
    rbuff_cat ( server->outstr, line );
    read_line ( message, line );
    p2x = atof ( line->buf );
    rbuff_cat ( server->outstr, line );
    read_line ( message, line );
    p2y = atof ( line->buf );
    rbuff_cat ( server->outstr, line );
    
    // color 
    read_line ( message, line );
    c.r = atof ( line->buf );
    rbuff_cat ( server->outstr, line );
    read_line ( message, line );
    c.g = atof ( line->buf );
    rbuff_cat ( server->outstr, line );
    read_line ( message, line );
    c.b = atof ( line->buf );
    rbuff_cat ( server->outstr, line );
    
    if ( type != SINGLE_SEGMENT_TYPE ) {
        read_line ( message, line );
        ang = atof ( line->buf );
        rbuff_cat ( server->outstr, line );
    }
    
    // friction, elasticity, and density
    read_line(message,line);
    friction = atof(line->buf);
    rbuff_cat(server->outstr, line);
    read_line(message,line);
    elasticity = atof(line->buf);
    rbuff_cat(server->outstr, line);
    read_line(message,line);
    density = atof(line->buf);
    rbuff_cat(server->outstr, line);
    
    // add the new shape to the cpSpace
    (server->body_count)++; // adding new body so increase body count
    
    // add shape routine depends on if cannon being used
    if(cannon == 0) {
        if( type == SINGLE_SEGMENT_TYPE) {
            core_add_single_segment_shape ( server->space, p1x, p1y, p2x, p2y, &c, friction, elasticity, density, server->body_count );
        } else {
            core_add_new_shape ( server->space, type, p1x, p1y, p2x, p2y, &c, ang, friction, elasticity, density, server->body_count );
        }
    } else {
        core_add_new_shape_with_impulse ( server->space, type, p1x, p1y, p2x, p2y, &c, ang, friction, elasticity, density, server->body_count, impulse, im_offset );
    }
    
    // add index to outgoing string
    char temp_string[10]; // string to hold index
    sprintf(temp_string, "%d\n", server->body_count);
    rbuff_scat(server->outstr, temp_string, strlen(temp_string));
    
    
    // finish message to broadcast
    rbuff_scat(server->outstr, "}\n", 2);
    
    
    // read back off instructions here
    broadcast_data ( server, NULL, 0 );
}



// parses instructions to make a new box in master cpSpace
// creates box
// sends out instructions to clients to make a new box
static void create_freestyle ( Server * server, char ** message, Rbuff * line ) {
    
    cpVect impulse = cpvzero;
    cpVect im_offset = cpvzero;
    int cannon = 0; // tells whether we should use impulse constructor
    
    
    //printf ( "Freestyle shape created!\n" );
    rbuff_clear_str ( server->outstr );
    rbuff_scat ( server->outstr, "SHAPE {\n", 8 );
    rbuff_scat ( server->outstr, "freestyle\n", 10 );
    
    // check to see if we are adding a projectile shape
    read_line ( message, line );
    if (strncmp(line->buf, "cannon", 6) == 0) {
        
        cannon = 1;
        
        read_line ( message, line );
        impulse.x = atof( line->buf);
        
        read_line ( message, line );
        impulse.y = atof( line->buf);
        
        read_line ( message, line );
        im_offset.x = atof( line->buf);
        
        read_line ( message, line );
        im_offset.y = atof( line->buf);
        
        read_line( message, line);
    }
    
    
    // run through the parameters to create new shape
    // color 
    Color c;
    c.r = atof ( line->buf );
    rbuff_cat ( server->outstr, line );
    read_line ( message, line );
    c.g = atof ( line->buf );
    rbuff_cat ( server->outstr, line );
    read_line ( message, line );
    c.b = atof ( line->buf );
    rbuff_cat ( server->outstr, line );
    
    // friction, elasticity, and density
    read_line ( message, line );
    double friction = atof ( line->buf );
    rbuff_cat ( server->outstr, line );
    read_line ( message, line );
    double elasticity = atof ( line->buf );
    rbuff_cat ( server->outstr, line );
    read_line ( message, line );
    double density = atof ( line->buf );
    rbuff_cat ( server->outstr, line );
    
    read_line ( message, line );
    int num_verts = atoi ( line->buf );
    rbuff_cat ( server->outstr, line );
    
    // sets of vertices 
    cpVect * verts = (cpVect *) malloc ( sizeof(cpVect) * num_verts);
    double x, y;
    for ( int i = 0; i < num_verts; i++ ) {
        
        read_line ( message, line );
        x = atof ( line->buf );
        rbuff_cat ( server->outstr, line );
        
        read_line ( message, line );
        y = atof ( line->buf );
        rbuff_cat ( server->outstr, line );
        
        verts[i].x = x;
        verts[i].y = y;        
    }
    
    // add the new shape to the cpSpace
    (server->body_count)++; // adding new body so increase body count
    
    
    if ( cannon == 0 ) {
        core_add_freestyle_shape ( server->space, verts , num_verts, &c, friction, elasticity, density, server->body_count );
    } else {
        core_add_freestyle_shape_with_impulse ( server->space, verts , num_verts, &c, friction, elasticity, density, server->body_count, impulse, im_offset );
    }
    
    // add index to outgoing string
    char temp_string[10]; // string to hold index
    sprintf ( temp_string, "%d\n", server->body_count );
    rbuff_scat ( server->outstr, temp_string, strlen ( temp_string ) );
    
    // finish message to broadcast
    rbuff_scat ( server->outstr, "}\n", 1 );
    
    
    // read back off instructions here
    broadcast_data ( server, NULL, 0 );
    
    free ( verts ); 
}



// reads off line from string, stores in address of
// returns 1 if not finished
// returns 0 if messsage has already finished
static int read_line ( char ** message, Rbuff * line ) {
    
    if ( (**message == EOF) || (**message == 0) ) return 0;
    rbuff_clear_str(line); // clears out last string
    
    
    int i = 0;
    while ( (**message >= 32) ) { // while char is readable
        
        if ( line->size <= i ) rbuff_grow ( line ); // makes sure buffer is long enough
        
        line->buf[i] = **message;
        (*message)++; // increment where message is pointing to
        i++;
        
    }
    
    if( line->buf[i-1] == '\n' ) {
        i--;
    }
    
    (*message)++;
    line->buf[i] = '\n';
    line->buf[i+1] = 0;
    line->ssize = i+1;
    
    
    return 1;
}

// helper function for for space update
// identifies number for nonstatic body
// cats ID number, x loc, y loc, ang to outstr->buf
// use 2 digits accuracy with f.p. numbers
static void server_read_body ( cpBody * body, void * data ) {
    
    Server * server = (Server *) data;
    
    int msize = 32; // buffer for writing line... more than enough space
    char *formatmessage = (char *) malloc ( sizeof (char) * msize );
    
    // get id number
    sprintf ( formatmessage, "%d\n", *( (int *) ( body->data ) ) );
    rbuff_scat ( server->outstr, formatmessage, strlen ( formatmessage ) );
    
    
    cpVect center = cpBodyGetPos ( body );
    // get x val
    sprintf ( formatmessage, "%.2f\n", center.x );
    rbuff_scat ( server->outstr, formatmessage, strlen ( formatmessage ) );
    
    
    // get y val
    sprintf ( formatmessage, "%.2f\n", center.y );
    rbuff_scat ( server->outstr, formatmessage, strlen ( formatmessage ) );
    
    double ang = cpBodyGetAngle(body);
    // get ang
    sprintf ( formatmessage, "%.2f\n", ang );
    rbuff_scat ( server->outstr, formatmessage, strlen ( formatmessage ) );
    
    // now we are done writing to outstr buffer
    
    free ( formatmessage );
    
    return;
    
    
}

// updates space with timestep
// if timestep is <= 0, then space is not updated, but broadcasted
static void server_space_update ( Server * server, const double time_step ) {
    
    if ( time_step > 0.0 ) {
        core_update_space ( server->space, time_step );
        
        GameInfo *info = (GameInfo *) (server->space->data);
        if ( info->gameisover == 1 ) {
            
            
            info->gameisover = 2;
            rbuff_clear ( server->outstr );
            rbuff_scat ( server->outstr, "GAMEOVER", 8 );
            
            char *formatmessage = (char *)malloc(80);
            sprintf( formatmessage, "\n%d", ((GameInfo *)server->space->data)->destroyed_planet);
            rbuff_scat ( server->outstr, formatmessage, strlen ( formatmessage ) );
            broadcast_data ( server, NULL, 0 );
            
            free(formatmessage);
            info->gameisover = 0;
        }
    }
    else {
        rbuff_clear ( server->outstr );
        
        // cat a format string "SPACE {\n" to outstr
        rbuff_scat ( server->outstr, "SPACE {\n", 8 );
        
        //  do a cpSpaceEachBody call here with server_read_body
        cpSpaceEachBody ( server->space, &server_read_body, server );
        
        // finish format string with "}\n"
        rbuff_scat ( server->outstr, "}\n", 2 );
        
        // send message to clients
        broadcast_data ( server, NULL, 0 );
    }
}


// sends all the contents of a space to a client when the client joins
static void server_send_whole_space ( const int fd, Server * server ) {
    
    server->message_to = fd;
    
    cpSpaceEachBody(server->space, &server_send_whole_body, (void *) server);
    
    // may also need to do something for server static body
    
    // send out a space update to all clients just to update them
    server_space_update(server, 0);
    
}


// sends all the contents of a body to a client when a client joins
static void server_send_whole_body ( cpBody * body, void * data ) {
    
    Server * server = (Server *) data;
    
    BodyInfo * bi = body->data;
    
    if ( bi->type == FREESTYLE_TYPE ) {
        send_freestyle_shape ( bi, server );
    } else { // circle, box, or single segment type
        send_shape ( bi, server );
    }
}

// sends message to create box, circle, or single segment shape to new client
static void send_shape ( BodyInfo * bi , Server * server) {
    
    rbuff_clear_str ( server->outstr );
    rbuff_scat(server->outstr, "SHAPE {\n", 8);
    
    // append message for type
    if( bi->type == CIRCLE_TYPE) {
        rbuff_scat(server->outstr, "circle\n", 7);
    } else if( bi->type == BOX_TYPE) {
        rbuff_scat ( server->outstr, "box\n", 4 );
    } else { // segment type
        rbuff_scat ( server->outstr, "segment\n", 8 ); 
    }
    
    
    char line[20]; // line to write numbers to message
    
    // run through the parameters to create new shape
    // prepare string to broadcast
    
    // coordinates 
    sprintf ( line, "%.3f\n", bi->p1x );
    rbuff_scat ( server->outstr, line, strlen(line) );
    sprintf ( line, "%.3f\n", bi->p1y );
    rbuff_scat ( server->outstr, line, strlen(line) );
    sprintf ( line, "%.3f\n", bi->p2x );
    rbuff_scat ( server->outstr, line, strlen(line) );
    sprintf ( line, "%.3f\n", bi->p2y );
    rbuff_scat ( server->outstr, line, strlen(line) );
    
    // color 
    sprintf ( line, "%.3f\n", bi->color->r );
    rbuff_scat ( server->outstr, line, strlen(line) );
    sprintf ( line, "%.3f\n", bi->color->g );
    rbuff_scat ( server->outstr, line, strlen(line) );
    sprintf ( line, "%.3f\n", bi->color->b );
    rbuff_scat ( server->outstr, line, strlen(line) );
    
    if ( bi->type != SINGLE_SEGMENT_TYPE ) {
        sprintf ( line, "%.3f\n", bi->ang );
        rbuff_scat ( server->outstr, line, strlen(line) );
    }
    
    // friction, elasticity, density, and index
    sprintf ( line, "%.3f\n", bi->friction );
    rbuff_scat ( server->outstr, line, strlen(line) );
    sprintf ( line, "%.3f\n", bi->elasticity );
    rbuff_scat ( server->outstr, line, strlen(line) );
    sprintf ( line, "%.3f\n", bi->density );
    rbuff_scat ( server->outstr, line, strlen(line) );
    sprintf ( line, "%d\n", bi->index );
    rbuff_scat ( server->outstr, line, strlen(line) );
    
    if ( bi->space_shape_type != 0 ) {
        sprintf ( line, "%d\n",bi->space_shape_type );
        rbuff_scat ( server->outstr, line, strlen(line) );
    }
    
    // finish message to broadcast
    rbuff_scat ( server->outstr, "}\n", 2 );
    
    // send to new client, unless we just want to keep outstr
    if ( server->message_to > 0 ) {
        format_outstr ( server->outstr );
        sendall ( server->message_to, server->outstr->buf, &(server->outstr->ssize) );
    }
    
}

// sends freestyle shape to new client
static void send_freestyle_shape ( BodyInfo * bi, Server * server ) {
    
    rbuff_clear_str ( server->outstr );
    rbuff_scat ( server->outstr, "SHAPE {\n", 8 );
    rbuff_scat ( server->outstr, "freestyle\n", 10 );
    
    
    char line[20]; // line to write numbers to message
    
    // run through the parameters to create new shape
    // prepare string to broadcast
    
    // color
    sprintf ( line, "%.3f\n", bi->color->r );
    rbuff_scat ( server->outstr, line, strlen(line) );
    sprintf ( line, "%.3f\n", bi->color->g );
    rbuff_scat ( server->outstr, line, strlen(line) );
    sprintf ( line, "%.3f\n", bi->color->b );
    rbuff_scat ( server->outstr, line, strlen(line ));
    
    // friction, elasticity, and density
    sprintf ( line, "%.3f\n", bi->friction );
    rbuff_scat ( server->outstr, line, strlen(line) );
    sprintf ( line, "%.3f\n", bi->elasticity );
    rbuff_scat ( server->outstr, line, strlen(line) );
    sprintf ( line, "%.3f\n", bi->density);
    rbuff_scat ( server->outstr, line, strlen(line) );
    
    // freestyle vertices 
    sprintf ( line, "%d\n", bi->num_verts );
    rbuff_scat ( server->outstr, line, strlen(line) );
    
    double x, y;
    for ( int i = 0; i < bi->num_verts; i++ ) {
        
        x = (bi->verts[i]).x;
        y = (bi->verts[i]).y;
        
        sprintf(line, "%.3f\n", x);
        rbuff_scat(server->outstr, line, strlen(line));
        
        sprintf(line, "%.3f\n", y);
        rbuff_scat(server->outstr, line, strlen(line));
        
    }
    
    // index 
    sprintf(line, "%d\n", bi->index );
    rbuff_scat(server->outstr, line, strlen(line));
    
    // finish message to broadcast
    rbuff_scat ( server->outstr, "}\n", 2 );
    
    // send to new client, unless we just want to keep outstr
    if (server->message_to > 0) {
        format_outstr ( server->outstr );
        sendall( server->message_to, server->outstr->buf, &(server->outstr->ssize));
    }
    
}

// creates asteroid
static void create_asteroid ( Server * server ) {
    
    // center of asteroid
    double centerx, centery;
    
    // direction that asteroid will approach from
    int direction = (rand() % 4);
    
    // choose a random center for the asteroid
    if ( direction == 0 ) { // come from bottom
        centerx = rand() % 100;
        centery = -10;
    } else if ( direction == 1 ) { // come from left
        centerx = 0;
        centery = rand() % CORE_MAX_HEIGHT;
    } else if ( direction == 2 ) { // come from top
        centerx = rand() % 100;
        centery = CORE_MAX_HEIGHT + 20;
    } else  { // come from right
        centerx = 150;
        centery = rand() % CORE_MAX_HEIGHT;
    }

    double rad = (rand() % 3) + 2;
    
    // set coordinates 
    double p1x = centerx - rad;
    double p1y = centery + rad;
    double p2x = centerx + rad;
    double p2y = centery - rad;
    
    int type = CIRCLE_TYPE;
    
    Color c;
    c.r = 0.0;
    c.g = 0.0;
    c.b = 0.0;
    
    double ang = 0.0;
    
    double friction = .7;
    double elasticity = 1.0;
    double density = 1;
    
    (server->body_count)++;
    int index = (server->body_count);
    
    cpVect impulse = cpvzero;
    cpVect offset = cpvzero;
    
    // set impulse on asteroid
    if ( direction == 0 ) { // come from bottom
        impulse.x = (rand() % 3) - 1;
        impulse.y = ASIM;
    } else if ( direction == 1 ) { // come from left
        impulse.x = ASIM;
        impulse.y = (rand() % 3) - 1;
    } else if ( direction == 2 ) { // come from top
        impulse.x = (rand() % 3) - 1;
        impulse.y = -ASIM;
    } else  { // come from right
        impulse.x = -ASIM;
        impulse.y = (rand() % 3) - 1;
    }

    
    cpBody * body = core_add_new_asteroid ( server->space, type, p1x, p1y, p2x, p2y, &c, ang, friction, elasticity, density,  index, impulse, offset );
    
    BodyInfo * bi = (BodyInfo *) body->data;
    bi->space_shape_type = 1;
    server->message_to = -1;
    
    send_shape ( bi , server );
    broadcast_data ( server, NULL, 0 );
}

// function to increase the frequency of asteroid creation
static double update_asteroid_timer ( double asteroid_timer_old ) {
    
    double asteroid_timer_new;
    
    // make wave pattern
    if ( asteroid_timer_old < ( 1.0 / 60.0 ) ) {
        asteroid_timer_new = 5.0; // reset wave if getting too fast
    } else if ( asteroid_timer_old > 4.0 ) {
        asteroid_timer_new = asteroid_timer_old * 0.98; // decrease timer slowly at beginning of wave
    } else if ( asteroid_timer_old > 2.0 ) {
        asteroid_timer_new = asteroid_timer_old * 0.90; // decrease timer faster in middle of wave
    } else if ( asteroid_timer_old > 1.0 ) {
        asteroid_timer_new = asteroid_timer_old * 0.95; // decrease timer slower towards end
    } else if ( asteroid_timer_old > 0.3 ) {
        asteroid_timer_new = asteroid_timer_old * 0.98; // decrease timer even slower
    } else {
        asteroid_timer_new = asteroid_timer_old * 0.5; // decrease timer very fast
    }
        
    return asteroid_timer_new;
}




//destroy user-created bodies after BODY_TIMEOUT seconds have passed.
static void body_timeout_check ( cpBody *body, void *data ) {
    
    time_t * now = (time_t *) data;
    
    BodyInfo * bi = (BodyInfo *) body->data;
     
    int sstype = bi->space_shape_type;
        

    if (sstype == 0) { // see if user-defined shape
        
        double time_diff = difftime(*now ,bi->creation_time);
        
        if( time_diff > BODY_TIMEOUT ) { // see if sufficient time has passed
        
            // removes body
            cpSpaceAddPostStepCallback( cpBodyGetSpace(body), (cpPostStepFunc)postStepRemoveBody, body, NULL );
            
        }
    
    }
    
    return;
}

// goes through all shapes in a cpSpace to decide which shapes to remove
static void server_remove_old_shapes ( Server * server, time_t now ) {
    
    cpSpaceEachBody ( server->space, &body_timeout_check, (void *) &now );
}

#ifndef TESTSERV

// main function for server program
// creates server, broadcasts state of a cpSpace to clients
int main ( int argc, char **argv ){
    
    // beginning print statement
    printf("Server for Box Drop\n");
    printf("CS 50 team 2\n");
    printf("press return to quit\n");
    
    
    Server *server = server_new ();
    setup_listener ( SERVER_PORT, server );
    //accept_connection (server);
    //transmit_data (server);
    
    time_t last_update, now;
    time_t last_asteroid;
    last_update = time(NULL);
    now = time(NULL);
    last_asteroid = time(NULL);
    
    double asteroid_timer = 5;
    
    char input = 0;
    while (input != 'q'){
        
        //        input = fgetc(stdin);
        
        *(server->read_fds) = *(server->master);
        if (select((server->fdmax)+1, server->read_fds, NULL, NULL, server->tv) == -1) {
            perror("select");
            exit(4);
        }
        
        // run through the existing connections looking for data to read
        for(int i = 0; i <= server->fdmax; i++) {
            
            if (FD_ISSET(i, server->read_fds)) { // we got one!!
                
                if (i == server->listenfd) { // new connection
                    
                    // form new connection
                    accept_connection( server );
                    
                } else {  // handle data from a client
                    
                    server->message_from = i; // message is from fd i
                    
                    if(i == STDIN) {
                        // quit if enter is hit
                        input = 'q';
                        break;
                    }
                    
                    receive_data( server, i );

                    // just send back contents given to server
                    rbuff_set(server->instr, server->outstr);
                    
                    server_parser(server);
                    
                    // clears data in buffers
                    rbuff_clear_str(server->instr);
                    rbuff_clear_str(server->outstr);
                    
                } // END handle data from client
            } // END got new incoming connection
        } // END looping through file descriptors
        
        // check to see if a space update needs to occur
        now = time(NULL);
        
        // remove user created shapes that have timed out
        server_remove_old_shapes( server, now);
        
        // create an asteroid if appropriate
        if( difftime(now, last_asteroid) >= asteroid_timer ) {
            last_asteroid = now;
            create_asteroid(server);
            asteroid_timer = update_asteroid_timer ( asteroid_timer );
            
        }
        
        // do several space updates before broadcasting
        int num_updates = 4;
        for( int k = 0; k < num_updates; k++) {
            server_space_update(server, SPACE_BROADCAST_TIMESTEP / num_updates );
        }
        
        
        // sleep until time for next cycle in while loop
        now = time(NULL);
        double time_left = SPACE_BROADCAST_TIMESTEP - difftime(now, last_update);
        if(time_left > 0.0) {
            
            struct timespec tim, tim2;
            tim.tv_sec = 0;
            tim.tv_nsec = (long) (time_left * 1000000000);
            
            if(nanosleep(&tim , &tim2) < 0 )   
            {
                printf("Nano sleep system call failed \n");
            }
            
        }
        
        last_update = time(NULL);
        
        // send state of space to clients
        server_space_update(server, 0);
    }
    
    printf("Server Quitting\n");
    
    server_destroy (server);
    
    return EXIT_SUCCESS;
}

#endif

#ifdef TESTSERV

//int main ( int argc, char **argv ){
    
//    Server * server = server_new();
    
//    server_destroy(server);
//}

#endif

