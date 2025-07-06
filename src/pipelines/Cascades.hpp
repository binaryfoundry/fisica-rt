#pragma once

#include "../Platform.hpp"
#include "../Graphics.hpp"

#include "../math/Math.hpp"

#include <memory>

namespace Pipelines
{
    class Cascades : public Pipeline
    {
    private:
        bool initialized = false;

        glm::mat4 projection;
        glm::mat4 view;

        float exposure = 1.0f;

        std::unique_ptr<FrameBuffer<TexDataFloatRGBA>> framebuffer;

        Shader frontbuffer_shader;
        Descriptor frontbuffer_set_0;

        Shader cascades_shader;
        Descriptor cascades_set_0;

    public:
        Cascades();

        void Init();

        void Deinit();

        void Draw(
            const glm::mat4 projection_,
            const glm::mat4 view_,
            const bool upscale);
    };
}
