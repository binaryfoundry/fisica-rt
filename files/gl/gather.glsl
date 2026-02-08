#version 300 es

#if defined(COMPILING_VS)

    #ifdef GL_ES
    precision highp float;
    precision highp int;
    #endif

    layout(location = 0) in vec3 position;
    layout(location = 1) in vec2 texcoord;
    out vec2 v_texcoord;
    void main() {
        v_texcoord = texcoord;
        vec2 pos = (position.xy - vec2(0.5)) * 2.0;
        gl_Position = vec4(pos, -1.0, 1.0);
    }

#elif defined(COMPILING_FS)

    #ifdef GL_ES
    precision highp float;
    precision highp int;
    #endif

    in vec2 v_texcoord;

    uniform sampler2D atlas;
    uniform float atlas_size;
    uniform float input_res_x;
    uniform float input_res_y;

    layout(location = 0) out vec4 out_color;

    #define PI 3.14159265359
    void main() {
        vec2 frag_coord = v_texcoord * vec2(input_res_x, input_res_y);

        float dirs_per_probe = 2.0;
        float probes_per_axis = atlas_size / dirs_per_probe;
        vec2 probe_spacing = vec2(input_res_x, input_res_y) / probes_per_axis;

        vec2 probe_idx_f = frag_coord / probe_spacing - vec2(0.5);
        ivec2 base_idx = ivec2(floor(probe_idx_f));
        vec2 frac = fract(probe_idx_f);

        vec3 fluence = vec3(0.0);
        float total_weight = 0.0;

        for (int dy = 0; dy < 2; dy++) {
            for (int dx = 0; dx < 2; dx++) {
                ivec2 probe_idx = base_idx + ivec2(dx, dy);
                if (probe_idx.x < 0 || probe_idx.x >= int(probes_per_axis) ||
                    probe_idx.y < 0 || probe_idx.y >= int(probes_per_axis))
                    continue;

                float w = (dx == 0 ? 1.0 - frac.x : frac.x) *
                         (dy == 0 ? 1.0 - frac.y : frac.y);
                total_weight += w;

                float dir_count = dirs_per_probe * dirs_per_probe;
                for (int d = 0; d < int(dir_count); d++) {
                    ivec2 texel = ivec2(probe_idx) * ivec2(int(dirs_per_probe)) +
                        ivec2(d % int(dirs_per_probe), d / int(dirs_per_probe));
                    texel = clamp(texel, ivec2(0), ivec2(int(atlas_size)) - ivec2(1));

                    vec4 val = texelFetch(atlas, texel, 0);
                    fluence += val.rgb * w;
                }
            }
        }

        if (total_weight > 0.0) {
            fluence /= (total_weight * dirs_per_probe * dirs_per_probe);
        }

        out_color = vec4(fluence, 1.0);
    }

#endif
