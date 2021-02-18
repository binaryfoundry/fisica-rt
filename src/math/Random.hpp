#pragma once

#include <random>

namespace Math
{
    using std::default_random_engine;
    using std::uniform_real_distribution;

    static default_random_engine generator;
    static uniform_real_distribution<double> distribution(0.0, 1.0);

    inline float unit_randf()
    {
        return static_cast<float>(distribution(generator));
    }

    template<typename T>
    inline T element_rand(T max_elements)
    {
        return static_cast<T>(floor(distribution(generator) * max_elements));
    }

    inline uint8_t byte_rand()
    {
        return static_cast<uint8_t>(floor(distribution(generator) * 255));
    }
}
