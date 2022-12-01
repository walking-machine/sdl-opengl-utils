#include "../gl_sdl_utils.hpp"
#include "../gl_sdl_2d.hpp"
#include "../gl_sdl_shape_obj.hpp"
#include <memory>

static float aspect = 1.0f;
static color magenta = { 255, 0, 255 };

static std::unique_ptr<shape> shapes[3] = { std::unique_ptr<shape>(new shape_tri({25.f, 25.f}, {25.f, 0.f}, {0.0f, 0.0f})),
                                            std::unique_ptr<shape>(new shape_rect({0.f, 0.f}, 15.f, 15.f)),
                                            std::unique_ptr<shape>(new shape_circle(8.f))
};

#define ARRAY_SIZE(_x) (sizeof(_x) / sizeof(*_x))

shape_manager_state<std::unique_ptr<shape>> manager_state = {
    .shapes = shapes,
    .num_shapes = ARRAY_SIZE(shapes),
    .first_to_draw = 0
};

space_2d space;

static float line_width = 1.0f;

int res_init()
{
    rect r {0.f, 0.f, 1.0f, 1.0f};
    use_rectangle(&space, &r, 100.0f);

    return init_2d();
}

#ifdef __EMSCRIPTEN__
    EM_JS(int, get_canvas_width, (), { return canvas.width; });
    EM_JS(int, get_canvas_height, (), { return canvas.height; });
#endif

int display()
{
    start_2d(&space);
    shape_circle *inter_circle = dynamic_cast<shape_circle*>(shapes[2].get());
    for (uint i = 0; i < 2; i++) {
        if (shapes[i]->intersects_circle(inter_circle))
            shapes[i]->set_color(magenta);
    }

    draw_all_shapes(&manager_state);

    return 0;
}

static void reset_viewport_to_window(SDL_Window *window)
{
    int w, h;
#ifndef __EMSCRIPTEN__
    SDL_GL_GetDrawableSize(window, &w, &h);
#else
    w = get_canvas_width();
    h = get_canvas_height();
#endif
    aspect = (float)w / (float)h;
    glViewport(0, 0, w, h);
}

static bool handle_mouse(SDL_Event *event)
{
    if (event->type != SDL_MOUSEMOTION &&
        event->type != SDL_MOUSEBUTTONUP &&
        event->type != SDL_MOUSEBUTTONDOWN)
        return false;

    assign_random_colors<std::unique_ptr<shape>>(&manager_state);
    try_drag_all_shapes<std::unique_ptr<shape>>(event, &manager_state, &space);

    return false;
}

static bool handle_keyboard(SDL_Event *event)
{
    switch(event->type) {
    case SDL_KEYDOWN:
        switch(event->key.keysym.sym) {
        case SDLK_LEFT:
            shapes[0]->rotate(0.2f);
            break;
        case SDLK_RIGHT:
            shapes[0]->rotate(-0.2f);
            break;
        }

        return true;
    }

    return false;
}

int main(int, char**)
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0)
    {
        std::cout << "SDL could not start, error: " << SDL_GetError() << "\n";
        return -1;
    }

    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL |
                                                     SDL_WINDOW_RESIZABLE |
                                                     SDL_WINDOW_ALLOW_HIGHDPI);

    SDL_Window* window = SDL_CreateWindow("Demo", SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED, 1280, 720,
                                          window_flags);

    auto gl_context = create_context(window);
    if (!gl_context) {
        std::cout << "SDL could not create a context, error: " << SDL_GetError() << "\n";
        return -1;
    }

    GLenum glew_ret = glewInit();
    if (glew_ret != GLEW_OK) {
        std::cout << "glew could not start, error: " << (unsigned long) glew_ret << "\n";
        return -1;
    }

    if (SDL_GL_MakeCurrent(window, gl_context)) {
        std::cout << "SDL could make context current, error: " << SDL_GetError() << "\n";
        return -1;
    }

#ifndef __EMSCRIPTEN__
    if (SDL_GL_SetSwapInterval(1))  /* Enable vsync */ {
        std::cout << "SDL could set swap interval, error: " << SDL_GetError() << "\n";
        return -1;
    }
#endif

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cout << "SDL could not get renderer, error: " << SDL_GetError() << "\n";
        return -1;
    }

    res_init();
    reset_viewport_to_window(window);

    bool done = false;
    while (!done)
    {
#ifdef __EMSCRIPTEN__
    emscripten_sleep(0);
#endif
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
                done = true;
            if (event.type == SDL_WINDOWEVENT &&
                event.window.event == SDL_WINDOWEVENT_CLOSE &&
                event.window.windowID == SDL_GetWindowID(window))
                done = true;

            handle_mouse(&event);
            handle_keyboard(&event);
        }

        glClearColor(0.4f, 0.0f, 0.4f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        int ret = display();
        if (ret)
            return ret;
        SDL_GL_SwapWindow(window);
    }

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}