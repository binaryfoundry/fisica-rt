#include "Noise.hpp"

#include <noise/sobol.h>
#include <noise/samplerBlueNoiseErrorDistribution_128x128_OptimizedFor_1spp.h>
#include <noise/samplerBlueNoiseErrorDistribution_128x128_OptimizedFor_2spp.h>
#include <noise/samplerBlueNoiseErrorDistribution_128x128_OptimizedFor_4spp.h>
#include <noise/samplerBlueNoiseErrorDistribution_128x128_OptimizedFor_8spp.h>
#include <noise/samplerBlueNoiseErrorDistribution_128x128_OptimizedFor_16spp.h>

#define DIM 128

namespace Noise
{
    static uint8_t bsample(
        uint32_t x, uint32_t y, uint32_t s,
        uint16_t m, uint32_t d)
    {
        float v;

        switch (m)
        {
        case 1:
            v = sbned_128x128_2d2d2d2d_1spp(x, y, s, d);
            break;
        case 2:
            v = sbned_128x128_2d2d2d2d_2spp(x, y, s, d);
            break;
        case 4:
            v = sbned_128x128_2d2d2d2d_4spp(x, y, s, d);
            break;
        case 8:
            v = sbned_128x128_2d2d2d2d_8spp(x, y, s, d);
            break;
        case 16:
            v = sbned_128x128_2d2d2d2d_16spp(x, y, s, d);
            break;
        }

        return static_cast<uint8_t>(v * 255);
    }

    void generate(
        std::unique_ptr<std::vector<TexDataByteRGBA>>& texture_data,
        uint16_t sample,
        uint16_t max_samples)
    {
        assert(texture_data->size() == DIM * DIM);

        for (uint32_t y = 0; y < DIM; y++)
        {
            for (uint32_t x = 0; x < DIM; x++)
            {
                uint8_t r = bsample(x, y, sample, max_samples, 0);
                uint8_t g = bsample(x, y, sample, max_samples, 1);
                uint8_t b = bsample(x, y, sample, max_samples, 2);
                uint8_t a = bsample(x, y, sample, max_samples, 3);

                size_t i = x + (y * DIM);
                (*texture_data)[i] = { r, g, b, a };
            }
        }
    }
}
