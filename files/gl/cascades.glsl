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

    layout(location = 0) out vec4 out_color;

    #define PI 3.14159265359
    #define RAY_COUNT 48
    #define NUM_STEPS 24
    #define NUM_CASCADES 6

    // Sample scene from input texture
    // Returns vec4(emission.rgb, opacity)
    vec4 get_scene(vec2 uv) {
        return texture(input_sampler, uv);
    }

    // Spatial hash for per-pixel angular jitter (reduces banding)
    float hash21(vec2 p) {
        p = fract(p * vec2(123.34, 456.21));
        p += dot(p, p + 45.32);
        return fract(p.x * p.y);
    }

    void main() {
        vec2 uv = v_texcoord;

        // Compute ray interval for this cascade level (in UV space)
        // Each cascade covers an exponentially growing distance range:
        //   C0: [0, base_dist]
        //   C1: [base_dist, 3*base_dist]
        //   C2: [3*base_dist, 7*base_dist]
        //   CN: [base_dist*(2^N - 1), base_dist*(2^(N+1) - 1)]
        float base_dist = base_interval_f / input_res_x;
        float interval_start = base_dist * (pow(2.0, cascade_level) - 1.0);
        float interval_end = base_dist * (pow(2.0, cascade_level + 1.0) - 1.0);

        // Add interval overlap to prevent light leaking at cascade boundaries.
        // Extends the ray march by the diagonal of the next cascade's probe
        // spacing, ensuring continuous coverage across the spatial transition
        // between cascades (per Yaazarai/GMShaders reference).
        if (cascade_level < float(NUM_CASCADES - 1)) {
            float next_probe_spacing = pow(2.0, cascade_level + 1.0) / input_res_x;
            interval_end += 1.414 * next_probe_spacing;
        }

        float step_size = (interval_end - interval_start) / float(NUM_STEPS);

        // Per-pixel angular jitter to break up banding artifacts.
        // Uses a spatial hash so the jitter is stable (no temporal noise).
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
                vec3 far_field = texture(prev_cascade, ray_end).rgb;
                ray_radiance += transmittance * far_field;
            }

            total_radiance += ray_radiance;
        }

        // Average over all ray directions
        total_radiance /= float(RAY_COUNT);

        out_color = vec4(total_radiance, 1.0);
    }

#endif
