/* graphics.c
 Jingwei Pan Team: EXIT_SUCCESS
 Mar 5 2013
*/

#include <chipmunk.h>
#include <gtk/gtk.h>
#include <stdlib.h>
#include <stdbool.h>

#include "core.h"
#include "gui.h"
#define local_frame_radius 3
#define SEGMENT_MIN 10

//PJW MIDNITE
#define INNER_BOX_OFFSET 5
// static draw functions 
static void draw_circle ( cairo_t * cr, cpShape *shape, GuiInfo *g);
static void draw_polygon ( cairo_t * cr, cpShape *shape );
static void draw_segment ( cairo_t *cr, cpShape *shape );
static void draw_local_frame ( cairo_t * cr );
static void draw_shape ( cpBody* body, cpShape *shape, void *cr );
static void draw_each_body ( cpBody *body, void *cr );
static void draw_hello ( GuiInfo * gi, void *cr);
static void draw_game_over ( GuiInfo * gi, void *cr);
static void draw_game_over_set_color( GuiInfo *gi, void * cr);

// set color function
static void set_color ( cairo_t * cr, Color * c );

// draw functions that draw temporary shapes 
static void graphics_draw_temp_box ( cairo_t *cr, GuiInfo *g, gint dragx, gint dragy );
static void graphics_draw_temp_circle ( cairo_t *cr, GuiInfo *g, gint dragx, gint dragy );
static void graphics_draw_temp_segment ( cairo_t *cr, GuiInfo *g, gint dragx, gint dragy );
static void graphics_draw_temp_freestyle ( cairo_t *cr, GuiInfo *g, gint dragx, gint dragy );
static gboolean graphics_check_valid_next_point ( GuiInfo *g, gint dragx, gint dragy );


// draws visual representation of cpSpace on cairo_t. If cpSpace * == NULL, don't draw anything
void graphics_draw_space ( cairo_t * cr, GuiInfo * gi ) {
    GameInfo * game_info = (GameInfo *) gi->space->data;
    
    if ( gi->space == NULL )
        return;
    
    //iterate through bodies in space and draw them
    cpSpaceEachBody ( gi->space, &draw_each_body, gi );
    draw_each_body ( cpSpaceGetStaticBody ( gi->space ), gi);



    if(gi->hello){
        draw_hello( gi , cr );
    }

    //if game is over
    if((game_info->gameisover) == 1){
        draw_game_over( gi, cr);
    }   
}

// draws each body in a space
static void draw_each_body ( cpBody *body, void *data ) {
    cpBodyEachShape ( body, &draw_shape, data );
}

static void draw_hello ( GuiInfo *g , void *cr){

    double xcenter = WINDOW_HEIGHT;
    double ycenter = WINDOW_HEIGHT/2;
    //double space = 70.0;

    cairo_select_font_face (cr, "Sans", CAIRO_FONT_SLANT_NORMAL,
                               CAIRO_FONT_WEIGHT_BOLD);

    //draw a message
    cairo_set_font_size (cr, 20.0);
    draw_game_over_set_color( g, cr );
    cairo_move_to (cr, xcenter-100, ycenter);
    cairo_set_source_rgb (cr, 0, 1.0, 1.0);
    cairo_show_text (cr, "Please load a level or connect to server to start the game!");

}

