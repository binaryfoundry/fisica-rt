#version 300 es

#if defined(COMPILING_VS)

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
    }

#elif defined(COMPILING_FS)

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
    }

#endif
