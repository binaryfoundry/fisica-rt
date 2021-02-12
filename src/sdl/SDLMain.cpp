#include "SDLMain.hpp"

#if !defined(EMSCRIPTEN)

#include "../Main.hpp"

#include "../gl/GL.hpp"

#include "SDL.hpp"
#include "SDLFile.hpp"
#include "SDLImgui.hpp"

#include <iostream>
#include <functional>
#include <stdint.h>
#include <map>

#include <EGL/egl.h>
#include <EGL/eglext.h>

static SDL_GLContext gl;

static EGLDisplay egl_display;
static EGLContext egl_context;
static EGLSurface egl_surface;

static int sdl_init_graphics();
static bool sdl_poll_events();

static std::map<int32_t, SDL_GameController*> sdl_controllers;

static int window_width = 320 * 4;
static int window_height = 240 * 4;

static Main m;

int main(int argc, char *argv[])
{
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
    {
        std::cout << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) < 0)
    {
        std::cout << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    sdl_imgui_initialise();
    sdl_init_graphics();

    m.Init();

    bool done = false;

    while (!done)
    {
        SDL_GetWindowSize(
            sdl_window,
            &sdl_window_width,
            &sdl_window_height);

        done = sdl_poll_events();

        sdl_imgui_update_input(sdl_window);
        sdl_imgui_update_cursor();

        m.Update();
        eglSwapBuffers(egl_display, egl_surface);
    }

    m.Deinit();

    sdl_imgui_destroy();
    SDL_GL_DeleteContext(gl);
    SDL_DestroyWindow(sdl_window);
    SDL_Quit();

    return 0;
}

static int sdl_init_graphics()
{
    sdl_window = SDL_CreateWindow(
        "ZXS",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        window_width,
        window_height,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);

    SDL_SysWMinfo info;
    SDL_VERSION(
        &info.version);

    const SDL_bool get_win_info = SDL_GetWindowWMInfo(
        sdl_window,
        &info);
    SDL_assert_release(
        get_win_info);

    const EGLNativeWindowType hWnd = reinterpret_cast<EGLNativeWindowType>(
        info.info.win.window);

    EGLint err;
    EGLint numConfigs;
    EGLint major_version;
    EGLint minor_version;
    EGLDisplay display;
    EGLContext context;
    EGLSurface surface;
    EGLConfig config;

    const EGLint config_attribs[] =
    {
        EGL_RED_SIZE,       8,
        EGL_GREEN_SIZE,     8,
        EGL_BLUE_SIZE,      8,
        EGL_ALPHA_SIZE,     8,
        EGL_DEPTH_SIZE,     24,
        EGL_SAMPLE_BUFFERS, 0,
        EGL_SAMPLES,        0,
        EGL_SURFACE_TYPE,   EGL_WINDOW_BIT,
        EGL_RENDERABLE_TYPE,EGL_OPENGL_ES3_BIT_KHR,
        EGL_BUFFER_SIZE,    16,
        EGL_NONE
    };

    const EGLint context_attribs[] =
    {
        EGL_CONTEXT_MAJOR_VERSION_KHR, 2,
        EGL_CONTEXT_MINOR_VERSION_KHR, 0,
        EGL_NONE
    };

    const EGLint surface_attribs[] =
    {
        //EGL_RENDER_BUFFER, EGL_BACK_BUFFER,
        //EGL_GL_COLORSPACE_KHR, EGL_GL_COLORSPACE_SRGB_KHR,
        //EGL_COLORSPACE, EGL_COLORSPACE_sRGB,
        //EGL_GL_COLORSPACE_KHR, EGL_GL_COLORSPACE_SRGB_KHR,
        EGL_NONE
    };

    display = eglGetDisplay(GetDC(hWnd));

    if (display == EGL_NO_DISPLAY)
        goto failed;
    if (!eglInitialize(
        display,
        &major_version,
        &minor_version))
        goto failed;
    if (!eglBindAPI(EGL_OPENGL_ES_API))
        goto failed;
    if (!eglGetConfigs(
        display,
        NULL,
        0,
        &numConfigs))
        goto failed;
    if (!eglChooseConfig(
        display,
        config_attribs,
        &config,
        1,
        &numConfigs))
        goto failed;
    if (numConfigs != 1) {
        goto failed;
    }

    surface = eglCreateWindowSurface(
        display,
        config,
        hWnd,
        surface_attribs);
    if (surface == EGL_NO_SURFACE)
        goto failed;
    context = eglCreateContext(
        display,
        config,
        EGL_NO_CONTEXT,
        context_attribs);
    if (context == EGL_NO_CONTEXT)
        goto failed;
    if (!eglMakeCurrent(
        display,
        surface,
        surface,
        context))
        goto failed;

failed:
    if ((err = eglGetError()) != EGL_SUCCESS)
    {
        std::cout << err << std::endl;
        return 1;
    }

    egl_display = display;
    egl_surface = surface;
    egl_context = context;

    const auto swap_interval = eglSwapInterval(egl_display, 1);

    //SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);

    const char *error = SDL_GetError();
    if (*error != '\0')
    {
        std::cout << error << std::endl;
        SDL_ClearError();
        return 1;
    }

    const GLubyte* version = glGetString(GL_VERSION);
    std::cout << "OpenGL Version: " << version << std::endl;

    return 0;
}

