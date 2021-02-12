#include "SDLMainWeb.hpp"

#if defined(EMSCRIPTEN)

#include "../Main.hpp"

#include "../gl/GL.hpp"

#include "SDL.hpp"
#include "SDLFile.hpp"
#include "SDLImgui.hpp"

#include <iostream>
#include <functional>
#include <stdint.h>
#include <map>

#include <emscripten.h>
#include <emscripten/html5.h>

static uint32_t element_width;
static uint32_t element_height;
static bool is_full_screen = false;

static void sdl_run();
static void sdl_update();
static void sdl_update_inputs();
static int sdl_init_graphics();

struct ControllerState
{
    bool b[15] = {
        false, false, false, false,
        false, false, false, false,
        false, false, false, false,
        false, false, false
    };
};

static std::map<int32_t, ControllerState> sdl_controllers;

Main m;

int main(int argc, char *argv[])
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0)
    {
        std::cout << "SDL error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    const char *error = SDL_GetError();
    if (*error != '\0')
    {
        std::cout << "SDL error: " << error;
        SDL_ClearError();
        return 1;
    }

    sdl_imgui_initialise();
    sdl_init_graphics();

    m.Init();

    sdl_run();

    m.Deinit();

    SDL_DestroyWindow(sdl_window);
    SDL_Quit();

    return 0;
}

static void sdl_update()
{
    double cssW, cssH;
    emscripten_get_element_css_size(0, &cssW, &cssH);
    sdl_window_width = static_cast<int>(cssW);
    sdl_window_height = static_cast<int>(cssH);

    if (emscripten_sample_gamepad_data() == EMSCRIPTEN_RESULT_SUCCESS)
    {
        int joy_count = emscripten_get_num_gamepads();

        for (int i = 0; i < joy_count; i++)
        {
            EmscriptenGamepadEvent state;
            emscripten_get_gamepad_status(i, &state);

            if (!state.connected || state.numButtons < 16)
                continue;

            ControllerState& cs = sdl_controllers[state.index];

            for (uint16_t i = 0; i < 16; i++)
            {
                if (!cs.b[i] && state.digitalButton[i])
                {
                    cs.b[i] = true;
                    sdl_controller_button_down_callback(i);
                }
                else if (cs.b[i] && !state.digitalButton[i])
                {
                    cs.b[i] = false;
                    sdl_controller_button_up_callback(i);
                }
            }
        }
    }

    sdl_imgui_update_input(sdl_window);
    sdl_imgui_update_cursor();
    m.Update();
}

static void sdl_update_inputs()
{

}

static int sdl_init_graphics()
{
    double cssW, cssH;
    emscripten_get_element_css_size(0, &cssW, &cssH);
    element_width = static_cast<uint32_t>(cssW);
    element_height = static_cast<uint32_t>(cssH);

    sdl_window = SDL_CreateWindow(
        "ZXSC",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        element_width,
        element_height,
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN);

    EmscriptenWebGLContextAttributes attr;
    emscripten_webgl_init_context_attributes(&attr);
    attr.alpha = 0;
    attr.depth = 1;
    attr.stencil = 0;
    attr.antialias = 1;
    attr.preserveDrawingBuffer = 0;
    attr.preferLowPowerToHighPerformance = 0;
    attr.failIfMajorPerformanceCaveat = 0;
    attr.enableExtensionsByDefault = 1;
    attr.premultipliedAlpha = 0;
    attr.explicitSwapControl = 0;
    attr.majorVersion = 3;
    attr.minorVersion = 0;

    EMSCRIPTEN_WEBGL_CONTEXT_HANDLE ctx = emscripten_webgl_create_context(
        0, &attr);

    emscripten_webgl_make_context_current(
        ctx);

    return 0;
}

EM_BOOL em_fullscreen_callback(
    int event_type,
    const EmscriptenFullscreenChangeEvent* fullscreen_change_event,
    void* user_data)
{
    is_full_screen = fullscreen_change_event->isFullscreen;
    if (is_full_screen)
    {
        int fullscreen_width = static_cast<int>(
            emscripten_get_device_pixel_ratio() *
            fullscreen_change_event->screenWidth + 0.5);
        int fullscreen_height = static_cast<int>(
            emscripten_get_device_pixel_ratio() *
            fullscreen_change_event->screenHeight + 0.5);

        element_width = static_cast<uint32_t>(fullscreen_width);
        element_height = static_cast<uint32_t>(fullscreen_height);
    }
    return false;
}

EM_BOOL em_pointerlock_callback(
    int event_type,
    const EmscriptenPointerlockChangeEvent* pointer_event,
    void* user_data)
{
    return false;
}

EM_BOOL em_mouse_click_callback(
    int event_type,
    const EmscriptenMouseEvent* mouse_event,
    void* user_data)
{
    return false;
}

EM_BOOL em_mouse_move_callback(
    int event_type,
    const EmscriptenMouseEvent *mouse_event,
    void* user_data)
{
    return false;
}

EM_BOOL on_canvassize_changed(
    int event_type,
    const void* reserved,
    void* user_data)
{
    int w, h, fs;
    double cssW, cssH;
    emscripten_get_element_css_size(0, &cssW, &cssH);
    element_width = static_cast<uint32_t>(cssW);
    element_height = static_cast<uint32_t>(cssH);

    printf("Canvas resized: %dx%d, canvas CSS size: %02gx%02g\n", w, h, cssW, cssH);
    return 0;
}

