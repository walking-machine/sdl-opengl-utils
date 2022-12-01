#include "gl_sdl_2d.hpp"
#include "gl_sdl_utils.hpp"

static GLuint prog_rect;
static GLuint prog_normal = 0;

const char vs_normal[] =
    "#version 300 es\n"
    "layout(location = 0) in vec2 pos;\n"
    "void main() {\n"
    "gl_Position = vec4(pos, 0.0f, 1.0f);\n"
    "}\n";

const char vs_rect[] =
    "#version 300 es\n"
    "layout(location = 0) in vec2 pos;\n"
    "uniform vec2 origin;\n"
    "uniform float aspect;\n"
    "uniform float scale_w;\n"
    "uniform float multi_w;\n"
    "uniform float multi_h;\n"
    "uniform vec2 offset;\n"
    "uniform float phi;\n"
    "\n"
    "void main() {\n"
    "vec2 pos_m = mat2(cos(phi), sin(phi), -sin(phi), cos(phi)) * pos + offset;\n"
    "vec2 loc = scale_w * vec2(multi_w * pos_m.x, multi_h * pos_m.y) + origin;\n"
    "vec2 opengl_coords = 2.0f * vec2(loc.x, loc.y * aspect) - vec2(1.0f, 1.0f);\n"
    "gl_Position = vec4(opengl_coords, 0.0f, 1.0f);\n"
    "}\n";

const char fs[] = 
    "#version 300 es\n"
    "precision mediump float;\n"
    "out vec4 frag_color;\n"
    "uniform vec4 draw_color;\n"
    "void main() {\n"
    "frag_color = draw_color;\n"
    "}\n";

int init_2d()
{
    GLuint shaders_normal[] = {
        read_shader(vs_normal, GL_VERTEX_SHADER),
        read_shader(fs, GL_FRAGMENT_SHADER)
    };

    GLuint shaders_rect[] = {
        read_shader(vs_rect, GL_VERTEX_SHADER),
        read_shader(fs, GL_FRAGMENT_SHADER)
    };

    color default_color = { 0, 0, 255 };

    prog_normal = create_program(shaders_normal, 2);
    prog_rect = create_program(shaders_rect, 2);
    set_draw_color(&default_color);
    return 0;
}

int use_opengl_coords(space_2d *space)
{
    space->use_normal = true;
    return 0;
}

int use_rectangle(space_2d *space, rect *drawing_space, float w_int)
{
    space->use_normal = false;
    space->origin = { drawing_space->x, drawing_space->y };
    space->w_loc = drawing_space->w;
    space->h_loc = drawing_space->h;
    space->w_int = w_int;
    return 0;
}

int start_2d(space_2d *space)
{
    glClear(GL_DEPTH_BUFFER_BIT);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    if (space->use_normal) {
        glUseProgram(prog_normal);
        return 0;
    }

    glUseProgram(prog_rect);
    GLint vp_rect[4];
    glGetIntegerv(GL_VIEWPORT, vp_rect);

    float aspect = (float)vp_rect[2] / (float)vp_rect[3];
    float scale_w = fabs(space->w_loc) / space->w_int;
    float multi_w = space->w_loc < 0.0f ? -1.0f : 1.0f;
    float multi_h = space->h_loc < 0.0f ? -1.0f : 1.0f;

    GLint aspect_loc = glGetUniformLocation(prog_rect, "aspect");
    GLint scale_w_loc = glGetUniformLocation(prog_rect, "scale_w");
    GLint multi_w_loc = glGetUniformLocation(prog_rect, "multi_w");
    GLint multi_h_loc = glGetUniformLocation(prog_rect, "multi_h");
    GLint origin_loc = glGetUniformLocation(prog_rect, "origin");

    glUniform1f(aspect_loc, aspect);
    glUniform1f(scale_w_loc, scale_w);
    glUniform1f(multi_w_loc, multi_w);
    glUniform1f(multi_h_loc, multi_h);
    glUniform2f(origin_loc, space->origin.x, space->origin.y);
    return 0;
}

#define ARRAY_SIZE(x) ((sizeof(x)) / (sizeof(*x)))

