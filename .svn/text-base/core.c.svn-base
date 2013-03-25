/*  core.c 
 core file for game of boxdrop
 CS 50   Team: EXIT_SUCCESS
 03/05/2013 Jing Wei Pan
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <chipmunk.h>
#include <math.h>
#include <assert.h>
#include "core.h"

#define DEFAULT_GRAVITY 0.0
#define SINGLE_SEGMENT_WIDTH 0.2
#define IMPULSE_MULTIPLIER 30

#define PI (3.141592653589793)
#define AST_VELO 60

#define ASIM 30

// homebase constants
#define SIDE_WIDTH 3
#define SIDE_HEIGHT 15
#define CORE_RADIUS 4
#define SIDE_OFFSET 0.5

// typedefs
// these structs are used in reading from files
typedef struct box Box;
struct box {
    char label;
    cpFloat x;
    cpFloat y;
    cpFloat angle;
    Color * color;
    cpFloat friction;
    cpFloat elasticity;
    cpFloat density;
    cpFloat width;
    cpFloat height;
    cpFloat orientation;
};

typedef struct circle Circle;
struct circle {
    char label;
    cpFloat x;
    cpFloat y;
    cpFloat angle;
    Color * color;
    cpFloat friction;
    cpFloat elasticity;
    cpFloat density;
    cpFloat radius;
    cpFloat orientation;
};


// static function declarations
// create/destroy functions for structs
static Box * box_new ();
static void box_destroy ( Box *box );
static Circle * circle_new ();
static void circle_destroy ( Circle * circle );
static DrawShapeInfo * draw_shape_info_new ();
static void draw_shape_info_destroy ( DrawShapeInfo *drawShapeInfo );
static GameInfo * game_info_new ();
static void game_info_destroy ( GameInfo *gameInfo );

// file parsing helper functions
static int core_load_level_parse ( cpSpace *space, FILE * fp );
static int parse_box ( FILE * fp, double * x, double * y, double * width, double * height, double * angle, Color * c, double * fric, double * elasticity, double * dens );
static int parse_ball ( FILE * fp, double * x, double * y, double * radius, double * angle, Color * c, double * fric, double * elasticity, double * dens );

// helper functions for loading a level
static void core_set_gravity ( cpSpace *space, const cpFloat gravity_val );
static cpShape *core_add_box_shape ( cpSpace *space, Box *box, const int index );
static cpShape *core_add_circle_shape ( cpSpace *space, Circle *circ, const int index );
static void core_add_static_box_shape ( cpSpace *space, Box *box );
static void core_add_static_circle_shape ( cpSpace *space, Circle *circ );

// destruction functions called when core_destroy_space is called
static void core_destroy_body ( cpBody *body, void *data );
static void core_destroy_shape ( cpShape *shape, void *data );

//iterator func
static void core_destroy_out_bodies ( cpBody *body, void *data );
static void core_destroy_out_shapes ( cpBody *body, cpShape *shape, cpSpace *space );

//poststep functions 
void postStepRemoveBody ( cpSpace *space, cpBody *body, void *unused );
static void postStepAddBody ( cpSpace *space, cpBody *body, void *unused );
static void postStepAddShape ( cpSpace *space, cpShape *shape, void *unused );


// conversion functions
static DrawShapeInfo* add_box_draw_shape_info ( Box *box );
static DrawShapeInfo* add_circle_draw_shape_info ( Circle *circle );

// update functions called when core_update_space is called
static void core_update_shape ( cpBody * body, cpShape * shape, void * data ) ;
static void core_update_body ( cpBody * body, void * data );

static void post_step_game_over ( cpSpace *space, cpShape *shape, void *unused );
static int core_begin_collision ( cpArbiter *arb, cpSpace *space, void *unused );

// freestyle creation functions
static void core_freestyle_center ( cpVect * verts, const int num_verts, cpVect * center );
static double core_freestyle_mass ( cpVect * verts, const int num_verts, const double density );
static double core_freestyle_moment ( cpVect * verts, const int num_verts, cpVect center, const double density );

// BodyInfo creation/destruction
static void destroy_body_info ( BodyInfo * bi );
BodyInfo * body_info_new ( const int num_verts );


// make cpSpace object to pass to GUI
cpSpace * core_create_space () {
    
    //create and return new cpSpace
    cpSpace *space;
    space = cpSpaceNew ();
    core_set_gravity ( space, DEFAULT_GRAVITY );
    
    cpSpaceAddCollisionHandler ( space, ASTEROID_COLLISION_TYPE, GOAL_COLLISION_TYPE, core_begin_collision, NULL, NULL, NULL, NULL );
    
    BodyInfo * bi = body_info_new ( 0 );
    bi->index = 0;
    space->staticBody->data = bi; // sets index of static body to 0
    
    GameInfo *gameInfo = game_info_new ();
    cpSpaceSetUserData(space, ( cpDataPointer ) gameInfo);
    
    cpEnableSegmentToSegmentCollisions ();
    return space;
}

// core_check_level_name
// checks to make sure that a level file name is in the correct ".level" format
// returns EXIT_SUCCESS or EXIT_FAILURE
int core_check_level_name ( char * file_name ) {
    
    int name_size = 0;
    while ( file_name[name_size] != 0 ) name_size++; // finds size of level name string
    
    if ( strcmp(&file_name[name_size - 6], ".level" ) == 0 ) {
        return EXIT_SUCCESS;
    } else {
        return EXIT_FAILURE;
    }
    
}

// core_load_level
// reads contents of file, adds corresponding cpShapes to cpSpace
// returns EXIT_SUCCESS if level loaded successfully
// returns EXIT_FAILURE if level loaded unsuccessfully (bad syntax)
int core_load_level ( cpSpace *space, char * filename ) {
    int err = core_check_level_name ( filename ); 
    
    if ( err == EXIT_FAILURE ) return EXIT_FAILURE;
    
    // opens file
    FILE * fp;
    if ( ( fp = fopen ( filename, "r" ) ) == NULL ) {
        fprintf ( stderr, "%s: File could not be opened\n\n", filename );
        return EXIT_FAILURE;
    }
    
    // if there is an error loading level inform the user
    err = core_load_level_parse ( space, fp );
    if ( err == EXIT_FAILURE ) {
        fprintf ( stderr, "Problem loading level\n" );
        return EXIT_FAILURE;
    }
    
    fclose ( fp );    
    return EXIT_SUCCESS;
}

// core_load_level
// reads contents of file, adds corresponding cpShapes to cpSpace
// returns EXIT_SUCCESS if level loaded successfully
// returns EXIT_FAILURE if level loaded unsuccessfully (bad syntax)
static int core_load_level_parse ( cpSpace *space, FILE * fp ) { 
    
    int max_line_size = 40; // maximum length of line that we want to read in
    
    // array to hold contents of each line
    char line [max_line_size];
    char * line_place = line; // holds address of start of line
    
    fgets ( line, max_line_size, fp );
    
    // check to see that first line has correct content
    if( strncmp ( line, "gravity ", 8 ) != 0 ) return EXIT_FAILURE;
    
    line_place = &line[8]; // advance line to next chars to be read
    double gravity = atof ( line_place ); // saves gravity input into a double
    core_set_gravity ( space, gravity ); // set gravity
    
    // check that next line is blank
    fgets ( line, max_line_size, fp );
    
    int exit = 1; // keeps track of whether to continue reading lines
    
    while ( exit == 1 ) {
        
        if ( fgets ( line, max_line_size, fp ) == 0 ) return EXIT_SUCCESS; // get next line.  If EOF, return
    
        if ( strncmp ( line, "box STATIC", 10 ) == 0 ) { // static box information to follow
            Box * box = box_new();
            
            if ( parse_box ( fp, &(box->x), &(box->y), &(box->width), &(box->height), &(box->angle), (box->color), &(box->friction), &(box->elasticity), &(box->density)) == EXIT_FAILURE ) {
                
                fprintf ( stderr, "Parsing error in box STATIC\n" );
                box_destroy ( box );
                return EXIT_FAILURE;
            };
            
            // add static box to space and destroy temporary box data 
            core_add_static_box_shape ( space, box );
            box_destroy ( box );
            
        } else if ( strncmp ( line, "box NONSTATIC", 13 ) == 0 ) {
            
            Box * box = box_new();
            
            if( parse_box ( fp, &(box->x), &(box->y), &(box->width), &(box->height), &(box->angle), (box->color), &(box->friction), &(box->elasticity), &(box->density) ) == EXIT_FAILURE ) {
                
                fprintf ( stderr, "Parsing error in box NONSTATIC\n" );
                box_destroy ( box );
                return EXIT_FAILURE;
            };
            
            // need to add mass, inertia since body moves
            core_add_box_shape ( space, box, 0 );
            
            box_destroy ( box );
            
        } else if ( strncmp ( line, "ball NONSTATIC", 14 ) == 0 ) {
            
            Circle * circ = circle_new ();
            
            if ( parse_ball ( fp, &(circ->x), &(circ->y), &(circ->radius), &(circ->angle), circ->color, &(circ->friction), &(circ->elasticity), &(circ->density)) == EXIT_FAILURE ) {
                
                fprintf ( stderr, "Parsing error in ball NONSTATIC\n" );
                circle_destroy ( circ );
                return EXIT_FAILURE;
            }
            
            // need to add mass, inertia since body moves
            core_add_circle_shape ( space, circ, 0 );
            circle_destroy ( circ );
            
        } else if ( strncmp ( line, "ball STATIC", 11) == 0 ) {
            
            Circle * circ = circle_new();
            
            if ( parse_ball ( fp, &(circ->x), &(circ->y), &(circ->radius), &(circ->angle), circ->color, &(circ->friction), &(circ->elasticity), &(circ->density) ) == EXIT_FAILURE ) {
                
                fprintf ( stderr, "Parsing error in ball STATIC\n" );
                circle_destroy ( circ );
                return EXIT_FAILURE;
            }
            
            core_add_static_circle_shape ( space, circ );
            circle_destroy ( circ );
  
        } else  // invalid input
            return EXIT_FAILURE;
        
        
        if ( fgets ( line, max_line_size, fp ) == NULL ) exit = 0; // get next line.  If EOF, get out
    }
    
    cpSpaceEachBody ( space, &core_update_body, (void *) NULL );
    
    return EXIT_SUCCESS;
} 

// parse box
// helper function for core_load_level
// parses file entries for a box
// returns if successful
static int parse_box ( FILE * fp, double * x, double * y, double * width, double * height, double * angle, Color * c, double * fric, double * elasticity, double * dens ) {
    
    int max_line_size = 32;
    char line[max_line_size]; // line to read in 
    
    // parse x and y
    if ( fgets ( line, max_line_size, fp ) == NULL ) return EXIT_FAILURE;
    *x = atof ( line ); 
    if ( fgets ( line, max_line_size, fp ) == NULL ) return EXIT_FAILURE;
    *y = atof ( line ); 
    
    // parse width, height, and angle 
    if ( fgets ( line, max_line_size, fp ) == NULL ) return EXIT_FAILURE;
    *width = atof ( line );
    if ( fgets ( line, max_line_size, fp ) == NULL ) return EXIT_FAILURE;
    *height = atof ( line );
    if ( fgets ( line, max_line_size, fp ) == NULL ) return EXIT_FAILURE;
    *angle = atof ( line );
    
    // parse color 
    if ( fgets ( line, max_line_size, fp ) == NULL ) return EXIT_FAILURE;
    c->r = atof ( line );
    if ( fgets ( line, max_line_size, fp ) == NULL ) return EXIT_FAILURE;
    c->g = atof ( line );
    if ( fgets ( line, max_line_size, fp ) == NULL ) return EXIT_FAILURE;
    c->b = atof ( line );
    
    // parse friction, elasticity, and density
    if ( fgets ( line, max_line_size, fp ) == NULL ) return EXIT_FAILURE;
    *fric = atof ( line );
    if ( fgets ( line, max_line_size, fp ) == NULL ) return EXIT_FAILURE;
    *elasticity = atof ( line );
    if ( fgets ( line, max_line_size, fp ) == NULL ) return EXIT_FAILURE;
    *dens = atof ( line );
    
    
    return EXIT_SUCCESS;
}

// parse ball
// helper function for core_load_level
// parses file entries for a ball
// returns EXIT_SUCCESS if successful
static int parse_ball ( FILE * fp, double * x, double * y, double * radius, double * angle, Color * c, double * fric, double * elasticity, double * dens ) {
    
    int max_line_size = 32;
    char line[max_line_size]; // line to read in 
    
    // parse x and y 
    if( fgets ( line, max_line_size, fp ) == NULL ) return EXIT_FAILURE;
    *x = atof ( line ); 
    if( fgets ( line, max_line_size, fp ) == NULL ) return EXIT_FAILURE;
    *y = atof ( line ); 
    
    // parse radius and angle
    if ( fgets ( line, max_line_size, fp ) == NULL ) return EXIT_FAILURE;
    *radius = atof ( line );
    if ( fgets ( line, max_line_size, fp ) == NULL ) return EXIT_FAILURE;
    *angle = atof ( line );
    
    // parse color 
    if ( fgets ( line, max_line_size, fp ) == NULL ) return EXIT_FAILURE;
    c->r = atof ( line );
    if ( fgets ( line, max_line_size, fp ) == NULL ) return EXIT_FAILURE;
    c->g = atof ( line );
    if ( fgets ( line, max_line_size, fp ) == NULL ) return EXIT_FAILURE;
    c->b = atof ( line );
    
    // parse friction, elasticity, and density
    if ( fgets ( line, max_line_size, fp ) == NULL ) return EXIT_FAILURE;
    *fric = atof ( line );
    if ( fgets ( line, max_line_size, fp ) == NULL ) return EXIT_FAILURE;
    *elasticity = atof ( line );
    if ( fgets ( line, max_line_size, fp ) == NULL ) return EXIT_FAILURE;
    *dens = atof ( line );

    
    
    return EXIT_SUCCESS;
}


//set gravity of space in y direction
static void core_set_gravity ( cpSpace *space, const cpFloat gravity_val ) {
    cpVect gravity = cpv ( 0, gravity_val );
    cpSpaceSetGravity ( space, gravity );
}

// adds nonstatic box shape to space
// currently does not roate box
static cpShape *core_add_box_shape ( cpSpace *space, Box *box, const int index ) {
    
    // calculate mass and moment of a box
    cpFloat mass = box->density * box->width * box->height;
    cpFloat moment = cpMomentForBox ( mass, box->width, box->height );
    
    // add body with mass and moment of a square to space
    cpBody *body = cpBodyNew ( mass, moment );
    cpSpaceAddPostStepCallback ( space, (cpPostStepFunc) postStepAddBody, body, NULL );
    
    cpBodySetPos ( body, cpv ( box->x, box->y ) );
    
    // set index of body
    BodyInfo * bi = body_info_new(0);
    bi->index = index;
    bi->type = BOX_TYPE;
    bi->p1x = box->x - (box->width) / 2.0;
    bi->p1y = box->y + (box->height) / 2.0;
    bi->p2x = box->x + (box->width) / 2.0;;
    bi->p2y = box->y - (box->height) / 2.0;
    bi->color->r = box->color->r;
    bi->color->g = box->color->g;
    bi->color->b = box->color->b;
    bi->friction = box->friction;
    bi->density = box->density;
    bi->elasticity = box->elasticity;
    
    body->data = bi;
    
    double hw = ( box->width ) / 2.0;
    double hh = ( box->height ) / 2.0;
    cpVect cpv1 = cpv ( -hw,-hh );
    cpVect cpv2 = cpv ( -hw, hh );
    cpVect cpv3 = cpv ( hw, hh );
    cpVect cpv4 = cpv ( hw, -hh );
    
    cpVect verts [4]= { cpv1, cpv2, cpv3, cpv4 };
    
    // add box collision shape to body
    cpShape *boxShape = cpPolyShapeNew ( body, 4, verts, cpv ( 0, 0 ) );
    cpSpaceAddPostStepCallback ( space, (cpPostStepFunc) postStepAddShape, boxShape, NULL );
    
    cpShapeSetFriction ( boxShape, box->friction );
    cpShapeSetElasticity ( boxShape, box->elasticity );
    cpBodySetAngle ( body, box->angle);
    
    DrawShapeInfo *info = add_box_draw_shape_info ( box );
    boxShape->data= ( cpDataPointer ) info;
    return boxShape;
}


// adds static box shape to space
static void core_add_static_box_shape ( cpSpace * space, Box * box ) {    
    
    // add box collision shape to body
    cpFloat hw = box->width / 2.0;
    cpFloat hh = box->height / 2.0;
    
    double x = box->x;
    double y = box->y;
    
    cpVect verts[] = {
        cpv ( x - hw, y - hh ),
        cpv ( x - hw, y + hh ),
        cpv ( x + hw, y + hh ),
        cpv ( x + hw, y - hh ),
    };
    
    cpShape * box_shape = cpPolyShapeNew ( space->staticBody, 4, verts, cpvzero );
    cpShapeSetFriction ( (cpShape *) box_shape, box->friction );
    cpShapeSetElasticity ( (cpShape *) box_shape, box->elasticity );
    
    cpSpaceAddPostStepCallback ( space, (cpPostStepFunc) postStepAddShape, box_shape, NULL );  
    
    DrawShapeInfo *info = add_box_draw_shape_info ( box );
    box_shape->data= ( cpDataPointer ) info;
    
}

// add segment shape to the space 
cpShape *core_add_single_segment_shape ( cpSpace * space, const double p1x, const double p1y, const double p2x, const double p2y, Color *color, const double friction, const double elasticity, const double density, const int index ) {
    
    // calculate mass and moment of a box
    double length = sqrt ( ( p1x - p2x ) * ( p1x - p2x ) + ( p1y - p2y ) * ( p1y - p2y ) );
    cpFloat mass = density * SINGLE_SEGMENT_WIDTH * length;
    
    //if segment is too small
    if ( mass <= .01 ) {
        return NULL;
    }
    
    cpFloat moment = cpMomentForBox ( mass, SINGLE_SEGMENT_WIDTH, length );
    
    // add body with mass and moment of a square to space
    cpBody *body = cpBodyNew ( mass, moment );
    cpSpaceAddPostStepCallback ( space, (cpPostStepFunc)postStepAddBody, body, NULL );
    cpBodySetPos ( body, cpv ( ( p1x + p2x ) / 2, ( p1y + p2y ) / 2 ) );
    
    // add body info to body
    BodyInfo * bi = body_info_new(0);
    bi->index = index;
    bi->type = SINGLE_SEGMENT_TYPE;
    bi->p1x = p1x;
    bi->p1y = p1y;
    bi->p2x = p2x;
    bi->p2y = p2y;
    bi->color->r = color->r;
    bi->color->g = color->g;
    bi->color->b = color->b;
    bi->friction = friction;
    bi->density = density;
    bi->elasticity = elasticity;
    
    body->data = bi;
    
    double hw = SINGLE_SEGMENT_WIDTH / 2.0;
    double hh = length / 2.0;
    cpVect cpv1 = cpv ( -hw, -hh );
    cpVect cpv2 = cpv ( -hw, hh );
    cpVect cpv3 = cpv ( hw, hh );
    cpVect cpv4 = cpv ( hw, -hh );
    
    cpVect verts [4]= {cpv1, cpv2, cpv3, cpv4};
    
    
    
    // add box collision shape to body
    cpShape *boxShape = cpPolyShapeNew ( body, 4, verts, cpv ( 0, 0 ) );
    cpSpaceAddPostStepCallback ( space, (cpPostStepFunc) postStepAddShape, boxShape, NULL );
    cpShapeSetFriction ( boxShape, friction );
    cpShapeSetElasticity ( boxShape, elasticity );
    
    double angle = -atan((p1x-p2x)/(p1y-p2y));
    cpBodySetAngle( body, angle);
    
    DrawShapeInfo *info = (DrawShapeInfo *) malloc ( sizeof ( DrawShapeInfo ) );
    info->x = 0;
    info->y = 0;
    info->color = (Color *) malloc ( sizeof ( Color ) );
    info->color ->r = color->r;
    info->color->g = color->g;
    info->color->b = color->b;
    info->type = SINGLE_SEGMENT_TYPE;
    
    boxShape->data = ( cpDataPointer ) info;
    return boxShape;
}

// adds a freestyle shape given a pointer to a an array of vertices and the number of vertices
//returns a cpbody due to the many shapes associated with one body
cpBody *core_add_freestyle_shape ( cpSpace * space, cpVect* verts , const int num_verts, Color *color, const double friction, const double elasticity, const double density, const int index ) {
    
    if ( num_verts <= 1 ) return NULL;
    
    // first determine center of mass of object
    cpVect center; 
    core_freestyle_center ( verts, num_verts, &center );
    
    // calculate mass and moment 
    cpFloat mass = core_freestyle_mass ( verts, num_verts, density );
    // dummy moment calculation
    cpFloat moment = core_freestyle_moment ( verts, num_verts, center, density );
    
    cpBody *body = cpBodyNew ( mass, moment );
    cpSpaceAddPostStepCallback ( space, (cpPostStepFunc)postStepAddBody, body, NULL );
    cpBodySetPos ( body, center );
    
    // set index of body
    // add body info to body
    BodyInfo * bi = body_info_new(num_verts);
    bi->index = index;
    bi->type = FREESTYLE_TYPE;
    bi->num_verts = num_verts;
    
    for ( int i = 0; i < num_verts; i++ ) {
        (bi->verts[i]).x = verts[i].x;
        (bi->verts[i]).y = verts[i].y;
    }
    
    bi->color->r = color->r;
    bi->color->g = color->g;
    bi->color->b = color->b;
    bi->friction = friction;
    bi->density = density;
    bi->elasticity = elasticity;
    
    body->data = bi;
    
    // add line segment collision shapes to body
    for ( int i = 0; i < num_verts - 1; i++ ) {
        
        cpVect offset_a = cpvmult ( cpvsub ( verts[i], center), 1.0 );
        cpVect offset_b = cpvmult ( cpvsub ( verts[i+1], center), 1.0 );
        
        cpShape * segment = cpSegmentShapeNew ( body, offset_a, offset_b, 0.1 );
        cpSpaceAddPostStepCallback ( space, (cpPostStepFunc) postStepAddShape, segment, NULL);
        segment->u = friction;
        
        DrawShapeInfo *info = draw_shape_info_new ();
        info->color->r = color->r;
        info->color->g = color->g;
        info->color->b = color->b;
        
        segment->data = ( cpDataPointer ) info;
    }
    return body;
}


// determines mass of freestyle shape
// does this by assigning mass to each segment... 
static double core_freestyle_mass ( cpVect * verts, const int num_verts, const double density ) {
    
    double mass = 0;
    for ( int i = 0; i < num_verts - 1; i++ ) {
        
        cpFloat seg_length = cpvdist ( verts[i], verts[i + 1] );
        mass += seg_length * density;
    }
    return mass;
}

// determines moment of freestyle shape
// dos this by determining individual moments of segments, 
//offsetting by distance to center squared
static double core_freestyle_moment ( cpVect * verts, const int num_verts, cpVect center, const double density ) {
    
    double moment = 0;
    for ( int i = 0; i < num_verts - 1; i++ ) {
        
        cpFloat seg_length = cpvdist ( verts[i], verts[i + 1] );
        double seg_mass = seg_length * density;
        
        double moment_seg = seg_mass * seg_length * seg_length / 12.0;   // moment around center of segment
        
        // moment around center of object
        cpVect seg_center = cpvadd ( verts[i], cpvmult ( cpvsub ( verts[i + 1], verts[i] ) ,seg_length / 2.0 ) ); // center of mass of segment
        cpFloat seg_disp = cpvdist ( seg_center, center ); // distance of center of mass from center 
        double moment_disp = seg_mass * seg_disp * seg_disp;
        
        moment += moment_seg + moment_disp;  // add the two moments to total moment
    }
    
    return moment;
}


// takes array of vertices, returns center x and y coordinates in address of x and y
// algorithm: take average x value average y value
static void core_freestyle_center ( cpVect * verts, const int num_verts, cpVect * center ) {
    
    cpFloat tot_length = 0.0;
    // assume uniform mass, so length can be substitued as mass
    cpVect center_of_mass = cpvzero;
    
    for ( int i = 0; i< num_verts - 1; i++ ) {
        cpFloat length = cpvdist ( verts[i], verts[i+1] );
        cpVect to_cent = cpvmult ( cpvsub ( verts[i+1], verts[i] ), 0.5 );
        cpVect loc_center = cpvadd ( verts[i], to_cent );
        
        center_of_mass = cpvadd ( center_of_mass, cpvmult ( loc_center, length ) );
        
        tot_length += length;
    }
    
    center_of_mass = cpvmult ( center_of_mass, 1/tot_length );
    
    center->x = center_of_mass.x;
    center->y = center_of_mass.y;
    
    return;
}

// adds nonstatic circle shape to space
// currently does not rotate
static cpShape *core_add_circle_shape ( cpSpace *space, Circle * circ, const int index ) {
    
    // calculate mass and moment of circle
    cpFloat mass = circ->density * M_PI * circ->radius * circ->radius;
    cpFloat moment = cpMomentForCircle ( mass, 0, circ->radius, cpvzero );
    
    // add body with mass and moment of a circle to space
    cpBody *body = cpBodyNew ( mass, moment );
    cpSpaceAddPostStepCallback ( space, (cpPostStepFunc) postStepAddBody, body, NULL );
    cpBodySetPos ( body, cpv ( circ->x, circ->y ) );
    
    // set index of body
    BodyInfo * bi = body_info_new(0);
    bi->index = index;
    bi->type = CIRCLE_TYPE;
    bi->p1x = circ->x - (circ->radius);
    bi->p1y = circ->y + (circ->radius);
    bi->p2x = circ->x + (circ->radius);
    bi->p2y = circ->y - (circ->radius);
    bi->color->r = circ->color->r;
    bi->color->g = circ->color->g;
    bi->color->b = circ->color->b;
    bi->friction = circ->friction;
    bi->density = circ->density;
    bi->elasticity = circ->elasticity;
    
    body->data = bi; 
    
    // adds circle collision shape to body
    cpShape *circShape = cpCircleShapeNew ( body, circ->radius, cpvzero );
    cpSpaceAddPostStepCallback ( space, (cpPostStepFunc) postStepAddShape, circShape, NULL );
    cpShapeSetFriction ( circShape, circ->friction );
    cpShapeSetElasticity ( circShape, circ->elasticity );
    //cpShapeSetCollisionType ( circShape , TARGET_COLLISION_TYPE );
    
    DrawShapeInfo *info = add_circle_draw_shape_info ( circ );
    circShape->data= ( cpDataPointer ) info;
    return circShape;
}

// adds static circle shape to space
static void core_add_static_circle_shape ( cpSpace * space, Circle * circ ) {
    
    cpVect offset = cpv ( circ->x, circ->y );
    
    // add circle collision shape to space
    cpShape * circ_shape = cpCircleShapeNew ( space->staticBody, circ->radius, offset );
    cpShapeSetFriction ( (cpShape *) circ_shape, circ->friction );
    cpShapeSetElasticity ( (cpShape *) circ_shape, circ->elasticity );
    
    cpSpaceAddPostStepCallback ( space, (cpPostStepFunc) postStepAddShape, circ_shape, NULL );    
    DrawShapeInfo *info = add_circle_draw_shape_info ( circ );
    circ_shape->data= ( cpDataPointer ) info;
    
}

//GUI calls this method to try to add a new shape drawn by user
cpShape *core_add_new_shape ( cpSpace *space, const int type, const double p1x, const double p1y, const double p2x, const double p2y, Color *color, const double orientation, const double friction, const double elasticity, const double density, const int index ) {
    
    //if the new shape is a circle, store the values in a circle struct, make the circle shape, and destroy the struct
    if ( type == CIRCLE_TYPE ) {
        
        Circle *circ = circle_new();
        
        //set radius
        if ( abs ( p1x - p2x ) > abs ( p1y - p2y ) ){
            circ->radius = abs ( p2y - p1y ) / 2;
        } 
        else {
            circ->radius = abs ( p2x - p1x ) / 2;
        }
        
        //set centerx
        if ( p1x > p2x ) {
            circ->x = p1x - circ->radius;
        }
        else { 
            circ->x = p1x + circ->radius;
        }
        
        //set centery
        if ( p1y > p2y ) {
            circ->y = p1y - circ->radius;
        }
        else {
            circ->y = p1y + circ->radius;
        }
        
        circ->color->r = color->r;
        circ->color->g = color->g;
        circ->color->b = color->b;
        circ->orientation = orientation;
        circ->friction = friction;
        circ->elasticity = elasticity;
        circ->density = density;
        
        cpShape * circ_shape = core_add_circle_shape ( space, circ, index );
        circle_destroy ( circ );
        return circ_shape;
    }
    
    //if the new shape is a box, store the values in a box struct, make the box shape, and destroy the struct
    if ( type == BOX_TYPE ) {
        
        Box *box = box_new();
        box->x = p1x + ( p2x - p1x ) / 2;
        box->y = p1y + ( p2y - p1y) / 2;
        box->width = abs ( p2x - p1x );
        box->height = abs ( p2y - p1y );
        box->color->r = color->r;
        box->color->g = color->g;
        box->color->b = color->b;
        box->orientation = orientation;
        box->friction = friction;
        box->elasticity = elasticity;
        box->density = density;
        
        if ( box->width < .001 || box->height < .001 ){
            box_destroy ( box );
            return NULL;
        }
        
        cpShape *box_shape = core_add_box_shape ( space, box, index );
        box_destroy ( box );
        return box_shape;
    }
    return NULL;
}


// creates a box, initializes all fields to 0
// appends color, initializes all fields to 0
static Box * box_new () {
    Box * box = (Box *) malloc ( sizeof (Box) );
    Color * c = (Color *) malloc ( sizeof (Color) );
    if ( c == NULL ) printf ( "color not allocated" );
    
    c->r = 0;
    c->g = 0;
    c->b = 0;
    box->color = c;
    
    box->x = 0;
    box->y = 0;
    box->density = 0;
    box->friction = 0;
    box->elasticity = 0;
    box->width = 0;
    box->height = 0;
    box->angle = 0;
    box->orientation = 0;
    
    return box;
}

// destroys box freeing box->color and box
static void box_destroy ( Box * box ) {
    
    if( box->color != NULL ) {
        free ( box->color );
    }
    
    free ( box );
    return;
}

// creates a circle, initializes all fields to 0
static Circle * circle_new () {
    Circle * circ = (Circle *) malloc ( sizeof ( Circle ) );
    Color * c = (Color *) malloc ( sizeof ( Color ) );
    
    c->r = 0;
    c->g = 0;
    c->b = 0;
    circ->color = c;
    
    circ->x = 0;
    circ->y = 0;
    circ->density = 0;
    circ->friction = 0;
    circ->elasticity = 0;
    circ->radius = 0;
    circ->angle = 0;
    circ->orientation = 0;
    
    return circ;
    
}

// destroys circle freeing circle->color and box
static void circle_destroy ( Circle * circle ) {
    free ( circle->color );
    free ( circle );
    
    return;
}

// create a new GameInfo
static GameInfo * game_info_new () {
    GameInfo * gameInfo = ( GameInfo * ) malloc ( sizeof( GameInfo ) );
    
    gameInfo->gameisover = 0;
    gameInfo->multiplayermode = 0;
    gameInfo->explode = false;

 //   gameInfo->game_over_step = 0;
 //   gameInfo->game_over_flux = 0;

    
    return gameInfo;
}

// creates a DrawShapeInfo struct to pass to the gui
static void game_info_destroy ( GameInfo *gameInfo ) {
    
    free ( gameInfo );
    return;
}


// creates a DrawShapeInfo struct to pass to the gui
static DrawShapeInfo * draw_shape_info_new () {
    DrawShapeInfo * drawShapeInfo = (DrawShapeInfo *) malloc( sizeof ( DrawShapeInfo ) );
    Color * c = (Color *) malloc ( sizeof( Color ) );
    if ( c == NULL ) printf( "color not allocated" );
    
    c->r = 0;
    c->g = 0;
    c->b = 0;
    drawShapeInfo->color = c;
    
    drawShapeInfo->type = 0;
    drawShapeInfo->x = 0;
    drawShapeInfo->y = 0;
    
    
    return drawShapeInfo;
    
}

// destroys drawShapeInfo freeing circle->color and box
static void draw_shape_info_destroy ( DrawShapeInfo * drawShapeInfo ) {
    if ( drawShapeInfo->color != NULL ) {
        free ( drawShapeInfo->color );
    }
    free ( drawShapeInfo );
    
    return;
}

//sets the draw shape info to be added to a box
static DrawShapeInfo* add_box_draw_shape_info ( Box *box ){
    
    DrawShapeInfo *info = draw_shape_info_new();
    info->type = BOX_TYPE;
    info->color->r = box->color->r;
    info->color->g = box->color->g;
    info->color->b = box->color->b;
    info->x = box->x;
    info->y = box->y;
    
    return info;
}

//sets the draw shape info to be added to a circle
static DrawShapeInfo* add_circle_draw_shape_info ( Circle *circle ){
    
    DrawShapeInfo *info = draw_shape_info_new();
    info->type = CIRCLE_TYPE;
    info->color->r = circle->color->r;
    info->color->g = circle->color->g;
    info->color->b = circle->color->b;
    info->x = circle->x;
    info->y = circle->y;
    
    return info;
}

// updates drawing info associated with a shape
static void core_update_shape ( cpBody * body, cpShape * shape, void * data ) {
    
    cpVect * pos = ( cpVect * ) data;
    double x = pos->x;
    double y = pos->y;
    
    DrawShapeInfo *info = ( DrawShapeInfo * ) ( shape->data );
    info->x = x;
    info->y = y;
    
}

// updates x and y coordinates of each shape attahced to a body
static void core_update_body ( cpBody * body, void *data ) {
    
    cpVect pos = cpBodyGetPos ( body );
    cpBodyEachShape ( body, &core_update_shape, (void *) &pos );
    
}

//update space with the given time step, one line
void core_update_space ( cpSpace *space, cpFloat time_step ) {
    
    GameInfo *info = (GameInfo *) (space->data);
    if ( info->gameisover == 1 || info->gameisover == 2 ) {
        printf ( "GAME IS OVER\n" );
        core_explode_planet(space);
    }
    
    cpSpaceStep ( space, time_step );
    cpSpaceEachBody ( space, &core_update_body, (void *) NULL );
    cpSpaceEachBody ( space, &core_destroy_out_bodies, (void *) NULL );
}

void core_explode_planet(cpSpace *space){
    GameInfo *gi = (GameInfo *)space->data;
    
    gi->explode = true;
    gi->explodex = CORE_MAX_HEIGHT*1.2;
    gi->explodey = CORE_MAX_HEIGHT/2;
    
    Color *color = (Color *)malloc(sizeof(Color));
    color->r = (double)rand()/(double)RAND_MAX;
    color->g = (double)rand()/(double)RAND_MAX;
    color->b = (double)rand()/(double)RAND_MAX;
    
    double impulsex = rand() % 10 -5;
    double impulsey = rand() % 10 -5;
    
    cpVect impulse = cpv(impulsex, impulsey);
    
    for (int i = 1; i<5; i++){
        core_add_new_shape_with_impulse ( space, CIRCLE_TYPE, gi->explodex+1, gi->explodey+1, gi->explodex-1, gi->explodey-1, color, 0, 0, 1, 1, 0, impulse, cpv(0, 0) );
    }
}

//destroys a body 
void core_destroy_body ( cpBody *body, void *data ) {
    destroy_body_info ( (BodyInfo *) body->data ); // free index of body
    cpBodyFree ( body );
}

//destroys a shape
void core_destroy_shape ( cpShape *shape, void *data ) {
    draw_shape_info_destroy ( shape->data );
    cpShapeFree ( shape );
}

//destroy all allocated memory of cpSpace struct
void core_destroy_space ( cpSpace *space ) {
    
    //iterates through shapes and bodies and destroys them
    cpSpaceEachShape ( space, &core_destroy_shape, (void *) NULL );
    cpSpaceEachBody ( space, &core_destroy_body, (void *) NULL );
    
    destroy_body_info(space->staticBody->data);
    
    game_info_destroy ( (GameInfo *) space->data );
    //frees space
    cpSpaceFree ( space );
}

//destroy bodies out of bound
static void core_destroy_out_bodies ( cpBody *body, void *data ) {
    
    cpVect pos = cpBodyGetPos ( body );
    
    if ( pos.y < -50 || pos.y > (CORE_MAX_HEIGHT + 50) || pos.x < - 50 || pos.x > 200) {
        
        cpSpaceAddPostStepCallback( cpBodyGetSpace(body), (cpPostStepFunc)postStepRemoveBody, body, NULL );
    }
    else return;
}

// poststep functions to add and remove bodies 
void postStepRemoveBody ( cpSpace *space, cpBody *body, void *unused ) {
    
    cpBodyEachShape ( body, (cpBodyShapeIteratorFunc)core_destroy_out_shapes, space );
    
    cpSpaceRemoveBody ( space, body );
    core_destroy_body ( body, NULL );
}

static void postStepAddBody ( cpSpace *space, cpBody *body, void *unused ) {
    cpSpaceAddBody ( space, body );
}

static void postStepAddShape ( cpSpace *space, cpShape *shape, void *unused ) {
    cpSpaceAddShape ( space, shape );
}

// iterator func called by poststep func
static void core_destroy_out_shapes ( cpBody *body, cpShape *shape, cpSpace *space ) {
    cpSpaceRemoveShape ( space, shape );
    core_destroy_shape ( shape , NULL );
}

// collision arbiter function 
static int core_begin_collision ( cpArbiter *arb, cpSpace *space, void *unused ) {
    CP_ARBITER_GET_SHAPES ( arb, a, b );
    
    ((GameInfo *)space->data)->destroyed_planet = ((BodyInfo *)(b->body->data))->index;
    cpSpaceAddPostStepCallback (space, (cpPostStepFunc)postStepRemoveBody, b->body, NULL);
    cpSpaceAddPostStepCallback ( space, (cpPostStepFunc)post_step_game_over, NULL, NULL );
    
    return 0;
}

// collision post step function -- what happens after a collision
static void post_step_game_over ( cpSpace *space, cpShape *shape, void *unused ) {
    //put in here what you want to happen when the end game collision happens    
    GameInfo *info = (GameInfo *) ( cpSpaceGetUserData(space) );
    info->gameisover = 1;
    
    printf ( "Collision happened between start and finish objects. Game is over.\n" );
}


// adds an impulse shape to the cpSpace 
cpBody * core_add_new_shape_with_impulse ( cpSpace *space, const int type, const double p1x, const double p1y, const double p2x, const double p2y, Color *color, const double orientation, const double friction, const double elasticity, const double density, const int index, cpVect impulse, cpVect offset ) {
    
    cpShape * shape;
    
    if ( type == BOX_TYPE || type == CIRCLE_TYPE ) {
        shape = core_add_new_shape ( space, type, p1x, p1y, p2x, p2y, color, orientation, friction, elasticity, density, index );
        
    } else {
        shape = core_add_single_segment_shape ( space, p1x, p1y, p2x, p2y, color, friction, elasticity, density, index );
        
    } if ( shape == NULL ) return NULL;
    
    DrawShapeInfo *info = cpShapeGetUserData ( shape );
    info->space_shape_type = SPACE_TYPE_PLANET_R;

    
    cpBody *body = cpShapeGetBody ( shape );
    
    impulse = cpv ( impulse.x * IMPULSE_MULTIPLIER, impulse.y * IMPULSE_MULTIPLIER );
    
    cpBodyApplyImpulse ( body, impulse, offset );
    
    return body;
}

// adds an asteroid shape to the cpSpace
cpBody * core_add_new_asteroid ( cpSpace *space, const int type, const double p1x, const double p1y, const double p2x, const double p2y, Color *color, const double orientation, const double friction, const double elasticity, const double density, const int index, cpVect impulse, cpVect offset ) {
    
    cpShape * shape;
    
    if ( type == CIRCLE_TYPE ) {
        shape = core_add_new_shape ( space, type, p1x, p1y, p2x, p2y, color, orientation, friction, elasticity, density, index );
        
    } else {
        shape = core_add_single_segment_shape ( space, p1x, p1y, p2x, p2y, color, friction, elasticity, density, index );
        
    } if ( shape == NULL ) return NULL;
    
    DrawShapeInfo * dsi = (DrawShapeInfo *) shape->data;
    dsi->space_shape_type = 1;
    
    // set collision type 
    cpShapeSetCollisionType ( shape, ASTEROID_COLLISION_TYPE );
    
    // apply impulse
    cpBody *body = cpShapeGetBody ( shape );
    impulse = cpv ( impulse.x * IMPULSE_MULTIPLIER, impulse.y * IMPULSE_MULTIPLIER );
    cpBodyApplyImpulse ( body, impulse, offset );
    
    return body;
}

// adds a fresstyle shape with impulse to the cpSpace 
void core_add_freestyle_shape_with_impulse ( cpSpace * space, cpVect* verts , const int num_verts, Color *color, const double friction, const double elasticity, const double density, const int index, cpVect impulse, cpVect offset ) {
    
    cpBody * body = core_add_freestyle_shape ( space, verts , num_verts, color, friction, elasticity, density, index );
    if ( body == NULL ) 
        return;
    
    // apply impulse
    impulse = cpv ( impulse.x*IMPULSE_MULTIPLIER, impulse.y*IMPULSE_MULTIPLIER );
    cpBodyApplyImpulse ( body, impulse, offset );
}

// frees memory associated with a BodyInfo
static void destroy_body_info ( BodyInfo * bi ) {
    
    if ( bi == NULL ) {
        return;
    }
    
    free ( bi->color );
    if ( bi->num_verts > 0 ) {
        free ( bi->verts );
    }
    free ( bi );
}

// creates a new body info with specified number of verts
BodyInfo * body_info_new ( const int num_verts ) {
    
    BodyInfo * bi = (BodyInfo *) malloc ( sizeof ( BodyInfo ) );
    assert(bi != NULL);
    
    bi->index = 0;
    bi->type = 0;
    
    Color * c = (Color *) malloc ( sizeof ( Color ) );
    assert(c != NULL);
    c->r = 0;
    c->g = 0;
    c->b = 0;
    
    bi->color = c;
    
    bi->friction = 0.0;
    bi->density = 0.0;
    bi->elasticity = 0.0;
    bi->p1x = 0.0;
    bi->p1y = 0.0;
    bi->p2x = 0.0;
    bi->p2y = 0.0;
    bi->ang = 0.0;
    
    bi->num_verts = num_verts;
    
    cpVect * verts;
    if ( num_verts > 0 ) {
        verts = (cpVect *) malloc ( num_verts * sizeof ( cpVect ) );
        assert ( verts != NULL );
    } else {
        verts = NULL;
    }
    bi->verts = verts;
    
    bi->space_shape_type = 0;
    
    bi->creation_time = time(NULL);
    
    return bi;
}

// creates asteroid
void core_create_asteroid ( cpSpace *space ) {
    
    // center of asteroid
    double centerx, centery;
    
    // direction that asteroid will approach from
    int direction = (rand() % 4);
    
    // choose a random center for the asteroid
    if( direction == 0) { // come from bottom
        centerx = rand() % 100;
        centery = -10;
    } else if (direction == 1) { // come from left
        centerx = 0;
        centery = rand() % CORE_MAX_HEIGHT + 50;
    } else if (direction == 2) { // come from top
        centerx = rand() % 100;
        centery = CORE_MAX_HEIGHT + 70;
    } else  { // come from right
        centerx = 150;
        centery = rand() % CORE_MAX_HEIGHT + 50;
    }
    
    double rad = (rand() % 3) + 2;    
    double p1x = centerx - rad;
    double p1y = centery + rad;
    double p2x = centerx + rad;
    double p2y = centery - rad;
    
    int type = CIRCLE_TYPE;
    
    Color c;
    c.r = 1.0;
    c.g = 1.0;
    c.b = 1.0;
    
    double ang = 0.0;
    
    double friction = .7;
    double elasticity = 1.0;
    double density = 1;
    
    cpVect impulse = cpvzero;
    cpVect offset = cpvzero;
    
    // set impulse on asteroid
    if( direction == 0) { // come from bottom
        impulse.x = (rand() % 3) - 1;
        impulse.y = ASIM;
    } else if (direction == 1) { // come from left
        impulse.x = ASIM;
        impulse.y = (rand() % 3) - 1;
    } else if (direction == 2) { // come from top
        impulse.x = (rand() % 3) - 1;
        impulse.y = -ASIM;
    } else  { // come from right
        impulse.x = -ASIM;
        impulse.y = (rand() % 3) - 1;
    }
    int index = 0;
    
    cpBody * body = core_add_new_asteroid ( space, type, p1x, p1y, p2x, p2y, &c, ang, friction, elasticity, density, index, impulse, offset );
    
    BodyInfo * bi = (BodyInfo *) body->data;
    bi->space_shape_type = 1;
}

// make a homebase
void core_make_homebase ( cpSpace *space, const double x, const double y, const int start_index, const int planet_num){
    
    Box *defense[4]; 
    
    double r = (double)rand()/(double)RAND_MAX;
    double g = (double)rand()/(double)RAND_MAX;
    double b = (double)rand()/(double)RAND_MAX;
    
    for( int i = 0; i < 4; i++){
        defense[i] = box_new();
        defense[i]->color->r = r;
        defense[i]->color->g = g;
        defense[i]->color->b = b;
        defense[i]->orientation = 0;
        defense[i]->friction = 0;
        defense[i]->elasticity = 1;
        defense[i]->density = 20;
        defense[i]->width = SIDE_WIDTH;
        defense[i]->height = SIDE_HEIGHT;
    }
    
    defense[0]->x = x-SIDE_HEIGHT/2;
    defense[0]->y = y-SIDE_OFFSET;
    defense[1]->x = x-SIDE_OFFSET;
    defense[1]->y = y+SIDE_HEIGHT/2;
    defense[2]->x = x+SIDE_HEIGHT/2;
    defense[2]->y = y+SIDE_OFFSET;
    defense[3]->x = x+SIDE_OFFSET;
    defense[3]->y = y-SIDE_HEIGHT/2;
    
    cpShape *side;
    BodyInfo *sidebi;

    for(int i = 0; i< 4; i++){

        side = core_add_box_shape (space, defense[i], start_index+i);
        cpBodySetAngle(side->body, PI*i/2);
        
        sidebi = (BodyInfo *) (side->body->data);
        sidebi->space_shape_type = -1; // make it so sides don't timeout

    }
    
    for(int i = 0; i< 4; i++){
       box_destroy (defense[i]); 
    }
    
    Circle *planet; 
    
    
    planet = circle_new();
    planet->color->r = r;
    planet->color->g = g;
    planet->color->b = b;
    planet->orientation = 0;
    planet->friction = 0;
    planet->elasticity = 1;
    planet->density = 200;
    planet->radius = CORE_RADIUS;
    
    planet->x = x;
    planet->y = y;
    
    cpShape *core = core_add_circle_shape (space, planet, start_index+4);
    
    cpShapeSetCollisionType ( core, GOAL_COLLISION_TYPE );

    //pjw midnite
    DrawShapeInfo *info = (DrawShapeInfo *)cpShapeGetUserData(core);
    info->originx = x;
    info->originy = y;

    BodyInfo *bi = (BodyInfo *)core->body->data;
    bi->originx = x;
    bi->originy = y;
    bi->space_shape_type = planet_num;
    
    info->space_shape_type = planet_num;
    
    circle_destroy (planet);
}

#ifdef TEST
// test program for checking to make sure that everything works as expected

int main () {
    
    char * level_name = "test2.level";
    cpSpace * space = core_create_space ();
    
    core_load_level ( space, level_name );  
    
    cpFloat timeStep = 1.0/60.0;
    for ( cpFloat time = 0; time < 2; time += timeStep ) {
        
        print_space(space);
        
        cpSpaceStep(space, timeStep);
    }
    
    core_destroy_space(space);
    
    return 0;  
}

#endif


