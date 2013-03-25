/* rbuff.h
 resizeable buffer for boxdrop program
 Nicole Hedley 
 Brad Nelson
 last updated 3/4/13
 */

#ifndef RBUFF_H
#define RBUFF_H

// data structure for a resizable character buffer
typedef struct rbuff Rbuff;
struct rbuff {
    char *buf; // buffer
    int size; // max size
    int ssize; // current string size
};

// creates new Rbuff
Rbuff * rbuff_new ();

// doubles size of buffer
void rbuff_grow ( Rbuff *rbuff );
// destroys rbuff
void rbuff_destroy ( Rbuff *rbuff );

// sets contents of rbset to be the same as the contents of rbget
void rbuff_set ( const Rbuff * rbget, Rbuff * rbset );

// clears contents of Rbuff
void rbuff_clear ( Rbuff * rb );
// only clears contents of Rbuff up to size of ssize
void rbuff_clear_str ( Rbuff * rb );

// concatenate the contents of one Rbuff to the end of another
void rbuff_cat ( Rbuff * write, Rbuff * read );

// concatenate the contents of a string to the end of an Rbuff
void rbuff_scat ( Rbuff * write, char * read, const int size );

// sets the ssize to size of string in buffer
// returns the size of the string
int rbuff_set_ssize ( Rbuff * rb );

#endif