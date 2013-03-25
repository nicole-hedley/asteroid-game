/*
 gui.c
 gui for boxdrop game.
 Team 2: EXIT_SUCCESS
 Tommy Kidder
 updated 03/05/13
 */

#define _BSD_SOURCE

#include "graphics.h"
#include "core.h"
#include "gui.h"
#include "rbuff.h"
#include "client.h"

#include <sys/time.h>
#include <time.h>
#include <stdlib.h>
#include <chipmunk.h>
#include <stddef.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <time.h>


#define TIMESTEP (1.0 / 60.0)
#define SERVER_TIMESTEP (1.0 / 120.0)
#define true 1
#define false 0

#define DEFAULT_ORIENTATION 0
#define DEFAULT_FRICTION 0.7
#define DEFAULT_ELASTICITY 0.5
#define DEFAULT_DENSITY 1
#define SEGMENT_MIN 10
#define MIN_RADIUS 1
#define CHAT_BOX_HEIGHT 200

// static function declarations
static GuiInfo * create_GuiInfo ( cpSpace * space );
static void destroy_GuiInfo ( GuiInfo *g );
static void get_main_menu ( GtkWidget ** vbox, gpointer guiinfo );
static void get_color_toolbar ( GtkWidget **vbox, gpointer data );
static void get_shape_toolbar ( GtkWidget **vbox, gpointer data );

// callbacks
static gboolean draw_cb ( GtkWidget *widget, cairo_t *cr, gpointer data );
static gboolean on_button_press ( GtkWidget* widget, GdkEventButton * event, gpointer data );
static gboolean on_button_release ( GtkWidget* widget, GdkEventButton * event, gpointer data );
gboolean on_mouse_motion (GtkWidget* widget, GdkEventButton * event, gpointer data);
static void gui_quick_message ( gchar *message, GtkWindow *window );

//time step functions
static void *draw_window_step ( void * data );
static void *recv_from_server_step ( void *data );

// called by time step functions to update data and perform drawing
static void draw_window ( GuiInfo *data );
static void recv_from_server ( GuiInfo *data );

// button callbacks 
static void gui_load_file ( GtkWidget *window, gpointer data );
static void gui_connect_server ( GtkWidget *window, gpointer data );
static void gui_restart ( GtkWidget *window, gpointer data );
static void gui_quit ( GtkWidget *window, gpointer data );

// chat box functions 
static void get_chatbox ( GtkWidget ** vbox, gpointer data );
static void send_message ( GtkWidget *window, gpointer data );
static void push_message ( char *message, gpointer data );

// client functions 
static void get_client_update ( gpointer data );
static int read_line ( char ** message, Rbuff * line );

// multiplayer space update functions
static void gui_multi_space_update ( GuiInfo * g, char ** message, Rbuff * line );
static void gui_multi_body_update ( cpBody * body, void * data);

// drawing functions 
static void gui_draw_box ( GtkButton *button, GuiInfo *g );
static void gui_draw_circle ( GtkButton *button, GuiInfo *g );
static void gui_draw_segment ( GtkButton *button, GuiInfo *g );
static void gui_draw_freestyle ( GtkButton *button, GuiInfo *g );
static void gui_draw_cannon ( GtkButton *button, GuiInfo *g );
static void gui_reset_cannon_mode ( GuiInfo *g );

static void gui_convert_freestyle_verts_to_cp ( GuiInfo *g );
static void gui_normalize_brightness ( GuiInfo *g );
static void gui_update_color_sample ( GuiInfo *g );

// parsing functions
static void parse_transmission ( gpointer data );
static void parse_box ( GuiInfo *g, char ** message, Rbuff * line );
static void parse_circle ( GuiInfo *g, char ** message, Rbuff * line );
static void parse_segment ( GuiInfo *g, char ** message, Rbuff * line );
static void parse_freestyle ( GuiInfo *g, char ** message, Rbuff * line );
static void parse_homebase ( GuiInfo *g, char ** message, Rbuff * line );


#ifdef TEST
int main( int argc, char ** argv ) {
    
    // initialize space * to be used in gtk callbacks
    cpSpace * space;
    space = NULL;
    
    //initialize graphics and gui
    gtk_init ( &argc, &argv );
    box_drop_gui_new ( space );    
    gtk_main ();
    return 0;
}
#endif

// set up gui 
void box_drop_gui_new ( cpSpace * space ){
    
    GuiInfo *g = create_GuiInfo ( space );
    
    GtkWidget *window;
    GtkWidget *vbox;
    
    //set up window, contains vbox
    window = gtk_window_new ( GTK_WINDOW_TOPLEVEL );
    g->window = window;
    gtk_window_set_position ( GTK_WINDOW ( window ), GTK_WIN_POS_CENTER );
    gtk_window_set_default_size ( GTK_WINDOW ( window ), WINDOW_HEIGHT + CHAT_BOX_HEIGHT+500, WINDOW_HEIGHT + CHAT_BOX_HEIGHT );
    gtk_window_set_title ( GTK_WINDOW ( window ), "Boxdrop" );
    
    //vbox, contains menu, toolbar, drawing area
    vbox = gtk_box_new ( GTK_ORIENTATION_VERTICAL, 1 );
    gtk_box_set_homogeneous ( GTK_BOX ( vbox ), FALSE );
    gtk_container_add ( GTK_CONTAINER ( window ), vbox );
    
    get_main_menu ( &vbox, g );
    get_color_toolbar ( &vbox, g );
    get_shape_toolbar ( &vbox, g );
    
    g->darea = gtk_drawing_area_new();
    gtk_box_pack_start(GTK_BOX(vbox), g->darea, TRUE, TRUE, 0);
    gtk_widget_set_size_request(g->darea, WINDOW_HEIGHT, WINDOW_HEIGHT);
    
    get_chatbox ( &vbox, g );
    
    
    // Callbacks 
    g_signal_connect ( G_OBJECT (g->darea), "draw", G_CALLBACK ( draw_cb ), g );
    g_signal_connect(G_OBJECT(g->darea), "button-press-event", G_CALLBACK(on_button_press), g);
    g_signal_connect(G_OBJECT(g->darea), "button-release-event", G_CALLBACK(on_button_release), g);

    g_signal_connect(G_OBJECT(g->darea), "motion-notify-event", G_CALLBACK(on_mouse_motion), g);
    g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(gui_quit), g);
    
    gtk_widget_add_events(g->darea, GDK_BUTTON_PRESS_MASK);
    gtk_widget_add_events(g->darea, GDK_BUTTON_RELEASE_MASK);
    gtk_widget_add_events(g->darea, GDK_BUTTON_MOTION_MASK);
    
    
    //initiate mutex
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    g->mutex = mutex;
    
    g->quit = false;
    //initiate threads
    pthread_t draw_thread;
    pthread_t recv_thread;
    
    //start threads running
    pthread_create(&draw_thread, NULL, draw_window_step, g);
    pthread_create(&recv_thread, NULL, recv_from_server_step, g);
    
    g->drawthread_over = false;
    g->recvthread_over = false;

    gtk_widget_show_all(window);
}


//step function to draw shapes, called by draw_thread
static void *draw_window_step(void * data) {
    GuiInfo *g = (GuiInfo *)data;
    
    while ( g->quit == false ) {
        draw_window ( data );
        
        struct timespec tim, tim2;
        tim.tv_sec = 0;
        tim.tv_nsec = TIMESTEP*1000*1000*1000;
        
        if ( nanosleep ( &tim , &tim2 ) < 0 )   
        {
            printf ( "Nano sleep system call failed \n" );

        }
    }
    g->drawthread_over = true;
    return NULL;
}
//step function to receive msgs from server, called by recv_thread
static void *recv_from_server_step(void *data) {
    GuiInfo *g = (GuiInfo *)data;

    while(g->quit == false){

        recv_from_server(data);
        
        struct timespec tim, tim2;
        tim.tv_sec = 0;
        tim.tv_nsec = TIMESTEP*1000*1000*1000;
        
        if ( nanosleep ( &tim , &tim2 ) < 0 ) {
            printf ( "Nano sleep system call failed \n" );
        }
        
    }
    g->recvthread_over = true;
    return NULL;
}

