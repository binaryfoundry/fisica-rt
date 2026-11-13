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
    uniform sampler2D scene_tex;
    uniform sampler2D seeds_tex;
    uniform float jump_distance;
    layout(location = 0) out vec4 out_color;

    void main() {
        ivec2 pixel = ivec2(gl_FragCoord.xy);
        if (jump_distance < 0.5) {
            bool occupied = texelFetch(scene_tex, pixel, 0).a > 0.0;
            out_color = vec4(occupied ? vec2(pixel) + 0.5 : vec2(-1.0), 0.0, 1.0);
            return;
        }

        ivec2 size = textureSize(seeds_tex, 0);
        vec2 nearest = vec2(-1.0);
        float nearest_squared = 1e20;
        for (int y = -1; y <= 1; ++y) {
            for (int x = -1; x <= 1; ++x) {
                ivec2 sample_pixel = pixel + ivec2(x, y) * int(jump_distance);
                if (any(lessThan(sample_pixel, ivec2(0))) ||
                    any(greaterThanEqual(sample_pixel, size))) continue;
                vec2 seed = texelFetch(seeds_tex, sample_pixel, 0).xy;
                if (seed.x < 0.0) continue;
                vec2 delta = seed - (vec2(pixel) + 0.5);
                float squared = dot(delta, delta);
                if (squared < nearest_squared) {
                    nearest = seed;
                    nearest_squared = squared;
                }
            }
        }
        out_color = vec4(nearest, 0.0, 1.0);
    }
#endif
