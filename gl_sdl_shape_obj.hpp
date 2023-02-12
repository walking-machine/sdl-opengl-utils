#ifndef GL_SDL_SHAPE_OBJ_H
#define GL_SDL_SHAPE_OBJ_H

#include "gl_sdl_2d.hpp"
#include <stdexcept>


class shape_circle;
class shape_rect;
class shape_tri;

class shape {
protected:
    float phi = 0;
    point origin = { 0, 0 };
    bool transformed = false;
    color draw_color = { 125, 125, 125 };
    bool draw_border = false;
    bool fill_in = true;
    bool enabled = true;
    virtual void apply_transform_internal() = 0; /* Without this, collision calls would need to compute a true position every time */
    virtual void draw_internal() = 0;
    virtual void draw_border_internal() = 0;
    virtual bool contains_point_internal(point p) = 0;
public:
    shape() { phi = 0; origin = {0,0}; }
    void draw();
    bool contains_point(point p);
    virtual bool intersects_with(shape *shape) = 0;
    virtual bool intersects_circle(shape_circle *circle) = 0;
    bool intersects_rect(shape_rect *rect) { throw std::runtime_error("NOT IMPLEMENTED"); }
    bool intersects_tri(shape_tri *tri) { throw std::runtime_error("NOT IMPLEMENTED"); }
    void set_color(color new_color) { draw_color = new_color; }
    void set_draw_border(bool draw_border) { this->draw_border = draw_border; }
    void set_fill_in(bool fill_in) { this->fill_in = fill_in; }
    void set_enabled(bool enable) { this->enabled = enable; }
    point get_offset() { return origin; }
    float get_rotation() { return phi; }
    void set_origin(point offset) {
        origin = offset;
        transformed = true;
    }
    void set_rotation(float angle) {
        phi = angle;
        transformed = true;
    }

    void apply_transform() {
        transformed = false;
        apply_transform_internal();
        phi = 0;
        origin = {0,0};
    }

    void reset_transform() {
        transformed = false;
        phi = 0.f;
        origin = { 0,0 };
    }

    void rotate(float rotation_angle) {
        phi += rotation_angle;
        transformed = true;
    }

    void move(vect vect) {
        origin.x += vect.x;
        origin.y += vect.y;
        transformed = true;
    }
};

class shape_circle : public shape {
private:
    circle data;
    friend class shape_rect;
    friend class shape_tri;
protected:
    virtual void apply_transform_internal() override;
    virtual void draw_internal() override;
public:
    shape_circle(circle original) : shape(), data { original } {}
    shape_circle(point center, float r) : shape_circle(circle{center, r}) {}
    shape_circle(float r = 5.f) : shape_circle({0,0}, r) {}
    virtual bool contains_point_internal(point p) override;
    virtual bool intersects_with(shape *shape) override { return shape->intersects_circle(this); }
    virtual bool intersects_circle(shape_circle *circle) override;
    virtual void draw_border_internal() override;
    bool intersects_rect(rect *neighbor);
    bool intersects_tri(tri *neighbor);
    bool intersects_another_circle(circle *neighbor);
    circle get_data() { return data; }
};

class shape_rect : public shape {
private:
    rect data;
protected:
    virtual void apply_transform_internal() override;
    virtual void draw_internal() override;
public:
    shape_rect(point start, float w, float h) : data{start.x, start.y, w, h} {}
    shape_rect(point start, point dest) : shape_rect(start, dest.x - start.x, dest.y - start.y) {}
    virtual bool contains_point_internal(point p) override;
    virtual bool intersects_with(shape *shape) override { return shape->intersects_rect(this); }
    virtual bool intersects_circle(shape_circle *circle) override;
    virtual void draw_border_internal() override;
};

class shape_tri : public shape {
private:
    tri data;
protected:
    virtual void apply_transform_internal() override;
    virtual void draw_internal() override;
public:
    shape_tri(point p1, point p2, point p3) : data {{p1, p2, p3}} {}
    shape_tri(point *points) : data {{points[0], points[1], points[2]}} {}
    virtual bool contains_point_internal(point p) override;
    virtual bool intersects_with(shape *shape) override { return shape->intersects_tri(this); }
    virtual bool intersects_circle(shape_circle *circle) override;
    virtual void draw_border_internal() override;
};

/* TODO : move the below to shape_utils.h? */
union SDL_Event;
extern color colors[18];
static const uint num_colors = 18;
static color red = {255, 0, 0};

point sdl_point_to_space_2d(SDL_Window *window, space_2d *space, point sdl_point);
vect sdl_vec_to_space_2d(SDL_Window *window, space_2d *space, vect sdl_vect);

template<typename S>
bool try_drag_shape(SDL_Event *event, S  &shape, space_2d *space)
{
    if (event->type != SDL_MOUSEMOTION || !(event->motion.state & SDL_BUTTON_LMASK))
        return false;
    
    vect dp = { (float)event->motion.xrel, (float)event->motion.yrel };
    point mp = { (float)event->motion.x, (float)event->motion.y };
    SDL_Window *window = SDL_GetWindowFromID(event->motion.windowID);

    mp = sdl_point_to_space_2d(window, space, mp);
    if (!shape->contains_point(mp))
        return false;

    dp = sdl_vec_to_space_2d(window, space, dp);
    shape->move(dp);
    return true;
}

template<typename S>
struct shape_manager_state {
    S *shapes;
    uint num_shapes = 0;
    uint first_to_draw = 0;
};

template<typename S>
bool try_drag_all_shapes(SDL_Event *event, shape_manager_state<S> *state,
                         space_2d *space)
{    
    for (uint i = 1; i <= state->num_shapes; i++) {
        uint idx = state->first_to_draw + state->num_shapes - i;
        idx %= state->num_shapes;

        if (!try_drag_shape(event, state->shapes[idx], space))
            continue;

        state->shapes[idx]->set_color(red);
        state->first_to_draw = (idx + 1) % state->num_shapes;
        return true;
    }

    return false;
}

template<typename S>
void draw_all_shapes(shape_manager_state<S> *state)
{
    for (uint i = 0; i < state->num_shapes; i++){
        uint idx = (state->first_to_draw + i) % state->num_shapes;
        state->shapes[idx]->draw();
    }
}

template<typename S>
void assign_random_colors(shape_manager_state<S> *state)
{
    for (uint i = 0; i < state->num_shapes; i++)
        state->shapes[i]->set_color(colors[i % num_colors]);
}
#endif
