/* core.h
header file for core.c
CS 50   Team: EXIT_SUCCESS
03/05/2013  Jing Wei Pan
*/

#ifndef CORE_H
#define CORE_H

#include <chipmunk.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <stdbool.h>

#define CORE_MAX_HEIGHT 60

#define LINE_TYPE 0
#define BOX_TYPE 1
#define CIRCLE_TYPE 2
#define FREESTYLE_TYPE 3
#define SINGLE_SEGMENT_TYPE 4
#define GOAL_TYPE 5

#define ASTEROID_COLLISION_TYPE 1
#define GOAL_COLLISION_TYPE 2

#define PLANET_LOST_OFFSET 16
#define PLANET_DANGER_OFFSET 13
#define LOST 0
#define DANGER 1
#define SAFE 2

#define SPACE_TYPE_CIRCLE 0
#define SPACE_TYPE_ASTEROID 1
#define SPACE_TYPE_EARTH 2
#define SPACE_TYPE_PLANET_P 3
#define SPACE_TYPE_PLANET_Y 4
#define SPACE_TYPE_PLANET_G 5
#define SPACE_TYPE_PLANET_R 6

// struct that contains user data for cpSpace object
typedef struct space_data Space_Data;
struct space_data {
  cpBody * temp_obj;
};

// struct that contains RGB values
typedef struct color Color;
struct color {
    
  double r;
  double g;
  double b;
  double rslide;
  double gslide;
  double bslide;
};

// struct that contains info for graphics
typedef struct draw_shape_info DrawShapeInfo;
struct draw_shape_info {
    int index; // corresponds to index on server
    int type;
    Color * color;
    double x;
    double y;
    int space_shape_type;
    
    double originx;
    double originy;
};

// a struct to hold information for a server on how to make a body in a SHAPE {} command
typedef struct body_info BodyInfo;
struct body_info {
    
    int index;
    int type;
    Color * color;
    
    double friction;
    double density;
    double elasticity;
    
    double p1x;
    double p1y;
    double p2x;
    double p2y;
    
    double ang;
    int num_verts;
    cpVect * verts;
    int space_shape_type;
    
    double originx;
    double originy;
    
    time_t creation_time;
};

// a struct that holds information about the current game state
typedef struct game_info GameInfo;
struct game_info {
    
    int multiplayermode;
    int gameisover;
    int *server_body_count;
    
    bool explode;
    double explodex;
    double explodey;
    int destroyed_planet;
};

typedef struct asteroid asteroid;
struct asteroid {
    
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


// make cpSpace object to pass to GUI
cpSpace * core_create_space ();

// check to see if the file name is properly formatted
int core_check_level_name ( char *file_name );

// load contents of the file into cpShapes
int core_load_level ( cpSpace *space, char * filename );

// GUI calls this method to try to add a new shape drawn by user
cpShape *core_add_new_shape ( cpSpace *space, const int type, const double p1x, const double p1y, const double p2x, const double p2y, Color *color, const double orientation, const double friction, const double elasticity, const double density, const int index );

// adds new freestyle shape
cpBody *core_add_freestyle_shape ( cpSpace * space, cpVect* verts , const int num_verts, Color *color, const double friction, const double elasticity, const double density, const int index );
// adds new segment
cpShape *core_add_single_segment_shape ( cpSpace * space, const double p1x, const double p1y, const double p2x, const double p2y, Color *color, const double friction, const double elasticity, const double density, const int index );

// add shapes with impulse
cpBody * core_add_new_shape_with_impulse ( cpSpace *space, const int type, const double p1x, const double p1y, const double p2x, const double p2y, Color *color, const double orientation, const double friction, const double elastcity, const double density, const int index, cpVect impulse, cpVect offset );
void core_add_freestyle_shape_with_impulse ( cpSpace * space, cpVect* verts , const int num_verts, Color *color, const double friction, const double elasticity, const double density, const int index, cpVect impulse, cpVect offset );
cpBody * core_add_new_asteroid ( cpSpace *space, const int type, const double p1x, const double p1y, const double p2x, const double p2y, Color *color, const double orientation, const double friction, const double elasticity, const double density, const int index, cpVect impulse, cpVect offset );
void core_create_asteroid ( cpSpace *space );

// update space with the given time step, one line
void core_update_space ( cpSpace *space, cpFloat time_step );

// destroy all allocated memory of cpSpace struct
void core_destroy_space ( cpSpace *space );

//pjw new
void postStepRemoveBody ( cpSpace *space, cpBody *body, void *unused );

void core_make_homebase(cpSpace *space, const double x, const double y, const int start_index, const int planet_num);

void core_explode_planet(cpSpace *space);
#endif
