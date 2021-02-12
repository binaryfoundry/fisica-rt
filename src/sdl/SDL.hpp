#pragma once

#include <functional>

#if !defined (EMSCRIPTEM)
#include <SDL.h>
#include <SDL_syswm.h>
#include <SDL_image.h>
#endif

enum class DPadDirection
{
    UP,
    DOWN,
    LEFT,
    RIGHT
};

extern SDL_Window* sdl_window;
extern int sdl_window_width;
extern int sdl_window_height;

extern std::function<void()> sdl_app_init;
extern std::function<void()> sdl_app_deinit;
extern std::function<void()> sdl_app_update;

extern std::function<void(uint16_t key)> sdl_key_up_callback;
extern std::function<void(uint16_t key)> sdl_key_down_callback;

extern std::function<void(uint16_t id)> sdl_controller_button_up_callback;
extern std::function<void(uint16_t id)> sdl_controller_button_down_callback;

extern std::function<void(DPadDirection dir)> sdl_controller_dpad_up_callback;
extern std::function<void(DPadDirection dir)> sdl_controller_dpad_down_callback;