// creates the chatbox and activates the gtk entry bar
static void get_chatbox ( GtkWidget ** vbox, gpointer data ) {
    
    GuiInfo *g = (GuiInfo *) data;
    ChatInfo *c = (ChatInfo *) malloc(sizeof(ChatInfo));
    
    g->chat = c;
    
    GtkTextBuffer *textbuff = gtk_text_buffer_new(NULL);
    GtkWidget *text_area = gtk_text_view_new_with_buffer(textbuff);
    GtkWidget *scrolledwindow = gtk_scrolled_window_new(NULL, NULL);
    GtkEntryBuffer *entrybuff = gtk_entry_buffer_new(NULL, 100);
    GtkWidget *entrybox = gtk_entry_new_with_buffer(entrybuff);
    
    gtk_text_view_set_editable(GTK_TEXT_VIEW(text_area), FALSE);  
    gtk_container_add(GTK_CONTAINER(scrolledwindow), text_area);
    
    gtk_entry_set_activates_default(GTK_ENTRY(entrybox), TRUE); 
    gtk_box_pack_end(GTK_BOX(*vbox), entrybox, FALSE, FALSE, 1);
    
    gtk_box_pack_end(GTK_BOX(*vbox), scrolledwindow, FALSE, FALSE, 1);
    
    c->textarea = text_area;
    c->entry = entrybox;
    
    g_signal_connect(G_OBJECT(entrybox), "activate", G_CALLBACK(send_message), g);
    
}


static void send_message(GtkWidget *window, gpointer data){
    GuiInfo *g = (GuiInfo *) data;
    
    ChatInfo *c = g->chat;
    Client *client = g->client;
    
    const gchar *message = gtk_entry_get_text ( GTK_ENTRY ( c->entry ) ); // get message from GTK entry
    
    if ( g->client == NULL ) return;
    
    rbuff_clear(client->outstr); // makes sure that client outstr is clear
    int maxsize = strlen(message) + 15; // size of format message
    char *formatmessage = (char *) malloc(sizeof(char) * maxsize);
    
    sprintf ( formatmessage, "CHAT {\n%s\n}\n", message ); // format message 
    
    // put the formatted message into the client's outstring and send it to the server with client_puts 
    rbuff_scat ( client->outstr, formatmessage, strlen ( formatmessage ) ); 
    client_puts ( client );
    free ( formatmessage );
    
    // clear the gtk entry 
    GtkEntryBuffer *buffer = gtk_entry_get_buffer ( GTK_ENTRY ( c->entry ) );
    gtk_entry_buffer_delete_text ( buffer, 0, -100 );
    
}

// put chat message into the textview box 
static void push_message(char *message, gpointer data){
    
    printf ( "%s\n", message );
    GuiInfo *g = (GuiInfo *) data;
    
    ChatInfo *c = g->chat;
    
    GtkTextBuffer *msgtext = gtk_text_view_get_buffer ( (GtkTextView *) c->textarea ); 
    GtkTextIter writer;

    gtk_text_buffer_get_iter_at_offset ( msgtext, &writer, -1 );
    gtk_text_buffer_insert ( msgtext, &writer, message, -1 );
}

// initialize a main menu
static void get_main_menu(GtkWidget ** vbox, gpointer data) {
    
    GuiInfo *g = ( GuiInfo * ) data;
    
    GtkWidget *menubar;
    GtkWidget *filemenu;
    
    menubar = gtk_menu_bar_new (); //menubar is a menu shell
    filemenu = gtk_menu_new (); //filemenu is a submenu
    
    // create menu items
    GtkWidget * file = gtk_menu_item_new_with_label ( "File" );
    GtkWidget * load = gtk_menu_item_new_with_label ( "Load" );
    GtkWidget * reset = gtk_menu_item_new_with_label ( "Reset" );
    GtkWidget * multiplayer = gtk_menu_item_new_with_label ( "Multiplayer" );
    GtkWidget * quit = gtk_menu_item_new_with_label ( "Quit" );
    
    // append the menu items to the appropriate menus
    gtk_menu_item_set_submenu ( GTK_MENU_ITEM ( file ), filemenu );
    gtk_menu_shell_append ( GTK_MENU_SHELL ( filemenu ), load );
    gtk_menu_shell_append ( GTK_MENU_SHELL ( filemenu ), reset );
    gtk_menu_shell_append ( GTK_MENU_SHELL ( filemenu ), multiplayer );
    gtk_menu_shell_append ( GTK_MENU_SHELL ( filemenu ), quit );
    gtk_menu_shell_append ( GTK_MENU_SHELL ( menubar ), file );
    
    // menu item callbacks 
    g_signal_connect ( G_OBJECT ( quit ), "activate", G_CALLBACK ( gui_quit ), g );
    g_signal_connect ( G_OBJECT ( load ), "activate", G_CALLBACK ( gui_load_file ), g );
    g_signal_connect ( G_OBJECT ( reset ), "activate", G_CALLBACK ( gui_restart ), g ); 
    g_signal_connect ( G_OBJECT ( multiplayer ), "activate", G_CALLBACK ( gui_connect_server ), g );
    
    gtk_box_pack_start ( GTK_BOX (*vbox), menubar, FALSE, FALSE, 3 );
}


// draw callback
static gboolean draw_cb ( GtkWidget *widget, cairo_t *cr, gpointer data ) {
    
    GuiInfo *g = ( GuiInfo * ) data;
    
    g->gi->cr = cr;
    
    //lock gtk thread
    pthread_mutex_lock(&(g->mutex));

    if (strcmp((char *)g->space->data, "./levels/default.level") == 0 || g->multiplayer == true){
        cairo_set_source_rgb ( cr, 0, 0, 0 );
    }
    else{
        cairo_set_source_rgb ( cr, 0, 0, 0 );
    }
    cairo_paint ( cr );
    
    if ( g->multiplayer == false ) {
        g->asteroidcount ++;
        
        if ( g->asteroidcount == 60 ) {
            for ( int i = 0; i < 2; i++ ) {
                core_create_asteroid ( g->space );
            }
            g->asteroidcount = 0;
        }
        
    }
    
    // draw temporary shapes if the drawing is in progress
    if ( g->draw_in_progress == true ) {
        graphics_draw_temp(cr, g);
        cairo_stroke(cr);
    }
    
    // if in homebase mode draw temporary homebase
    if ( g->homebase_mode == true ) {
        graphics_draw_temp_homebase(cr, g);
        cairo_stroke(cr);
    }
    
    // if in cannon mode draw the cannon shape
    if ( g->cannon_mode == true ) {
        graphics_draw_cannon ( cr, (void*)g );
        cairo_stroke ( cr );
    }

    graphics_draw_space ( cr, g );
    
    pthread_mutex_unlock(&(g->mutex));
    return false;
}


// load file callback
static void gui_load_file (GtkWidget *widget, gpointer data){
    
    
    GtkWidget * window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_title(GTK_WINDOW(window), "Load File");

    
    GuiInfo *g = ( GuiInfo * ) data;
    g->hello = false;
    
    g->homebase_mode = true;
    if ( g->multiplayer == true ) {
        gui_quick_message ( (gchar *) "Cannot load file in multiplayer mode!", GTK_WINDOW ( window ) );
        return;
    }
    
    GtkWidget *dialog;
    dialog = gtk_file_chooser_dialog_new ( "Open File",
                                          (GtkWindow *) window, //parent window
                                          GTK_FILE_CHOOSER_ACTION_OPEN, //only open file action type allowed
                                          GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, //adds "cancel" item
                                          GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, //adds "open" item
                                          NULL ); //ending stocking items
    
    gint response = gtk_dialog_run ( GTK_DIALOG ( dialog ) );
    
    // if the user picks a level, check the level name, then load the lvel
    if ( response  == GTK_RESPONSE_ACCEPT) {
        
        char *level;
        
        level = gtk_file_chooser_get_filename ( GTK_FILE_CHOOSER ( dialog ) );
        
        GuiInfo *g = ( GuiInfo * ) data;
        
        cpSpace * space = core_create_space ();
        g->space = space;
        g->filename = level;
        int err = core_check_level_name ( level );
        printf ( "Error in core_check_level_name = %d\n\n", err );
        
        if ( err == EXIT_FAILURE ) return;
        
        // opens file
        FILE * fp;
        if ( (fp = fopen(level, "r") ) == NULL ) {
            fprintf ( stderr, "%s: File could not be opened\n\n", level );
            return;
        }
        
        err = core_load_level ( g->space, level );
        
        printf ( "Error in core_load_level = %d\n", err );
        fclose ( fp );
        gtk_widget_destroy ( dialog );
        gtk_widget_destroy ( window );
        
    } else if ( response == GTK_RESPONSE_CANCEL ) { // if the user cancels destroy the dialog
        
        gtk_widget_destroy (dialog);
        gtk_widget_destroy ( window );
    }   
}

