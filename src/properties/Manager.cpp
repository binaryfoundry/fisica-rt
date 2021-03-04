#include "Manager.hpp"

namespace Properties
{
    void Manager::Add(IProperty* property)
    {
        properties.push_back(property);
    }

    void Manager::Update(const float time_step)
    {
        std::vector<size_t> removals;

        for (size_t i = 0; i < properties.size(); i++)
        {
            IProperty* p = properties[i];
            if (p->InterpolatorCount() != 0)
            {
                p->Update(time_step);
            }
            else
            {
                removals.push_back(i);
            }
        }

        for (auto& index : removals)
        {
            properties.erase(properties.begin() + index);
        }
    }
}
