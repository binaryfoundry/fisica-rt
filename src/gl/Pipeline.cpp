#include "Pipeline.hpp"

#include "../Noise.hpp"

#include <sstream>
#include <algorithm>

namespace GL
{
    static const std::vector<float> quad_vertices_data
    {
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f,
        0.0f, 0.0f, 0.0f,
        0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f, 0.0f,
        1.0f, 1.0f
    };

    static const std::vector<uint32_t> quad_indices_data
    {
         0, 1, 2, 2, 3, 0
    };

    Pipeline::Pipeline()
    {
    }

    void Pipeline::Init()
    {
        frontbuffer_shader.Load("files/gl/frontbuffer.glsl");
        raytracing_shader.Load("files/gl/raytracing.glsl");
        environment_shader.Load("files/gl/environment.glsl");

        frontbuffer_shader.Link();
        environment_shader.Link();

        quad_vertex_buffer = GL::GenBuffer(
            quad_vertices_data);

        quad_index_buffer = GL::GenBufferIndex(
            quad_indices_data);

        camera_uniforms =
            std::make_unique<GL::UniformBuffer<CameraUniforms>>();

        environment =
            std::make_unique<GL::FrameBuffer<TexDataFloatRGBA>>();

        scene = std::make_unique<GL::Texture2D<TexDataFloatRGBA>>(
            scene_data_width,
            scene_data_height);

        environment->Create(
            256,
            256,
            true);

        glBindFramebuffer(
            GL_FRAMEBUFFER,
            environment->gl_frame_handle);

        glViewport(
            0, 0,
            environment->Width(),
            environment->Height());

        environment_shader.BindAttributes();

        DrawQuad();

        glBindTexture(
            GL_TEXTURE_2D,
            NULL);

        glUseProgram(
            NULL);

        glBindFramebuffer(
            GL_FRAMEBUFFER,
            0);
    }

    void Pipeline::Deinit()
    {
        environment->Delete();
        scene->Delete();
        noise->Delete();

        frontbuffer_shader.Delete();
        raytracing_shader.Delete();
        environment_shader.Delete();

        glDeleteBuffers(
            1, &quad_vertex_buffer);

        glDeleteBuffers(
            1, &quad_index_buffer);
    }

    void Pipeline::InitRaytracing(
        const uint32_t framebuffer_width,
        const uint32_t framebuffer_height,
        const uint16_t samples,
        const uint16_t bounces)
    {
        noise = std::make_unique<GL::Texture2D<TexDataByteRGBA, 64>>(
            128,
            128);

        uint16_t noise_samples = samples * bounces;

        for (uint16_t i = 0; i < noise_samples; i++)
        {
            Noise::generate(noise->Data(i), i, noise_samples);
        }

        noise->Update();

        std::stringstream defines;
        defines << "#define SAMPLES " << samples << std::endl;
        defines << "#define BOUNCES " << bounces << std::endl;

        raytracing_shader.Link(defines.str());

        camera_uniforms =
            std::make_unique<GL::UniformBuffer<CameraUniforms>>();

        scene_uniforms =
            std::make_unique<GL::UniformBuffer<SceneUniforms>>();

        framebuffer =
            std::make_unique<GL::FrameBuffer<TexDataFloatRGBA>>();

        framebuffer->Create(
            framebuffer_width,
            framebuffer_height,
            true);

        GL::CheckError();
    }

    void Pipeline::DeinitRaytracing()
    {
        if (framebuffer == nullptr)
        {
            return;
        }

        framebuffer->Delete();
        noise->Delete();
        camera_uniforms->Delete();
        scene_uniforms->Delete();

        raytracing_shader.Delete();
    }

    inline size_t sampler_index(
        uint16_t x, uint16_t y, uint32_t w)
    {
        return x + (y * w);
    }

    void Pipeline::Update(
        const std::vector<Sphere>& geometry)
    {
        const auto num_geometry = geometry.size();
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
            num_geometry);
    }

    void Pipeline::Draw(
        const uint32_t window_width,
        const uint32_t window_height,
        const std::unique_ptr<Camera>& camera,
        const bool upscale)
    {
        glDisable(GL_CULL_FACE);
        glCullFace(GL_BACK);

        scene->Update();

        // Draw to FBO

        glBindFramebuffer(
            GL_FRAMEBUFFER,
            framebuffer->gl_frame_handle);

        glViewport(
            0, 0,
            framebuffer->Width(),
            framebuffer->Height());

        raytracing_shader.BindAttributes();

        camera->Validate();
        camera_uniforms->object.view =
            camera->View();
        camera_uniforms->object.projection =
            camera->Projection();
        camera_uniforms->object.viewport =
            camera->viewport;
        camera_uniforms->object.position = glm::vec4(
            camera->position, 1.0f);
        camera_uniforms->object.exposure = glm::vec4(
            camera->exposure, 0.0, 0.0, 0.0);
        camera_uniforms->Update();

        scene_uniforms->Update();

        // ...

        DrawQuad();

        // Render to front buffer

        glViewport(
            0,
            0,
            window_width,
            window_height);

        glClearColor(0, 0, 0, 1);

        glClear(
            GL_COLOR_BUFFER_BIT |
            GL_DEPTH_BUFFER_BIT |
            GL_STENCIL_BUFFER_BIT);

        const float window_aspect =
            static_cast<float>(window_width) /
            window_height;

        const float framebuffer_ratio =
            static_cast<float>(framebuffer->Width()) /
            static_cast<float>(framebuffer->Height());

        const float aspect = window_aspect / framebuffer_ratio;
        const bool wide = window_width / framebuffer_ratio > window_height;

        glm::vec3 scale = wide ?
            glm::vec3(std::floor(window_width / aspect), window_height, 1) :
            glm::vec3(window_width, std::floor(window_height * aspect), 1);

        if (!upscale)
        {
            scale.x = std::min<float>(
                scale.x, static_cast<float>(framebuffer->Width()));

            scale.y = std::min<float>(
                scale.y, static_cast<float>(framebuffer->Height()));
        }

        const float hpos =
            std::round((window_width / 2) - (scale.x / 2));

        const float vpos =
            std::round((window_height / 2) - (scale.y / 2));

        const glm::mat4 proj = glm::ortho<float>(
            0,
            static_cast<float>(window_width),
            static_cast<float>(window_height),
            0,
            -1.0f,
            1.0f);

        glm::mat4 view = glm::mat4();

        view = glm::translate(
            view,
            glm::vec3(hpos, vpos, 0.0f));

        view = glm::scale(
            view,
            scale);

        frontbuffer_shader.BindAttributes();

        DrawQuad();

        GL::CheckError();
    }

    void Pipeline::DrawQuad()
    {
        glBindBuffer(
            GL_ARRAY_BUFFER,
            quad_vertex_buffer);

        glBindBuffer(
            GL_ELEMENT_ARRAY_BUFFER,
            quad_index_buffer);

        glDrawElements(
            GL_TRIANGLES,
            static_cast<GLsizei>(quad_indices_data.size()),
            GL_UNSIGNED_INT,
            static_cast<char const*>(0));
    }
}
