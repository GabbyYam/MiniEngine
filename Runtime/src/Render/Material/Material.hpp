#pragma once

#include "Render/Shader/Shader.hpp"
#include <future>
#include <memory>
namespace suplex {
    struct Property
    {
    };

    struct Material
    {
        std::shared_ptr<Property> property;
        std::shared_ptr<Shader>   shader;
    };
}  // namespace suplex