// multiplayer callback. connects the client to the server of given address and port 
static void gui_connect_server ( GtkWidget *widget, gpointer data ){
    
    GtkWidget * window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_title(GTK_WINDOW(window), "Server Connect");
    
    GuiInfo *g = ( GuiInfo * ) data;
    g->hello = false;
    
    GtkWidget *dialog, *label, *content_area, *server_entry, *port_entry, *username_entry;
    GtkEntryBuffer *server_buffer, *port_buffer, *username_buffer;
    
    
    
    
    /* Create the widgets */
    dialog = gtk_dialog_new_with_buttons ("Multiplayer",
                                          (GtkWindow *)window,
                                          GTK_DIALOG_DESTROY_WITH_PARENT,
                                          GTK_STOCK_OK,
                                          GTK_RESPONSE_ACCEPT,
                                          NULL);
    
    content_area = gtk_dialog_get_content_area (GTK_DIALOG (dialog));

    label = gtk_label_new ("Connect to Server");
    
    const gchar *server_buffer_initial = "127.0.0.1";
    gint server_initial_length = 14;
    const gchar *port_buffer_initial = "4123";
    gint port_initial_length = 4;
    const gchar *username_buffer_initial = "Type a name here!";
    gint username_initial_length = 17;
    
    server_buffer = gtk_entry_buffer_new(server_buffer_initial, server_initial_length);
    gtk_entry_buffer_set_max_length (server_buffer, 20);
    server_entry = gtk_entry_new_with_buffer(server_buffer);
    
    
    port_buffer = gtk_entry_buffer_new(port_buffer_initial, port_initial_length);
    gtk_entry_buffer_set_max_length (port_buffer, 10);
    port_entry = gtk_entry_new_with_buffer(port_buffer);
    
    username_buffer = gtk_entry_buffer_new(username_buffer_initial, username_initial_length);
    gtk_entry_buffer_set_max_length (username_buffer, 20);
    username_entry = gtk_entry_new_with_buffer(username_buffer);
    
    
    gtk_dialog_add_action_widget(GTK_DIALOG(dialog), server_entry, GTK_RESPONSE_NONE);
    gtk_dialog_add_action_widget(GTK_DIALOG(dialog), port_entry, GTK_RESPONSE_NONE);
    gtk_dialog_add_action_widget(GTK_DIALOG(dialog), username_entry, GTK_RESPONSE_NONE);
    

    // Add the label, and show everything we've added to the dialog
    gtk_container_add (GTK_CONTAINER (content_area), label);
    gtk_widget_show_all (dialog);
    
    gint response = gtk_dialog_run (GTK_DIALOG (dialog));
    
    if ( response  == GTK_RESPONSE_ACCEPT ) {
        const gchar *address =  gtk_entry_get_text(GTK_ENTRY(server_entry));
        const gchar *port =  gtk_entry_get_text(GTK_ENTRY(port_entry));
        const gchar *username = gtk_entry_get_text(GTK_ENTRY(username_entry));
        
        
        g->client = client_connect ( (char *) address, atoi(port) );
        if ( g->client == NULL )
            printf ("connection failed");
        else {
            
            gui_restart ( g->window, g );
            g->multiplayer = true;
            printf ("connection successful");
            
            // send NAME {} message here
            rbuff_clear(g->client->outstr);
            rbuff_scat(g->client->outstr, "NAME {\n", 7);
            rbuff_scat(g->client->outstr, (gchar *) username , strlen(username));
            rbuff_scat(g->client->outstr, "\n}\n", 3);
            client_puts(g->client);
            
        }
    }
    
    gtk_widget_destroy(dialog);
    gtk_widget_destroy ( window );
    
    if (g->multiplayer == true){
        gui_quick_message("Click on an ideal location to build your home base", (GtkWindow *)g->window);
        g->homebase_mode = true;
    }
}

// parses the transmission from the server and acts according to the type of server message
static void parse_transmission ( gpointer data ) {
    
    GuiInfo *g = ( GuiInfo * ) data;
    Client *client = g->client;
    Rbuff *line = rbuff_new ();
    
    char * message = client->instr->buf; // starts at beginning of message
    while ( read_line ( &message, line ) == 1 ) {
        
        if ( strncmp ( line->buf, "GAMEOVER", 8 ) == 0 ){
            printf("message from outerspace: \n%s\n", message);
            printf ( "One planet exploded.\n ");
            printf("Homeplanet index: %d", g->homebase_index);
            
            read_line(&message, line);
            int destroyed_planet = atoi(line->buf);
            if(destroyed_planet == g->homebase_index){
                GameInfo * gi = (GameInfo *)g->space->data;
                gi->explode = true;
                gi->explodex = CORE_MAX_HEIGHT*1.2;
                gi->explodey = CORE_MAX_HEIGHT/2;
                gi->gameisover = 1;
                g->multiplayer = false;
            }
        }
        // if it is a chat message, push the message into the textview
        if ( strncmp ( line->buf, "CHAT {", 6 ) == 0 ) {

            read_line ( &message, line );
            push_message ( line->buf, data );
        }
        
        // if it is a space message, update the space
        else if ( strncmp ( line->buf, "SPACE {", 7 ) == 0 ) {
            
            gui_multi_space_update ( g, &message, line );
        }
        
        // if it is a shape message, adds the shape to the cpSpace
        else if ( strncmp ( line->buf, "SHAPE {", 7 ) == 0 ) {
            
            read_line ( &message, line );
            
            if ( strncmp ( line->buf, "box", 3 ) == 0 ) {
                parse_box ( g, &message, line );
            }
            if ( strncmp ( line->buf, "circle", 6 ) == 0 ) {
                parse_circle ( g, &message, line );
            }
            if ( strncmp ( line->buf, "segment", 7 ) == 0 ) {
                parse_segment ( g, &message, line );
            }
            if ( strncmp ( line->buf, "freestyle", 9 ) == 0 ){
                parse_freestyle ( g, &message, line );
            }
        }
        
        // parse the homebase
        else if ( strncmp ( line->buf, "HOMEBASE {", 10 ) == 0 ) {
            read_line ( &message, line );
            parse_homebase ( g, &message, line );
        }

    }
    
    rbuff_destroy ( line );
    rbuff_clear ( client->instr );
}

// parse the homebase
static void parse_homebase(GuiInfo *g, char ** message, Rbuff * line){
    double homebasex = atof( line->buf);
    
    read_line ( message, line );
    double homebasey = atof( line->buf);
    
    read_line(message, line);
    int start_index = atoi(line->buf);
    
    read_line(message, line);
    int planet_num = atoi(line->buf);

    core_make_homebase(g->space, homebasex, homebasey, start_index, planet_num);
    
    if(g->home_planet_set == false && g->homebase_mode == false){
        g->homebase_index = start_index + 4;
        g->home_planet_set = true;
    }
}


// parses instructions to make a new box in master cpSpace, creates box, sends out instructions to client for new box
static void parse_box ( GuiInfo *g, char ** message, Rbuff * line ) {
    
    // run through the parameters to create new shape and prepares string to broadcast
    
    //coordinates
    read_line ( message, line );
    double p1x = atof ( line->buf );
    read_line ( message, line );
    double p1y = atof ( line->buf );
    read_line ( message, line );
    double p2x = atof ( line->buf );
    read_line ( message, line );
    double p2y = atof ( line->buf );
    
    // color 
    Color c;
    read_line ( message, line );
    c.r = atof ( line->buf );
    read_line ( message, line );
    c.g = atof ( line->buf );
    read_line ( message, line );
    c.b = atof ( line->buf );
    
    // angle, friction, elasticity, density
    read_line ( message, line );
    double ang = atof ( line->buf );
    read_line ( message, line );
    double friction = atof ( line->buf );
    read_line ( message, line );
    double elasticity = atof ( line->buf );
    read_line ( message, line );
    double density = atof ( line->buf );
    
    // index
    read_line ( message, line );
    int index = atoi ( line->buf );
    
    // add the new shape to the cpSpace
    core_add_new_shape ( g->space, BOX_TYPE, p1x, p1y, p2x, p2y, &c, ang, friction, elasticity, density, index );
    
}

// parses instructions to make a new circle in master cpSpace, creates circle, sends out instructions to client for new circle
static void parse_circle ( GuiInfo *g, char ** message, Rbuff * line ) {
    
    // run through the parameters to create new shape, prepares shape to broadcast
    
    // x and y coordinates
    read_line ( message, line );
    double p1x = atof ( line->buf );
    read_line ( message, line );
    double p1y = atof ( line->buf );
    read_line ( message, line );
    double p2x = atof ( line->buf );
    read_line ( message, line );
    double p2y = atof ( line->buf );
    
    // color
    Color c;
    read_line ( message, line );
    c.r = atof ( line->buf );
    read_line ( message, line );
    c.g = atof ( line->buf );
    read_line ( message, line );
    c.b = atof ( line->buf );
    
    // angle, friction, elasticity, density
    read_line ( message, line );
    double ang = atof ( line->buf );
    read_line ( message, line );
    double friction = atof ( line->buf );
    read_line ( message, line );
    double elasticity = atof ( line->buf );
    read_line ( message, line );
    double density = atof( line->buf );
    
    // index
    read_line ( message, line );
    int index = atoi ( line->buf );
    
    
    // add the new shape to the cpSpace
    cpShape * shape = core_add_new_shape ( g->space, CIRCLE_TYPE, p1x, p1y, p2x, p2y, &c, ang, friction, elasticity, density, index );
    
    DrawShapeInfo * dsi = ( DrawShapeInfo * ) shape->data;
    read_line (message, line);
    if(strncmp(line->buf, "}", 1) != 0) {
        int sstype = atoi(line->buf);
        dsi->space_shape_type = sstype;
    }

}

