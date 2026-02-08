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

    uniform sampler2D input_sampler;
    uniform sampler2D prev_cascade;

    uniform float cascade_level;
    uniform float cascade_res_x;
    uniform float cascade_res_y;
    uniform float input_res_x;
    uniform float input_res_y;
    uniform float ray_count_f;
    uniform float base_interval_f;
    uniform float prev_cascade_res;
    uniform float bilinear_fix;
    uniform float ringing_fix;
    uniform float srgb_falloff;
    uniform float use_4x_branching;

    layout(location = 0) out vec4 out_color;

    #define PI 3.14159265359
    #define NUM_STEPS 24
    #define NUM_CASCADES 6

    const float GAMMA = 2.2;

    // sRGB to linear (for emission when painted in sRGB)
    vec3 toLinear(vec3 c) {
        return pow(max(c, vec3(0.0)), vec3(GAMMA));
    }

    // Sample scene from input texture
    // Returns vec4(emission.rgb, opacity)
    vec4 get_scene(vec2 uv) {
        vec4 val = texture(input_sampler, uv);
        if (srgb_falloff > 0.5) {
            val.rgb = toLinear(val.rgb);
        }
        return val;
    }

    // Spatial hash for per-pixel angular jitter (reduces banding)
    float hash21(vec2 p) {
        p = fract(p * vec2(123.34, 456.21));
        p += dot(p, p + 45.32);
        return fract(p.x * p.y);
    }

    // Sample prev_cascade with optional bilinear/ringing fixes.
    // - Bilinear fix: sample at 4 probe centers and interpolate (reduces occlusion artifacts)
    // - Ringing fix: sample at nearest probe center (reduces ringing at cascade boundaries)
    vec3 sample_prev_cascade(vec2 uv) {
        if (prev_cascade_res < 1.0) return vec3(0.0);

        vec2 texel_coord = uv * prev_cascade_res;
        vec2 texel_frac = fract(texel_coord);
        ivec2 texel_base = ivec2(floor(texel_coord));

        // Clamp to valid range for texelFetch
        ivec2 res = ivec2(int(prev_cascade_res));
        texel_base = clamp(texel_base, ivec2(0), res - ivec2(1));

        if (ringing_fix > 0.5) {
            // Ringing fix: nearest-neighbor at probe center
            ivec2 nearest = clamp(ivec2(texel_coord + 0.5), ivec2(0), res - ivec2(1));
            return texelFetch(prev_cascade, nearest, 0).rgb;
        }

        if (bilinear_fix > 0.5) {
            // Bilinear fix: sample at 4 probe centers, manual interpolation
            // Ensures rays align with coarser cascade's probe grid for continuity
            ivec2 p00 = clamp(texel_base, ivec2(0), res - ivec2(1));
            ivec2 p10 = clamp(texel_base + ivec2(1, 0), ivec2(0), res - ivec2(1));
            ivec2 p01 = clamp(texel_base + ivec2(0, 1), ivec2(0), res - ivec2(1));
            ivec2 p11 = clamp(texel_base + ivec2(1, 1), ivec2(0), res - ivec2(1));

            vec3 c00 = texelFetch(prev_cascade, p00, 0).rgb;
            vec3 c10 = texelFetch(prev_cascade, p10, 0).rgb;
            vec3 c01 = texelFetch(prev_cascade, p01, 0).rgb;
            vec3 c11 = texelFetch(prev_cascade, p11, 0).rgb;

            vec3 c0 = mix(c00, c10, texel_frac.x);
            vec3 c1 = mix(c01, c11, texel_frac.x);
            return mix(c0, c1, texel_frac.y);
        }

        // Vanilla: standard texture lookup (implicit bilinear)
        return texture(prev_cascade, uv).rgb;
    }

    void main() {
        vec2 uv = v_texcoord;

        // Compute ray interval for this cascade level (in UV space)
        // Reference uses 4x branching (Yaazarai, m4xc): interval length scale = 4^level
        // C0: [0, base], C1: [base, 5*base], C2: [5*base, 21*base], ...
        float base_dist = base_interval_f / input_res_x;
        float interval_start;
        float interval_end;

        if (use_4x_branching > 0.5) {
            // 4x branching (Yaazarai, m4xc): interval length = 4^level
            // C0: [0, base], C1: [base, 5*base], C2: [5*base, 21*base], ...
            float scale_cur = pow(4.0, cascade_level);
            interval_start = base_dist * (scale_cur - 1.0) / 3.0;
            interval_end = interval_start + base_dist * scale_cur;
        } else {
            // 2x branching
            interval_start = base_dist * (pow(2.0, cascade_level) - 1.0);
            interval_end = base_dist * (pow(2.0, cascade_level + 1.0) - 1.0);
        }

        // Add interval overlap to prevent light leaking at cascade boundaries.
        if (cascade_level < float(NUM_CASCADES - 1)) {
            float next_probe_spacing = pow(2.0, cascade_level + 1.0) / input_res_x;
            interval_end += 1.414 * next_probe_spacing;
        }

        float step_size = (interval_end - interval_start) / float(NUM_STEPS);

        int RAY_COUNT = int(ray_count_f);

        // Per-pixel angular jitter to break up banding artifacts.
        float jitter = hash21(uv * vec2(cascade_res_x, cascade_res_y));
        float angle_offset = jitter * (2.0 * PI / float(RAY_COUNT));

        vec3 total_radiance = vec3(0.0);

        for (int r = 0; r < RAY_COUNT; r++) {
            // Evenly distributed ray directions with half-pixel offset + jitter
            float angle = (float(r) + 0.5) / float(RAY_COUNT) * 2.0 * PI + angle_offset;
            vec2 dir = vec2(cos(angle), sin(angle));

            vec3 ray_radiance = vec3(0.0);
            float transmittance = 1.0;

            // March along the ray within this cascade's interval
            for (int s = 0; s < NUM_STEPS; s++) {
                float t = interval_start + step_size * (float(s) + 0.5);
                vec2 sample_pos = uv + dir * t;

                // Boundary check
                if (sample_pos.x < 0.0 || sample_pos.x > 1.0 ||
                    sample_pos.y < 0.0 || sample_pos.y > 1.0) break;

                vec4 scene_val = get_scene(sample_pos);

                // Accumulate emission weighted by current transmittance
                ray_radiance += transmittance * scene_val.rgb;

                // Update transmittance (Beer's law style occlusion)
                transmittance *= (1.0 - scene_val.a);

                // Early termination if fully occluded
                if (transmittance < 0.01) break;
            }

            // Hierarchical cascade merging: add far-field radiance
            // from the previous (coarser) cascade at the ray endpoint
            if (cascade_level < float(NUM_CASCADES - 1)) {
                vec2 ray_end = uv + dir * interval_end;
                ray_end = clamp(ray_end, vec2(0.001), vec2(0.999));
                vec3 far_field = sample_prev_cascade(ray_end);
                ray_radiance += transmittance * far_field;
            }

            total_radiance += ray_radiance;
        }

        // Average over all ray directions
        total_radiance /= float(RAY_COUNT);

        out_color = vec4(total_radiance, 1.0);
    }

#endif
