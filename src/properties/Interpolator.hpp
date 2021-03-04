#pragma once

#include "../math/Math.hpp"

#include <optional>

namespace Properties
{
    template<typename T>
    class Interpolator
    {
    private:
        EasingFunction func;

        std::optional<Callback> on_complete;

        T value_0;
        T value_1;
        float time = 0;
        float duration = 0;
        bool complete = false;

        template<class A> A Interpolate(
            const A origin,
            const A target,
            const float t)
        {
            return origin + ((target - origin) * t);
        }

        glm::quat Interpolate(
            const glm::quat origin,
            const glm::quat target,
            const float t)
        {
            return glm::slerp(origin, target, t);
        }

    public:
        Interpolator() = default;

        Interpolator(
            const T origin,
            const T target,
            const float duration,
            const EasingFunction func,
            const Callback callback) :
            value_0(origin),
            value_1(target),
            duration(duration),
            func(func)
        {
            on_complete = callback;
        }

        bool Complete() const
        {
            return complete;
        }

        T [[nodiscard]] Update(const float time_step)
        {
            time += time_step;
            float percent = time / duration;
            float t = Ease(func, percent);

            if (t >= 1.0f)
            {
                complete = true;
                t = 1.0f;

                if (on_complete.has_value())
                {
                    on_complete.value()();
                }
            }
            return Interpolate(value_0, value_1, t);
        }
    };
}
