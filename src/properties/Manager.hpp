#pragma once

#include <memory>
#include <vector>

#include "IProperty.hpp"

namespace Properties
{
    class Manager
    {
    private:
        std::vector<IProperty*> properties;

    public:
        Manager() = default;
        Manager(const Manager&) = delete;

        void Add(IProperty* property);
        void Update(const float time_step);
    };
}