static void draw_game_over ( GuiInfo *gi , void *cr){

    double xcenter = WINDOW_HEIGHT;
    double ycenter = WINDOW_HEIGHT/2;
    //double space = 70.0;

    cairo_select_font_face (cr, "Sans", CAIRO_FONT_SLANT_NORMAL,
                               CAIRO_FONT_WEIGHT_BOLD);

    cairo_set_font_size (cr, 70.0);
    draw_game_over_set_color( gi, cr );
    cairo_move_to (cr, xcenter, ycenter);
    

    //expand text horizontally

    cairo_move_to (cr, xcenter - 230 - (400 * (1 - .1 * gi->game_over_step) * sin(gi->game_over_flux * M_PI)), ycenter);
    cairo_show_text (cr, "G");
    cairo_move_to (cr, xcenter - 170 - (300 * (1 - .1 * gi->game_over_step) * sin(gi->game_over_flux * M_PI)), ycenter);
    cairo_show_text (cr, "A");
    cairo_move_to (cr, xcenter - 107 - (200 * (1 - .1 * gi->game_over_step) * sin(gi->game_over_flux * M_PI)), ycenter);
    cairo_show_text (cr, "M");
    cairo_move_to (cr, xcenter - 27 - (100 * (1 - .1 * gi->game_over_step) * sin(gi->game_over_flux * M_PI)), ycenter);
    cairo_show_text (cr, "E");
    cairo_move_to (cr, xcenter + 50 + (100 * (1 - .1 * gi->game_over_step) * sin(gi->game_over_flux * M_PI)), ycenter);
    cairo_show_text (cr, "O");
    cairo_move_to (cr, xcenter + 110 + (200 * (1 - .1 * gi->game_over_step) * sin(gi->game_over_flux * M_PI)), ycenter);
    cairo_show_text (cr, "V");
    cairo_move_to (cr, xcenter + 165 + (300 * (1 - .1 * gi->game_over_step) * sin(gi->game_over_flux * M_PI)), ycenter);
    cairo_show_text (cr, "E");
    cairo_move_to (cr, xcenter + 215 + (400 * (1 - .1 * gi->game_over_step) * sin(gi->game_over_flux * M_PI)), ycenter);
    cairo_show_text (cr, "R");


}


//oscillates the color
static void draw_game_over_set_color( GuiInfo * gi, void * cr){

    double cp = gi->game_over_flux;
    double step = gi->game_over_step;

    double r, g, b;
    

    if( cp < 1.0/6.0){  //red --> orange
        r = 1.0;
        g = 6.0 * cp;
        b = 0;
    } else if( cp < 2.0/6.0){  //orange --> yellow
        r = 1.0 - ((cp - 1.0/6.0) * 6.0);
        g = 1.0;
        b = 0;
    } else if( cp < 3.0/6.0){ //yellow -> green
        r = 0;
        g = 1.0;
        b = 6 * (cp - 2.0/6.0);
    } else if( cp < 4.0/6.0){ //green --> teal
        r = 0;
        g = 1.0 - ((cp - 3.0/6.0) * 6.0);
        b = 1.0;
    } else if( cp < 5.0/6.0){ //teal --> blue
        r =  ( cp - 4.0/6.0 ) * 6.0;
        g = 0;
        b = 1.0;
    } else if( cp < 6.0/6.0){ //blue --> purple
        r = 1.0;
        g = 0;
        b = 1.0 - ((cp - 5.0/6.0) * 6.0);
    }

    cairo_set_source_rgb (cr, r, g, b);

    //increment the color value slightly
    cp = cp + (.01 * 1/6);
    gi->game_over_flux = cp;  //50 draws per color phase

    step = step + (.01 * 1/6);
    gi->game_over_step = step;

    //reset cycle if it reaches 1.0
    if(gi->game_over_flux > 1.0){
        gi->game_over_flux = 0;
    }


}
    

//translates coordinates in the drawing area (such as click coordinates) into a cpVect
void graphics_click_to_cpvec ( cpVect *vect, int x, int y ) {
    
    //scale x, y to components of a unit vector
    double xscaled = ( (double)x ) / WINDOW_HEIGHT;
    double yscaled = 1 - ( ( (double)y ) / WINDOW_HEIGHT );
    
    //reset length of the vector to match CORE_MAX_HEIGHT
    *vect = cpv ( (cpFloat) (xscaled * CORE_MAX_HEIGHT), (cpFloat) (yscaled * CORE_MAX_HEIGHT ) );
}

// translates cpVect into coordinates in the drawing area
void graphics_cpvec_to_click ( cpVect *vect, int *x, int *y ) {
    
    //scale vect to a unit vector
    double xscale = ( (*vect).x / (double)CORE_MAX_HEIGHT );
    double yscale = 1 - ( (*vect).y/ (double)CORE_MAX_HEIGHT );
    
    //reset length of vector to match WINDOW_HEIGHT
    int scaledx = WINDOW_HEIGHT * xscale;
    int scaledy = WINDOW_HEIGHT * yscale;
    *x = scaledx;
    *y = scaledy;
}

