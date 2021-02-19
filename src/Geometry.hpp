#pragma once

#include "math/Math.hpp"

struct Sphere
{
    Sphere() {};
    Sphere(
        glm::vec4 geom,
        glm::vec4 albedo,
        glm::vec2 material,
        glm::vec2 refraction) :
        geom(geom),
        albedo(albedo),
        material(material),
        refraction(refraction)
    {
    }

    glm::vec4 geom;       // xyz = position, w = radius
    glm::vec4 albedo;     // xyz = rgb
    glm::vec2 material;   // x = smoothness, y = metalness
    glm::vec2 refraction; // x = refractive, y = refract index
};
