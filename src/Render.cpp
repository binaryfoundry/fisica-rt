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

const float fixed_aspect = 16.0f / 9.0f;

Render::Render()
{
}

void Render::Init(
    const uint32_t framebuffer_width,
    const uint32_t framebuffer_height)
{
    quad_vertex_buffer = OpenGL::GenBuffer(
        quad_vertices_data);

    quad_index_buffer = OpenGL::GenBufferIndex(
        quad_indices_data);

    OpenGL::GenFrameBuffer(
        framebuffer_width,
        framebuffer_height,
        OpenGL::TextureFormat::RGBA32F,
        true,
        framebuffer);

    frontbuffer_shader_program = OpenGL::LinkShader(
        frontbuffer_vertex_shader_string,
        frontbuffer_fragment_shader_string);

    OpenGL::GLCheckError();

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

    raytracing_shader_program = OpenGL::LinkShader(
        raytracing_vertex_shader_string,
        raytracing_fragment_shader_string_test);

    OpenGL::GLCheckError();

    transform = std::make_unique<OpenGL::UniformBuffer<Transform>>();

    OpenGL::GLCheckError();
}

void Render::Deinit()
{
    framebuffer.Delete();
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
    const std::unique_ptr<Camera>& camera)
{
    camera->Validate();

    glDisable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    // Draw to FBO

    glBindFramebuffer(
        GL_FRAMEBUFFER,
        framebuffer.frame);

    glViewport(
        0, 0,
        framebuffer.width,
        framebuffer.height);

    glUseProgram(
        raytracing_shader_program);

    DrawQuad();

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

    const float aspect = window_aspect / fixed_aspect;
    const bool wide = window_width / fixed_aspect > window_height;

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
        framebuffer.texture);

    glGenerateMipmap(
        GL_TEXTURE_2D);

    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_MIN_FILTER,
        GL_NEAREST);

    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_MAG_FILTER,
        GL_NEAREST);

    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_WRAP_S,
        GL_CLAMP_TO_EDGE);

    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_WRAP_T,
        GL_CLAMP_TO_EDGE);

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
