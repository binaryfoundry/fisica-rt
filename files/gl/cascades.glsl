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
    uniform sampler2D distance_tex;
    uniform sampler2D prev_atlas;
    uniform float cascade_level;
    uniform float cascade_count;
    uniform float base_interval_f;
    uniform float srgb_falloff;
    layout(location = 0) out vec4 out_color;

    const float TAU = 6.28318530718;

    // Every interval stores (radiance, transmittance). Empty space has
    // alpha 1; opaque emitters and walls have alpha 0.
    vec4 trace_interval(vec2 start, vec2 end) {
        ivec2 resolution = textureSize(scene_tex, 0);
        vec2 delta = end - start;
        float ray_length = length(delta);
        vec2 direction = delta / max(ray_length, 0.00001);
        float traveled = 0.0;

        // Jump through empty space; walk pixel boundaries near surfaces.
        // A ray crosses at most width + height pixel boundaries.
        int max_steps = resolution.x + resolution.y + 1;
        for (int iteration = 0; iteration < max_steps && traveled < ray_length; ++iteration) {
            vec2 pos = start + direction * traveled;
            if (any(lessThan(pos, vec2(0.0))) ||
                any(greaterThanEqual(pos, vec2(resolution)))) break;

            ivec2 pixel = ivec2(floor(pos));
            vec4 surface = texelFetch(scene_tex, pixel, 0);
            if (surface.a > 0.0) {
                vec3 emission = max(surface.rgb, vec3(0.0));
                if (srgb_falloff > 0.5) emission = pow(emission, vec3(2.2));
                return vec4(emission, 0.0);
            }

            vec2 seed = texelFetch(distance_tex, pixel, 0).xy;
            if (seed.x < 0.0) break; // The scene is empty.

            // Seeds are pixel centers. Include their pixel extents in the
            // distance bound rather than jumping straight to the centers.
            float safe_distance = max(length(seed - pos) - 1.414214, 0.0);
            vec2 boundary = floor(pos) + step(vec2(0.0), direction);
            vec2 to_boundary = (boundary - pos) /
                mix(vec2(0.000001), direction, greaterThan(abs(direction), vec2(0.000001)));
            if (abs(direction.x) < 0.000001) to_boundary.x = 1e20;
            if (abs(direction.y) < 0.000001) to_boundary.y = 1e20;
            float pixel_step = max(min(to_boundary.x, to_boundary.y), 0.0) + 0.001;
            traveled += max(safe_distance, pixel_step);
        }
        return vec4(0.0, 0.0, 0.0, 1.0);
    }

    vec4 merge_probe(ivec2 probe, ivec2 tile, ivec2 probe_count,
        vec2 ray_start, vec2 direction, float interval_end) {
        vec2 center = (vec2(probe) + 0.5) *
            vec2(textureSize(scene_tex, 0)) / vec2(probe_count);
        vec4 near_interval = trace_interval(ray_start, center + direction * interval_end);
        vec4 far_interval = texelFetch(prev_atlas, tile + probe, 0);
        return vec4(near_interval.rgb + near_interval.a * far_interval.rgb,
            near_interval.a * far_interval.a);
    }

    vec4 upper_radiance(int direction_index, vec2 probe_position, int tiles,
        vec2 ray_start, vec2 direction, float interval_end) {
        ivec2 size = textureSize(prev_atlas, 0) / tiles;
        ivec2 tile = ivec2(direction_index % tiles, direction_index / tiles) * size;
        vec2 position = probe_position * 0.5 - 0.25;
        ivec2 base = ivec2(floor(position));
        vec2 weight = fract(position);

        // Clamp probes inside this direction's tile. Clamping packed atlas
        // coordinates instead would sample a different ray at the edges.
        ivec2 p00 = clamp(base, ivec2(0), size - 1);
        ivec2 p10 = clamp(base + ivec2(1, 0), ivec2(0), size - 1);
        ivec2 p01 = clamp(base + ivec2(0, 1), ivec2(0), size - 1);
        ivec2 p11 = clamp(base + ivec2(1, 1), ivec2(0), size - 1);
        // Merge each reprojected interval BEFORE interpolation. Interpolating
        // far radiance first lets probes on the other side of a wall leak light.
        vec4 c00 = merge_probe(p00, tile, size, ray_start, direction, interval_end);
        vec4 c10 = merge_probe(p10, tile, size, ray_start, direction, interval_end);
        vec4 c01 = merge_probe(p01, tile, size, ray_start, direction, interval_end);
        vec4 c11 = merge_probe(p11, tile, size, ray_start, direction, interval_end);
        return mix(mix(c00, c10, weight.x), mix(c01, c11, weight.x), weight.y);
    }

    void main() {
        // Direction-first packing with four rays averaged per texel, as in
        // radiance-cascades.com. Level zero holds one fluence sample per probe.
        int tiles = int(exp2(cascade_level));
        ivec2 size = textureSize(scene_tex, 0);
        // Every cascade has the same atlas dimensions.
        ivec2 probe_count = textureSize(prev_atlas, 0) / tiles;
        ivec2 coord = ivec2(gl_FragCoord.xy);
        ivec2 probe = coord % probe_count;
        ivec2 ray_tile = coord / probe_count;
        int first_ray = (ray_tile.x + tiles * ray_tile.y) * 4;
        float ray_count = float(tiles * tiles * 4);
        vec2 center = (vec2(probe) + 0.5) * vec2(size) / vec2(probe_count);

        float scale = pow(4.0, cascade_level);
        float interval_start = base_interval_f * (scale - 1.0) / 3.0;
        float interval_end = interval_start + base_interval_f * scale;
        // Cover the angular footprint of the ray's sector. Without overlap,
        // a child ray can start beyond a wall that its parent ray missed.
        interval_end /= cos(0.5 * TAU / ray_count);
        bool last_cascade = cascade_level >= cascade_count - 1.0;
        if (last_cascade) interval_end = max(interval_end, length(vec2(size)));

        vec4 radiance = vec4(0.0);
        for (int ray = 0; ray < 4; ++ray) {
            int index = first_ray + ray;
            float angle = TAU * (float(index) + 0.5) / ray_count;
            vec2 direction = vec2(cos(angle), sin(angle));
            vec2 ray_start = center + direction * interval_start;
            vec4 merged = vec4(0.0, 0.0, 0.0, 1.0);
            if (all(greaterThanEqual(ray_start, vec2(0.0))) &&
                all(lessThan(ray_start, vec2(size)))) {
                merged = last_cascade
                    ? trace_interval(ray_start, center + direction * interval_end)
                    : upper_radiance(index, vec2(probe), tiles * 2,
                        ray_start, direction, interval_end);
            }
            radiance += merged * 0.25;
        }
        out_color = radiance;
    }
#endif
