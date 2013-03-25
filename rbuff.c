/* rbuff.c
 resizeable character buffer for boxdrop program
 Nicole Hedley 
 Brad Nelson
 last updated 3/4/13
 */

#include "rbuff.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

// initial number of characters that an rbuff should hold
#define INITIAL_BUFFER_SIZE 256

// Initialize new rbuff
Rbuff * rbuff_new (){
     
     Rbuff *rbuff = ( Rbuff * ) malloc ( sizeof ( Rbuff ) );
     
     rbuff->ssize = 0;
     rbuff->size = INITIAL_BUFFER_SIZE;
     rbuff->buf = (char *) malloc ( INITIAL_BUFFER_SIZE * sizeof(char) );
     assert ( rbuff->buf != NULL );
    
    return rbuff;
}

// double the size of an rbuff's buffer
void rbuff_grow ( Rbuff *rbuff ) {
    
    rbuff->size *= 2;
    
    rbuff->buf = realloc ( rbuff->buf, rbuff->size * sizeof ( char ) );
    assert ( rbuff->buf != NULL );
}

// free the memory in an rbuff
void rbuff_destroy ( Rbuff *rbuff ){
    free ( rbuff->buf );
    free ( rbuff );
}

// set the contents of rbget to rbset
void rbuff_set ( const Rbuff * rbget, Rbuff * rbset ) {
    
    rbuff_clear ( rbset ); // clear memory in rbset
    
    // make rbset large enough to hold contents of rbget
    while ( rbset->size < rbget->size ) {
        rbuff_grow ( rbset );
    }
    
    rbset->ssize = rbget->ssize;
    
    // fills contents of rbset with contents of rbget
    for ( int i = 0; i < rbget->size; i++ ){
        rbset->buf[i] = rbget->buf[i];
    }
}

// clears contents of rbuff
void rbuff_clear ( Rbuff * rb ) {
    
    for ( int i = 0; i < rb->size; i++ ) {
        rb->buf[i] = 0;
    }
    
    rb->ssize = 0;
}

// clears the contents of rbuff up to its string size
void rbuff_clear_str ( Rbuff * rb ) {
    
    for ( int i = 0; i < rb->ssize; i++ ) {
        rb->buf[i] = 0;
    }
    
    rb->ssize = 0;
}

// concatenate contents of read rbuff to write rbuff
void rbuff_cat ( Rbuff * write, Rbuff * read ) {
    
    rbuff_scat ( write, read->buf, read->ssize );
}

// concatenate the contents of a string to the end of an Rbuff
void rbuff_scat ( Rbuff * write, char * read, const int size ) {
    
    // get rid of terminating char for string
    if ( write->ssize > 0 ) {
        while ( write->ssize > 0 && write->buf[write->ssize - 1] <= 0 ) {
            ( write->ssize )--;
        } 
    }
    
    // make destination large enough to hold total buffer
    while ( write->size <= write->ssize + size + 1 ) 
        rbuff_grow ( write ); 
    
    // add the buffer of read up to ssize to the buffer of write
    int i = 0;
    for ( i = 0; i < size; i++ ) {
        if ( read[i] == 0 ) {
            write->buf[i + write->ssize ] = ' ';
        } else {
            write->buf[i + write->ssize ] = read[i];
        }
    }
     
    write->buf[write->ssize + i + 1] = 0; // make sure string is null-terminated
    
    write->ssize += size ; // set new length of string in destination buffer
}

// finds the size of a string in the rbuff buffer, sets the ssize of the rbuff to that size
int rbuff_set_ssize ( Rbuff * rb ) {
    
    int s = 0;
    while ( rb->buf[s] != 0 ) {
        s++;
    }
    
    s++;
    rb->ssize = s;
    
    return rb->ssize;
}


#ifdef TESTRB
// test routine for growing
int main ( int argc, char **argv )  {
    
    Rbuff * rb = rbuff_new ();
    printf ( "starting rbuff size: %d \n", rb->size );
    
    char str[20] = "hello everyone! ";
    printf ( "str = %s, len = %d \n", str, (int) strlen(str) );
    
    for ( int i = 1; i < 30; i++ ) {
        rbuff_scat ( rb, str, strlen(str) );
        printf ( "i = %d\n", i );
        printf ( "rbuff ssize: %d \n", rb->ssize );
        printf ( "rbuff size: %d \n", rb->size );
        
    }
    
    printf ( "rbuff buffer:\n %s\n", rb->buf );
    
    Rbuff * rb2 = rbuff_new ();
    rbuff_cat ( rb2, rb );
    printf ( "rbuff 2 ssize: %d \n", rb2->ssize );
    printf ( "rbuff 2 size: %d \n", rb2->size );
    
    rbuff_destroy ( rb );
    rbuff_destroy ( rb2 );
}
#endif


// test routine for concatenation    
#ifdef TESTRBCAT

int main ( int argc, char **argv )  {


    Rbuff * rb1= rbuff_new ();
    rbuff_scat ( rb1, "hello hello", 11 );
    
    printf ( "rb1->buf: %s\n", rb1->buf );
    printf ( "rb1->ssize: %d\n", rb1->ssize );
    printf ( "rb1->size: %d\n", rb1->size );
    
    
    rbuff_scat ( rb1, " hello", 6 );
    
    printf ( "rb1->buf: %s\n", rb1->buf );
    printf ( "rb1->ssize: %d\n", rb1->ssize );
    printf ( "rb1->size: %d\n", rb1->size );


    Rbuff * rb2= rbuff_new ();
    rbuff_scat ( rb2, " Brad!", 6 );
    
    printf ( "rb2->buf: %s\n", rb2->buf );
    printf ( "rb2->ssize: %d\n", rb2->ssize );
    printf ( "rb2->size: %d\n", rb2->size );
    
    rbuff_cat(rb1, rb2);

    printf ( "rb1->buf: %s\n", rb1->buf );
    printf ( "rb1->ssize: %d\n", rb1->ssize );
    printf ( "rb1->size: %d\n", rb1->size );


    return 1;
}

#endif
