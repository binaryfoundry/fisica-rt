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

extern int sdl_window_width;
extern int sdl_window_height;

extern bool sdl_mouse_captured;
extern int16_t sdl_mouse_x;
extern int16_t sdl_mouse_y;
extern int16_t sdl_mouse_delta_x;
extern int16_t sdl_mouse_delta_y;
extern int16_t sdl_captured_mouse_x;
extern int16_t sdl_captured_mouse_y;
extern float sdl_captured_mouse_delta_x;
extern float sdl_captured_mouse_delta_y;

extern std::function<void(SDL_Scancode key)> sdl_key_up_callback;
extern std::function<void(SDL_Scancode key)> sdl_key_down_callback;

extern std::function<void(uint16_t id)> sdl_controller_button_up_callback;
extern std::function<void(uint16_t id)> sdl_controller_button_down_callback;

extern std::function<void(DPadDirection dir)> sdl_controller_dpad_up_callback;
extern std::function<void(DPadDirection dir)> sdl_controller_dpad_down_callback;
