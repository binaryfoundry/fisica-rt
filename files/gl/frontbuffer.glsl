#version 300 es

#if defined(COMPILING_VS)
    precision highp float;
    uniform mat4 view;
    uniform mat4 projection;
    layout(location = 0) in vec3 position;
    layout(location = 1) in vec2 texcoord;
    out vec2 v_texcoord;
    void main() {
        v_texcoord = texcoord;
        gl_Position = projection * view * vec4(position, 1.0);
    }
#elif defined(COMPILING_FS)
    precision highp float;
    precision highp int;
    precision highp sampler2D;
    uniform float exposure;
    uniform float srgb_falloff;
    uniform sampler2D tex;
    in vec2 v_texcoord;
    layout(location = 0) out vec4 out_color;

    void main() {
        ivec2 size = textureSize(tex, 0);
        vec2 position = v_texcoord * vec2(size) - 0.5;
        ivec2 base = ivec2(floor(position));
        vec2 weight = fract(position);
        vec3 c00 = texelFetch(tex, clamp(base, ivec2(0), size - 1), 0).rgb;
        vec3 c10 = texelFetch(tex, clamp(base + ivec2(1, 0), ivec2(0), size - 1), 0).rgb;
        vec3 c01 = texelFetch(tex, clamp(base + ivec2(0, 1), ivec2(0), size - 1), 0).rgb;
        vec3 c11 = texelFetch(tex, clamp(base + ivec2(1, 1), ivec2(0), size - 1), 0).rgb;
        vec3 color = max(mix(mix(c00, c10, weight.x), mix(c01, c11, weight.x), weight.y) * exposure, vec3(0.0));
        if (srgb_falloff > 0.5) color = pow(color, vec3(1.0 / 2.2));
        out_color = vec4(clamp(color, 0.0, 1.0), 1.0);
    }
#endif
