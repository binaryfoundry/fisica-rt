#version 300 es

#if defined(COMPILING_VS)
    precision highp float;
    layout(location = 0) in vec3 position;
    layout(location = 1) in vec2 texcoord;
    out vec2 v_texcoord;
    void main() {
        v_texcoord = texcoord;
        gl_Position = vec4(position.xy * 2.0 - 1.0, -1.0, 1.0);
    }
#elif defined(COMPILING_FS)
    precision highp float;
    precision highp int;
    precision highp sampler2D;

    in vec2 v_texcoord;
    uniform sampler2D atlas;
    uniform sampler2D scene_tex;
    uniform float srgb_falloff;
    layout(location = 0) out vec4 out_color;

    void main() {
        // Cascade zero already contains the average of all four directions.
        // Explicit interpolation works without float-linear extensions.
        ivec2 size = textureSize(atlas, 0);
        vec2 position = v_texcoord * vec2(size) - 0.5;
        ivec2 base = ivec2(floor(position));
        vec2 weight = fract(position);
        vec3 c00 = texelFetch(atlas, clamp(base, ivec2(0), size - 1), 0).rgb;
        vec3 c10 = texelFetch(atlas, clamp(base + ivec2(1, 0), ivec2(0), size - 1), 0).rgb;
        vec3 c01 = texelFetch(atlas, clamp(base + ivec2(0, 1), ivec2(0), size - 1), 0).rgb;
        vec3 c11 = texelFetch(atlas, clamp(base + ivec2(1, 1), ivec2(0), size - 1), 0).rgb;
        vec3 fluence = mix(mix(c00, c10, weight.x), mix(c01, c11, weight.x), weight.y);

        ivec2 scene_size = textureSize(scene_tex, 0);
        vec4 surface = texelFetch(scene_tex, clamp(ivec2(v_texcoord * vec2(scene_size)), ivec2(0), scene_size - 1), 0);
        if (surface.a > 0.0) {
            fluence = max(surface.rgb, vec3(0.0));
            if (srgb_falloff > 0.5) fluence = pow(fluence, vec3(2.2));
        }
        out_color = vec4(fluence, 1.0);
    }
#endif
