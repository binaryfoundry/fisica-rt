#pragma once

#include "gl/GL.hpp"
#include "gl/Math.hpp"

class GUI
{
private:
    GLuint gl_shader_program = 0;

    GLuint g_AttribLocationTex;
    GLuint g_AttribLocationProjMtx;
    GLuint g_AttribLocationVtxPos;
    GLuint g_AttribLocationVtxUV;
    GLuint g_AttribLocationVtxColor;

    unsigned int g_VboHandle = 0, g_ElementsHandle = 0;

    GLuint g_FontTexture = 0;

public:
    GUI();

    void Init();
    void Deinit();
    void Draw(
        const uint32_t window_width,
        const uint32_t window_height);
};
