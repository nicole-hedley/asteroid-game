/*
 boxdrop.c
 main program to be run in boxdrop executable
 team 2: EXIT_SUCCESS
 BJN 2/17/13
*/

#include "gui.h"
#include <chipmunk.h>

int main ( int argc, char ** argv ) {
    
    // initialize space * to be used in gtk callbacks
    cpSpace * space = core_create_space ();
    core_load_level ( space, "./levels/default.level" );
    
    gtk_init ( &argc, &argv );
    
    box_drop_gui_new ( space );
    
    gtk_main ();
    
    return 0;   
}
