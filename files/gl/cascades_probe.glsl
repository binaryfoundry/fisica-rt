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

    uniform sampler2D scene_tex;
    uniform sampler2D prev_atlas;

    uniform float cascade_level;
    uniform float atlas_size;
    uniform float prev_atlas_size;
    uniform float input_res_x;
    uniform float input_res_y;
    uniform float base_interval_f;
    uniform float srgb_falloff;

    layout(location = 0) out vec4 out_color;

    #define PI 3.14159265359
    #define NUM_STEPS 48
    #define NUM_CASCADES 6
    #define DIR_SCALING 2.0

    const float GAMMA = 2.2;

    vec3 toLinear(vec3 c) {
        return pow(max(c, vec3(0.0)), vec3(GAMMA));
    }

    vec4 get_scene(vec2 uv) {
        vec4 val = texture(scene_tex, uv);
        if (srgb_falloff > 0.5) val.rgb = toLinear(val.rgb);
        return val;
    }

    vec4 raymarch(vec2 ray_start, vec2 ray_end, float max_len) {
        vec2 inv_size = vec2(1.0) / vec2(input_res_x, input_res_y);
        vec2 delta = ray_end - ray_start;
        float len = min(length(delta), max_len);
        vec2 ray_dir = len > 1e-6 ? normalize(delta) : vec2(0.0);
        float step_size = max_len / float(NUM_STEPS);

        vec3 radiance = vec3(0.0);
        float transmittance = 1.0;

        for (float t = 0.0; t < len && transmittance > 0.01; t += step_size) {
            vec2 pos = ray_start + ray_dir * t;
            vec2 uv = pos * inv_size;
            if (uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0) break;

            vec4 sample_val = get_scene(uv);
            radiance += transmittance * sample_val.rgb;
            transmittance *= (1.0 - sample_val.a);
        }
        return vec4(radiance, transmittance);
    }

    vec4 merge_intervals(vec4 near, vec4 far) {
        return vec4(near.rgb + near.a * far.rgb, near.a * far.a);
    }

    void main() {
        vec2 frag_coord = v_texcoord * atlas_size;

        float dirs_per_probe = pow(2.0, cascade_level + 1.0);
        float probes_per_axis = atlas_size / dirs_per_probe;

        ivec2 texel = ivec2(floor(frag_coord));
        ivec2 cell_size = ivec2(int(dirs_per_probe));
        ivec2 probe_idx = texel / cell_size;
        ivec2 dir_idx2 = texel % cell_size;
        int dir_idx = dir_idx2.x + dir_idx2.y * cell_size.x;

        if (probe_idx.x >= int(probes_per_axis) || probe_idx.y >= int(probes_per_axis)) {
            out_color = vec4(0.0, 0.0, 0.0, 1.0);
            return;
        }

        vec2 probe_spacing = vec2(input_res_x, input_res_y) / probes_per_axis;
        vec2 probe_pos = (vec2(probe_idx) + vec2(0.5)) * probe_spacing;
        vec2 probe_uv = probe_pos / vec2(input_res_x, input_res_y);

        float dir_count = dirs_per_probe * dirs_per_probe;
        float angle = 2.0 * PI * (float(dir_idx) + 0.5) / dir_count;
        vec2 dir = vec2(cos(angle), sin(angle));

        float base_dist = base_interval_f / input_res_x;
        float scale_cur = pow(4.0, cascade_level);
        float interval_start = base_dist * (scale_cur - 1.0) / 3.0;
        float interval_end = interval_start + base_dist * scale_cur;

        vec2 ray_start_uv = probe_uv + dir * interval_start;
        vec2 ray_start = ray_start_uv * vec2(input_res_x, input_res_y);

        float interval_len_uv = interval_end - interval_start;
        float max_ray_len = interval_len_uv * length(vec2(input_res_x, input_res_y));

        if (cascade_level >= float(NUM_CASCADES - 1)) {
            vec2 ray_end_uv = probe_uv + dir * interval_end;
            vec2 ray_end = ray_end_uv * vec2(input_res_x, input_res_y);
            vec4 hit = raymarch(ray_start, ray_end, max_ray_len);
            out_color = vec4(hit.rgb, 1.0 - hit.a);
            return;
        }

        float prev_dirs = dirs_per_probe * 2.0;
        float prev_probes = probes_per_axis / 2.0;
        vec2 prev_probe_spacing = vec2(input_res_x, input_res_y) / prev_probes;

        vec2 prev_probe_idx_f = vec2(probe_idx) / 2.0;
        ivec2 base_idx = ivec2(floor(prev_probe_idx_f));
        vec2 frac = fract(prev_probe_idx_f);

        vec4 merged = vec4(0.0);
        float weight_sum = 0.0;

        for (int dy = 0; dy < 2; dy++) {
            for (int dx = 0; dx < 2; dx++) {
                ivec2 prev_probe_idx = base_idx + ivec2(dx, dy);
                if (prev_probe_idx.x < 0 || prev_probe_idx.x >= int(prev_probes) ||
                    prev_probe_idx.y < 0 || prev_probe_idx.y >= int(prev_probes))
                    continue;

                float w = (dx == 0 ? 1.0 - frac.x : frac.x) *
                         (dy == 0 ? 1.0 - frac.y : frac.y);
                weight_sum += w;

                vec2 prev_probe_uv = (vec2(prev_probe_idx) + vec2(0.5)) / prev_probes;
                vec2 ray_end_uv = prev_probe_uv + dir * interval_end;
                vec2 ray_end = ray_end_uv * vec2(input_res_x, input_res_y);

                vec4 hit = raymarch(ray_start, ray_end, max_ray_len);

                vec3 far_rgb = vec3(0.0);
                float far_vis = 0.0;
                for (int d = 0; d < 2; d++) {
                    int prev_dir_idx = dir_idx * 2 + d;
                    ivec2 prev_cell = ivec2(int(prev_dirs));
                    ivec2 prev_texel = ivec2(prev_probe_idx) * prev_cell +
                        ivec2(prev_dir_idx % prev_cell.x, prev_dir_idx / prev_cell.x);
                    prev_texel = clamp(prev_texel, ivec2(0), ivec2(int(prev_atlas_size)) - ivec2(1));

                    vec4 prev_val = texelFetch(prev_atlas, prev_texel, 0);
                    far_rgb += prev_val.rgb;
                    far_vis += (1.0 - prev_val.a);
                }
                vec4 far_merged = vec4(far_rgb / 2.0, far_vis / 2.0);

                merged += merge_intervals(hit, far_merged) * w;
            }
        }

        if (weight_sum > 0.0) {
            merged /= weight_sum;
        } else {
            vec2 ray_end_uv = probe_uv + dir * interval_end;
            vec2 ray_end = ray_end_uv * vec2(input_res_x, input_res_y);
            vec4 hit_fallback = raymarch(ray_start, ray_end, max_ray_len);
            merged = merge_intervals(hit_fallback, vec4(0.0, 0.0, 0.0, 1.0));
        }

        out_color = vec4(merged.rgb, 1.0 - merged.a);
    }

#endif
