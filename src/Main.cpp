#include "Main.hpp"

#include "sdl/SDL.hpp"
#include "sdl/SDLFile.hpp"

#include "imgui/imgui.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

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

    noise = std::make_unique<GL::Texture2D<TexDataByteRGBA>>();

    uint32_t noise_width;
    uint32_t noise_height;
    std::vector<TexDataByteRGBA> noise_data;

    FileLoadTexture2D_RGBA8(
        "files/blue_noise_128.png",
        noise_width,
        noise_height,
        noise_data);

    noise->Create(
        noise_width,
        noise_height,
        noise_data);

    environment = std::make_unique<GL::Texture2D<TexDataFloatRGBA>>();

    int env_width, env_height, env_channels;
    float* env_raw_data = stbi_loadf(
        "files/loc00184-22-2k.hdr",
        &env_width,
        &env_height,
        &env_channels,
        STBI_rgb_alpha);
    size_t env_raw_data_size =
        env_width * env_height;

    std::vector<TexDataFloatRGBA> env_data(
        env_width * env_height);
    memcpy(
        &env_data[0],
        env_raw_data,
        env_width * env_height * 4 * sizeof(float));
    stbi_image_free(env_raw_data);

    environment->Create(
        env_width,
        env_height,
        env_data);

    scene_data = std::make_unique<std::vector<TexDataFloatRGBA>>(
        scene_data_width * scene_data_height);

    scene = std::make_unique<GL::Texture2D<TexDataFloatRGBA>>();
    scene->Create(
        scene_data_width,
        scene_data_height,
        *scene_data);

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
    noise->Delete();

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

    scene->Update(*scene_data);

    render.Draw(
        sdl_window_width,
        sdl_window_height,
        camera,
        environment,
        noise,
        scene);

    gui.Draw(
        sdl_window_width,
        sdl_window_height);
}
