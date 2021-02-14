#pragma once

#include "Math.hpp"

struct Transform
{
    glm::mat4 view;
    glm::mat4 projection;
    glm::mat4 inverse_projection;
    glm::mat4 inverse_view_rotation;
    glm::vec4 viewport;
    glm::vec4 camera_position;
    glm::vec4 exposure;
};

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
