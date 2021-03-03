#version 300 es

#if defined(COMPILING_VS)

    #ifdef GL_ES
    precision lowp float;
    precision lowp int;
    #endif

    in vec4 pos;
    in vec2 texcoord;
    out vec2 v_texcoord;
    void main() {
        v_texcoord = texcoord;
        gl_Position = vec4((pos.xy - vec2(0.5))  * 2.0, -1.0, 1.0);
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

    int rand_sample = 0;
    vec4 rand_value;

    void rand_update() {
        vec2 coords = gl_FragCoord.xy /
            vec2(textureSize(rand_sampler, 0));
        rand_value = texture(
            rand_sampler,
            vec3(coords, rand_sample));
        rand_sample++;
    }

    vec3 rand_cos_hemisphere(const vec3 n) {
        vec2 r = rand_value.xy;
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
        vec2 r = rand_value.zw * 6.2831;
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

        coords += rand_value.xy / viewport.zw;

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
        float roughness;
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

    float plane_size = 25.0;

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
                return
                    h.position.x < plane_size &&
                    h.position.x > -plane_size &&
                    h.position.z < plane_size &&
                    h.position.z > -plane_size;
            }
        }
        return false;
    }

    float EnvBRDFApprox(float NoV, float roughness) {
        vec4 c0 = vec4(-1, -0.0275, -0.572, 0.022);
        vec4 c1 = vec4(1, 0.0425, 1.0, -0.04);
        vec4 r = roughness * c0 + c1;
        float a004 = min(r.x * r.x, exp2(-9.28 * NoV)) * r.x + r.y;
        vec2 AB = vec2(-1.04, 1.04) * a004 + r.zw;
        return AB.y;
    }

    void trace_world(inout Ray r, inout vec4 acc, inout int is_hit, bool end) {
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

        m_temp = Material(
            vec3(1.0), 0.0, 0.0, 0.0, 0.0, 1.0);

        if (Plane_hit(r, h_temp)) {
            if (h_temp.t < h.t) {
                h = h_temp;
                m = m_temp;
                found = 1.0;
            }
        }

        float NoV = dot(
            normalize(h.normal),
            normalize(-r.direction));

        vec3 f0 = m.albedo;

        float fresnel_prob = rand_value.w;
        float fresnel_val = EnvBRDFApprox(NoV, m.roughness);
        float fresnel = fresnel_prob > fresnel_val ? 0.0 : 1.0;

        float diffuse_prob = end ? 1.0 : rand_value.z;
        vec3 reflect_vec = reflect(r.direction, h.normal);
        vec3 diffuse_vec = rand_cos_hemisphere(h.normal);

        vec3 specular_vec = mix(
            reflect_vec,
            rand_hemisphere_direction(h.normal),
            m.roughness);

        vec3 reflect_norm = mix(
            diffuse_vec,
            specular_vec,
            diffuse_prob < m.metalness ? 1.0 : 0.0);

        reflect_norm = mix(reflect_norm, reflect_vec, fresnel);
        f0 = mix(f0, vec3(1.0), fresnel);

        //if (found) {
            is_hit = int(found);
            r.origin = mix(r.origin, h.position, found);
            r.direction = mix(r.direction, reflect_norm, found);
            acc.xyz *= mix(vec3(1.0), f0, found);
            acc.w += mix(0.0, h.t, found);
        //}
    }

    vec4 trace(Ray r) {
        int is_hit = 0;
        vec4 acc = vec4(1.0, 1.0, 1.0, 0.0);
        for (int i = 0; i < BOUNCES; i++) {
            rand_update();
            is_hit = 0;
            trace_world(r, acc, is_hit, i == BOUNCES - 1);
        }
        acc.xyz *= environment_emissive(r.direction);
        return acc;
    }

    void main() {
        Ray r = Ray_screen(v_texcoord);

        vec4 acc;
        for (int s = 0; s < SAMPLES; s++) {
            acc += trace(r);
        }
        acc /= float(SAMPLES);

        out_color = vec4(acc.xyz * exposure.x , 1.0);
    }

#endif
