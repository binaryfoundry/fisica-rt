#include "Noise.hpp"

#include <noise/samplerBlueNoiseErrorDistribution_128x128_OptimizedFor_4spp.h>

#define DIM 128

namespace Noise
{
    static uint8_t bsample(uint32_t x, uint32_t y, uint32_t s, uint32_t d)
    {
        return static_cast<uint8_t>(sbned_128x128_2d2d2d2d_4spp(x, y, s, d) * 255);
    }

    void generate(
        std::unique_ptr<std::vector<TexDataByteRGBA>>& texture_data,
        uint16_t sample)
    {
        assert(texture_data->size() == DIM * DIM);

        for (uint32_t y = 0; y < DIM; y++)
        {
            for (uint32_t x = 0; x < DIM; x++)
            {
                uint8_t r = bsample(x, y, sample, 0);
                uint8_t g = bsample(x, y, sample, 1);
                uint8_t b = bsample(x, y, sample, 2);
                uint8_t a = bsample(x, y, sample, 3);

                size_t i = x + (y * DIM);
                (*texture_data)[i] = { r, g, b, a };
            }
        }
    }
}
