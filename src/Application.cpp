#include "Application.hpp"

#include "Input.hpp"
#include "imgui/imgui.h"
#include "math/Random.hpp"

#include <array>

#if !defined(EMSCRIPTEN)
#include "sdl/Main.hpp"
#else
#include "sdl/MainWeb.hpp"
#endif

int main(int argc, char* argv[])
{
    std::unique_ptr<IApplication> app = std::make_unique<Application>();
    return sdl_init(app);
}

static const char* resolution_labels[] {
    "352x240",
    "480x360",
    "858x480",
    "1280x720",
    "1920x1080"
};

static const char* samples_labels[] {
    "1",
    "2",
    "4",
    "8",
    "16"
};

static const char* bounces_labels[] {
    "1",
    "2",
    "4"
};

void Application::Init()
{
    camera = std::make_unique<Camera>();
    camera->position = glm::vec3(0, 5, 35);

    pipeline.Init();

    gui.Init();

    key_down_callback = [=](Scancode key)
    {
        switch (key)
        {
        case Scancode::S_W:
            forward_speed = move_speed;
            break;
        case Scancode::S_S:
            forward_speed = -move_speed;
            break;
        case Scancode::S_A:
            strafe_speed = move_speed;
            break;
        case Scancode::S_D:
            strafe_speed = -move_speed;
            break;
        default:
            break;
        }
    };

    key_up_callback = [=](Scancode key)
    {
        switch (key)
        {
        case Scancode::S_W:
            forward_speed = 0.0f;
            break;
        case Scancode::S_S:
            forward_speed = 0.0f;
            break;
        case Scancode::S_A:
            strafe_speed = 0.0f;
            break;
        case Scancode::S_D:
            strafe_speed = 0.0f;
            break;
        default:
            break;
        }
    };

    SetupScene();

    fps_time = timer_start();
}

void Application::Deinit()
{
    pipeline.DeinitRaytracing();
    pipeline.Deinit();
    gui.Deinit();
}

void Application::SetupScene()
{
    const int32_t placement_radius = 3;
    const size_t num_materials = 4;

    geometry =
    {
        Sphere(
            glm::vec3(0.0f, 5.0f, 0.0f),
            5.0f,
            Material(
                glm::vec3(1.0f),
                0.0f, 1.0f, 0.0f, 0.0f)),
        Sphere(
            glm::vec3(-20.0f, 5.0f, 0.0f),
            5.0f,
            Material(
                glm::vec3(0.0f, 0.5f, 0.5f),
                1.0f, 0.0f, 0.0f, 0.0f)),
        Sphere(
            glm::vec3(20.0f, 5.0f, 0.0f),
            5.0f,
            Material(
                glm::vec3(0.83f, 0.69f, 0.22f),
                0.0f, 1.0f, 1.0f, 0.5f))
    };

    const std::array<Material, num_materials> materials =
    {
        Material(
            glm::vec3(1.0f),
            0.4f, 1.0f, 0.0f, 0.0f),
        Material(
            glm::vec3(0.6, 0.4, 0.5f),
            1.0f, 0.0f, 0.0f, 0.0f),
        Material(
            glm::vec3(0.83f, 0.69f, 0.22f),
            0.0f, 1.0f, 1.0f, 0.5f),
        Material(
            glm::vec3(0.4f, 0.5f, 0.6f),
            0.0f, 0.2f, 0.0f, 0.0f)
    };

    for (int32_t z = -placement_radius; z < placement_radius; z++)
    {
        for (int32_t x = -placement_radius; x < placement_radius; x++)
        {
            Material material = materials[Math::element_rand(
                num_materials)];

            auto geom = Sphere(glm::vec3(
                z + 0.9f * Math::unit_randf(),
                0.2f,
                x + 0.9f * Math::unit_randf()) * 5.0f,
                0.2f * 5.0f,
                material);

            geometry.push_back(
                geom);
        }
    }
}