//draws the given shape in the cairo context.  It will extract relevant data such as type of shape, size, position, color, and rotation from the cpShape and pass on this information to the following functions.
static void draw_shape ( cpBody *body, cpShape *shape, void *data ) {    
    GuiInfo * g = (GuiInfo *) data;


    DrawShapeInfo *info = cpShapeGetUserData ( shape );

    cairo_t *cr = g->gi->cr;
    //set cairo rgb
    Color *color = info->color;
    set_color ( cr, color );
    
    //get collision type
    int type = info->type;
    
    //get cp center, convert to da center
    int centerX, centerY;
    cpVect center = cpBodyGetPos ( body );
    graphics_cpvec_to_click ( &center, &centerX, &centerY );    
    cpVect da_center = cpv ( centerX, centerY );
    
    //get rotation
    double rot = cpBodyGetAngle ( body );
    
    // save current scaling/rotation transform so we can set things back when doene
    cairo_matrix_t saved_transform;
    cairo_get_matrix ( cr, &saved_transform );
    
    // apply a translation and a rotation to the transformation matrix
    cairo_translate ( cr, da_center.x, da_center.y );
    cairo_rotate ( cr, -rot );
    
    if ( type == LINE_TYPE ) {
        draw_segment ( cr, shape );
    }
    else if ( type == BOX_TYPE ) {
        draw_polygon ( cr, shape );
    }
    else if ( type == CIRCLE_TYPE ) {
        draw_circle ( cr, shape, (GuiInfo *) g );
    }
    else if ( type == SINGLE_SEGMENT_TYPE ) {
        draw_polygon ( cr, shape );
    }
    
    // set back to original scaling/rotation
    cairo_set_matrix(cr, &saved_transform);
}


// Draws a rectangle of a given height and width centered at center all given in cpSpace coordinates.
void draw_polygon ( cairo_t * cr, cpShape *shape ) {
    
    //get the number of vertices and the body of passed shape
    int n = cpPolyShapeGetNumVerts ( shape );
    cpVect *da_verts = (cpVect *) malloc ( n*sizeof ( cpVect ) ); //array of vectors translated into drawing area
    cpBody *body = cpShapeGetBody(shape);
    
    //draw static polygon, reset center, since static.
    if ( cpBodyIsStatic ( body ) == true ) {
        //static bodies are centered at origin, translate matrix back
        cairo_translate ( cr, 0, -WINDOW_HEIGHT );
        
        //loop through all vertices to convert to da coordinates
        for ( int i = 0; i < n; i++ ) {
            
            cpVect v = cpPolyShapeGetVert ( shape, i );
            int x, y;
            graphics_cpvec_to_click ( &v, &x, &y );
            da_verts[i] = cpv ( x, y );
        }
    }
    
    //draw non-static box, conversion takes flipped y-axis into consideration by negating y values
    else {
        
        //loop through all vertices to convert to da coordinates
        for ( int i = 0; i < n; i++ ) {
            cpVect v = cpPolyShapeGetVert ( shape, i );
            da_verts[i] = cpv ( v.x * WINDOW_HEIGHT / CORE_MAX_HEIGHT, -v.y * WINDOW_HEIGHT / CORE_MAX_HEIGHT);
        }
    }
    
    //draw polygon
    cairo_move_to ( cr, da_verts[0].x, da_verts[0].y );
    for ( int j = 1; j < n; j++ ) {
        cairo_line_to ( cr, da_verts[j].x, da_verts[j].y );
    }  
    cairo_line_to ( cr, da_verts[0].x, da_verts[0].y ); //back to first vertex
    
    //pjw midnite
    //draw inner box
    cairo_stroke ( cr );

    cairo_move_to ( cr, da_verts[0].x+INNER_BOX_OFFSET, da_verts[0].y-INNER_BOX_OFFSET );
    cairo_line_to ( cr, da_verts[1].x+INNER_BOX_OFFSET, da_verts[1].y+INNER_BOX_OFFSET );
    cairo_line_to ( cr, da_verts[2].x-INNER_BOX_OFFSET, da_verts[2].y+INNER_BOX_OFFSET );
    cairo_line_to ( cr, da_verts[3].x-INNER_BOX_OFFSET, da_verts[3].y-INNER_BOX_OFFSET );
    cairo_line_to ( cr, da_verts[0].x+INNER_BOX_OFFSET, da_verts[0].y-INNER_BOX_OFFSET ); //back to first vertex
    cairo_fill(cr);


    cairo_stroke ( cr );
    
    free ( da_verts );
    draw_local_frame ( cr );
}

