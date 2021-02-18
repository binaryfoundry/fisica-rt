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

void Main::SetupScene()
{
    const int32_t placement_radius = 3;
    const size_t num_materials = 4;

    const std::array<glm::vec4, num_materials> materials =
    {
        glm::vec4(0.8, 1.0, 0.0, 0.0),
        glm::vec4(0.0, 0.0, 0.0, 0.0),
        glm::vec4(1.0, 1.0, 1.0, 0.5),
        glm::vec4(0.7, 0.3, 0.0, 0.0)
    };

    std::vector<Sphere> shapes
    {
        {
            glm::vec4(0.0, 5.0, 0.0, 5.0),
            glm::vec4(1.0, 1.0, 1.0, 0.0),
            glm::vec2(0.8, 1.0),
            glm::vec2(0.0, 0.0)
        },
        {
            glm::vec4(-20.0, 5.0, 0.0, 5.0),
            glm::vec4(1.0, 1.0, 1.0, 0.0),
            glm::vec2(0.0, 0.0),
            glm::vec2(0.0, 0.0)
        },
        {
            glm::vec4(20.0, 5.0, 0.0, 5.0),
            glm::vec4(1.0, 1.0, 1.0, 0.0),
            glm::vec2(1.0, 1.0),
            glm::vec2(1.0, 0.5)
        }
    };

    for (int32_t z = -placement_radius; z < placement_radius; z++)
    {
        for (int32_t x = -placement_radius; x < placement_radius; x++)
        {
            glm::vec4 material = materials[Math::element_rand(
                num_materials)];

            shapes.push_back(
            {
                glm::vec4(
                    z + 0.9 * Math::unit_randf(),
                    0.2,
                    x + 0.9 * Math::unit_randf(),
                    0.2f) * 5.0f,
                glm::vec4(1.0, 1.0, 1.0, 0.0),
                glm::vec2(material.x, material.y),
                glm::vec2(material.z, material.w)
            });
        }
    }
}