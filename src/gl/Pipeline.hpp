#pragma once

#include "../math/Math.hpp"

#include "OpenGL.hpp"
#include "Shader.hpp"

namespace GL
{
    struct CameraUniforms
    {
        glm::mat4 view;
        glm::mat4 projection;
        glm::vec4 viewport;
        glm::vec4 position;
        glm::vec4 exposure;
    };

    class Pipeline
    {
    protected:
        uint32_t window_width = 0;
        uint32_t window_height = 0;

        void DrawQuad(Shader& shader);
        void FrontBuffer();

    public:
        virtual void Init() = 0;

        virtual void Deinit() = 0;

        void SetWindowSize(
            const uint32_t window_width_,
            const uint32_t window_height_);
    };
}
