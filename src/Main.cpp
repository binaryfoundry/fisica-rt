#include "Main.hpp"

#include "sdl/SDL.hpp"
#include "imgui/imgui.h"
#include "math/Random.hpp"

#include <array>

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

    SetupScene();
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
        sdl_window_height;

    camera->orientation.yaw +=
        static_cast<float>(sdl_captured_mouse_delta_x) /
        (mouse_speed * window_aspect_ratio);

    camera->orientation.pitch +=
        static_cast<float>(-sdl_captured_mouse_delta_y) /
        (mouse_speed);

    camera->viewport = glm::vec4(
        0, 0,
        raytracing_framebuffer_width,
        raytracing_framebuffer_height);

    render.Update(geometry);

    render.Draw(
        sdl_window_width,
        sdl_window_height,
        camera);

    gui.Draw(
        sdl_window_width,
        sdl_window_height);
}

void Main::SetupScene()
{
    const int32_t placement_radius = 3;
    const size_t num_materials = 4;

    const std::array<Material, num_materials> materials =
    {
        Material(glm::vec3(1.0f), 0.8f, 1.0f, 0.0f, 0.0f),
        Material(glm::vec3(1.0f), 0.0f, 0.0f, 0.0f, 0.0f),
        Material(glm::vec3(1.0f), 1.0f, 1.0f, 1.0f, 0.5f),
        Material(glm::vec3(1.0f), 0.7f, 0.3f, 0.0f, 0.0f)
    };

    geometry =
    {
        Sphere(
            glm::vec3(0.0f, 5.0f, 0.0f), 5.0f,
            Material(glm::vec3(1.0f), 0.8f, 1.0f, 0.0f, 0.0f)),
        Sphere(
            glm::vec3(-20.0f, 5.0f, 0.0f), 5.0f,
            Material(glm::vec3(1.0f), 0.0f, 0.0f, 0.0f, 0.0f)),
        Sphere(
            glm::vec3(20.0f, 5.0f, 0.0f), 5.0f,
            Material(glm::vec3(1.0f), 1.0f, 1.0f, 1.0f, 0.5f))
    };

    for (int32_t z = -placement_radius; z < placement_radius; z++)
    {
        for (int32_t x = -placement_radius; x < placement_radius; x++)
        {
            Material material = materials[Math::element_rand(
                num_materials)];

            geometry.push_back(
                    Sphere(glm::vec3(
                        z + 0.9f * Math::unit_randf(),
                        0.2f,
                        x + 0.9f * Math::unit_randf()) * 5.0f,
                    0.2f * 5.0f,
                    material));
        }
    }
}