// Draws a circle of a given radius centered at center all given in cpSpace coordinates
void draw_circle ( cairo_t * cr, cpShape *shape, GuiInfo *g) {
    
    DrawShapeInfo *info = cpShapeGetUserData ( shape );
    int circ_type = info -> space_shape_type;

    if( circ_type > SPACE_TYPE_CIRCLE && circ_type <= SPACE_TYPE_PLANET_R){ //PLANET OR ASTEROID
        cairo_surface_t *surface;
        if( circ_type == SPACE_TYPE_ASTEROID ) {  
            surface  = g->gi->asteroid;
        } else if ( circ_type == SPACE_TYPE_EARTH ){
            surface  = g->gi->earth;
        } else if ( circ_type == SPACE_TYPE_PLANET_R ){
            surface  = g->gi->planet_r;
        } else if ( circ_type == SPACE_TYPE_PLANET_Y ){
            surface  = g->gi->planet_y;
        } else if ( circ_type == SPACE_TYPE_PLANET_G ){
            surface  = g->gi->planet_g;
        } else if ( circ_type == SPACE_TYPE_PLANET_P ){
            surface  = g->gi->planet_p;
        }
        
        //get body and offset value of circle
        cpVect offset = cpCircleShapeGetOffset ( shape );
    
        //change offset values to da_coordinates
        double offset_x, offset_y;
        offset_x = offset.x * (double) WINDOW_HEIGHT / (double) CORE_MAX_HEIGHT ;
        offset_y = -offset.y * (double) WINDOW_HEIGHT / (double) CORE_MAX_HEIGHT ;
        cairo_translate ( cr, offset_x, offset_y ); //move to circle center
    
        //translate radius into da coordinates
        double radius = cpCircleShapeGetRadius ( shape );
        double da_radius = radius * WINDOW_HEIGHT / CORE_MAX_HEIGHT;
    
        int width, height;
        double scalex, scaley;

        width = cairo_image_surface_get_width (surface);
        height = cairo_image_surface_get_height (surface);
        scalex = 2.0 * da_radius/width;
        scaley = 2.0 * da_radius/height;

        cairo_scale(cr, scalex, scaley);
        cairo_set_source_surface(cr, surface, offset_x - (da_radius/scalex), offset_y - (da_radius/scaley));
        cairo_paint(cr);
        
    } else  { //CIRCLE TYPE IS NOT PLANET OR ASTEROID
        cairo_stroke(cr);
         //get body and offset value of circle
        cpBody *body = cpShapeGetBody(shape);
        cpVect offset = cpCircleShapeGetOffset(shape);
    
         //change offset values to da_coordinates
         double offset_x, offset_y;
         offset_x = offset.x*(double)WINDOW_HEIGHT/(double)CORE_MAX_HEIGHT ;
         offset_y = -offset.y*(double)WINDOW_HEIGHT/(double)CORE_MAX_HEIGHT ;
         cairo_translate(cr, offset_x, offset_y); //move to circle center
    
        //translate radius into da coordinates
        double radius = cpCircleShapeGetRadius(shape);
        double da_radius = radius*WINDOW_HEIGHT/CORE_MAX_HEIGHT;
    
         // static circle case
        if(cpBodyIsStatic(body) == true){
            cairo_arc (cr,
                   0, 0,
                   da_radius,
                   0, 2 * G_PI);
            cairo_fill(cr);
            cairo_stroke(cr); //draw circle
        }
    
        // nonstatic circle
        else {
            cairo_arc (cr,
            offset_x, offset_y,
            da_radius,
            0, 2 * G_PI);
            cairo_fill(cr);
            cairo_stroke(cr); //draw circle
            draw_local_frame(cr);
        }
    }
    
}



