/* gui.h
 graphical user interface for boxdrop program
 Nicole Hedley
 Team 2: EXIT_SUCCESS
 2/14/13
 */

#ifndef GUI_H
#define GUI_H

#include "core.h"
#include "client.h"
#include <chipmunk.h>
#include <gtk/gtk.h>

#define WINDOW_HEIGHT 400

typedef struct chat_info ChatInfo;
struct chat_info {
  GtkWidget *textarea;
  GtkWidget *entry;

};

typedef struct graphics_info GraphicsInfo;
struct graphics_info {

    cairo_surface_t *asteroid;
    cairo_surface_t *earth;
    cairo_surface_t *planet_r;
    cairo_surface_t *planet_y;
    cairo_surface_t *planet_g;
    cairo_surface_t *planet_p;

    cairo_t *cr;

};


typedef struct gui_info GuiInfo;
struct gui_info {
    
    Color * color;
    GtkWidget *sample;
    char * colorspec;
    GdkRGBA *samplecolor;

    
    gdouble mousex;
    gdouble mousey;
    gdouble clickx;
    gdouble clicky;
    
    cpVect * verts;
    int num_verts;
    
    cpSpace * space;
    char * filename;
    
    GtkWidget *window;
    GtkWidget *darea;
    int draw_state;
    gboolean draw_in_progress;
    
    gdouble orientation;
    gdouble friction;
    gdouble elasticity;
    gdouble density;

    ChatInfo *chat;
    GraphicsInfo *gi;
    Client * client;
    gboolean multiplayer;
    
    pthread_mutex_t mutex;
    
    //might need a cannon struct for these
    gboolean cannon_mode;
    gboolean cannon_drawn;
    gboolean cannon_fired;
    gdouble cannon_clickx;
    gdouble cannon_clicky;
    gdouble cannon_dragx;
    gdouble cannon_dragy;
    cpVect *cannon_verts;
    int cannon_num_verts;
    int cannon_type;
    gdouble cannon_impulse_startx;
    gdouble cannon_impulse_starty;
    gdouble cannon_impulse_endx;
    gdouble cannon_impulse_endy;
    gboolean draw_cannon;
    
    int asteroidcount;
    
    //pjw hb
    gboolean homebase_mode;
    gboolean draw_homebase;

    double game_over_flux;
    double game_over_step;
    
    gboolean quit;
    gboolean hello;
    gboolean drawthread_over;
    gboolean recvthread_over;
    
    gint homebase_index;
    gboolean home_planet_set;
};

// called in boxdrop to load everything in the gui and start the game
void box_drop_gui_new ( cpSpace *space );


#endif


