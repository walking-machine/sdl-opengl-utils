#include "gl_sdl_geometry.hpp"
#include <cmath>
#include <limits>


void rotate_tri(tri *tri, float angle)
{
    float s = sinf(angle);
    float c = cosf(angle);

    for (unsigned int i = 0; i < 3; i++) {
        float x = tri->points[i].x;
        float y = tri->points[i].y;
        tri->points[i] = { x * c - y * s, x * s + y * c };
    }
}

void move_tri(tri *tri, vect v)
{
    for (unsigned int i = 0; i < 3; i++) {
        float x = tri->points[i].x;
        float y = tri->points[i].y;
        tri->points[i] = { x + v.x, y + v.y };
    }
}

void move_rect(rect *rect, vect v)
{
    rect->x += v.x;
    rect->y += v.y;
}

void move_circle(circle *circle, vect v)
{
    circle->center.x += v.x;
    circle->center.y += v.y;
}

bool point_in_circle(point p, circle *circle)
{
    float dist_x = p.x - circle->center.x;
    float dist_y = p.y - circle->center.y;
    float dist_sq = dist_x * dist_x + dist_y * dist_y;

    return dist_sq <= circle->radius * circle->radius;
}

#define swapf(_x, _y) { float temp = _x; _x = _y; _y = temp; }

bool point_in_rect(point p, rect *rect)
{
    float x_min = rect->x;
    float x_max = rect->x + rect->w;
    float y_min = rect->y;
    float y_max = rect->y + rect->h;
    if (rect->w < 0.0f)
        swapf(x_min, x_max);
    if (rect->h < 0.0f)
        swapf(y_min, y_max);

    return p.x <= x_max && p.x >= x_min && p.y <= y_max && p.y >= y_min;
}

static float cross_z(const vect *v1, const vect *v2)
{
    return v1->x * v2->y - v1->y * v2->x;
}

static bool points_on_same_side(const point *p1, const point *p2,
                                const point *a, const point *b)
{
    vect a_p1 = { p1->x - a->x, p1->y - a->y };
    vect a_p2 = { p2->x - a->x, p2->y - a->y };
    vect a_b = { b->x - a->x, b->y - a->y };

    float z1 = cross_z(&a_b, &a_p1);
    float z2 = cross_z(&a_b, &a_p2);

    return z1 * z2 >= 0;
}

bool point_in_tri(point p, tri *tri)
{
    point a = tri->points[0];
    point b = tri->points[1];
    point c = tri->points[2];
    return points_on_same_side(&p, &a, &b, &c) &&
           points_on_same_side(&p, &b, &c, &a) &&
           points_on_same_side(&p, &c, &a, &b);
}

static float point_point_dist_sq(point p1, point p2)
{
    return (p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y);
}

static float line_point_dist_sq(point p1, point p2, point p3)
{
    float u = (p3.x - p1.x) * (p2.x - p1.x) + (p3.y - p1.y) * (p2.y - p1.y);
    u /= point_point_dist_sq(p1, p2);

    point p4 = { p1.x + u * (p2.x - p1.x), p1.y + u * (p2.y - p1.y) };
    if (u <= 0.0f) p4 = p1;
    else if (u >= 1.0f) p4 = p2;

    return point_point_dist_sq(p3, p4);
}

bool intersect(circle *circle_1, circle *circle_2)
{
    float r2 = (circle_1->radius + circle_2->radius) *
               (circle_1->radius + circle_2->radius);

    return point_point_dist_sq(circle_1->center, circle_2->center) < r2;
}

bool intersect(circle *circle, rect *rect)
{
    float r2 = circle->radius * circle->radius;
    point p1 = { rect->x, rect->y };
    point p2 = { rect->x + rect->w, rect->y };
    point p3 = { rect->x + rect->w, rect->y + rect->h };
    point p4 = { rect->x, rect->y + rect->h };

    return point_in_rect(circle->center, rect) ||
           line_point_dist_sq(p1, p2, circle->center) < r2 ||
           line_point_dist_sq(p2, p3, circle->center) < r2 ||
           line_point_dist_sq(p3, p4, circle->center) < r2 ||
           line_point_dist_sq(p4, p1, circle->center) < r2;
}

bool intersect(circle *circle, tri *tri)
{
    float r2 = circle->radius * circle->radius;

    return point_in_tri(circle->center, tri) ||
           line_point_dist_sq(tri->points[0], tri->points[1], circle->center) < r2 ||
           line_point_dist_sq(tri->points[1], tri->points[2], circle->center) < r2 ||
           line_point_dist_sq(tri->points[2], tri->points[0], circle->center) < r2;
}

bool intersect(circle *circle, line *line)
{
    float r2 = circle->radius * circle->radius;
    return line_point_dist_sq(line->start, line->end, circle->center) < r2;
}