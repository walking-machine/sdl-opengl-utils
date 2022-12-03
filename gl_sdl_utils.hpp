#ifndef GL_SDL_UTILS_H
#define GL_SDL_UTILS_H

#include <GL/glew.h>
#include <SDL.h>
#include <SDL_image.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif
#include <SDL_opengl.h>

#define GLM_FORCE_PURE
#ifndef __EMSCRIPTEN__
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/quaternion.hpp>
#else
#include <glm.hpp>
#include <gtc/type_ptr.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtx/transform.hpp>
#include <gtc/quaternion.hpp>
#endif

#include <iostream>
#include <vector>
#include <string>

/* OpenGL error handling */
void printShaderLog(GLuint shader);
void printProgramLog(int prog);
bool checkOpenGLError();

GLuint read_shader(std::string shader_path, unsigned int flags);
GLuint read_shader(const char *shader_src, unsigned int flags);
GLuint create_program(std::vector<GLuint> shaders);
GLuint create_program(const GLuint *shaders, uint num_shaders);

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
    channel_sizes sizes;
    bool use_double_buffer = true;
};

SDL_GLContext create_context(SDL_Window *window);
SDL_GLContext create_context(SDL_Window *window, minimal_context_cfg *cfg);

/* Loading opengl textures with SDL_image */
GLuint load_tex(std::string path);
GLuint load_cubemap(std::vector<std::string> file_paths);

glm::quat quat_from_axis_angle(glm::vec3 axis, float angle);

#endif /* OPENGL_SDL_UTILS_H */