static bool sdl_poll_events()
{
    SDL_Event event;
    SDL_PumpEvents();

    ImGuiIO& io = ImGui::GetIO();

    uint16_t key = 0;
    SDL_GameController* controller;
    SDL_GameControllerAxis axis;

    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_QUIT:
            return true;
            break;

        case SDL_APP_DIDENTERFOREGROUND:
            break;

        case SDL_APP_DIDENTERBACKGROUND:
            break;

        case SDL_APP_LOWMEMORY:
            break;

        case SDL_APP_TERMINATING:
            break;

        case SDL_APP_WILLENTERBACKGROUND:
            break;

        case SDL_APP_WILLENTERFOREGROUND:
            break;

        case SDL_MOUSEMOTION:
            break;

        case SDL_WINDOWEVENT:
            switch (event.window.event)
            {
                case SDL_WINDOWEVENT_RESIZED:
                {
                    break;
                }
            }

        case SDL_MOUSEBUTTONUP:
            break;

        case SDL_MOUSEBUTTONDOWN:
            if (event.button.button == SDL_BUTTON_LEFT)
            {
                g_MousePressed[0] = true;
            }
            if (event.button.button == SDL_BUTTON_RIGHT)
            {
                g_MousePressed[1] = true;
            }
            if (event.button.button == SDL_BUTTON_MIDDLE)
            {
                g_MousePressed[2] = true;
            }
            break;

        case SDL_MOUSEWHEEL:
            if (event.wheel.x > 0)
            {
                io.MouseWheelH += 1;
            }
            if (event.wheel.x < 0)
            {
                io.MouseWheelH -= 1;
            }
            if (event.wheel.y > 0)
            {
                io.MouseWheel += 1;
            }
            if (event.wheel.y < 0)
            {
                io.MouseWheel -= 1;
            }
            break;

        case SDL_TEXTINPUT:
            io.AddInputCharactersUTF8(event.text.text);
            break;

        case SDL_KEYDOWN:
            sdl_key_down_callback(static_cast<uint16_t>(event.key.keysym.sym));
            break;

        case SDL_KEYUP:
            sdl_key_up_callback(static_cast<uint16_t>(event.key.keysym.sym));
            key = event.key.keysym.scancode;
            IM_ASSERT(key >= 0 && key < IM_ARRAYSIZE(io.KeysDown));
            io.KeysDown[key] = (event.type == SDL_KEYDOWN);
            io.KeyShift = ((SDL_GetModState() & KMOD_SHIFT) != 0);
            io.KeyCtrl = ((SDL_GetModState() & KMOD_CTRL) != 0);
            io.KeyAlt = ((SDL_GetModState() & KMOD_ALT) != 0);
            io.KeySuper = false;
            break;

        case SDL_CONTROLLERAXISMOTION:
            axis = (SDL_GameControllerAxis)event.caxis.axis;
            switch (axis)
            {
                case SDL_CONTROLLER_AXIS_LEFTX:
                    break;
                case SDL_CONTROLLER_AXIS_LEFTY:
                    break;
            };
            break;

        case SDL_CONTROLLERBUTTONDOWN:
            sdl_controller_button_down_callback(
                static_cast<uint16_t>(event.cbutton.button));
            break;

        case SDL_CONTROLLERBUTTONUP:
            sdl_controller_button_up_callback(
                static_cast<uint16_t>(event.cbutton.button));
            break;

        case SDL_CONTROLLERDEVICEADDED:
            controller = SDL_GameControllerOpen(event.cdevice.which);
            sdl_controllers[event.cdevice.which] = controller;
            break;

        case SDL_CONTROLLERDEVICEREMOVED:
            if (sdl_controllers.find(event.cdevice.which) != sdl_controllers.end())
            {
                auto controller = sdl_controllers[event.cdevice.which];
                SDL_GameControllerClose(controller);
                sdl_controllers[event.cdevice.which] = nullptr;
            }
            break;
        }
    }

    return false;
}

#endif
