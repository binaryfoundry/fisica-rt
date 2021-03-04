#pragma once

#include <memory>
#include <vector>

#include "IProperty.hpp"

namespace Properties
{
    class Manager
    {
        friend class IProperty;

    private:
        std::vector<IProperty*> properties;
        void Add(IProperty* property);

    public:
        Manager() = default;
        Manager(const Manager&) = delete;

        void Update(const float time_step);
    };
}
