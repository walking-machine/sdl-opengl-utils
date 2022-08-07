#include "opengl_sdl_utils.hpp"
#include <fstream>

void printShaderLog(GLuint shader) {
    int len = 0;
    int chWrittn = 0;
    char *log;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
    if (len > 0) {
	    log = (char *)malloc(len);
	    glGetShaderInfoLog(shader, len, &chWrittn, log);
	    std::cout << "Shader Info Log: " << log << std::endl;
	    free(log);
    }
}

void printProgramLog(int prog) {
    int len = 0;
    int chWrittn = 0;
    char *log;
    glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &len);
    if (len > 0) {
		log = (char *)malloc(len);
		glGetProgramInfoLog(prog, len, &chWrittn, log);
		std::cout << "Program Info Log: " << log << std::endl;
		free(log);
    }
}

bool checkOpenGLError() {
    bool foundError = false;
    int glErr = glGetError();
    while (glErr != GL_NO_ERROR) {
		std::cout << "glError: " << glErr << std::endl;
		foundError = true;
		glErr = glGetError();
    }
    return foundError;
}


static GLenum sdl_surf_data_fmt(SDL_Surface *surf)
{
    switch (surf->format->BytesPerPixel)
    {
    case 3:
        return GL_RGB;
    case 4:
        return GL_RGBA;
    }

    std::cout << "Was not able to guess format for " <<
                 surf->format->BytesPerPixel << " bytes per pixel\n";
    return 0;
}


void fill_tex_with_image(std::string path, GLuint texture,
                         GLenum img_target)
{
    SDL_Surface* surf = IMG_Load(path.c_str());
    if (!surf) {
        std::cout << "SDL could not load the image for a tex " << SDL_GetError() << "\n";
        return;
    }

    GLenum data_fmt = sdl_surf_data_fmt(surf);
    if (!data_fmt) {
        SDL_FreeSurface(surf);
        return;
    }

    glTexImage2D(img_target, 0, GL_RGBA, surf->w, surf->h, 0, data_fmt,
                 GL_UNSIGNED_BYTE, surf->pixels);
    checkOpenGLError();

    SDL_FreeSurface(surf);
}

GLuint load_tex(std::string path)
{
    GLuint texture;
    
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    fill_tex_with_image(path, texture, GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);

    return texture;
}

/* order: X+,X-,Y+,Y-,Z+,Z- */
GLuint load_cubemap(std::vector<std::string> file_paths)
{
    GLuint cubemap;

    if (file_paths.size() != 6) {
        std::cout << "6 textures must be provided for a cubemap, got " <<
                     file_paths.size() << "\n";
        return 0;
    }

    glGenTextures(1, &cubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);

    for (unsigned int i = 0; i < 6; i++) {
        fill_tex_with_image(file_paths[i], cubemap, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i);
        checkOpenGLError();
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    return cubemap;
}

GLuint read_shader(std::string shader_path, unsigned int flags)
{
    std::ifstream in(shader_path);
    std::string code_str((std::istreambuf_iterator<char>(in)),
                         std::istreambuf_iterator<char>());

    GLuint shader = glCreateShader(flags);
    const char *c_str = (GLchar *)code_str.c_str();
    glShaderSource(shader, 1, &c_str, NULL);
    glCompileShader(shader);
    printShaderLog(shader);
    return shader;
}

GLuint create_program(std::vector<GLuint> shaders)
{
    GLuint program = glCreateProgram();
    for (auto shader : shaders) {
        glAttachShader(program, shader);
    }
    glLinkProgram(program);
    printProgramLog(program);
    return program;
}

SDL_GLContext create_context(SDL_Window *window, const minimal_context_cfg *cfg)
{
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                        !cfg->use_deprecated_api ? SDL_GL_CONTEXT_PROFILE_CORE :
                        SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, cfg->v_major);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, cfg->v_minor);

    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, cfg->sizes.red);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, cfg->sizes.green);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, cfg->sizes.blue);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, cfg->use_double_buffer ? 1 : 0);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, cfg->sizes.depth);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, cfg->sizes.stencil);

    return SDL_GL_CreateContext(window);
}

SDL_GLContext create_context(SDL_Window *window)
{
    minimal_context_cfg cfg;
    return create_context(window, &cfg);
}

glm::quat quat_from_axis_angle(glm::vec3 axis, float angle)
{
    angle *= 0.5f;
    float half_sin = sinf(angle);
    float half_cos = cosf(angle);

    float w = half_cos;
    axis *= half_sin;

    return glm::quat(w, axis.x, axis.y, axis.z);
}