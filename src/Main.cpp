#include "Main.hpp"

#include "sdl/SDL.hpp"
#include "sdl/SDLFile.hpp"

#include "imgui/imgui.h"

//if (backend->IsScanCodeDown(Scancode::SCANCODE_W))
//{
//    position += camera->direction * speed;
//}
//if (backend->IsScanCodeDown(Scancode::SCANCODE_S))
//{
//    position += camera->direction * -speed;
//}
//if (backend->IsScanCodeDown(Scancode::SCANCODE_A))
//{
//    strafe(speed);
//}
//if (backend->IsScanCodeDown(Scancode::SCANCODE_D))
//{
//    strafe(-speed);
//}

void Main::Init()
{
    camera = std::make_unique<Camera>();

    noise_0 = std::make_unique<GL::Texture2D<TexDataByteRGBA>>();
    noise_0->Load("files/output_256x256_tri.bmp");

    noise_1 = std::make_unique<GL::Texture2D<TexDataByteRGBA>>();
    noise_1->Load("files/output_256x256_tri.bmp");

    environment = std::make_unique<GL::Texture2D<TexDataFloatRGBA>>();
    environment->Load("files/loc00184-22-2k.hdr");

    scene = std::make_unique<GL::Texture2D<TexDataFloatRGBA>>();
    scene->Create(
        scene_data_width,
        scene_data_height);

    render.Init(
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
    environment->Delete();
    scene->Delete();
    noise_0->Delete();
    noise_1->Delete();

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
        "Cursor keys, Ctrl.");

    ImGui::SliderFloat("Exposure", &exposure, 0, 2);

    ImGui::End();

    const float window_aspect_ratio =
        static_cast<float>(sdl_window_width) /
        static_cast<float>(sdl_window_height);

    orientation.yaw +=
        static_cast<float>(sdl_captured_mouse_delta_x) /
        (150.0f * window_aspect_ratio);

    orientation.pitch +=
        static_cast<float>(-sdl_captured_mouse_delta_y) / 150.0f;

    camera->SetPosition(position);
    camera->SetAngles(orientation);

    //scene->Update(*scene_data);

    render.Draw(
        sdl_window_width,
        sdl_window_height,
        exposure,
        camera,
        environment,
        noise_0,
        noise_1,
        scene);

    gui.Draw(
        sdl_window_width,
        sdl_window_height);
}