// parses instructions to make a new segment in master cpSpace, creates segment, sends out instructions to segment for new circle
static void parse_segment ( GuiInfo *g, char ** message, Rbuff * line ) {
    
    // run through the parameters to create new shape, prepares shape to broadcast
    
    // x and y coordinates 
    read_line ( message, line );
    double p1x = atof ( line->buf );
    read_line ( message, line );
    double p1y = atof ( line->buf );
    read_line ( message, line );
    double p2x = atof ( line->buf );
    read_line ( message, line );
    double p2y = atof ( line->buf );
    
    // color 
    Color c;
    read_line ( message, line );
    c.r = atof ( line->buf );
    read_line ( message, line );
    c.g = atof ( line->buf );
    read_line ( message, line );
    c.b = atof ( line->buf );
    
    // friction, elasticity, density
    read_line ( message, line );
    double friction = atof ( line->buf );
    read_line ( message, line );
    double elasticity = atof ( line->buf );
    read_line ( message, line );
    double density = atof ( line->buf );
    
    // index
    read_line ( message, line );
    int index = atoi ( line->buf );
    
    // add the new shape to the cpSpace
    core_add_single_segment_shape ( g->space, p1x, p1y, p2x, p2y, &c, friction, elasticity, density, index );
}

// parses instructions to make a new segment in master cpSpace, creates segment, sends out instructions to segment for new circle
static void parse_freestyle (GuiInfo *g, char ** message, Rbuff * line){
    
    // run through the parameters to create new shape, prepares shape to broadcast
    
    // color 
    Color c;
    read_line ( message, line );
    c.r = atof ( line->buf );
    read_line ( message, line );
    c.g = atof ( line->buf );
    read_line ( message, line );
    c.b = atof ( line->buf );
    
    // friction, elasticity, and density
    read_line ( message, line );
    double friction = atof ( line->buf );
    read_line ( message, line );
    double elasticity = atof ( line->buf );
    read_line ( message, line );
    double density = atof ( line->buf );
    
    // number of vertices 
    read_line ( message, line );
    int num_verts = atoi ( line->buf );
    
    // sets of x and y vertices 
    cpVect * verts = (cpVect *) malloc ( sizeof(cpVect) * num_verts );
    double x, y;
    for ( int i = 0; i < num_verts; i++ ) {
        
        read_line ( message, line );
        x = atof ( line->buf );
        
        read_line ( message, line );
        y = atof ( line->buf );
        
        verts[i].x = x;
        verts[i].y = y;
    }
    
    // index 
    read_line ( message, line );
    int index = atoi ( line->buf );
    
    // add the new shape to the cpSpace
    core_add_freestyle_shape ( g->space, verts , num_verts, &c, friction, elasticity, density, index );
    
    free ( verts );
}


// updates the cpSpace in a multiplayer game when a SPACE{} message is sent
static void gui_multi_space_update ( GuiInfo * g, char ** message, Rbuff * line ) {
    
    read_line ( message, line );

    if ( (*message)[0] == '}' ) return;
    
    int table_size = 64;
    
    // update table to keep track of what is being updated, and its coordinates
    double * update_table = (double *) malloc ( sizeof (double) * table_size );
    
    // parse the rest of the message
    while ( strncmp ( *message, "}", 1 ) != 0 && (*message)[0] != 0 ) {
        
        int index = atoi ( line->buf );
        // check to see that table is large enough.  If not make larger
        while ( table_size < ( index + 1 ) * 4 ) {
            table_size *= 2;
            update_table = (double *) realloc ( update_table, sizeof(double) * table_size );
        }
        
        read_line ( message, line );
        double x = atof ( line->buf);
        read_line( message, line );
        double y = atof ( line->buf );
        read_line( message, line );
        double ang = atof ( line->buf );

        update_table[index * 4] = x;
        update_table[index * 4 + 1] = y;
        update_table[index * 4 + 2] = ang;
        update_table[index * 4 + 3] = index;
        read_line ( message, line );
        
    }
    
    cpSpaceEachBody ( g->space, &gui_multi_body_update, update_table );
}

// updates a cpBody given information in an update table
static void gui_multi_body_update ( cpBody * body, void * data ) {
    
    double * update_table = (double *) data;
    int index = *((int *) body->data);
    
    double x = update_table[index * 4];
    double y = update_table[index * 4 + 1];
    double ang = update_table[index * 4 + 2];
    int update_index = update_table[index * 4 + 3];
    
    if ( update_index == 0 ) {
        cpSpaceAddPostStepCallback( cpBodyGetSpace(body), (cpPostStepFunc)postStepRemoveBody, body, NULL );
    }
    else {
        body->p.x = x;
        body->p.y = y;
        body->a = ang;
    }
    
    return;
}

// reads off line from string, stores in address of
// returns 1 if not finished and 0 if messsage has already finished
static int read_line ( char ** message, Rbuff * line ) {
    
    // if message has finished return 0
    if ( (**message == EOF) || (**message == 0) ) return 0;
    
    rbuff_clear_str ( line ); // clears out last string
    
    int i = 0;
    while( (**message >= 32) ) { // while char is readable
        
        if ( line->size <= i ) rbuff_grow ( line ); // makes sure buffer is long enough
        
        line->buf[i] = **message;
        (*message)++; // increment where message is pointing to
        i++;
    }
    
    (*message)++;
    line->buf[i] = '\n';
    line->buf[i+1] = 0;
    line->ssize = i+1;
    
    return 1;
}

// updates the space if not in multiplayer mode and redraws the space 
static void draw_window ( GuiInfo *g ){
    
    if ( g->space == NULL ) return;
    
    if ( g->multiplayer == false ){
        pthread_mutex_lock(&(g->mutex));
        core_update_space ( g->space, (double)( TIMESTEP ) );
        
        pthread_mutex_unlock(&(g->mutex));
    }
    
    gtk_widget_queue_draw ( GTK_WIDGET ( g->darea ) );
    
    return;
}

// if in multiplayer mode get the updated space from the server 
static void recv_from_server ( GuiInfo *g ){
    
    if ( g->multiplayer == false )
        return;
    
    //update from server
    get_client_update(g);    
}

// updates client from the server and parses the transmission
static void get_client_update ( gpointer data ) {
    
    GuiInfo *g = ( GuiInfo * ) data;
    Client *client = g->client;
    
    if ( client != NULL ) {
        
        client_gets ( client );
        
        //lock resources while parsing
        pthread_mutex_lock(&(g->mutex));
        
        if ( client->instr != NULL ) {
            if ( client->instr->buf[0] != 0 ) {             
                parse_transmission ( data );
                rbuff_clear ( client->instr );
            }
        }
        
        //unlock after parsing is over
        pthread_mutex_unlock(&(g->mutex));
    }
}


// when the user presses the mouse the x and y coordinates should be saved
gboolean on_button_press ( GtkWidget* widget, GdkEventButton * event, gpointer data ) {
    
    GuiInfo *g = ( GuiInfo * ) data;
    if(g->homebase_mode == false){
        
        g->draw_in_progress = true;
        
        if ( event->type == GDK_BUTTON_PRESS ) {
            g->clickx = event->x;
            g->clicky = event->y;
            g->mousex = event->x;
            g->mousey = event->y;
        }
        
        if ( g->draw_state == FREESTYLE_TYPE ) {
            //set first vertex, initial click
            g->verts[0] = cpv ( g->clickx, g->clicky );
            g->num_verts = 1;
        }
        
        // if we are drawing a cannon
        if ( g->cannon_mode == true ) {
            
            // but the cannon hasn't been drawn yet
            if ( g->cannon_drawn == false ) {
                g->cannon_fired = false;
                
                g->cannon_type = g->draw_state;
                g->cannon_clickx = event->x;
                g->cannon_clicky = event->y;
                
                if ( g->draw_state == FREESTYLE_TYPE ) {
                    
                    //set first vertex, initial click
                    g->cannon_verts[0] = cpv ( g->clickx, g->clicky );
                    g->cannon_num_verts = 1;
                }
            }
            
            //if cannon hasn't been fired yet
            else if ( g->cannon_fired == false ){
                g->cannon_impulse_startx = event->x;
                g->cannon_impulse_starty = event->y;
            }
        }
    }
    
    else {
        g->draw_homebase = true;
        g->mousex = event->x;
        g->mousey = event->y;
    }
    return false;
}

