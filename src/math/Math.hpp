#pragma once

const double PI = 3.141592653589793;
const float PIf = 3.141592653589793f;

#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#if !defined(EMSCRIPTEN)
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_SIMD_AVX2
#endif

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat2x2.hpp>
#include <glm/mat3x3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>

struct bvec4
{
public:
    bvec4(uint8_t r, uint8_t g, uint8_t b, uint8_t a) :
        r(r), g(g), b(b), a(a) { }
    bvec4() {}
    uint8_t r; uint8_t g; uint8_t b;  uint8_t a;
};

using TexDataByteRGBA = bvec4;
using TexDataFloatRGBA = glm::vec4;
using TexDataFloatRGB = glm::vec3;
