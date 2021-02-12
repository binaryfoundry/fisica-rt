#include "SDL.hpp"

SDL_Window* sdl_window = nullptr;
int sdl_window_width;
int sdl_window_height;

std::function<void(uint16_t key)> sdl_key_up_callback;
std::function<void(uint16_t key)> sdl_key_down_callback;

std::function<void(uint16_t id)> sdl_controller_button_up_callback;
std::function<void(uint16_t id)> sdl_controller_button_down_callback;

std::function<void(DPadDirection dir)> sdl_controller_dpad_up_callback;
std::function<void(DPadDirection dir)> sdl_controller_dpad_down_callback;
