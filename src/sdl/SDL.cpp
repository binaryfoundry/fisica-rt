#include "SDL.hpp"

SDL_Window* sdl_window = nullptr;
int sdl_window_width;
int sdl_window_height;

bool sdl_mouse_captured = false;
int16_t sdl_mouse_x = -1;
int16_t sdl_mouse_y = -1;
int16_t sdl_mouse_delta_x = 0;
int16_t sdl_mouse_delta_y = 0;
int16_t sdl_captured_mouse_x = -1;
int16_t sdl_captured_mouse_y = -1;
int16_t sdl_captured_mouse_delta_x = 0;
int16_t sdl_captured_mouse_delta_y = 0;

std::function<void(uint16_t key)> sdl_key_up_callback;
std::function<void(uint16_t key)> sdl_key_down_callback;

std::function<void(uint16_t id)> sdl_controller_button_up_callback;
std::function<void(uint16_t id)> sdl_controller_button_down_callback;

std::function<void(DPadDirection dir)> sdl_controller_dpad_up_callback;
std::function<void(DPadDirection dir)> sdl_controller_dpad_down_callback;
