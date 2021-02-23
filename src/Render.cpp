#include "Render.hpp"

#include "Shaders.hpp"

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

Render::Render()
{
}

void Render::Init()
{
    quad_vertex_buffer = GL::GenBuffer(
        quad_vertices_data);

    quad_index_buffer = GL::GenBufferIndex(
        quad_indices_data);

    camera_uniforms = std::make_unique<GL::UniformBuffer<CameraUniforms>>();

    noise = std::make_unique<GL::Texture2D<TexDataByteRGBA, 4>>(128, 128);
    noise->Load("files/output_128x128_tri.bmp", 0);
    noise->Load("files/output_128x128_tri.bmp", 1);
    noise->Load("files/output_128x128_tri.bmp", 2);
    noise->Load("files/output_128x128_tri.bmp", 3);
    noise->Update();

    environment = std::make_unique<GL::Texture2D<TexDataFloatRGBA>>(
        "files/loc00184-22-2k.hdr");

    scene = std::make_unique<GL::Texture2D<TexDataFloatRGBA>>(
        scene_data_width,
        scene_data_height);

    frontbuffer_shader_program = GL::LinkShader(
        frontbuffer_vertex_shader_string,
        frontbuffer_fragment_shader_string);

    GL::CheckError();

    frontbuffer_position_attribute_location = glGetAttribLocation(
        frontbuffer_shader_program,
        "position");

    frontbuffer_texcoord_attribute_location = glGetAttribLocation(
        frontbuffer_shader_program,
        "texcoord");

    frontbuffer_projection_uniform_location = glGetUniformLocation(
        frontbuffer_shader_program,
        "projection");

    frontbuffer_view_uniform_location = glGetUniformLocation(
        frontbuffer_shader_program,
        "view");

    frontbuffer_texture_uniform_location = glGetUniformLocation(
        frontbuffer_shader_program,
        "tex");
}

void Render::Deinit()
{
    camera_uniforms->Delete();
    scene_uniforms->Delete();

    environment->Delete();
    scene->Delete();
    noise->Delete();

    glDeleteProgram(
        frontbuffer_shader_program);

    glDeleteProgram(
        raytracing_shader_program);

    glDeleteBuffers(
        1, &quad_vertex_buffer);

    glDeleteBuffers(
        1, &quad_index_buffer);
}

void Render::InitRaytracing(
    const uint32_t framebuffer_width,
    const uint32_t framebuffer_height)
{
    raytracing_shader_program = GL::LinkShader(
        raytracing_vertex_shader_string,
        raytracing_fragment_shader_string);

    GL::CheckError();

    raytracing_camera_uniform_location = glGetUniformBlockIndex(
        raytracing_shader_program,
        "camera");

    raytracing_scene_uniform_location = glGetUniformBlockIndex(
        raytracing_shader_program,
        "scene");

    raytracing_noise_texture_uniform_location = glGetUniformLocation(
        raytracing_shader_program,
        "rand_sampler");

    raytracing_environment_texture_uniform_location = glGetUniformLocation(
        raytracing_shader_program,
        "environment_sampler");

    raytracing_scene_texture_uniform_location = glGetUniformLocation(
        raytracing_shader_program,
        "scene_sampler");

    camera_uniforms = std::make_unique<GL::UniformBuffer<CameraUniforms>>();
    scene_uniforms = std::make_unique<GL::UniformBuffer<SceneUniforms>>();

    framebuffer = std::make_unique<GL::FrameBuffer<TexDataFloatRGBA>>();

    framebuffer->Create(
        framebuffer_width,
        framebuffer_height,
        true);

    GL::CheckError();
}

void Render::DeinitRaytracing()
{
    framebuffer->Delete();

    glDeleteProgram(
        raytracing_shader_program);
}

inline size_t sampler_index(
    uint16_t x, uint16_t y, uint32_t w)
{
    return x + (y * w);
}

void Render::Update(
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
            m.smoothness,
            m.metalness,
            m.refraction,
            m.refractive_index);
    }

    scene_uniforms->object.num_geometry = static_cast<uint32_t>(
        num_geometry);
}

