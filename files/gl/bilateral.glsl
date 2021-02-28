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
    precision highp float;
    precision highp int;
    #endif

    in vec2 v_texcoord;

    uniform sampler2D txtr;

    layout(location = 0) out vec4 out_color;

    #define SIGMA 5.0
    #define BSIGMA 0.1
    #define MSIZE 20

    float normpdf(in float x, in float sigma) {
        return 0.39894 * exp(-0.5 * x * x / (sigma * sigma)) / sigma;
    }

    float normpdf3(in vec3 v, in float sigma) {
        return 0.39894 * exp(-0.5 * dot(v, v) / (sigma * sigma)) / sigma;
    }

    void main() {
        vec2 size = vec2(textureSize(txtr, 0));

        vec3 c = texture(txtr, vec2(v_texcoord.x, v_texcoord.y)).rgb;

        const int kSize = (MSIZE - 1) / 2;
        float kernel[MSIZE];
        vec3 final_colour = vec3(0.0);

        float Z = 0.0;
        for (int j = 0; j <= kSize; ++j) {
            kernel[kSize + j] = kernel[kSize - j] = normpdf(float(j), SIGMA);
        }

        vec3 cc;
        float factor;
        float bZ = 1.0 / normpdf(0.0, BSIGMA);

        for (int i = -kSize; i <= kSize; ++i) {
            for (int j = -kSize; j <= kSize; ++j) {
                vec2 fc = v_texcoord + (vec2(float(i), float(j)) / size);
                cc = texture(txtr, vec2(fc.x, fc.y)).rgb;
                factor = normpdf3(cc - c, BSIGMA) * bZ *
                    kernel[kSize + j] * kernel[kSize + i];
                Z += factor;
                final_colour += factor * cc;
            }
        }

        out_color = vec4(final_colour, 1.0);
    }

#endif
