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

void Render::Init(
    const uint32_t framebuffer_width,
    const uint32_t framebuffer_height)
{
    quad_vertex_buffer = GL::GenBuffer(
        quad_vertices_data);

    quad_index_buffer = GL::GenBufferIndex(
        quad_indices_data);

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

    raytracing_shader_program = GL::LinkShader(
        raytracing_vertex_shader_string,
        raytracing_fragment_shader_string);

    GL::CheckError();

    raytracing_transform_uniform_location = glGetUniformBlockIndex(
        raytracing_shader_program,
        "transform");

    raytracing_noise_0_texture_uniform_location = glGetUniformLocation(
        raytracing_shader_program,
        "rand_sampler_0");

    raytracing_noise_1_texture_uniform_location = glGetUniformLocation(
        raytracing_shader_program,
        "rand_sampler_1");

    raytracing_environment_texture_uniform_location = glGetUniformLocation(
        raytracing_shader_program,
        "environment_sampler");

    raytracing_scene_texture_uniform_location = glGetUniformLocation(
        raytracing_shader_program,
        "scene_sampler");

    framebuffer = std::make_unique<GL::FrameBuffer<TexDataFloatRGBA>>();
    transform = std::make_unique<GL::UniformBuffer<Transform>>();

    framebuffer->Create(framebuffer_width, framebuffer_height, true);

    GL::CheckError();
}

void Render::Deinit()
{
    framebuffer->Delete();
    transform->Delete();

    glDeleteProgram(
        frontbuffer_shader_program);

    glDeleteBuffers(
        1, &quad_vertex_buffer);

    glDeleteBuffers(
        1, &quad_index_buffer);
}

void Render::Draw(
    const uint32_t window_width,
    const uint32_t window_height,
    const float exposure,
    const std::unique_ptr<Camera>& camera,
    const std::unique_ptr<GL::Texture2D<TexDataFloatRGBA>>& environment,
    const std::unique_ptr<GL::Texture2D<TexDataByteRGBA>>& noise_0,
    const std::unique_ptr<GL::Texture2D<TexDataByteRGBA>>& noise_1,
    const std::unique_ptr<GL::Texture2D<TexDataFloatRGBA>>& scene)
{
    camera->Validate();

    glDisable(GL_CULL_FACE);
    glCullFace(GL_BACK);

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

    transform->object.view =
        camera->view;
    transform->object.projection =
        camera->projection;
    transform->object.inverse_projection =
        camera->inverse_projection;
    transform->object.inverse_view_rotation =
        camera->inverse_view_rotation;
    transform->object.viewport =
        glm::vec4(0.0f, 0.0f, framebuffer->Width(), framebuffer->Height());
    transform->object.camera_position =
        glm::vec4(camera->position, 1.0);
    transform->object.exposure =
        glm::vec4(exposure);
    transform->Update();

    glBindBufferBase(
        GL_UNIFORM_BUFFER,
        raytracing_transform_uniform_location,
        transform->gl_buffer_handle);

    glUniformBlockBinding(
        raytracing_shader_program,
        raytracing_transform_uniform_location,
        raytracing_transform_uniform_location);

    // ...

    glActiveTexture(
        GL_TEXTURE0);

    glBindTexture(
        GL_TEXTURE_2D,
        noise_0->gl_texture_handle);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glUniform1i(
        raytracing_noise_0_texture_uniform_location,
        0);

    // ...

    glActiveTexture(
        GL_TEXTURE0);

    glBindTexture(
        GL_TEXTURE_2D,
        noise_1->gl_texture_handle);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glUniform1i(
        raytracing_noise_1_texture_uniform_location,
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
