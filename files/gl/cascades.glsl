#version 300 es

#if defined(COMPILING_VS)

    #ifdef GL_ES
    precision lowp float;
    precision lowp int;
    #endif

    layout(location = 0) in vec3 position;
    layout(location = 1) in vec2 texcoord;
    out vec2 v_texcoord;
    void main() {
        v_texcoord = texcoord;
        vec2 pos = (position.xy - vec2(0.5)) * 2.0;
        gl_Position = vec4(vec2(pos.x, -pos.y), -1.0, 1.0);
    }

#elif defined(COMPILING_FS)

    #ifdef GL_ES
    precision lowp float;
    precision lowp int;
    precision lowp sampler2DArray;
    #endif

    uniform sampler2D scene_sampler;

    layout(location = 0) out vec4 out_color;

    void main() {
        out_color = vec4(0.0, 1.0, 0.0, 1.0);
    }

#endif