int set_draw_color(color *color)
{
    GLuint progs[] = { prog_rect, prog_normal };
    GLfloat f_color[] = { color->r / 255.0f, color->g / 255.0f,
                          color->b / 255.0f, color->a / 255.0f };

    for (uint i = 0; i < ARRAY_SIZE(progs); i++) {
        GLint loc = glGetUniformLocation (progs[i], "draw_color");

        if (loc >= 0)
            glUniform4fv(loc, 1, f_color);
    }

    return 0;
}

static GLint get_cur_uniform_loc(const char *name)
{
    GLint prog = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &prog);
    if (prog <= 0)
        return -1;

    GLint loc = glGetUniformLocation(prog, name);
    if (loc < 0)
        return -1;
    
    return loc;
}

int set_rot_angle(float phi) {
    GLint loc = glGetUniformLocation(prog_rect, "phi");
    if (loc < 0)
        std::runtime_error("Current program does not contain a rotation component");

    glUniform1f(loc, phi);
    return 0;
}

int set_offset(point *offset) {
    GLint loc = get_cur_uniform_loc("offset");
    if (loc < 0)
        std::runtime_error("Current program does not contain an offset component");

    glUniform2f(loc, offset->x, offset->y);
    return 0;
}

static void draw_tri_generic(tri *tri, bool border)
{
    GLfloat verts[] = { tri->points[0].x, tri->points[0].y,
                        tri->points[1].x, tri->points[1].y,
                        tri->points[2].x, tri->points[2].y };

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, verts);
    glEnableVertexAttribArray(0);
    glDrawArrays(border ? GL_LINE_LOOP : GL_TRIANGLES, 0, 3);
}

/* TODO : use actual input */
int draw_tri(tri *tri)
{
    draw_tri_generic(tri, false);
    return 0;
}

int draw_tri_border(tri *tri)
{
    draw_tri_generic(tri, true);
    return 0;
}

static void draw_rect_generic(rect *rect, bool border)
{
    GLfloat verts[] = { rect->x, rect->y,
                        rect->x + rect->w, rect->y,
                        rect->x + rect->w, rect->y + rect->h,
                        rect->x, rect->y + rect->h,
                        rect->x + rect->w, rect->y + rect->h,
                        rect->x, rect->y };

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, verts);
    glEnableVertexAttribArray(0);
    glDrawArrays(border ? GL_LINE_LOOP : GL_TRIANGLES, 0, border ? 4 : 6);
}

int draw_rect(rect *rect)
{
    draw_rect_generic(rect, false);
    return 0;
}

int draw_rect_border(rect *rect)
{
    draw_rect_generic(rect, true);
    return 0;
}

#define PI 3.1415926f
#define CIRCLE_DIVS 16

static void draw_circle_generic(circle *circle, bool border)
{
    GLfloat verts[2 * CIRCLE_DIVS + 4];
    uint offset = 0;
    if (!border) {
        verts[0] = circle->center.x;
        verts[1] = circle->center.y;
        offset = 1;
    }

    for (uint i = offset; i < CIRCLE_DIVS + offset; i++) {
        float phi = 2.0f * PI * (float)i / (float)CIRCLE_DIVS;
        verts[i * 2] = circle->center.x + circle->radius * cosf(phi);
        verts[i * 2 + 1] = circle->center.y + circle->radius * sinf(phi);
    }

    if (!border) {
        verts[CIRCLE_DIVS * 2 + 2] = verts[2];
        verts[CIRCLE_DIVS * 2 + 3] = verts[3];
    }

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, verts);
    glEnableVertexAttribArray(0);
    if (border)
        glDrawArrays(GL_LINE_LOOP, 0, CIRCLE_DIVS);
    else
        glDrawArrays(GL_TRIANGLE_FAN, 0, ARRAY_SIZE(verts) / 2);
}

int draw_circle(circle *circle)
{
    draw_circle_generic(circle, false);
    return 0;
}

int draw_circle_border(circle *circle)
{
    draw_circle_generic(circle, true);
    return 0;
}

void set_line_width(float w)
{
    glLineWidth(w);
}