// when the user releases the mouse a shape should be drawn
gboolean on_button_release ( GtkWidget* widget, GdkEventButton * event, gpointer data ) {
    
    GuiInfo *g = ( GuiInfo * ) data;
    
    if ( event->type == GDK_BUTTON_RELEASE ) {
        
        cpVect vectstart = cpv ( 0, 0 );
        cpVect vectfinish = cpv ( 0, 0 );
        graphics_click_to_cpvec ( &vectstart, g->clickx, g->clicky );
        graphics_click_to_cpvec ( &vectfinish, event->x, event->y );
        
        
        if ( g->multiplayer == false ) {
            
            //if not drawing cannon
            if( g->cannon_mode == false ) {
                
                // draw box
                if ( g->draw_state == BOX_TYPE ) {
                    
                    core_add_new_shape ( g->space, BOX_TYPE, vectstart.x, vectstart.y, vectfinish.x, vectfinish.y, g->color, g->orientation, g->elasticity, g->friction, g->density, 0 );
                }
                // draw a circle
                if ( g->draw_state == CIRCLE_TYPE && ( abs ( vectfinish.x-vectstart.x ) > MIN_RADIUS) && ( abs ( vectfinish.y - vectstart.y ) > MIN_RADIUS ) ) {
                    
                    core_add_new_shape ( g->space, CIRCLE_TYPE, vectstart.x, vectstart.y, vectfinish.x, vectfinish.y, g->color, g->orientation, g->elasticity, g->friction, g->density, 0 );
                }
                // draw a segment
                if ( g->draw_state == SINGLE_SEGMENT_TYPE ) {
                    
                    core_add_single_segment_shape ( g->space, vectstart.x, vectstart.y, vectfinish.x, vectfinish.y, g->color, g->friction, g->elasticity, g->density, 0 );
                }
                //draw free style
                if ( g->draw_state == FREESTYLE_TYPE ) {
                    
                    gui_convert_freestyle_verts_to_cp ( g );
                    core_add_freestyle_shape ( g->space, g->verts, g->num_verts, g->color, g->friction, g->elasticity, g->density, 0 );
                    g->num_verts = 0;
                }
            }
            
            //cannon mode is true
            else {
                
                if ( g->cannon_drawn == false ) { //finishing drawing cannon
                    g->cannon_dragx = event->x;
                    g->cannon_dragy = event->y;
                    g->cannon_drawn = true;
                }
                
                else { //cannon already drawn //finishing drawing impulse vector
                    g->cannon_impulse_endx = event->x;
                    g->cannon_impulse_endy = event->y;
                    
                    g->cannon_fired = true;
                    g->cannon_drawn = false;
                    
                    //get click and end points of cannon
                    cpVect vectstart = cpv ( 0, 0 );
                    cpVect vectfinish = cpv ( 0, 0 );
                    graphics_click_to_cpvec ( &vectstart, g->cannon_clickx, g->cannon_clicky );
                    graphics_click_to_cpvec ( &vectfinish, g->cannon_dragx, g->cannon_dragy );
                    
                    //get impulse and offset of cannon, offset not yet implemented
                    double impulsex = ( g->cannon_impulse_endx - g->cannon_impulse_startx ) *CORE_MAX_HEIGHT / WINDOW_HEIGHT;
                    double impulsey = ( g->cannon_impulse_starty - g->cannon_impulse_endy ) *CORE_MAX_HEIGHT / WINDOW_HEIGHT;
                    cpVect impulse = cpv ( impulsex, impulsey );

                    // draw box with impulse
                    if ( g->draw_state == BOX_TYPE ) {
                        
                        core_add_new_shape_with_impulse ( g->space, BOX_TYPE, vectstart.x, vectstart.y, vectfinish.x, vectfinish.y, g->color, g->orientation, g->friction, g->elasticity, g->density, 0, impulse, cpv ( 0, 0 ) );
                    }
                    
                    // draw circle with impulse 
                    if ( g->draw_state == CIRCLE_TYPE && ( abs ( vectfinish.x - vectstart.x ) > MIN_RADIUS ) && ( abs ( vectfinish.y - vectstart.y ) > MIN_RADIUS ) ) {
                        
                        core_add_new_shape_with_impulse ( g->space, CIRCLE_TYPE, vectstart.x, vectstart.y, vectfinish.x, vectfinish.y, g->color, g->orientation, g->friction, g->elasticity, g->density, 0, impulse, cpv ( 0, 0) );
                    }
                    
                    // draw segment with impulse
                    if ( g->draw_state == SINGLE_SEGMENT_TYPE ) {
                        core_add_new_shape_with_impulse ( g->space, SINGLE_SEGMENT_TYPE, vectstart.x, vectstart.y, vectfinish.x, vectfinish.y, g->color, 0, g->friction, g->elasticity, g->density, 0, impulse, cpv ( 0, 0 ) );
                    }
                    
                    // draw free style with impulse
                    if ( g->draw_state == FREESTYLE_TYPE ) {
                        
                        gui_convert_freestyle_verts_to_cp ( g );
                        core_add_freestyle_shape_with_impulse ( g->space, g->cannon_verts, g->cannon_num_verts, g->color, g->friction, g->elasticity, g->density, 0, impulse, cpv(0, 0) );
                        g->num_verts = 0;
                    }
                } 
            }
            
            if ( g->homebase_mode == true ) {
                core_make_homebase ( g->space, g->mousex * CORE_MAX_HEIGHT / WINDOW_HEIGHT, ( WINDOW_HEIGHT - g->mousey ) * CORE_MAX_HEIGHT / WINDOW_HEIGHT, 1, 2 );
                g->homebase_mode = false;
            }
        }
        
        else { // if in multiplayer mode, send shape requests to the server
            Client *client = g->client;
            rbuff_clear ( client->outstr );
            
            int msize = 200; // size of format message
            char *formatmessage = (char *) malloc ( sizeof (char) * msize );
            
            if ( g->homebase_mode == true ) {

                //send message to server to set homebase
                
                double x = (g->mousex) * CORE_MAX_HEIGHT / WINDOW_HEIGHT;
                double y = ( WINDOW_HEIGHT - g->mousey ) * CORE_MAX_HEIGHT / WINDOW_HEIGHT;
                Rbuff *homebase_msg = rbuff_new ();
                sprintf ( formatmessage, "HOMEBASE {\n" );
                
                rbuff_scat ( homebase_msg, formatmessage, strlen ( formatmessage ) );
                
                sprintf( formatmessage, "%f\n%f\n}\n", x, y);
                rbuff_scat ( homebase_msg, formatmessage, strlen ( formatmessage ) );
                
                rbuff_scat ( client->outstr, homebase_msg->buf, strlen ( homebase_msg->buf ) );
                
                g->draw_homebase = false;
                g->homebase_mode = false;
            }
            else{
                double im_x = 0;
                double im_y = 0;
                
                if ( g->cannon_mode == true ) {
                    if ( g->cannon_drawn == false ) { //finishing drawing cannon
                        g->cannon_dragx = event->x;
                        g->cannon_dragy = event->y;
                        g->cannon_drawn = true;
                        g->draw_in_progress = false;
                        return FALSE;
                    } else {
                        g->cannon_impulse_endx = event->x;
                        g->cannon_impulse_endy = event->y;
                        
                        g->cannon_fired = true;
                        g->cannon_drawn = false;
                        
                        //get click and end points of cannon
                        vectstart = cpv ( 0, 0 );
                        vectfinish = cpv ( 0, 0 );
                        graphics_click_to_cpvec ( &vectstart, g->cannon_clickx, g->cannon_clicky );
                        graphics_click_to_cpvec ( &vectfinish, g->cannon_dragx, g->cannon_dragy );
                        
                        //get impulse and offset of cannon, offset not yet implemented
                        im_x = ( g->cannon_impulse_endx - g->cannon_impulse_startx ) *CORE_MAX_HEIGHT / WINDOW_HEIGHT;
                        im_y = ( g->cannon_impulse_starty - g->cannon_impulse_endy ) *CORE_MAX_HEIGHT / WINDOW_HEIGHT;                        
                    }
                }
                
                // send box request
                if ( g->draw_state == BOX_TYPE ) {
                    
                    if ( g->cannon_mode == false ) {
                        
                        sprintf ( formatmessage, "REQ {\nbox\n%f\n%f\n%f\n%f\n%f\n%f\n%f\n%f\n%f\n%f\n%f\n}\n", vectstart.x, vectstart.y, vectfinish.x, vectfinish.y,  g->color->r, g->color->g, g->color->b,  g->orientation, g->friction, g->elasticity, g->density );
                    } else {
                        sprintf ( formatmessage, "REQ {\nbox\ncannon\n%f\n%f\n%f\n%f\n%f\n%f\n%f\n%f\n%f\n%f\n%f\n%f\n%f\n%f\n%f\n}\n", im_x, im_y, 0.0, 0.0,vectstart.x, vectstart.y, vectfinish.x, vectfinish.y,  g->color->r, g->color->g, g->color->b,  g->orientation, g->friction, g->elasticity, g->density );
                    }
                    
                    rbuff_scat ( client->outstr, formatmessage, strlen ( formatmessage ) );
                    
                }
                
                // send circle request
                if ( g->draw_state == CIRCLE_TYPE && ( abs ( vectfinish.x - vectstart.x ) > MIN_RADIUS ) && ( abs ( vectfinish.y - vectstart.y ) > MIN_RADIUS ) ) {
                    
                    if ( g->cannon_mode == false ) {
                        sprintf ( formatmessage, "REQ {\ncircle\n%f\n%f\n%f\n%f\n%f\n%f\n%f\n%f\n%f\n%f\n%f\n}\n", vectstart.x, vectstart.y, vectfinish.x, vectfinish.y, g->color->r, g->color->g, g->color->b, g->orientation, g->friction, g->elasticity, g->density );
                    } else {
                        
                        sprintf ( formatmessage, "REQ {\ncircle\ncannon\n%f\n%f\n%f\n%f\n%f\n%f\n%f\n%f\n%f\n%f\n%f\n%f\n%f\n%f\n%f\n}\n", im_x, im_y, 0.0, 0.0,vectstart.x, vectstart.y, vectfinish.x, vectfinish.y, g->color->r, g->color->g, g->color->b, g->orientation, g->friction, g->elasticity, g->density );
                    }
                    printf ( "\nFORMATTED MESSAGE IS \n%s\n", formatmessage );
                    
                    rbuff_scat ( client->outstr, formatmessage, strlen ( formatmessage ) );
                    
                }
                
                // send segment request
                if ( g->draw_state == SINGLE_SEGMENT_TYPE ) {
                    
                    if ( g->cannon_mode == false ) {
                        sprintf ( formatmessage, "REQ {\nsegment\n%f\n%f\n%f\n%f\n%f\n%f\n%f\n%f\n%f\n%f\n}\n", vectstart.x, vectstart.y, vectfinish.x, vectfinish.y, g->color->r, g->color->g, g->color->b, g->friction, g->elasticity, g->density );
                    } else {

                        sprintf ( formatmessage, "REQ {\nsegment\ncannon\n%f\n%f\n%f\n%f\n%f\n%f\n%f\n%f\n%f\n%f\n%f\n%f\n%f\n%f\n}\n", im_x, im_y, 0.0, 0.0,vectstart.x, vectstart.y, vectfinish.x, vectfinish.y, g->color->r, g->color->g, g->color->b, g->friction, g->elasticity, g->density );
                    }
                    printf ( "\nFORMATTED MESSAGE IS \n%s\n", formatmessage );
                    
                    rbuff_scat ( client->outstr, formatmessage, strlen ( formatmessage ) );
                }
                
                // send freestyle request
                if ( g->draw_state == FREESTYLE_TYPE ) {
                    
                    if ( g->num_verts <= 1 ) {
                        free ( formatmessage );
                        g->draw_in_progress = false;
                        return FALSE;
                    }
                    
                    Rbuff *freestylereq = rbuff_new ();
                    sprintf ( formatmessage, "REQ {\nfreestyle\n" );
                    
                    rbuff_scat ( freestylereq, formatmessage, strlen ( formatmessage ) );
                    
                    if ( g->cannon_mode == true ) {

                        sprintf( formatmessage, "cannon\n%f\n%f\n%f\n%f\n", im_x, im_y, 0.0, 0.0);
                        rbuff_scat ( freestylereq, formatmessage, strlen ( formatmessage ) );
                    }
                    
                    int strsize = 500;
                    char *restofstring = (char *) malloc ( sizeof (char) * strsize );
                    if (g->cannon_mode == false ) {
                        sprintf ( restofstring, "%.3f\n%.3f\n%.3f\n%.3f\n%.3f\n%.3f\n%d\n",  g->color->r, g->color->g, g->color->b, g->friction, g->elasticity, g->density, g->num_verts);
                    } else {
                        sprintf ( restofstring, "%.3f\n%.3f\n%.3f\n%.3f\n%.3f\n%.3f\n%d\n",  g->color->r, g->color->g, g->color->b, g->friction, g->elasticity, g->density, g->cannon_num_verts);
                    }
                    rbuff_scat (freestylereq, restofstring, strlen ( restofstring ) );
                    
                    free ( restofstring );
                    
                    int vertsize = 20;
                    char *vertwrite = (char *) malloc ( sizeof (char) * vertsize );
                    
                    // write each pair of vertices to the request string 
                    if ( g->cannon_mode == true) {
                        gui_convert_freestyle_verts_to_cp(g);
                        for ( int i = 0; i < g->cannon_num_verts; i++ ) {
                            
                            gdouble x = g->cannon_verts[i].x;
                            gdouble y = g->cannon_verts[i].y;
                            
                            sprintf ( vertwrite, "%.3f\n%.3f\n", x, y );
                            rbuff_scat ( freestylereq, vertwrite, strlen ( vertwrite ) );
                        }
                        
                    } else {
                        for ( int i = 0; i < g->num_verts; i++ ) {
                            
                            cpVect vertvect = cpv ( 0, 0 );
                            graphics_click_to_cpvec ( &vertvect, g->verts[i].x, g->verts[i].y );
                            
                            gdouble x = vertvect.x;
                            gdouble y = vertvect.y;
                            
                            sprintf ( vertwrite, "%.3f\n%.3f\n", x, y );
                            rbuff_scat ( freestylereq, vertwrite, strlen ( vertwrite ) );
                        }
                    }
                    
                    free ( vertwrite );
                    rbuff_scat ( freestylereq, "}\n", 3 );
                    
                    printf ( "THIS is my freestyle message: \n%s\n", freestylereq->buf );
                    rbuff_set_ssize ( freestylereq );
                    rbuff_cat ( client->outstr, freestylereq );
                    
                    
                    rbuff_destroy ( freestylereq );
                }
                
                
            }
            // send the message to the server and free the format message
            client_puts ( client );
            free ( formatmessage );
        }
    }
    //finished drawing
    g->draw_in_progress = false;
    return FALSE;
}

