#include "Main.hpp"

#include "sdl/SDL.hpp"
#include "imgui/imgui.h"

void Main::Init()
{
    camera = std::make_unique<Camera>();
    camera->position = glm::vec3(0, 5, 35);

    render.Init();

    render.InitRaytracing(
        raytracing_framebuffer_width,
        raytracing_framebuffer_height);

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
    render.DeinitRaytracing();
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

    ImGui::LabelText(
        "Controls",
        "Mouse, WSAD.");

    ImGui::SliderFloat(
        "Exposure",
        &camera->exposure,
        0,
        2);

    ImGui::End();

    const float window_aspect_ratio =
        static_cast<float>(sdl_window_width) /
        static_cast<float>(sdl_window_height);

    camera->orientation.yaw +=
        static_cast<float>(sdl_captured_mouse_delta_x) /
        (mouse_sensitivity * window_aspect_ratio);

    camera->orientation.pitch +=
        static_cast<float>(-sdl_captured_mouse_delta_y) /
        (mouse_sensitivity);

    camera->viewport = glm::vec4(
        0, 0,
        raytracing_framebuffer_width,
        raytracing_framebuffer_height);

    // TODO write scene data to texture
    //scene->Update();

    render.Draw(
        sdl_window_width,
        sdl_window_height,
        camera);

    gui.Draw(
        sdl_window_width,
        sdl_window_height);
}
