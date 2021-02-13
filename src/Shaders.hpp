#include <string>

static const std::string frontbuffer_vertex_shader_string =
R"(#version 300 es
    #ifdef GL_ES
    precision mediump float;
    #endif
    uniform mat4 projection;
    uniform mat4 view;
    layout(location = 0) in vec3 position;
    layout(location = 1) in vec2 texcoord;
    out vec2 v_texcoord;
    void main()
    {
        v_texcoord = texcoord;
        gl_Position = projection * view * vec4(position, 1.0);
    })";

static const std::string frontbuffer_fragment_shader_string =
R"(#version 300 es
    #ifdef GL_ES
    precision mediump float;
    #endif
    in vec2 v_texcoord;
    uniform sampler2D tex;
    vec3 to_linear_approx(vec3 v) { return pow(v, vec3(2.2)); }
    vec3 to_gamma_approx(vec3 v) { return pow(v, vec3(1.0 / 2.2)); }
    layout(location = 0) out vec4 out_color;
    void main()
    {
        vec3 c = texture(tex, v_texcoord).xyz;
        out_color = vec4(to_gamma_approx(c), 1.0);
    })";


const std::string raytracing_vertex_shader_string =
    R"(#version 300 es
    #if defined(GLES)
    precision lowp float;
    precision lowp int;
    #endif

    in vec4 position;
    in vec2 texcoord;

    out vec2 v_texcoord;

    layout(std140) uniform transform{
        mat4 view;
        mat4 projection;
        mat4 inverse_projection;
        mat4 inverse_view_rotation;
        vec4 viewport;
        vec4 camera_position;
        vec4 exposure;
    };

    void main() {
        v_texcoord = texcoord;
        gl_Position = vec4(position.xy, -1.0, 1.0);
    })";