// mouse position getter
gboolean on_mouse_motion ( GtkWidget* widget, GdkEventButton * event, gpointer data ) {
    
    GuiInfo *g = ( GuiInfo * ) data;
    g->mousex = event->x;
    g->mousey = event->y;
    return true;
}

//helper method for freestyle drawing, converts all verts to chipmunk coordinates.
static void gui_convert_freestyle_verts_to_cp ( GuiInfo *g ) {
    
    if ( g->cannon_mode == false ) {
        for ( int i = 0; i < g->num_verts; i++ ) {
            cpVect core_vect;
            graphics_click_to_cpvec ( &core_vect, g->verts[i].x, g->verts[i].y );
            g->verts[i] = core_vect;
        }
    }
    
    if ( g->cannon_mode == true ) {
        for ( int i = 0; i < g->cannon_num_verts; i++ ) {
            cpVect core_vect;
            graphics_click_to_cpvec ( &core_vect, g->cannon_verts[i].x, g->cannon_verts[i].y );
            g->cannon_verts[i] = core_vect;
        }
    }
}

// quit button callback. quits the game and destroys all allocated memory
static void gui_quit ( GtkWidget *window, gpointer data ) {
    
    GuiInfo *g = ( GuiInfo * ) data;
    g->quit = true;
    printf( "i quit\n");
    while(g->drawthread_over == false || g->recvthread_over ==false){};
    
    if ( g->space != NULL ) {
        printf ( "Space is not null.\n" );
        
        cpSpaceAddPostStepCallback ( g->space, (cpPostStepFunc)core_destroy_space, NULL, NULL );
        g->space = NULL;
    }
    
    destroy_GuiInfo ( g );
    gtk_main_quit();
}

// restart button callback. resets the current space to the original level space, only works in single player mode 
static void gui_restart ( GtkWidget *window, gpointer data ) {
    
    GuiInfo *g = ( GuiInfo * ) data;
    g->hello = false;
    
    if ( g->multiplayer == true ) {
        gui_quick_message((gchar *) "Cannot restart in multiplayer mode!", GTK_WINDOW(window));
        return;
    }
    
    pthread_mutex_lock(&g->mutex);
    cpSpaceAddPostStepCallback (g->space, (cpPostStepFunc)core_destroy_space, NULL, NULL);
    pthread_mutex_unlock(&g->mutex);

    cpSpace * space = core_create_space();
    g->space = space;
    
    g->homebase_mode = true;
    
    if (g->filename[0] == 0) return;
    // opens file
    FILE * fp;
    if ( (fp = fopen(g->filename, "r") ) == NULL ) {
        fprintf(stderr, "%s: File could not be opened\n\n", g->filename);
        return;
    }

    core_load_level ( g->space, g->filename );
}