// function that draws a segment
void draw_segment ( cairo_t *cr, cpShape *shape ) {
    
    cpBody *body = cpShapeGetBody ( shape );
    
    cpVect A = cpSegmentShapeGetA ( shape );
    cpVect B = cpSegmentShapeGetB ( shape );

    if ( cpBodyIsStatic ( body ) == true ) {
        
        cairo_translate( cr, 0, -WINDOW_HEIGHT );
        
        //convert to da
        int da_Ax, da_Ay, da_Bx, da_By;
        graphics_cpvec_to_click ( &A, &da_Ax, &da_Ay );
        graphics_cpvec_to_click ( &B, &da_Bx, &da_By );
        
        cairo_move_to ( cr, da_Ax, da_Ay );
        cairo_line_to ( cr, da_Bx, da_By );
        cairo_stroke ( cr );
    }
    
    // nonstatic line
    else {
        
        //convert to da length
        double Ax, Bx, Ay, By;
        Ax = A.x * WINDOW_HEIGHT / CORE_MAX_HEIGHT;
        Ay = A.y * WINDOW_HEIGHT / CORE_MAX_HEIGHT;
        Bx = B.x * WINDOW_HEIGHT / CORE_MAX_HEIGHT;
        By = B.y * WINDOW_HEIGHT / CORE_MAX_HEIGHT;
        
        //draw segment
        cairo_move_to ( cr, Ax, -Ay );
        cairo_line_to ( cr, Bx, -By );
        cairo_stroke ( cr ); 
        draw_local_frame ( cr );
    }
}

// Sets the color of a cairo context given a Color pointer c.  If the pointer is set to NULL, the function should default to black.
static void set_color ( cairo_t * cr, Color * c ) {
    if ( c == NULL ) {
        cairo_set_source_rgb ( cr, 0, 0, 0 );
    }
    else {
        cairo_set_source_rgb ( cr, c->r, c->g, c->b );
    }
}

// Draws a small local frame axis at the center of an object.  Should rotate/translate with object.
static void draw_local_frame ( cairo_t * cr ) {
    
    cairo_set_source_rgb ( cr, 1, 0, 0 );
    cairo_move_to ( cr, 0, -local_frame_radius );
    cairo_line_to ( cr, 0, local_frame_radius );
    cairo_stroke ( cr );
    cairo_set_source_rgb ( cr, 0, 0, 1 );
    cairo_move_to ( cr, -local_frame_radius, 0 );
    cairo_line_to ( cr, local_frame_radius, 0 );
    cairo_stroke ( cr );
}

// draws a temporary shape in cannon mode 
void graphics_draw_temp ( cairo_t *cr, GuiInfo * g ) {
    
    cairo_set_source_rgb ( cr, g->color->r, g->color->g, g->color->b );

    gint dragx, dragy;

    dragx = g->mousex;
    dragy = g->mousey;

    //set cannon drag corners
    if ( g->draw_cannon == true ) {
        dragx = g->cannon_dragx;
        dragy = g->cannon_dragy;
    }
    
    //set draw impulse vector mode
    int oldtype = g->draw_state;
    if ( g->cannon_mode == true && g->cannon_drawn == true && g->draw_cannon == false ) {
        g->draw_state = SINGLE_SEGMENT_TYPE;
    }
    
    double dash []= {0.5, 0.5};
    cairo_set_dash ( cr, dash, 1, 0 );
    
    if ( g->draw_state == BOX_TYPE ) { // box
       graphics_draw_temp_box ( cr, g, dragx, dragy );
    }
    else if ( g->draw_state == CIRCLE_TYPE ) { // circle
        graphics_draw_temp_circle ( cr, g, dragx, dragy );
    }
    else if ( g->draw_state == SINGLE_SEGMENT_TYPE ) { // segment
        graphics_draw_temp_segment ( cr, g, dragx, dragy );
    }
    else if ( g->draw_state == FREESTYLE_TYPE ) { // freestyle
        graphics_draw_temp_freestyle ( cr, g, dragx, dragy );
    }
    
    g->draw_state = oldtype;
}

// draws a temporary image for a box 
static void graphics_draw_temp_box ( cairo_t *cr, GuiInfo *g, gint dragx, gint dragy ){
    
    gint clickx = g->clickx;
    gint clicky = g->clicky;
    
    if ( g->draw_cannon == true ) {
        clickx = g->cannon_clickx;
        clicky = g->cannon_clicky;
    }
    
    cairo_set_source_rgb ( cr, g->color->r, g->color->g, g->color->b );
    cairo_move_to ( cr, clickx, clicky );
    cairo_line_to ( cr, clickx, dragy );
    cairo_line_to ( cr, dragx, dragy );
    cairo_line_to ( cr, dragx, clicky );
    cairo_line_to ( cr, clickx, clicky );
}

