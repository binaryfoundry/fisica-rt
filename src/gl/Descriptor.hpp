#pragma once

#include <map>

#include "OpenGL.hpp"

#include "../properties/Property.hpp"

using namespace Properties;

namespace GL
{
    class Descriptor
    {
    public:
        std::map<std::string, GLTextureResource> textures;
        std::map<std::string, GLBufferResource> uniform_blocks;
        std::map<std::string, Property<glm::mat4>> uniform_mat4s;
    };
}
