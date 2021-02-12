#include "Main.hpp"

#include "sdl/SDL.hpp"
#include "sdl/SDLFile.hpp"

#include "imgui/imgui.h"

void Main::Init()
{
    camera = std::make_unique<Camera>(
        1.0f,
        75.0f,
        glm::vec3(0, 0, 0),
        Angles());

    render.Init(
        1280,
        720);

    gui.Init();

    sdl_key_up_callback = [=](uint16_t key)
    {
    };

    sdl_key_down_callback = [=](uint16_t key)
    {
    };
}

void Main::Deinit()
{
    render.Deinit();
    gui.Deinit();
}

void Main::Update()
{
    ImGui::NewFrame();

    ImGui::Begin(
        "Menu",
        NULL,
        ImGuiWindowFlags_AlwaysAutoResize);

    ImGui::End();

    render.Draw(
        sdl_window_width,
        sdl_window_height);

    gui.Draw(
        sdl_window_width,
        sdl_window_height);
}