// draw a temporary image for circle
static void graphics_draw_temp_circle ( cairo_t *cr, GuiInfo *g, gint dragx, gint dragy ) {
    
    gint clickx = g->clickx;
    gint clicky = g->clicky;
    
    if ( g->draw_cannon == true ) {
        clickx = g->cannon_clickx;
        clicky = g->cannon_clicky;
    }
    
    double temp_radius, temp_centerx, temp_centery;
    
    //set radius
    if ( abs ( clickx-dragx ) > abs ( clicky - dragy ) ){
        temp_radius = abs ( (double) clicky - dragy ) / 2;
    }
        
    else {
        temp_radius = abs ( (double) clickx - dragx ) / 2;
    }
        
    //set centerx
    if ( clickx > dragx ) {
        temp_centerx = (double) clickx - temp_radius;
    }
    else {
        temp_centerx = (double) clickx + temp_radius;
    }
        
    //set centery
    if ( clicky > dragy ) {
        temp_centery = (double) clicky - temp_radius;
    }
    else {
        temp_centery = (double) clicky + temp_radius;

    }

    cairo_arc ( cr, temp_centerx, temp_centery, temp_radius, 0, 2 * G_PI );
}

// draws a temporary image for a segment
static void graphics_draw_temp_segment ( cairo_t *cr, GuiInfo *g, gint dragx, gint dragy ) {
    gint clickx = g->clickx;
    gint clicky = g->clicky;
    
    if ( g->draw_cannon == true ) {
        clickx = g->cannon_clickx;
        clicky = g->cannon_clicky;
    }
    
    cairo_move_to ( cr, clickx, clicky );
    cairo_line_to ( cr, dragx, dragy );
}

//draw temporary image for freestyle
static void graphics_draw_temp_freestyle ( cairo_t *cr, GuiInfo *g, gint dragx, gint dragy ) {
   
   //add next point
    if ( graphics_check_valid_next_point ( g, dragx, dragy ) == true ) {
       // printf("valid");
        g->verts[g->num_verts] = cpv ( dragx, dragy );
        g->num_verts ++;
        
        if ( g->cannon_mode == true ) {
            g->cannon_verts[g->cannon_num_verts] = cpv ( dragx, dragy );
            g->cannon_num_verts ++;
        }
    }
    
    //connect all availabe vertices
    if ( g->draw_cannon == true ) {
            cairo_line_to ( cr, g->cannon_verts[0].x, g->cannon_verts[0].y );
        for ( int i = 1; i < g->cannon_num_verts; i++ ) {
            cairo_line_to ( cr, g->cannon_verts[i].x, g->cannon_verts[i].y );
        }
    }
    
    else {
        cairo_line_to ( cr, g->verts[0].x, g->verts[0].y );
        for ( int i = 1; i < g->num_verts; i++ ) {
            cairo_line_to ( cr, g->verts[i].x, g->verts[i].y );
        }
    }
}

//check to see if the next point is too close to the last to make a new segment
static gboolean graphics_check_valid_next_point ( GuiInfo *g, gint dragx, gint dragy ) {
    gint lastx = g->verts[g->num_verts -1].x;
    gint lasty = g->verts[g->num_verts -1].y;
    
    if ( abs ( dragx - lastx ) < SEGMENT_MIN && abs ( dragy - lasty ) < SEGMENT_MIN )
        return false;
    else
        return true;
}

//keep cannon shape on the screen
void graphics_draw_cannon ( cairo_t *cr, void *data ) {

    GuiInfo *g = (GuiInfo *) data;
    if ( g->cannon_drawn == true && g->cannon_fired == false ) {
        
        g->draw_cannon = true;
        int oldtype = g->draw_state;
        g->draw_state = g->cannon_type;
        
        graphics_draw_temp ( cr, g );
        
        g->draw_state = oldtype;
        g->draw_cannon = false;

    }
}

void graphics_draw_temp_homebase(cairo_t *cr, void *data){
    GuiInfo *g = (GuiInfo *) data;
    if (g->homebase_mode == true && g->draw_homebase == true){
        double dash []= {0.5, 0.5};
        cairo_set_dash ( cr, dash, 5, 0 );
        cairo_set_source_rgb ( cr, g->color->r, g->color->g, g->color->b );
        cairo_move_to ( cr, g->mousex +40, g->mousey +40 );
        cairo_line_to ( cr, g->mousex +40, g->mousey -40 );
        cairo_line_to ( cr, g->mousex -40, g->mousey -40 );
        cairo_line_to ( cr, g->mousex -40, g->mousey +40  );
        cairo_line_to ( cr, g->mousex +40, g->mousey +40);
    }
}