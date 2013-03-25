/*
graphics.h
takes care of painting the state of a cpSpace on the screen with cairo
BJN 3/5/13
*/

#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <cairo.h>
#include <chipmunk.h>
#include "gui.h"

// draws visual representation of cpSpace on cairo_t
// if cpSpace * == NULL, don't draw anything
void graphics_draw_space ( cairo_t * cr, GuiInfo * g );

// translates coordinates in the drawing area (such as click coordinates) into a cpVect
void graphics_click_to_cpvec ( cpVect *vect, const int x, const int y );

// translates cpVect into coordinates in the drawing area
void graphics_cpvec_to_click ( cpVect *vect, int * x, int * y );

// draws a temporary shape 
void graphics_draw_temp ( cairo_t *cr, GuiInfo * g );

// draws a cannon shape
void graphics_draw_cannon ( cairo_t *cr, void *g );

// draw a temporary homebase
void graphics_draw_temp_homebase ( cairo_t *cr, void *data );

#endif

