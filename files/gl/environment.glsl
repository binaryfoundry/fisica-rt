#version 300 es

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

    vec3 env_color = vec3(0.18867, 0.49784, 0.66160) * 0.5;
    vec3 env_direction = vec3(0.7071, 0.7071, 0.0);
    vec3 env_spot_intensity = vec3(3.0);

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

    float env_phase(float alpha, float g) {
        float a = 3.0 * (1.0 - g * g);
        float b = 2.0 * (2.0 + g * g);
        float c = 1.0 + alpha * alpha;
        float d = pow(1.0 + g  *g - 2.0  * g * alpha, 1.5);
        return (a / b) * (c / d);
    }

    vec3 env_cie(vec3 v) {
        v = normalize(v);
        float g = dot(v, env_direction);
        float s = env_direction.y;
        float a = (0.91 + 10.0 * exp(-3.0 * acos(g)) + 0.45 * g * g) *
            (1.0 - exp(-0.32 / max(v.y, 0.0)));
        float b = (0.91 + 10.0 * exp(-3.0 * acos(s)) + 0.45 * s * s) *
            (1.0 - exp(-0.32));
        vec3 c = env_color * a / b;

        float alpha = dot(v, env_direction);
        float spot = smoothstep(0.0, 25.0, env_phase(alpha, 0.995));

        return mix(c, env_spot_intensity, spot);
    }

    float horizon_extinction(vec3 position, vec3 dir, float radius){
        float u = dot(dir, -position);
        if(u<0.0){
            return 1.0;
        }
        vec3 near = position + u*dir;
        if(length(near) < radius){
            return 0.0;
        }
        else{
            vec3 v2 = normalize(near)*radius - position;
            float diff = acos(dot(normalize(v2), dir));
            return smoothstep(0.0, 1.0, pow(diff*2.0, 3.0));
        }
    }

    void main() {
        vec3 n = equirect_to_spherical(v_texcoord);
        vec3 env = env_cie(n);

        env *= horizon_extinction(
            vec3(0.0, surface_height, 0.0),
            n,
            surface_height - 0.05);

        out_color = vec4(env, 1.0);
    }

#endif
