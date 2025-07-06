#include "Cascades.hpp"

#include <sstream>
#include <algorithm>

#define WIDTH 512
#define HEIGHT 512

namespace Pipelines
{
    Cascades::Cascades()
    {
    }

    void Cascades::Init()
    {
        frontbuffer_shader.Load("files/gl/frontbuffer.glsl");
        frontbuffer_shader.Link();

        cascades_shader.Load("files/gl/cascades.glsl");
        cascades_shader.Link();

        //camera_uniforms =
        //    std::make_unique<UniformBuffer<CameraUniforms>>();

        //environment =
        //    std::make_unique<FrameBuffer<TexDataFloatRGBA>>();

        //scene = std::make_unique<Texture2D<TexDataFloatRGBA>>(
        //    scene_data_width,
        //    scene_data_height);

        //environment->Create(
        //    256,
        //    256,
        //    true);

        framebuffer =
            std::make_unique<FrameBuffer<TexDataFloatRGBA>>();

        framebuffer->Create(
            WIDTH,
            HEIGHT,
            true);

        frontbuffer_set_0.SetSampler2D(
            "tex",
            *framebuffer,
            Filter::NEAREST,
            Filter::NEAREST,
            Wrap::CLAMP_TO_EDGE,
            Wrap::CLAMP_TO_EDGE);

        frontbuffer_set_0.SetUniformFloat(
            "exposure",
            &exposure);

        frontbuffer_set_0.SetUniformMat4(
            "view",
            &view);

        frontbuffer_set_0.SetUniformMat4(
            "projection",
            &projection);

        frontbuffer_shader.Set(
            frontbuffer_set_0,
            0);
    }

    void Cascades::Deinit()
    {
        //environment->Delete();
        //scene->Delete();
        //noise->Delete();
        framebuffer->Delete();
        frontbuffer_shader.Delete();
        cascades_shader.Delete();
        //environment_shader.Delete();
    }

    //void Cascades::InitRaytracing(
    //    const uint32_t framebuffer_width,
    //    const uint32_t framebuffer_height,
    //    const uint16_t samples,
    //    const uint16_t bounces)
    //{
        /*noise = std::make_unique<GL::Texture2D<TexDataByteRGBA, 64>>(
            128,
            128);

        const uint16_t noise_samples = samples * bounces;

        for (uint16_t i = 0; i < noise_samples; i++)
        {
            //Noise::generate(
            //    noise->Data(i),
            //    i,
            //    noise_samples);
        }

        noise->Update();

        std::stringstream defines;
        defines << "#define SAMPLES " << samples << std::endl;
        defines << "#define BOUNCES " << bounces << std::endl;

        raytracing_shader.Link(
            defines.str());

        camera_uniforms =
            std::make_unique<UniformBuffer<CameraUniforms>>();

        scene_uniforms =
            std::make_unique<UniformBuffer<SceneUniforms>>();

        framebuffer =
            std::make_unique<FrameBuffer<TexDataFloatRGBA>>();

        framebuffer->Create(
            framebuffer_width,
            framebuffer_height,
            true);

        frontbuffer_set_0.SetSampler2D(
            "tex",
            *framebuffer,
            Filter::NEAREST,
            Filter::NEAREST,
            Wrap::CLAMP_TO_EDGE,
            Wrap::CLAMP_TO_EDGE);

        frontbuffer_set_0.SetUniformMat4(
            "view",
            &view);

        frontbuffer_set_0.SetUniformMat4(
            "projection",
            &projection);

        frontbuffer_set_0.SetUniformFloat(
            "exposure",
            &exposure);

        frontbuffer_shader.Set(
            frontbuffer_set_0,
            0);

        raytracing_set_0.SetUniformBlock(
            "camera",
            *camera_uniforms);

        raytracing_set_0.SetUniformBlock(
            "scene",
            *scene_uniforms);

        raytracing_set_0.SetSampler2DArray(
            "rand_sampler",
            *noise,
            Filter::NEAREST,
            Filter::NEAREST,
            Wrap::REPEAT,
            Wrap::REPEAT);

        raytracing_set_0.SetSampler2D(
            "scene_sampler",
            *scene,
            Filter::NEAREST,
            Filter::NEAREST,
            Wrap::CLAMP_TO_EDGE,
            Wrap::CLAMP_TO_EDGE);

        raytracing_set_0.SetSampler2D(
            "environment_sampler",
            *environment,
            Filter::LINEAR,
            Filter::LINEAR,
            Wrap::REPEAT,
            Wrap::REPEAT);

        raytracing_shader.Set(
            raytracing_set_0,
            0);*/
   // }

   // void Cascades::DeinitRaytracing()
    //{
    //    if (framebuffer == nullptr)
    //    {
     //       return;
    //    }

        /*framebuffer->Delete();
        noise->Delete();
        camera_uniforms->Delete();
        scene_uniforms->Delete();

        raytracing_shader.Delete();*/
    //}

    inline size_t sampler_index(
        uint16_t x, uint16_t y, uint32_t w)
    {
        return x + (y * w);
    }

    //void Cascades::Update(
    //    const std::vector<Sphere>& geometry)
    //{
        /*const auto num_geometry = geometry.size();
        auto& data = *scene->Data();

        for (uint16_t i = 0; i < num_geometry; i++)
        {
            Sphere s = geometry[i];
            Material& m = s.material;

            size_t idx = sampler_index(
                0, i, scene_data_width);

            data[idx + 0] = glm::vec4(
                s.position,
                s.radius);

            data[idx + 1] = glm::vec4(
                m.albedo,
                0.0f);

            data[idx + 2] = glm::vec4(
                m.roughness,
                m.metalness,
                m.refraction,
                m.refractive_index);
        }

        scene_uniforms->object.num_geometry = static_cast<uint32_t>(
            num_geometry);*/
    //}

    void Cascades::Draw(
        const glm::mat4 projection_,
        const glm::mat4 view_,
        const bool upscale)
    {
        // Draw to FBO

        framebuffer->Bind();

        Clear();

        //scene_uniforms->Update();

        DrawQuad(
            cascades_shader);

        // Render to front buffer
        projection = projection_;
        view = view_;

        FrontBuffer();

        Clear();

        DrawQuad(
            frontbuffer_shader);
    }
}
