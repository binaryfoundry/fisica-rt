#include "GUI.hpp"

#include "imgui/imgui.h"

static const std::string vertex_shader_string =
    R"(#version 100
    #ifdef GL_ES
    precision mediump float;
    #endif
    uniform mat4 ProjMtx;
    attribute vec2 Position;
    attribute vec2 UV;
    attribute vec4 Color;
    varying vec2 Frag_UV;
    varying vec4 Frag_Color;
    void main()
    {
        Frag_UV = UV;
        Frag_Color = Color;
        gl_Position = ProjMtx * vec4(Position.xy, 0.0, 1.0);
    })";

static const std::string fragment_shader_string =
    R"(#version 100
    #ifdef GL_ES
    precision mediump float;
    #endif
    uniform sampler2D Texture;
    varying vec2 Frag_UV;
    varying vec4 Frag_Color;
    void main()
    {
        gl_FragColor = Frag_Color * texture2D(Texture, Frag_UV.st);
    })";

GUI::GUI()
{
}

void GUI::Init()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2(1280, 720);
    io.DeltaTime = 1.0f / 60.0f;

    unsigned char* pixels;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(
        &pixels,
        &width,
        &height);

    gl_shader_program = OpenGL::LinkShader(
        vertex_shader_string,
        fragment_shader_string);

    g_AttribLocationTex = glGetUniformLocation(
        gl_shader_program,
        "Texture");

    g_AttribLocationProjMtx = glGetUniformLocation(
        gl_shader_program,
        "ProjMtx");

    g_AttribLocationVtxPos = static_cast<GLuint>(glGetAttribLocation(
        gl_shader_program,
        "Position"));

    g_AttribLocationVtxUV = static_cast<GLuint>(glGetAttribLocation(
        gl_shader_program,
        "UV"));

    g_AttribLocationVtxColor = static_cast<GLuint>(glGetAttribLocation(
        gl_shader_program,
        "Color"));

    glGenBuffers(
        1,
        &g_VboHandle);

    glGenBuffers(
        1,
        &g_ElementsHandle);

    glGenTextures(
        1,
        &g_FontTexture);

    glBindTexture(
        GL_TEXTURE_2D,
        g_FontTexture);

    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_MIN_FILTER,
        GL_LINEAR);

    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_MAG_FILTER,
        GL_LINEAR);

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGBA,
        width,
        height,
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        pixels);

    io.Fonts->TexID = (ImTextureID)(intptr_t)g_FontTexture;

    OpenGL::GLCheckError();
}

void GUI::Deinit()
{
    glDeleteProgram(
        gl_shader_program);

    glDeleteTextures(
        1,
        &g_FontTexture);
}

void GUI::Draw(
    const uint32_t window_width,
    const uint32_t window_height)
{
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2(
        static_cast<float>(window_width),
        static_cast<float>(window_height));

    ImGui::Render();

    const glm::mat4 proj = glm::ortho<float>(
        0,
        static_cast<float>(window_width),
        static_cast<float>(window_height),
        0,
        -1.0f,
        1.0f);

    glEnable(
        GL_BLEND);

    glBlendEquation(
        GL_FUNC_ADD);

    glBlendFunc(
        GL_SRC_ALPHA,
        GL_ONE_MINUS_SRC_ALPHA);

    glDisable(
        GL_CULL_FACE);

    glDisable(
        GL_DEPTH_TEST);

    glEnable(
        GL_SCISSOR_TEST);

    glUseProgram(
        gl_shader_program);

    glUniform1i(
        g_AttribLocationTex,
        0);

    glUniformMatrix4fv(
        g_AttribLocationProjMtx,
        1,
        GL_FALSE,
        &proj[0][0]);

    glBindBuffer(
        GL_ARRAY_BUFFER,
        g_VboHandle);

    glBindBuffer(
        GL_ELEMENT_ARRAY_BUFFER,
        g_ElementsHandle);

    glEnableVertexAttribArray(
        g_AttribLocationVtxPos);

    glEnableVertexAttribArray(
        g_AttribLocationVtxUV);

    glEnableVertexAttribArray(
        g_AttribLocationVtxColor);

    glVertexAttribPointer(
        g_AttribLocationVtxPos,
        2,
        GL_FLOAT,
        GL_FALSE,
        sizeof(ImDrawVert),
        (GLvoid*)IM_OFFSETOF(ImDrawVert, pos));

    glVertexAttribPointer(
        g_AttribLocationVtxUV,
        2,
        GL_FLOAT,
        GL_FALSE,
        sizeof(ImDrawVert),
        (GLvoid*)IM_OFFSETOF(ImDrawVert, uv));

    glVertexAttribPointer(
        g_AttribLocationVtxColor,
        4,
        GL_UNSIGNED_BYTE,
        GL_TRUE,
        sizeof(ImDrawVert),
        (GLvoid*)IM_OFFSETOF(ImDrawVert, col));

    const ImDrawData* draw_data = ImGui::GetDrawData();

    const ImVec2 clip_off = draw_data->DisplayPos;
    const ImVec2 clip_scale = draw_data->FramebufferScale;

    for (int n = 0; n < draw_data->CmdListsCount; n++)
    {
        const ImDrawList* cmd_list = draw_data->CmdLists[n];

        glBufferData(
            GL_ARRAY_BUFFER,
            (GLsizeiptr)cmd_list->VtxBuffer.Size * (int)sizeof(ImDrawVert),
            (const GLvoid*)cmd_list->VtxBuffer.Data,
            GL_STREAM_DRAW);

        glBufferData(
            GL_ELEMENT_ARRAY_BUFFER,
            (GLsizeiptr)cmd_list->IdxBuffer.Size * (int)sizeof(ImDrawIdx),
            (const GLvoid*)cmd_list->IdxBuffer.Data,
            GL_STREAM_DRAW);

        for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
        {
            const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];

            ImVec4 clip_rect;
            clip_rect.x = (pcmd->ClipRect.x - clip_off.x) * clip_scale.x;
            clip_rect.y = (pcmd->ClipRect.y - clip_off.y) * clip_scale.y;
            clip_rect.z = (pcmd->ClipRect.z - clip_off.x) * clip_scale.x;
            clip_rect.w = (pcmd->ClipRect.w - clip_off.y) * clip_scale.y;

            if (clip_rect.x < window_width &&
                clip_rect.y < window_height &&
                clip_rect.z >= 0.0f &&
                clip_rect.w >= 0.0f)
            {
                glScissor(
                    (int)clip_rect.x,
                    (int)(window_height - clip_rect.w),
                    (int)(clip_rect.z - clip_rect.x),
                    (int)(clip_rect.w - clip_rect.y));

                glBindTexture(
                    GL_TEXTURE_2D,
                    (GLuint)(intptr_t)pcmd->TextureId);

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

                glDrawElements(
                    GL_TRIANGLES,
                    (GLsizei)pcmd->ElemCount,
                    sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT,
                    (void*)(intptr_t)(pcmd->IdxOffset * sizeof(ImDrawIdx)));
            }
        }
    }

    glDisable(GL_BLEND);
    glDisable(GL_SCISSOR_TEST);
}