void Application::Update()
{
    const float time_ms = timer_end(fps_time);
    fps_time = timer_start();
    fps_time_avg = fps_alpha * fps_time_avg + (1.0f - fps_alpha) * time_ms;

    const float fps_scale = std::max<float>(
        1.0f / std::min<float>(5.0f, fps_time_avg / 16.66666f), 0.1f);

    const bool reinit_pipeline = GuiUpdate();

    const float window_aspect_ratio =
        static_cast<float>(window_width) /
        window_height;

    camera->orientation.yaw +=
        static_cast<float>(captured_mouse_delta_x) /
        (mouse_speed * fps_scale * window_aspect_ratio);

    camera->orientation.pitch +=
        static_cast<float>(-captured_mouse_delta_y) /
        (mouse_speed * fps_scale);

    camera->viewport = glm::vec4(
        0, 0,
        raytracing_framebuffer_width,
        raytracing_framebuffer_height);

    camera->Strafe(strafe_speed / fps_scale);
    camera->Forward(forward_speed / fps_scale);

    if (reinit_pipeline)
    {
        pipeline.DeinitRaytracing();

        pipeline.InitRaytracing(
            raytracing_framebuffer_width,
            raytracing_framebuffer_height,
            raytracing_samples,
            raytracing_bounces);
    }

    pipeline.Update(geometry);

    pipeline.Draw(
        window_width,
        window_height,
        camera,
        upscale);

    gui.Draw(
        window_width,
        window_height);
}

bool Application::GuiUpdate()
{
    ImGui::NewFrame();

    ImGui::Begin(
        "Menu",
        NULL,
        ImGuiWindowFlags_AlwaysAutoResize);

    ImGui::LabelText(
        "Controls",
        "Mouse, WSAD.");

    ImGui::Checkbox(
        "Upscale",
        &upscale);

    ImGui::SliderFloat(
        "Exposure",
        &camera->exposure,
        0,
        2);

    ImGui::Combo(
        "Resolution",
        &selected_resolution,
        resolution_labels,
        IM_ARRAYSIZE(resolution_labels));

    ImGui::Combo(
        "Samples",
        &selected_samples,
        samples_labels,
        IM_ARRAYSIZE(samples_labels));

    ImGui::Combo(
        "Bounces",
        &selected_bounces,
        bounces_labels,
        IM_ARRAYSIZE(bounces_labels));

    ImGui::Text(
        "Application average %.3f ms/frame (%.1f FPS)",
        fps_time_avg,
        1000.0f / fps_time_avg);

    ImGui::End();

    bool reinit_pipeline = false;

    uint32_t new_width = 0;
    uint32_t new_height = 0;

    switch (selected_resolution)
    {
    case 0:
        new_width = 352;
        new_height = 240;
        break;
    case 1:
        new_width = 480;
        new_height = 360;
        break;
    case 2:
        new_width = 858;
        new_height = 480;
        break;
    case 3:
        new_width = 1280;
        new_height = 720;
        break;
    case 4:
        new_width = 1920;
        new_height = 1080;
        break;
    default:
        break;
    };

    if (new_height != raytracing_framebuffer_height)
    {
        raytracing_framebuffer_width = new_width;
        raytracing_framebuffer_height = new_height;
        reinit_pipeline = true;
    }

    uint16_t new_samples = 0;

    switch (selected_samples)
    {
    case 0:
        new_samples = 1;
        break;
    case 1:
        new_samples = 2;
        break;
    case 2:
        new_samples = 4;
        break;
    case 3:
        new_samples = 8;
        break;
    case 4:
        new_samples = 16;
        break;
    }

    if (new_samples != raytracing_samples)
    {
        raytracing_samples = new_samples;
        reinit_pipeline = true;
    }

    uint16_t new_bounces = 0;

    switch (selected_bounces)
    {
    case 0:
        new_bounces = 1;
        break;
    case 1:
        new_bounces = 2;
        break;
    case 2:
        new_bounces = 4;
        break;
    }

    if (new_bounces != raytracing_bounces)
    {
        raytracing_bounces = new_bounces;
        reinit_pipeline = true;
    }

    return reinit_pipeline;
}
