#pragma once

#include "math/Math.hpp"

struct Material
{
    Material(
        glm::vec3 albedo,
        float smoothness,
        float metalness,
        float refraction,
        float refractive_index) :
        albedo(albedo),
        smoothness(smoothness),
        metalness(metalness),
        refraction(refraction),
        refractive_index(refractive_index)
    {
    }

    glm::vec3 albedo;
    float smoothness;
    float metalness;
    float refraction;
    float refractive_index;
    float padding_0;
};

struct Sphere
{
    Sphere(
        glm::vec3 position,
        float radius,
        Material material) :
        position(position),
        radius(radius),
        material(material)
    {
    }

    glm::vec3 position;
    float radius;
    Material material;
};
