#ifndef OPENGL_SDL_UTILS_H
#define OPENGL_SDL_UTILS_H

#include <GL/glew.h>
#include <SDL.h>
#include <SDL_image.h>
#include <GL/glew.h>
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <SDL_opengles2.h>
#else
#include <SDL_opengl.h>
#endif

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include <iostream>
#include <vector>
#include <string>

/* OpenGL error handling */
void printShaderLog(GLuint shader);
void printProgramLog(int prog);
bool checkOpenGLError();

GLuint read_shader(std::string shader_name, unsigned int flags);
GLuint create_program(std::vector<GLuint> shaders);

/* SDL window with opengl context */

/* Those are in bits */
struct channel_sizes
{
    int red = 8;
    int green = 8;
    int blue = 8;
    int depth = 24;
    int stencil = 8;
};

struct minimal_context_cfg
{
    int v_major = 3;
    int v_minor = 2;
    channel_sizes sizes = {};
    bool use_double_buffer = true;
    bool use_deprecated_api = false;
};

SDL_GLContext create_context(SDL_Window *window);
SDL_GLContext create_context(SDL_Window *window, const minimal_context_cfg *cfg);

/* Loading opengl textures with SDL_image */
GLuint load_tex(std::string path);
GLuint load_cubemap(std::vector<std::string> file_paths);

glm::quat quat_from_axis_angle(glm::vec3 axis, float angle);

#endif /* OPENGL_SDL_UTILS_H */