// create a GuiInfo struct to store all the values that need to be passed around
static void render_graphics_images( GuiInfo *g ){
    
    char * asteroidfp =  "./asteroidicons/Asteroid.png";
    char * Earthfp =  "./asteroidicons/Earth.png";
    char * planet_rfp =  "./asteroidicons/RedPlanet.png";
    char * planet_gfp =  "./asteroidicons/GreenPlanet.png";
    char * planet_yfp =  "./asteroidicons/GoldPlanet.png";
    char * planet_pfp =  "./asteroidicons/PurplePlanet.png";
    
    GraphicsInfo * graphics_info = (GraphicsInfo *) malloc(sizeof(GraphicsInfo));
    
    cairo_surface_t *a_image = cairo_image_surface_create_from_png (asteroidfp);
    cairo_surface_t *e_image = cairo_image_surface_create_from_png (Earthfp);
    cairo_surface_t *r_image = cairo_image_surface_create_from_png (planet_rfp);
    cairo_surface_t *g_image = cairo_image_surface_create_from_png (planet_gfp);
    cairo_surface_t *y_image = cairo_image_surface_create_from_png (planet_yfp);
    cairo_surface_t *p_image = cairo_image_surface_create_from_png (planet_pfp);
    
    graphics_info->asteroid = a_image;
    graphics_info->earth = e_image;
    graphics_info->planet_r = r_image;
    graphics_info->planet_g = g_image;
    graphics_info->planet_y = y_image;
    graphics_info->planet_p = p_image;
    
    g->gi = graphics_info;
}


// create a GuiInfo struct to store all the values that need to be passed around
static GuiInfo * create_GuiInfo(cpSpace * space){
    
    GuiInfo *g = (GuiInfo *) malloc(sizeof(GuiInfo));
    
    Color *c = (Color *) malloc(sizeof(Color));
    c->r = 1;
    c->g = 1;
    c->b = 1;
    c->rslide = 100;
    c->gslide = 100;
    c->bslide = 100;

    
    GdkRGBA *samplecolor = (GdkRGBA *) malloc ( sizeof ( GdkRGBA ) );
    char *spec = (char *) malloc ( sizeof (char) * 100 );
    
    // currently holds up to 1000 line segments
    cpVect * vs = (cpVect *) malloc(1000 * sizeof(cpVect));
    cpVect *cannon_vs = (cpVect *) malloc(1000 * sizeof(cpVect));
    
    char *filename = malloc(sizeof(char) * 60);
    // clear memory in filename
    for( int i = 0; i<60; i++) {
        filename[i] = 0;
    }
    g->filename = filename;
    
    g->draw_state = FREESTYLE_TYPE;
    g->space = space;
    g->color = c;
    g->colorspec = spec;
    g->samplecolor = samplecolor;
    g->sample = NULL;
    g->verts = vs;
    g->num_verts = 0;
    g->orientation = DEFAULT_ORIENTATION;
    g->friction = DEFAULT_FRICTION;
    g->elasticity = DEFAULT_ELASTICITY;
    g->density = DEFAULT_DENSITY;
    g->draw_in_progress = false;
    g->client = NULL;   
    g->chat = NULL;
    g->gi = NULL;
    g->multiplayer = false;
    g->homebase_mode = true;
    g->draw_homebase = false;
    g->hello = true;
    
    //cannon initiation
    g->cannon_verts = cannon_vs;
    g->cannon_num_verts = 0;
    g->cannon_mode = false;
    g->cannon_drawn = false;
    g->cannon_fired = false;
    g->draw_cannon = false;
    
    g->asteroidcount = 0;
    g->game_over_flux = 0;
    g->game_over_step = 0;
    
    render_graphics_images(g);
    g->home_planet_set = false;

    return g;    
}

// free space from GuiInfo struct
static void destroy_GuiInfo ( GuiInfo *g ) {
    
    // destroy color 
    if ( g->colorspec != NULL ) {
        free ( g->colorspec );
    }
    if ( g->sample != NULL ) {
        gtk_widget_destroy( g->sample );
    }
    if ( g->samplecolor != NULL ) {
        gdk_rgba_free ( g->samplecolor );
    }
    if ( g->color != NULL ) {
        free ( g->color );
    }
    // destroy chat and graphicsinfo
    if ( g-> chat != NULL ) {
        free ( g->chat );
    }
    if ( g-> gi != NULL){
        free ( g->gi );
    }
    
    // destroy verts 
    if ( g->verts != NULL ) {
        free ( g->verts );
    }
    if ( g->cannon_verts != NULL ) {
        free ( g->cannon_verts );
    }
    
    // destroy space, filename, and client
    if ( g->space != NULL ) {
        core_destroy_space ( g->space );
    }
    if ( g->filename != NULL ) {
        free ( g->filename );
    }
    if ( g->client != NULL ) {
        free ( g->client );
    }
    if ( g != NULL ) free ( g );
}

// functions to change the colors when the toolbar is clicked
static void changecolor_red ( GtkWidget *slider, gpointer data ) {
    GuiInfo *g = ( GuiInfo * ) data;
    
    g->color->rslide = gtk_range_get_value ( GTK_RANGE ( slider ) );
    g->friction = g->color->rslide/100;
    
    gui_normalize_brightness ( g );
    gui_update_color_sample ( g );
}

static void changecolor_green ( GtkWidget *slider, gpointer data ){
    GuiInfo *g = ( GuiInfo * ) data;
    
    g->color->gslide = gtk_range_get_value ( GTK_RANGE ( slider ) );
    g->elasticity = g->color->gslide/100;
    
    gui_normalize_brightness ( g );
    gui_update_color_sample ( g );
    
}

static void changecolor_blue ( GtkWidget *slider, gpointer data ){
    GuiInfo *g = ( GuiInfo * ) data;
    
    g->color->bslide = gtk_range_get_value ( GTK_RANGE ( slider ) );
    g->density = g->color->rslide/20 + .1;
    
    gui_normalize_brightness ( g );
    gui_update_color_sample ( g );
    
}

// normalizes the color brightness 
static void gui_normalize_brightness ( GuiInfo *g ) {
    
    g->color->r = .25 + g->color->rslide * .0075;
    g->color->g = .25 + g->color->gslide * .0075;
    g->color->b = .25 + g->color->bslide * .0075;
}

// creates the color toolbar
static void get_color_toolbar ( GtkWidget **vbox, gpointer data ){
    GuiInfo *g = ( GuiInfo * ) data;
    
    GtkToolItem *redicon;
    GtkToolItem *greenicon;
    GtkToolItem *blueicon;
    GtkWidget *toolbar;
    GtkWidget *colorsample;
    
    // red 
    GtkWidget *redscale = gtk_scale_new_with_range ( GTK_ORIENTATION_HORIZONTAL, (gdouble) 0, (gdouble) 100, (gdouble) 1 );
    GtkToolItem *redslider = gtk_tool_item_new ();
    gtk_tool_item_set_expand ( redslider, TRUE );
    gtk_container_add ( GTK_CONTAINER ( redslider ), redscale );
    
    // green 
    GtkWidget *greenscale = gtk_scale_new_with_range ( GTK_ORIENTATION_HORIZONTAL, (gdouble) 0, (gdouble) 100, (gdouble) 1 );
    GtkToolItem *greenslider = gtk_tool_item_new ();
    gtk_tool_item_set_expand ( greenslider, TRUE );
    gtk_container_add ( GTK_CONTAINER ( greenslider ), greenscale );
    
    // blue 
    GtkWidget *bluescale = gtk_scale_new_with_range ( GTK_ORIENTATION_HORIZONTAL, (gdouble) 0, (gdouble) 100, (gdouble) 1 );
    GtkToolItem *blueslider = gtk_tool_item_new ();
    gtk_tool_item_set_expand ( blueslider, TRUE );
    gtk_container_add ( GTK_CONTAINER ( blueslider ), bluescale );
    
    
    toolbar = gtk_toolbar_new();
    gtk_toolbar_set_style(GTK_TOOLBAR(toolbar), GTK_TOOLBAR_ICONS);
    
    gtk_container_set_border_width ( GTK_CONTAINER ( toolbar ), 2 );
    
    const gchar *rtext = "Red";
    redicon = gtk_tool_item_new ();
    GtkWidget *rlabel = gtk_label_new ( rtext );
    gtk_container_add ( GTK_CONTAINER ( redicon ), rlabel );
    gtk_range_set_value ( GTK_RANGE(redscale), g->color->rslide );
    gtk_toolbar_insert ( GTK_TOOLBAR ( toolbar ), GTK_TOOL_ITEM ( redicon ), -1 );
    gtk_toolbar_insert ( GTK_TOOLBAR ( toolbar ), redslider, -1 );
    
    
    const gchar *gtext = "Green";
    greenicon = gtk_tool_item_new ();
    GtkWidget *glabel = gtk_label_new ( gtext );
    gtk_container_add ( GTK_CONTAINER ( greenicon ), glabel );
    gtk_range_set_value ( GTK_RANGE(greenscale), g->color->rslide );
    gtk_toolbar_insert ( GTK_TOOLBAR ( toolbar ), GTK_TOOL_ITEM ( greenicon ), -1 );
    gtk_toolbar_insert ( GTK_TOOLBAR ( toolbar ), greenslider, -1 );
    
    const gchar *btext = "Blue";
    blueicon = gtk_tool_item_new ();
    GtkWidget *blabel = gtk_label_new ( btext );
    gtk_container_add ( GTK_CONTAINER ( blueicon ), blabel );
    gtk_range_set_value ( GTK_RANGE(bluescale), g->color->rslide );
    gtk_toolbar_insert ( GTK_TOOLBAR ( toolbar ), GTK_TOOL_ITEM ( blueicon ), -1 );
    gtk_toolbar_insert ( GTK_TOOLBAR ( toolbar ), blueslider, -1 );
    
    
    GtkToolItem *sampleicon = gtk_tool_item_new ();
    colorsample = gtk_color_button_new ();
    g->sample = colorsample;
    gtk_container_add ( GTK_CONTAINER ( sampleicon ), colorsample );
    gtk_toolbar_insert ( GTK_TOOLBAR ( toolbar ), sampleicon, -1 );

    gui_update_color_sample ( g );
    
    g_signal_connect ( G_OBJECT ( redscale ), "value_changed", G_CALLBACK ( changecolor_red ), g );
    g_signal_connect ( G_OBJECT ( greenscale ), "value_changed", G_CALLBACK ( changecolor_green ), g );
    g_signal_connect ( G_OBJECT ( bluescale ), "value_changed", G_CALLBACK ( changecolor_blue ), g );
    
    
    gtk_box_pack_start ( GTK_BOX (*vbox), toolbar, FALSE, FALSE, 5 );
}


