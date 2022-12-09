#include "gl_sdl_shape_obj.hpp"
#include "gl_sdl_2d.hpp"
#include "gl_sdl_geometry.hpp"
#include <SDL_events.h>
#include <iostream>

void shape::draw() {
    if (!enabled)
        return;
    set_draw_color(&draw_color);
    if (fill_in)
        draw_internal();
    if (draw_border) {
        /* Use complementary color for borders */
        color border_color = { (Uint8)((Uint8)255 - draw_color.r),
                               (Uint8)((Uint8)255 - draw_color.g),
                               (Uint8)((Uint8)255 - draw_color.b),
                               draw_color.a };
        set_draw_color(&border_color);
        draw_border_internal();
    }
}

bool shape::contains_point(point p)
{
    if (!enabled)
        return false;
    return contains_point_internal(p);
}

void shape_circle::apply_transform_internal()
{
    /* TODO : rotate circle around origin? (need a function for that) */
    move_circle(&data, origin);
}

void shape_circle::draw_internal()
{
    set_offset(&origin);
    //  set_rot_angle(phi);
    set_rot_angle(0);
    draw_circle(&data);
}

void shape_circle::draw_border_internal()
{
    set_offset(&origin);
    set_rot_angle(0);
    draw_circle_border(&data);
}

bool shape_circle::contains_point_internal(point p)
{
    if (!transformed)
        return point_in_circle(p, &data);

    shape_circle pseudo = *this;
    pseudo.apply_transform();
    return pseudo.contains_point(p);
}

bool shape_circle::intersects_circle(shape_circle * circle_arg)
{
    if (!transformed)
        return circle_arg->intersects_another_circle(&data);

    shape_circle pseudo = *this;
    pseudo.apply_transform();
    return circle_arg->intersects_another_circle(&pseudo.data);
}

bool shape_circle::intersects_rect(rect *neighbor)
{
    if (!transformed)
        return intersect(&data, neighbor);

    shape_circle pseudo = *this;
    pseudo.apply_transform();
    return pseudo.intersects_rect(neighbor);
}

bool shape_circle::intersects_tri(tri *neighbor)
{
    if (!transformed)
        return intersect(&data, neighbor);

    shape_circle pseudo = *this;
    pseudo.apply_transform();
    return pseudo.intersects_tri(neighbor);
}

bool shape_circle::intersects_another_circle(circle *neighbor)
{
    if (!transformed)
        return intersect(&data, neighbor);

    shape_circle pseudo = *this;
    pseudo.apply_transform();
    return pseudo.intersects_another_circle(neighbor);
}

void shape_rect::apply_transform_internal()
{
    /* TODO : rotation */
    move_rect(&data, origin);
}

void shape_rect::draw_internal()
{
    set_offset(&origin);
    //  set_rot_angle(phi); <- Maybe worth putting this part into the base class?
    set_rot_angle(0);
    draw_rect(&data);
}

void shape_rect::draw_border_internal()
{
    set_offset(&origin);
    set_rot_angle(0);
    draw_rect_border(&data);
}

bool shape_rect::contains_point_internal(point p)
{
    if (!transformed)
        return point_in_rect(p, &data);

    /* TODO : move this logic into the base class */
    shape_rect pseudo = *this;
    pseudo.apply_transform();
    return pseudo.contains_point(p);
}

bool shape_rect::intersects_circle(shape_circle * circle)
{
    if (!transformed)
        return circle->intersects_rect(&data);

    shape_rect pseudo = *this;
    pseudo.apply_transform();
    return circle->intersects_rect(&pseudo.data);
}

void shape_tri::apply_transform_internal()
{
    rotate_tri(&data, phi);
    move_tri(&data, origin);
}

void shape_tri::draw_internal()
{
    set_offset(&origin);    /* TODO : Pass all points by value */
    set_rot_angle(phi);
    draw_tri(&data);
}

void shape_tri::draw_border_internal()
{
    set_offset(&origin);
    set_rot_angle(phi);
    draw_tri_border(&data);
}

bool shape_tri::contains_point_internal(point p)
{
    if (!transformed)
        return point_in_tri(p, &data);
    
    shape_tri pseudo = *this;
    pseudo.apply_transform();
    return pseudo.contains_point(p);
}

bool shape_tri::intersects_circle(shape_circle * circle)
{
    if (!transformed)
        return circle->intersects_tri(&data);
    
    shape_tri pseudo = *this;
    pseudo.apply_transform();
    return circle->intersects_tri(&pseudo.data);
}

point sdl_point_to_space_2d(SDL_Window *window, space_2d *space, point sdl_point)
{
    /* TODO : opengl coordinates */
    int w_int, h_int;
    SDL_GetWindowSize(window, &w_int, &h_int);

    float w = (float)w_int;
    float h = (float)h_int;
    float scale = space->w_int / space->w_loc;
    sdl_point.y = h - sdl_point.y;
    sdl_point = { (sdl_point.x / w - space->origin.x) * scale,
                  (sdl_point.y / w - space->origin.y) * scale };
    return sdl_point;
}

vect sdl_vec_to_space_2d(SDL_Window *window, space_2d *space, vect sdl_vect)
{
    /* TODO : opengl coordinates */
    int w_int;
    SDL_GetWindowSize(window, &w_int, NULL);

    float w = (float)w_int;     /* Normalize */
    sdl_vect.y = -sdl_vect.y;
    float scale = space->w_int / space->w_loc;
    sdl_vect = { sdl_vect.x / w * scale,
                 sdl_vect.y / w * scale };
    return sdl_vect;
}

color colors[18] = {{230, 25, 75}, {60, 180, 75}, {255, 225, 25},
                           {0, 130, 200}, {245, 130, 48}, {70, 240, 240},
                           {240, 50, 230}, {250, 190, 212}, {0, 128, 128},
                           {220, 190, 255}, {170, 110, 40}, {255, 250, 200},
                           {128, 0, 0}, {170, 255, 195}, {0, 0, 128},
                           {128, 128, 128}, {255, 255, 255}, {0, 0, 0}};
