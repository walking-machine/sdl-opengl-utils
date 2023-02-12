#ifndef GL_SDL_GEOMETRY_H
#define GL_SDL_GEOMETRY_H

struct rect {
    float x;
    float y;
    float w;
    float h;
};

struct point {
    float x;
    float y;
};

struct space_2d {
    bool use_normal;
    point origin;
    float w_loc;
    float h_loc;
    float w_int;
};

typedef point vect;

struct circle {
    point center;
    float radius;
};

struct line {
    point start;
    point end;
};

struct tri {
    point points[3];
};

void rotate_tri(tri *tri, float angle);
void move_tri(tri *tri, vect v);
void move_rect(rect *rect, vect v);
void move_circle(circle *circle, vect v);

bool point_in_circle(point p, circle *circle);
bool point_in_rect(point p, rect *rect);
bool point_in_tri(point p, tri *tri);

bool intersect(circle *circle_1, circle *circle_2);
bool intersect(circle *circle, rect *rect);
bool intersect(circle *circle, tri *tri);
bool intersect(circle *circle, line *line);

#endif
