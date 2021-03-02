#version 300 es

#define CIE

#if defined(COMPILING_VS)

    #ifdef GL_ES
    precision lowp float;
    precision lowp int;
    #endif

    #define PI    3.141592653589793
    #define TWOPI 6.283185307179587

    in vec4 position;
    in vec2 texcoord;
    out vec2 v_texcoord;
    void main() {
        v_texcoord = texcoord * vec2(TWOPI, PI);
        gl_Position = vec4((position.xy - vec2(0.5))  * 2.0, -1.0, 1.0);
    }

#elif defined(COMPILING_FS)

    #ifdef GL_ES
    precision lowp float;
    #endif

    float surface_height = 0.99;
    float range = 0.01;
    float intensity = 1.8;
    int step_count = 16;

    vec4 Kr = vec4(
        0.18867780436772762,
        0.4978442963618773,
        0.6616065586417131,
        1.0);

    vec4 light_direction = vec4(1.0, 1.0, 1.0, 1.0);
    float rayleigh_brightness = 64.0 / 10.0;
    float mie_brightness = 200.0 / 1000.0;
    float spot_brightness = 10.0;
    float scatter_strength = 28.0 / 1000.0;
    float rayleigh_strength = 239.0 / 1000.0;
    float mie_strength = 264.0 / 10000.0;
    float rayleigh_collection_power = 81.0 / 100.0;
    float mie_collection_power = 39.0 / 100.0;
    float mie_distribution = 63.0 / 100.0;

    in vec2 v_texcoord;
    layout(location = 0) out vec4 out_color;

    vec3 equirect_to_spherical(vec2 t) {
        float theta = t.y;
        float phi = t.x;

        vec3 u = vec3(0.0);
        u.x = sin(phi) * sin(theta) * -1.0;
        u.y = cos(theta) * -1.0;
        u.z = cos(phi) * sin(theta) * -1.0;
        return u;
    }

    float phase(float alpha, float g) {
        float a = 3.0 * (1.0 - g * g);
        float b = 2.0 * (2.0 + g * g);
        float c = 1.0 + alpha * alpha;
        float d = pow(1.0 + g  *g - 2.0  * g * alpha, 1.5);
        return (a / b) * (c / d);
    }

    float horizon_extinction(vec3 position, vec3 dir, float radius) {
        float u = dot(dir, -position);
        if(u<0.0) {
            return 1.0;
        }
        vec3 near = position + u * dir;
        if(length(near) < radius) {
            return 0.0;
        }
        else {
            vec3 v2 = normalize(near) * radius - position;
            float diff = acos(dot(normalize(v2), dir));
            return smoothstep(0.0, 1.0, pow(diff * 2.0, 3.0));
        }
    }

    float atmospheric_depth(vec3 position, vec3 dir) {
        float a = dot(dir, dir);
        float b = 2.0 * dot(dir, position);
        float c = dot(position, position)-1.0;
        float det = b * b - 4.0 * a * c;
        float detSqrt = sqrt(det);
        float q = (-b - detSqrt) / 2.0;
        float t1 = c / q;
        return t1;
    }

    vec3 absorb(float dist, vec3 color, float factor) {
        return color - color * pow(Kr.xyz, vec3(factor / dist));
    }

    vec3 cie(vec3 v) {
        vec3 color = vec3(1.0) * 0.5;
        vec3 dir = vec3(0.7071, 0.7071, 0.0);
        vec3 intensity = vec3(3.0);

        v = normalize(v);
        float g = dot(v, dir);
        float s = dir.y;
        float a = (0.91 + 10.0 * exp(-3.0 * acos(g)) + 0.45 * g * g) *
            (1.0 - exp(-0.32 / max(v.y, 0.0)));
        float b = (0.91 + 10.0 * exp(-3.0 * acos(s)) + 0.45 * s * s) *
            (1.0 - exp(-0.32));
        vec3 c = color * a / b;

        float alpha = dot(v, dir);
        float spot = smoothstep(0.0, 25.0, phase(alpha, 0.995));

        return mix(c, intensity, spot);
    }

    void main() {

#if defined(CIE)

        vec3 n = equirect_to_spherical(v_texcoord);
        vec3 env = cie(n) * horizon_extinction(
            vec3(0.0, surface_height, 0.0),
            n,
            surface_height - 0.25);

        out_color = vec4(env, 1.0);

#else

        vec3 direction = normalize(-light_direction.xyz);

        vec3 eyedir = equirect_to_spherical(v_texcoord);

        float alpha = dot(eyedir, -direction);

        float rayleigh_factor = phase(alpha, -0.01) *
            rayleigh_brightness;

        float mie_factor = phase(alpha, mie_distribution) *
            mie_brightness;

        float spot = smoothstep(0.0, 25.0, phase(alpha, 0.995)) *
            spot_brightness;

        vec3 eye_position = vec3(0.0, surface_height, 0.0);

        float eye_depth = atmospheric_depth(eye_position, eyedir);

        float step_length = eye_depth / float(step_count);

        float eye_extinction = horizon_extinction(
            eye_position,
            eyedir,
            surface_height - 0.25);

        vec3 rayleigh_collected = vec3(0.0);
        vec3 mie_collected = vec3(0.0);

        for(int i = 0; i < step_count; i++) {
            float sample_distance = step_length * float(i);

            vec3 position = eye_position + eyedir * sample_distance;

            float extinction = horizon_extinction(
                position,
                -direction,
                surface_height);

            float sample_depth = atmospheric_depth(
                position,
                -direction);

            vec3 influx = absorb(
                sample_depth,
                vec3(intensity),
                scatter_strength) * extinction;

            rayleigh_collected += absorb(
                sample_distance,
                Kr.xyz * influx,
                rayleigh_strength);

            mie_collected += absorb(
                sample_distance,
                influx,
                mie_strength);
        }

        float rayleigh_power = pow(
            eye_depth, rayleigh_collection_power);

        float mie_power = pow(
            eye_depth, mie_collection_power);

        rayleigh_collected =
            rayleigh_collected *
            rayleigh_power *
            eye_extinction;

        mie_collected = mie_collected * mie_power * eye_extinction;

        rayleigh_collected /= float(step_count);
        mie_collected /= float(step_count);

        vec3 final_color = vec3(
            spot * mie_collected +
            mie_factor * mie_collected +
            rayleigh_factor * rayleigh_collected);

        out_color = vec4(final_color, 1.0);
#endif
    }

#endif
