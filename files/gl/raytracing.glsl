#version 300 es

#if defined(COMPILING_VS)

    #ifdef GL_ES
    precision lowp float;
    precision lowp int;
    #endif

    in vec4 position;
    in vec2 texcoord;
    out vec2 v_texcoord;
    void main() {
        v_texcoord = texcoord;
        gl_Position = vec4((position.xy - vec2(0.5))  * 2.0, -1.0, 1.0);
    }

#elif defined(COMPILING_FS)

    #ifdef GL_ES
    precision lowp float;
    precision lowp int;
    precision lowp sampler2DArray;
    #endif

    #define PI 3.1415926535897932384626433832795
    #define PHI 1.61803398875
    #define FLT_MAX 3.402823466e+38

    #define SAMPLES 4
    #define BOUNCES 4

    const float t_min = 0.003;
    const float t_max = 100000.0;

    in vec2 v_texcoord;
    layout(location = 0) out vec4 out_color;

    uniform sampler2DArray rand_sampler;
    uniform sampler2D scene_sampler;
    uniform sampler2D environment_sampler;

    vec2 env_spherical_to_equirect(vec3 n) {
        vec2 uv = vec2(atan(n.z, n.x), asin(n.y));
        uv *= vec2(0.1591, 0.3183);
        uv += 0.5;
        return uv;
    }

    vec3 environment_emissive(vec3 n) {
        return texture(
            environment_sampler,
            env_spherical_to_equirect(n)).xyz;
    }

    const vec2 rand_offsets[4] = vec2[](
        vec2(0.25, 0.25),
        vec2(0.25, 0.75),
        vec2(0.75, 0.25),
        vec2(0.75, 0.75));

    vec2 rand_offset = vec2(0.0);
    int rand_sample = 0;

    vec4 rand_fetch() {
        vec2 coords = gl_FragCoord.xy /
            vec2(textureSize(rand_sampler, 0));
        return texture(rand_sampler, vec3(coords + rand_offset, rand_sample));
    }

    vec2 rand2() {
        vec4 s = rand_fetch();
        return vec2(s.x, s.y);
    }

    vec3 rand_cos_hemisphere(const vec3 n) {
        vec2 r = rand2();
        vec3  uu = normalize(cross(n, vec3(0.0, 1.0, 1.0)));
        vec3  vv = cross(uu, n);
        float ra = sqrt(r.y);
        float rx = ra * cos(6.2831 * r.x);
        float ry = ra * sin(6.2831 * r.x);
        float rz = sqrt(1.0 - r.y);
        vec3  rr = vec3(rx * uu + ry * vv + rz * n);
        return normalize(rr);
    }

    vec3 rand_sphere_direction() {
        vec2 r = rand2() * 6.2831;
        return vec3(sin(r.x) * vec2(sin(r.y), cos(r.y)), cos(r.x));
    }

    vec3 rand_hemisphere_direction(const vec3 n) {
        vec3 dr = rand_sphere_direction();
        return dot(dr, n) * dr;
    }

    layout(std140) uniform camera{
        mat4 view;
        mat4 projection;
        vec4 viewport;
        vec4 position;
        vec4 exposure;
    };

    layout(std140) uniform scene{
        int num_geometry;
    };

    struct Ray {
        vec3 origin;
        vec3 direction;
    };

    vec3 Ray_at(Ray r, float t) {
        return r.origin + t * r.direction;
    }

    Ray Ray_screen(vec2 coords) {
        float zoom = 0.5;
        float aspect = viewport.w / viewport.z;
        float size = 1.0 / zoom;

        vec4 h = vec4(size * 2.0, 0.0, 0.0, 1.0);
        vec4 v = vec4(0.0, size * 2.0 * aspect, 0.0, 1.0);
        vec4 c = vec4(-size, -size * aspect, -1.5, 1.0);

        coords += rand2() / viewport.zw;

        h = h * view;
        v = v * view;
        c = c * view;

        vec4 direction = c + coords.x * h + coords.y * v;
        // TODO Fix OpenGL Y axis.
        direction.y = -direction.y;
        return Ray(position.xyz, normalize(direction.xyz));
    }

    struct Material {
        vec3 albedo;
        float smoothness;
        float metalness;
        float refraction;
        float refractive_index;
        float padding_0;
    };

    struct Hit {
        float t;
        vec3 position;
        vec3 normal;
    };

    struct Plane {
        vec3 position;
        float padding_0;
        vec3 normal;
        float padding_1;
    };

    struct Sphere {
        vec3 position;
        float radius;
    };

    bool Sphere_hit(Sphere s, Ray r, inout Hit h) {
        vec3 oc = r.origin - s.position;
        float a = dot(r.direction, r.direction);
        float b = dot(oc, r.direction);
        float c = dot(oc, oc) - s.radius * s.radius;
        float d = b * b - a * c;
        if (d > 0.0) {
            float t = (-b - sqrt(b * b - a * c)) / a;
            if (t < t_max && t > t_min) {
                h.t = t;
                h.position = Ray_at(r, t);
                h.normal = (h.position - s.position) / s.radius;
                return true;
            }
        }
        return false;
    }

    bool Plane_hit(Ray r, inout Hit h) {
        vec3 position;
        vec3 normal = vec3(0.0, 1.0, 0.0);
        float d = dot(-normal, r.direction);
        if (d > t_min) {
            vec3 v = position - r.origin;
            float t = dot(v, -normal) / d;
            if (t >= t_min) {
                h.t = t;
                h.position = Ray_at(r, t);
                h.normal = normal;
                return true;
            }
        }
        return false;
    }

    void trace_world(inout Ray r, inout vec4 acc, inout int is_hit) {
        Hit h = Hit(FLT_MAX, r.origin, r.direction);
        Hit h_temp;
        Material m;
        Material m_temp;
        float found = 0.0;
        for (int i = 0; i < num_geometry; i++) {
            vec4 dat0 = texelFetch(scene_sampler, ivec2(0, i), 0);
            vec4 dat1 = texelFetch(scene_sampler, ivec2(1, i), 0);
            vec4 dat2 = texelFetch(scene_sampler, ivec2(2, i), 0);

            m_temp = Material(
                dat1.xyz, dat2.x, dat2.y, dat2.z, dat2.w, 1.0);
            Sphere s = Sphere(
                dat0.xyz, dat0.w);

            if (Sphere_hit(s, r, h_temp)) {
                if (h_temp.t < h.t) {
                    h = h_temp;
                    m = m_temp;
                    found = 1.0;
                }
            }
        }

        if (Plane_hit(r, h_temp)) {
            if (h_temp.t < h.t) {
                h = h_temp;
                m = m_temp;
                found = 1.0;
            }
        }

        vec3 rv = reflect(r.direction, h.normal);
        vec3 dv = rand_cos_hemisphere(h.normal);

        //if (found) {
            is_hit = int(found);
            r.origin = mix(r.origin, h.position, found);
            //r.direction = mix(r.direction, rv, found);
            r.direction = mix(r.direction, dv, found);
            acc.xyz *= mix(vec3(1.0), m.albedo, found);
            acc.w += mix(0.0, h.t, found);
        //}
    }

    vec4 trace(Ray r) {
        int is_hit = 0;
        vec4 acc = vec4(1.0, 1.0, 1.0, 0.0);
        for (int i = 0; i < BOUNCES; i++) {
            rand_sample = i;
            is_hit = 0;
            trace_world(r, acc, is_hit);
        }
        acc.xyz *= environment_emissive(r.direction);
        acc.xyz *= float(1 - is_hit);
        return acc;
    }

    void main() {
        Ray r = Ray_screen(v_texcoord);

        vec4 acc;
        for (int s = 0; s < SAMPLES; s++) {
            rand_offset = rand_offsets[s];
            acc += trace(r);
        }
        acc /= float(SAMPLES);

        vec3 env = environment_emissive(r.direction);
        acc.xyz = mix(acc.xyz, env, min(acc.w / 150.0, 1.0));

        out_color = vec4(acc.xyz * exposure.x , 1.0);
    }

#endif
