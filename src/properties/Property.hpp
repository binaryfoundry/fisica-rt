#pragma once

#include <memory>
#include <functional>

#include "Easing.hpp"
#include "Manager.hpp"
#include "IProperty.hpp"
#include "Interpolator.hpp"

namespace Properties
{
    template<typename T>
    class Property : public IProperty
    {
    private:
        T value;

        std::vector<Interpolator<T>> interpolators;

    public:
        Property() = default;
        Property(const Property<T>&) = delete;

        Property(
            const T value) :
            value(value)
        {
        }

        T Value() const
        {
            return value;
        }

        size_t InterpolatorCount()
        {
            return interpolators.size();
        }

        void Update(const float time_step)
        {
            std::vector<size_t> removals;

            for (size_t j = 0; j < interpolators.size(); j++)
            {
                auto& i = interpolators[j];
                value = i.Update(time_step);

                if (i.Complete())
                {
                    removals.push_back(j);
                }
            }

            for (auto& index : removals)
            {
                interpolators.erase(interpolators.begin() + index);
            }
        }

        void Animate(
            std::unique_ptr<Manager>& manager,
            const T start,
            const T end,
            const float seconds,
            const EasingFunction func,
            const Callback callback = []() {})
        {
            if (InterpolatorCount() == 0)
            {
                manager->Add(this);
            }

            Interpolator<T> it(start, end, seconds, func,
                std::move(callback));

            interpolators.push_back(it);
        }
    };
}