void enter_full_screen()
{
    if (!is_full_screen)
    {
        EM_ASM(JSEvents.inEventHandler = true);
        EM_ASM(JSEvents.currentEventHandler =
        {
            allowsDeferredCalls:true
        });
        EmscriptenFullscreenStrategy s;
        memset(&s, 0, sizeof(s));
        s.scaleMode = EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_HIDEF;
        s.canvasResolutionScaleMode = EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_HIDEF;
        s.filteringMode = EMSCRIPTEN_FULLSCREEN_FILTERING_DEFAULT;
        s.canvasResizedCallback = on_canvassize_changed;
        EMSCRIPTEN_RESULT ret = emscripten_request_fullscreen_strategy(
            0, 1, &s);
        is_full_screen = true;
    }
}

extern "C"
{
    EMSCRIPTEN_KEEPALIVE
        void external_enter_full_screen()
    {
        enter_full_screen();
    }
}

EM_BOOL em_key_down_callback(
    int event_type,
    const EmscriptenKeyboardEvent* key_event,
    void* user_data)
{
    sdl_key_down_callback(static_cast<uint16_t>(key_event->keyCode));
    return false;
}

EM_BOOL em_key_up_callback(
    int event_type,
    const EmscriptenKeyboardEvent* key_event,
    void *user_data)
{
    sdl_key_up_callback(static_cast<uint16_t>(key_event->keyCode));
    return false;
}

EM_BOOL em_resize_callback(
    int event_type,
    const EmscriptenUiEvent* e,
    void* user_data)
{
    double cssW, cssH;
    emscripten_get_element_css_size(0, &cssW, &cssH);
    element_width = static_cast<uint32_t>(cssW);
    element_height = static_cast<uint32_t>(cssH);
    return 0;
}

static int Emscripten_ConvertUTF32toUTF8(
    Uint32 codepoint,
    char* text)
{
    if (codepoint <= 0x7F)
    {
        text[0] = (char)codepoint;
        text[1] = '\0';
    }
    else if (codepoint <= 0x7FF)
    {
        text[0] = 0xC0 | (char)((codepoint >> 6) & 0x1F);
        text[1] = 0x80 | (char)(codepoint & 0x3F);
        text[2] = '\0';
    }
    else if (codepoint <= 0xFFFF)
    {
        text[0] = 0xE0 | (char)((codepoint >> 12) & 0x0F);
        text[1] = 0x80 | (char)((codepoint >> 6) & 0x3F);
        text[2] = 0x80 | (char)(codepoint & 0x3F);
        text[3] = '\0';
    }
    else if (codepoint <= 0x10FFFF)
    {
        text[0] = 0xF0 | (char)((codepoint >> 18) & 0x0F);
        text[1] = 0x80 | (char)((codepoint >> 12) & 0x3F);
        text[2] = 0x80 | (char)((codepoint >> 6) & 0x3F);
        text[3] = 0x80 | (char)(codepoint & 0x3F);
        text[4] = '\0';
    }
    else
    {
        return SDL_FALSE;
    }
    return SDL_TRUE;
}

static EM_BOOL em_handle_key_press(
    int event_type,
    const EmscriptenKeyboardEvent* key_event,
    void* user_data)
{
    char text[5];
    if (Emscripten_ConvertUTF32toUTF8(key_event->charCode, text))
    {
        ImGuiIO& io = ImGui::GetIO();
        io.AddInputCharactersUTF8(text);
    }
    return SDL_GetEventState(SDL_TEXTINPUT) == SDL_ENABLE;
}

static EM_BOOL em_wheel_callback(
    int event_type,
    const EmscriptenWheelEvent* wheel_event,
    void* user_data)
{
    ImGuiIO& io = ImGui::GetIO();
    if (wheel_event->deltaX > 0) io.MouseWheelH -= 1;
    if (wheel_event->deltaX < 0) io.MouseWheelH += 1;
    if (wheel_event->deltaY > 0) io.MouseWheel -= 1;
    if (wheel_event->deltaY < 0) io.MouseWheel += 1;
    return SDL_TRUE;
}

static EM_BOOL em_gamepadconnected_callback(
    int event_type,
    const EmscriptenGamepadEvent* gamepad_event,
    void* user_data)
{
    uint32_t id = (uint32_t)gamepad_event->index;
    if (gamepad_event->connected)
    {
        sdl_controllers[id] = {};
    }
    return SDL_TRUE;
}

static EM_BOOL em_gamepaddisconnected_callbackk(
    int event_type,
    const EmscriptenGamepadEvent* gamepad_event,
    void* user_data)
{
    uint32_t id = (uint32_t)gamepad_event->index;
    if (sdl_controllers.find(id) != sdl_controllers.end())
    {
        sdl_controllers.erase(id);
    }
    return SDL_TRUE;
}

static void sdl_run()
{
    emscripten_set_pointerlockchange_callback(
        NULL, NULL, true, em_pointerlock_callback);
    emscripten_set_fullscreenchange_callback(
        NULL, NULL, true, em_fullscreen_callback);
    emscripten_set_click_callback(
        NULL, NULL, true, em_mouse_click_callback);
    emscripten_set_mousemove_callback(
        NULL, NULL, true, em_mouse_move_callback);
    emscripten_set_keydown_callback(
        NULL, NULL, true, em_key_down_callback);
    emscripten_set_keyup_callback(
        NULL, NULL, true, em_key_up_callback);
    emscripten_set_resize_callback(
        NULL, NULL, true, em_resize_callback);
    emscripten_set_keypress_callback(
        NULL, NULL, true, em_handle_key_press);
    emscripten_set_wheel_callback(
        NULL, NULL, true, em_wheel_callback);
    emscripten_set_gamepadconnected_callback(
        NULL, true, em_gamepadconnected_callback);
    emscripten_set_gamepaddisconnected_callback(
        NULL, true, em_gamepaddisconnected_callbackk);

    emscripten_set_element_css_size(
        NULL, element_width, element_height);

    emscripten_set_main_loop(
        sdl_update, 0, true);
}

#endif
