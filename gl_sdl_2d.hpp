#ifndef GL_SDL_2D_H
#define GL_SDL_2D_H

#include <SDL.h>
#include "gl_sdl_geometry.hpp"

typedef SDL_Color color;

int set_rot_angle(float phi);
int set_offset(point *offset);

int init_2d();  /* Load programs */
int destroy_2d();

int use_opengl_coords(space_2d *space);
int use_rectangle(space_2d *space, rect *drawing_space, float w_int);
int start_2d(space_2d *space);

int set_draw_color(color *color);

int draw_rect(rect *rect);
int draw_tri(tri *tri);
int draw_circle(circle *circle);
int draw_line(line *line);

int draw_tri_border(tri *tri);
int draw_rect_border(rect *rect);
int draw_circle_border(circle *circle);
int draw_line_border(line *line);

void set_line_width(float w);

#endif