static void get_shape_toolbar ( GtkWidget **vbox, gpointer data ){
    
    GuiInfo *g = ( GuiInfo * ) data;
    
    GtkToolItem *box_item;
    GtkToolItem *circle_item;
    GtkToolItem *segment_item;
    GtkToolItem *freestyle_item;
    GtkToolItem *cannon_item;
    
    GtkWidget *toolbar;
    gint width = 25, height = 25;
    
    const char *boxfp = "./toolbaricons/squarepic.png";
    const char *circlefp = "./toolbaricons/circlepic.png";
    const char *segmentfp = "./toolbaricons/segmentpic.png";
    const char *freestylefp = "./toolbaricons/freestylepic.png";
    const char *cannonfp = "./toolbaricons/cannonballpic.png";
    
    toolbar = gtk_toolbar_new ();
    gtk_toolbar_set_style ( GTK_TOOLBAR(toolbar), GTK_TOOLBAR_ICONS );
    gtk_container_set_border_width ( GTK_CONTAINER(toolbar), 2 );
    
    // box
    GdkPixbuf *box_pix_icon = gdk_pixbuf_new_from_file_at_size ( boxfp, width, height, NULL ); 
    GtkWidget *box_img_icon = gtk_image_new_from_pixbuf ( box_pix_icon );
    box_pix_icon = NULL;
    box_item = gtk_tool_button_new ( box_img_icon, NULL );
    gtk_toolbar_insert ( GTK_TOOLBAR(toolbar), box_item, -1 );
    
    // circle 
    GdkPixbuf *circle_pix_icon = gdk_pixbuf_new_from_file_at_size ( circlefp, width, height, NULL );
    GtkWidget *circle_img_icon = gtk_image_new_from_pixbuf ( circle_pix_icon );
    circle_pix_icon = NULL;
    circle_item = gtk_tool_button_new ( circle_img_icon, NULL );
    gtk_toolbar_insert ( GTK_TOOLBAR ( toolbar ), circle_item, -1 );
    
    // segment
    GdkPixbuf *segment_pix_icon = gdk_pixbuf_new_from_file_at_size ( segmentfp, width, height, NULL ); 
    GtkWidget *segment_img_icon = gtk_image_new_from_pixbuf ( segment_pix_icon );
    segment_pix_icon = NULL;
    segment_item = gtk_tool_button_new ( segment_img_icon, NULL ); 
    gtk_toolbar_insert ( GTK_TOOLBAR ( toolbar ), segment_item, -1 );
    
    // freestyle 
    GdkPixbuf *freestyle_pix_icon = gdk_pixbuf_new_from_file_at_size ( freestylefp, width, height, NULL ); 
    GtkWidget *freestyle_img_icon = gtk_image_new_from_pixbuf ( freestyle_pix_icon );
    freestyle_pix_icon = NULL;
    freestyle_item = gtk_tool_button_new ( freestyle_img_icon, NULL ); 
    gtk_toolbar_insert ( GTK_TOOLBAR ( toolbar ), freestyle_item, -1 );
    
    // cannon 
    GdkPixbuf *cannon_pix_icon = gdk_pixbuf_new_from_file_at_size ( cannonfp, width, height, NULL ); 
    GtkWidget *cannon_img_icon = gtk_image_new_from_pixbuf ( cannon_pix_icon );
    cannon_pix_icon = NULL; 
    cannon_item = gtk_tool_button_new ( cannon_img_icon, NULL ); 
    gtk_toolbar_insert ( GTK_TOOLBAR ( toolbar ), cannon_item, -1 );
    
    
    gtk_box_pack_start ( GTK_BOX (*vbox), toolbar, FALSE, FALSE, 5 );
    
    // set callbacks
    g_signal_connect ( box_item, "clicked", G_CALLBACK ( gui_draw_box ), g );
    g_signal_connect ( circle_item, "clicked", G_CALLBACK ( gui_draw_circle ), g );
    g_signal_connect ( segment_item, "clicked", G_CALLBACK ( gui_draw_segment ), g );
    g_signal_connect ( freestyle_item, "clicked", G_CALLBACK ( gui_draw_freestyle ), g );
    g_signal_connect ( cannon_item, "clicked",  G_CALLBACK( gui_draw_cannon ), g );
    
}

// changes the color sample to the current color 
static void gui_update_color_sample ( GuiInfo *g ) {
    
    
    int red = (int) ( g->color->r * 255.0 );
    int green = (int) ( g->color->g * 255.0 );
    int blue = (int) ( g->color->b * 255.0 );
    
    sprintf ( g->colorspec, "rgb(%d,%d,%d)", red, green, blue );
    
    gdk_rgba_parse ( g->samplecolor, g->colorspec );
    
    gtk_color_chooser_set_rgba ( (GtkColorChooser *) g->sample, g->samplecolor );
}

// pops up a warning to the user 
static void gui_quick_message ( gchar *message, GtkWindow *window ) {
    
    GtkWidget *dialog, *label, *content_area;
    
    /* Create the widgets */
    dialog = gtk_dialog_new_with_buttons ( "Alert!", window, GTK_DIALOG_DESTROY_WITH_PARENT, GTK_STOCK_OK, GTK_RESPONSE_NONE, NULL );
    
    content_area = gtk_dialog_get_content_area ( GTK_DIALOG ( dialog ) );
    label = gtk_label_new ( message );
    
    g_signal_connect_swapped ( dialog, "response", G_CALLBACK ( gtk_widget_destroy ), dialog );
    
    gtk_container_add ( GTK_CONTAINER ( content_area ), label );
    gtk_widget_show_all ( dialog );
}


// Callback function to set draw state to box
static void gui_draw_box ( GtkButton *button, GuiInfo *g ) {
    g->draw_state = BOX_TYPE;
    gui_reset_cannon_mode ( g );
    
}

// Callback function to set draw state to circle
static void gui_draw_circle ( GtkButton *button, GuiInfo *g ) {
    g->draw_state = CIRCLE_TYPE;
    gui_reset_cannon_mode ( g );
    
}

// Callback function to set draw state to segment
static void gui_draw_segment ( GtkButton *button, GuiInfo *g ) {
    g->draw_state = SINGLE_SEGMENT_TYPE;
    gui_reset_cannon_mode ( g );
    
}

// Callback function to set draw state to freestyle
static void gui_draw_freestyle ( GtkButton *button, GuiInfo *g ) {
    g->draw_state = FREESTYLE_TYPE;
    gui_reset_cannon_mode ( g );
    
}

// Callback function to set cannon mode
static void gui_draw_cannon ( GtkButton *button, GuiInfo *g ) {
    gui_reset_cannon_mode ( g );
    g->cannon_mode = true;
}

// resets the cannon mode 
static void gui_reset_cannon_mode ( GuiInfo *g ){
    
    g->cannon_mode = false;
    g->cannon_drawn = false;
    g->cannon_fired = false;
    g->draw_cannon = false;
    g->cannon_num_verts = 0;
}


