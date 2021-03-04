#pragma once

#include <functional>

namespace Properties
{
    using Callback = std::function<void()>;

    class IProperty
    {
    public:
        virtual void Update(float time_step) = 0;
        virtual size_t InterpolatorCount() = 0;
    };
}