void Render::Draw(
    const uint32_t window_width,
    const uint32_t window_height,
    const std::unique_ptr<Camera>& camera)
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

    glUseProgram(
        raytracing_shader_program);

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

    glBindBufferBase(
        GL_UNIFORM_BUFFER,
        raytracing_camera_uniform_location,
        camera_uniforms->gl_buffer_handle);

    glUniformBlockBinding(
        raytracing_shader_program,
        raytracing_camera_uniform_location,
        raytracing_camera_uniform_location);

    // ...

    glBindBufferBase(
        GL_UNIFORM_BUFFER,
        raytracing_scene_uniform_location,
        scene_uniforms->gl_buffer_handle);

    glUniformBlockBinding(
        raytracing_shader_program,
        raytracing_scene_uniform_location,
        raytracing_scene_uniform_location);

    // ...

    glActiveTexture(
        GL_TEXTURE0);

    glBindTexture(
        GL_TEXTURE_2D_ARRAY,
        noise->gl_texture_handle);

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glUniform1i(
        raytracing_noise_texture_uniform_location,
        0);

    // ...

    glActiveTexture(
        GL_TEXTURE1);

    glBindTexture(
        GL_TEXTURE_2D,
        environment->gl_texture_handle);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glUniform1i(
        raytracing_environment_texture_uniform_location,
        1);

    // ...

    glActiveTexture(
        GL_TEXTURE2);

    glBindTexture(
        GL_TEXTURE_2D,
        scene->gl_texture_handle);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glUniform1i(
        raytracing_scene_texture_uniform_location,
        2);

    // ...

    DrawQuad();

    glBindTexture(
        GL_TEXTURE_2D,
        NULL);

    glUseProgram(
        NULL);

    glBindFramebuffer(
        GL_FRAMEBUFFER,
        0);

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

    const glm::vec3 scale = wide ?
        glm::vec3(std::floor(window_width / aspect), window_height, 1) :
        glm::vec3(window_width, std::floor(window_height * aspect), 1);

    const float hpos = wide ?
        std::round((window_width / 2) - (scale.x / 2)) : 0;

    const float vpos = wide ?
        0 : std::round((window_height / 2) - (scale.y / 2));

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

    glUseProgram(
        frontbuffer_shader_program);

    glUniformMatrix4fv(
        frontbuffer_projection_uniform_location,
        1,
        false,
        &proj[0][0]);

    glUniformMatrix4fv(
        frontbuffer_view_uniform_location,
        1,
        false,
        &view[0][0]);

    glActiveTexture(
        GL_TEXTURE0);

    glBindTexture(
        GL_TEXTURE_2D,
        framebuffer->gl_texture_handle);

    glGenerateMipmap(
        GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glUniform1i(
        frontbuffer_texture_uniform_location,
        0);

    DrawQuad();

    glUseProgram(
        NULL);

    glBindTexture(
        GL_TEXTURE_2D,
        NULL);


    GL::CheckError();
}

void Render::DrawQuad()
{
    glBindBuffer(
        GL_ARRAY_BUFFER,
        quad_vertex_buffer);

    glEnableVertexAttribArray(0);

    glVertexAttribPointer(
        frontbuffer_position_attribute_location,
        3,
        GL_FLOAT,
        GL_FALSE,
        5 * sizeof(GLfloat),
        (GLvoid*)0);

    glEnableVertexAttribArray(1);

    glVertexAttribPointer(
        frontbuffer_texcoord_attribute_location,
        2,
        GL_FLOAT,
        GL_FALSE,
        5 * sizeof(GLfloat),
        (GLvoid*)(3 * sizeof(GLfloat)));

    glBindBuffer(
        GL_ELEMENT_ARRAY_BUFFER,
        quad_index_buffer);

    glDrawElements(
        GL_TRIANGLES,
        static_cast<GLsizei>(quad_indices_data.size()),
        GL_UNSIGNED_INT,
        static_cast<char const*>(0));
}
