#include "Shaders.hpp"

std::string frontbuffer_vertex_shader_string =
    R"(#version 300 es
    #ifdef GL_ES
    precision mediump float;
    #endif
    uniform mat4 projection;
    uniform mat4 view;
    layout(location = 0) in vec3 position;
    layout(location = 1) in vec2 texcoord;
    out vec2 v_texcoord;
    void main() {
        v_texcoord = texcoord;
        gl_Position = projection * view * vec4(position, 1.0);
    })";

std::string frontbuffer_fragment_shader_string =
    R"(#version 300 es
    #ifdef GL_ES
    precision mediump float;
    #endif
    in vec2 v_texcoord;
    uniform sampler2D tex;
    layout(location = 0) out vec4 out_color;

    const mat3 ACESInputMat = mat3(
        0.59719, 0.35458, 0.04823,
        0.07600, 0.90834, 0.01566,
        0.02840, 0.13383, 0.83777);
    const mat3 ACESOutputMat = mat3(
         1.60475, -0.53108, -0.07367,
        -0.10208,  1.10813, -0.00605,
        -0.00327, -0.07276,  1.07602);

    vec3 RRTAndODTFit(vec3 v) {
        vec3 a = v * (v + 0.0245786) - 0.000090537;
        vec3 b = v * (0.983729 * v + 0.4329510) + 0.238081;
        return a / b;
    }

    vec3 tone_map(vec3 color, float exposure) {
        color = (color * exposure) * ACESInputMat;
        color = RRTAndODTFit(color);
        color = color * ACESOutputMat;
        color = clamp(color, 0.0, 1.0);
        return color;
    }

    const float gamma = 2.2;
    vec3 to_linear_approx(vec3 v) { return pow(v, vec3(gamma)); }
    vec3 to_gamma_approx(vec3 v) { return pow(v, vec3(1.0 / gamma)); }

    void main() {
        vec3 c = texture(tex, v_texcoord).xyz;
        out_color = vec4(to_gamma_approx(tone_map(c, 1.0f)), 1.0);
    })";

std::string raytracing_vertex_shader_string =
    R"(#version 300 es
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
    })";

std::string raytracing_fragment_shader_string =
    R"(#version 300 es
    #ifdef GL_ES
    precision lowp float;
    precision lowp int;
    #endif

    #define PI 3.1415926535897932384626433832795
    #define PHI 1.61803398875

    #define SAMPLES 4

    in vec2 v_texcoord;
    layout(location = 0) out vec4 out_color;

    layout(std140) uniform transform{
        mat4 view;
        mat4 projection;
        mat4 inverse_projection;
        mat4 inverse_view_rotation;
        vec4 viewport;
        vec4 camera_position;
        vec4 exposure;
    };

    uniform sampler2D rand_sampler_0;
    uniform sampler2D rand_sampler_1;

    const mat2 rand_trans = mat2(
        cos(PHI), -sin(PHI),
        sin(PHI), cos(PHI)) * PHI;

    vec2 rand_0_state = vec2(1.0, PHI);
    vec2 rand_1_state = vec2(PHI, 1.0);

    float rand() {
        vec2 coords = gl_FragCoord.xy /
            vec2(textureSize(rand_sampler_0, 0)) + rand_0_state;
        rand_0_state = rand_trans * rand_0_state;
        return texture(rand_sampler_0, coords).x;
    }

    vec2 rand2() {
        vec2 coords = gl_FragCoord.xy /
            vec2(textureSize(rand_sampler_1, 0)) + rand_1_state;
        rand_1_state = rand_trans * rand_1_state;
        return vec2(rand(), texture(rand_sampler_1, coords).x);
    }

    struct Ray {
        vec3 origin;
        vec3 direction;
    };

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
        return Ray(camera_position.xyz, normalize(direction.xyz));
    }

    uniform sampler2D scene_sampler;

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

    void main() {
        Ray r = Ray_screen(v_texcoord);
        vec3 e = environment_emissive(r.direction);

        vec3 acc = vec3(0.0, 0.0, 0.0);

        for (int s = 0; s < SAMPLES; s++) {
            acc += e * rand();
        }

        acc /= float(SAMPLES);

        out_color = vec4(acc * exposure.x , 1.0);
    })";