const std::string raytracing_fragment_shader_string =
    R"(#version 300 es
    #if defined(GLES)
    precision lowp float;
    precision lowp int;
    #endif

    #define BRUTE_FORCE true
    #define FLT_MAX 3.402823466e+38
    #define PI 3.1415926535897932384626433832795
    #define EPSILON 0.00001

    #define BOUNCES 3
    #define SAMPLES 4
    #define MAX_SPHERES 512
    #define MAX_NODES 512
    #define NUM_PLANES 1

    const float t_min = 0.003;
    const float t_max = 100000.0;

    in vec2 v_texcoord;

    layout(location = 0) out vec4 out_color;

    struct Ray {
        vec3 origin;
        vec3 direction;
    };

    struct Hit {
        float t;
        int exists;
        vec3 position;
        vec3 normal;
        vec3 incident;
        vec3 albedo;
        vec3 emissive;
        float metalness;
        float smoothness;
        float refraction;
        float ref_index;
        float cosine;
        float specular;
    };

    struct Sphere {
        vec4 geom;       // xyz = position, w = radius
        vec4 albedo;     // xyz = rgb
        vec2 material;   // x = smoothness, y = metalness
        vec2 refraction; // x = refractive, y = refract index
    };

    struct Plane {
        vec4 position;   // xyz = position
        vec4 normal;     // xyz = normal
        vec4 albedo;     // xyz = rgb
        vec2 material;   // x = smoothness, y = metalness
        vec2 refraction; // x = refractive, y = refract index
    };

    struct Node {
        vec4 min;
        vec4 max;
        int start_idx;
        int end_idx;
        int left;
        int right;
    };

    layout(std140) uniform transform{
        mat4 view;
        mat4 projection;
        mat4 inverse_projection;
        mat4 inverse_view_rotation;
        vec4 viewport;
        vec4 camera_position;
        vec4 exposure;
    };

    layout(std140) uniform buffer_sizes{
        int num_nodes;
        int num_spheres;
        int unused_2;
        int unused_3;
    };

    Plane[NUM_PLANES] planes = Plane[](
        Plane(
            vec4(0.0, 0.0, 0.0, 0.0),
            vec4(0.0, 1.0, 0.0, 0.0),
            vec4(1.0, 1.0, 1.0, 1.0),
            vec2(0.0, 0.0), vec2(0.0, 0.0))
        );

    uniform sampler2D noise_sampler;
    uniform sampler2D scene_sampler;
    uniform sampler2D bvh_sampler;
    uniform samplerCube environment_sampler;

    float rand_seed;

    void rand_init() {
        vec2 coords = gl_FragCoord.xy /
            vec2(textureSize(noise_sampler, 0));
        rand_seed = texture(noise_sampler, coords).x;
    }

    float rand() {
        rand_seed = mod(rand_seed * 1.1234567893490423, 13.0);
        return fract(sin(rand_seed += 0.1) * 43758.5453123);
    }

    vec2 rand2() {
        rand_seed = mod(rand_seed * 1.1234567893490423, 13.0);
        return fract(sin(vec2(rand_seed += 0.1, rand_seed += 0.1)) *
            vec2(43758.5453123, 22578.1459123));
    }

    vec3 normal_to_color(vec3 n) {
        return 0.5 * (n + vec3(1.0));
    }

    vec3 random_sphere_direction() {
        vec2 r = rand2() * 6.2831;
        return vec3(sin(r.x) * vec2(sin(r.y), cos(r.y)), cos(r.x));
    }

    vec3 random_hemi_direction(const vec3 n) {
        vec3 dr = random_sphere_direction();
        return dot(dr, n) * dr;
    }

    vec3 random_cos_hemi_direction(const vec3 n) {
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

    float env_brdf_approx(float specularity, float roughness, float NoV) {
        vec4 c0 = vec4(-1, -0.0275, -0.572, 0.022);
        vec4 c1 = vec4(1, 0.0425, 1.0, -0.04);
        vec4 r = roughness * c0 + c1;
        float a004 = min(r.x * r.x, exp2(-9.28 * NoV)) * r.x + r.y;
        vec2 AB = vec2(-1.04, 1.04) * a004 + r.zw;
        return specularity * AB.x + AB.y;
    }

    float Hit_NoV(Hit h) {
        return max(0.0, dot(
            normalize(h.normal),
            normalize(-h.incident)));
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
        return Ray(camera_position.xyz, direction.xyz);
    }

    vec3 Ray_at(Ray r, float t) {
        return r.origin + t * r.direction;
    }

    void Sphere_hit(Sphere s, Ray r, inout Hit h) {
        vec3 oc = r.origin - s.geom.xyz;
        float a = dot(r.direction, r.direction);
        float b = dot(oc, r.direction);
        float c = dot(oc, oc) - s.geom.w * s.geom.w;
        float d = b * b - a * c;
        h.exists = 0;
        if (d > 0.0) {
            float t = (-b - sqrt(b*b - a * c)) / a;
            if (t < t_max && t > t_min) {
                h.t = t;
                h.exists = 1;
                h.position = Ray_at(r, t);
                h.normal = (h.position - s.geom.xyz) / s.geom.w;
                h.incident = normalize(r.direction);
                h.albedo = s.albedo.xyz;
                h.smoothness = s.material.x;
                h.metalness = s.material.y;
                h.refraction = s.refraction.x;
                h.ref_index = s.refraction.y;
                h.cosine = Hit_NoV(h);
                return;
            }
        }
    }

    void Plane_hit(Plane p, Ray r, inout Hit h) {
        float d = dot(-p.normal.xyz, r.direction);
        h.exists = 0;
        if (d > t_min) {
            vec3 v = p.position.xyz - r.origin;
            float t = dot(v, -p.normal.xyz) / d;
            if (t >= t_min) {
                h.t = t;
                h.exists = 1;
                h.position = Ray_at(r, t);
                h.normal = p.normal.xyz;
                h.incident = normalize(r.direction);
                h.albedo = p.albedo.xyz;
                h.smoothness = p.material.x;
                h.metalness = p.material.y;
                h.refraction = p.refraction.x;
                h.ref_index = p.refraction.y;
                h.cosine = Hit_NoV(h);
            }
        }
    }

    bool Bounds_hit(vec4 mn, vec4 mx, Ray r) {
        vec3 dirfrac = 1.0 / r.direction;
        float t1 = (mn.x - r.origin.x) * dirfrac.x;
        float t2 = (mx.x - r.origin.x) * dirfrac.x;
        float t3 = (mn.y - r.origin.y) * dirfrac.y;
        float t4 = (mx.y - r.origin.y) * dirfrac.y;
        float t5 = (mn.z - r.origin.z) * dirfrac.z;
        float t6 = (mx.z - r.origin.z) * dirfrac.z;
        float tmin = max(max(min(t1, t2), min(t3, t4)), min(t5, t6));
        float tmax = min(min(max(t1, t2), max(t3, t4)), max(t5, t6));
        return !(tmax < 0.0 || tmin > tmax);
    }

    Hit Hit_closer(Hit new, Hit old) {
        if (new.exists == 1 && new.t > t_min && new.t < old.t) {
            return new;
        }
        return old;
    }

    vec3 sky_emissive(vec3 n) {
        n = normalize(n);
        n.y = -n.y;
        return texture(
            environment_sampler,
            normalize(n)).xyz;
    }

    int[MAX_NODES] stack;
    int stack_pointer = 1;
    int s_index = 0;

    void Stack_reset() {
        stack[0] = 0;
        stack_pointer = 1;
        s_index = 0;
    }

    void Stack_push(int value) {
        stack[stack_pointer++] = value;
    }

    void Stack_pop() {
        s_index = stack[--stack_pointer];
    }

    void Ray_world(Ray r, inout Hit closest_hit) {
        r.direction = normalize(r.direction);

        Hit current_hit;
        current_hit.exists = 0;

        closest_hit.t = FLT_MAX;
        closest_hit.exists = 0;

    #if defined(BRUTE_FORCE)
        for (int i = 0; i < num_spheres; i++) {
            vec4 geom = texelFetch(scene_sampler, ivec2(0, i), 0);
            vec4 albedo = texelFetch(scene_sampler, ivec2(1, i), 0);
            vec4 mat = texelFetch(scene_sampler, ivec2(2, i), 0);
            Sphere s = Sphere(geom, albedo, mat.xy, mat.zw);
            Sphere_hit(s, r, current_hit);
            closest_hit = Hit_closer(current_hit, closest_hit);
        }
    #else
        Stack_reset();
        while (stack_pointer > 0) {
            Stack_pop();
            int j = s_index;
            vec4 n_min = texelFetch(bvh_sampler, ivec2(0, j), 0);
            vec4 n_max = texelFetch(bvh_sampler, ivec2(1, j), 0);
            vec4 n_info = texelFetch(bvh_sampler, ivec2(2, j), 0);
            Node n = Node(
                n_min, n_max,
                int(n_info.x), int(n_info.y),
                int(n_info.z), int(n_info.w));
            if (n.start_idx != -1) {
                for (int i = n.start_idx; i < n.end_idx; i++) {
                    vec4 geom = texelFetch(scene_sampler, ivec2(0, i), 0);
                    vec4 albedo = texelFetch(scene_sampler, ivec2(1, i), 0);
                    vec4 mat = texelFetch(scene_sampler, ivec2(2, i), 0);
                    Sphere s = Sphere(geom, albedo, mat.xy, mat.zw);
                    Sphere_hit(s, r, current_hit);
                    closest_hit = Hit_closer(current_hit, closest_hit);
                }
            }
            else if (Bounds_hit(n.min, n.max, r)) {
                Stack_push(n.left);
                Stack_push(n.right);
            }
        }
    #endif

        for (int i = 0; i < NUM_PLANES; i++) {
            Plane_hit(planes[i], r, current_hit);
            closest_hit = Hit_closer(current_hit, closest_hit);
        }

        if (closest_hit.exists == 0) {
            closest_hit.emissive = sky_emissive(r.direction);
            closest_hit.incident = r.direction;
        }
    }

    Hit[BOUNCES] hits;

    void main() {
        rand_init();

        vec3 col = vec3(0.0, 0.0, 0.0);

        for (int s = 0; s < SAMPLES; s++) {
            Hit h;
            h.exists = 1;

            Ray r = Ray_screen(v_texcoord);

            for (int b = 0; b < BOUNCES; b++) {
                h.emissive = vec3(0.0, 0.0, 0.0);
                h.albedo = vec3(1.0, 1.0, 1.0);

                if (h.exists == 1) {
                    Ray_world(r, h);

                    float reflectance = env_brdf_approx(
                        1.0 - h.metalness,
                        1.0 - h.smoothness,
                        h.cosine);

                    h.specular = mix(h.smoothness * rand(), 1.0, h.metalness) < 0.5 ? 0.0 : 1.0;

                    vec3 diffuse_normal = random_cos_hemi_direction(
                        h.normal);

                    vec3 reflect_normal = normalize(
                        reflect(r.direction, h.normal));

                    vec3 refract_normal = normalize(
                        refract(r.direction, h.normal, h.ref_index));

                    refract_normal = reflectance > rand() ?
                        reflect_normal :
                        refract_normal;

                    vec3 specular_normal = h.refraction < 0.5 ?
                        reflect_normal :
                        refract_normal;

                    specular_normal = normalize(mix(
                        diffuse_normal,
                        specular_normal,
                        h.smoothness));

                    r.origin = h.position;
                    r.direction = mix(
                        diffuse_normal,
                        specular_normal,
                        h.specular);
                }

                hits[b] = h;
            }

            vec3 gather = vec3(0.0, 0.0, 0.0);

            // This hack adds env color to paths that never missed
            Hit last_hit = hits[BOUNCES - 1];
            if (last_hit.exists == 1) {
                gather += sky_emissive(last_hit.incident);
            }

            // Plays the role of stack unwinding.
            for (int b = BOUNCES - 1; b >= 0; b--) {
                Hit h = hits[b];
                if (h.exists == 1) {
                    vec3 diffuse_albedo = h.albedo / PI;
                    vec3 specular_albedo = mix(
                        vec3(0.04, 0.04, 0.04),
                        h.albedo,
                        h.metalness);
                    gather *= mix(
                        diffuse_albedo,
                        specular_albedo,
                        h.specular);
                }
                gather += h.emissive;
            }
            col += gather;
        }

        col /= float(SAMPLES);

        col = mix(sky_emissive(hits[0].incident), col, min(100.0 / hits[0].t, 1.0));
        out_color = vec4(col, 1.0);
    })";
