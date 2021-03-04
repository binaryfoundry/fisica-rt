#include "Easing.hpp"

namespace Properties
{
    float Ease(const EasingFunction func, const float t)
    {
        float k = t - 1;
        switch (func)
        {
        case EasingFunction::Linear:
            return t;
        case EasingFunction::EaseInQuad:
            return t * t;
        case EasingFunction::EaseOutQuad:
            return t * (2 - t);
        case EasingFunction::EaseInOutQuad:
            return t < .5 ? 2 * t * t :
                -1 + (4 - 2 * t) * t;
        case EasingFunction::EaseInCubic:
            return t * t * t;
        case EasingFunction::EaseOutCubic:
            return k * k * k + 1;
        case EasingFunction::EaseInOutCubic:
            return t < .5 ? 4 * t * t * t :
                (t - 1) * (2 * t - 2) * (2 * t - 2) + 1;
        case EasingFunction::EaseInQuart:
            return t * t * t * t;
        case EasingFunction::EaseOutQuart:
            return 1 - k * k * k * k;
        case EasingFunction::EaseInOutQuart:
            return t < .5 ? 8 * t * t * t * t :
                1 - 8 * k * k * k * k;
        case EasingFunction::EaseInQuint:
            return t * t * t * t * t;
        case EasingFunction::EaseOutQuint:
            return 1 + k * k * k * t * t;
        case EasingFunction::EaseInOutQuint:
            return t < .5 ? 16 * t * t * t *  t * t :
                1 + 16 * k * k * k * k * k;
        }
        return 0;
    }
}
