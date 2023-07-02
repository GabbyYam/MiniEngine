#pragma once
#include "Render/Camera/Camera.hpp"
#include "Render/Camera/LightCamera.hpp"
#include "Render/Buffer/Framebuffer.hpp"
#include "Render/Config/Config.hpp"
#include "Render/Postprocess/PostProcess.hpp"
#include "Render/Texture/CubeMap.hpp"
#include "Scene/Entity/Entity.hpp"
#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <memory>

using namespace glm;

namespace suplex {
    enum class PolygonMode { Shaded, WireFrame };
    enum class FogType { None, Linear, Exponential, ExponentialSquare };

    struct LightSetting
    {
        // Light space camera
        std::shared_ptr<Camera> cameraLS = std::make_shared<Camera>(45.0f, 0.01f, 64.0f, ProjectionType::Orthogonal);

        // Light Parameters
        vec3  lightColor{0.2, 1.0, 0.2f};
        float lightIntensity = 7.0f;
        bool  castShadow     = true;
        bool  useEnvMap      = false;
    };

    struct PostprocessSetting
    {
        bool enablePostprocess = true;

        // Tone Mapping
        float           exposure        = 0.8f;
        ToneMappingType tonemappingType = ToneMappingType::ACES;

        // Bloom
        bool  enableBloom       = true;
        float bloomThreshold    = 2.0f;
        float bloomIntensity    = 0.1f;
        float bloomFilterRadius = 0.005f;

        // Anti-aliasing
        bool enableFXAA = true;

        // Fog
        bool    enableFog  = true;
        FogType fogType    = FogType::None;
        float   fogDensity = 0.3f;
    };

    struct PBRSetting
    {
        // material parameters
        vec3  baseColor = vec3(0.6);
        float metallic  = 0.0;
        float roughness = 0.1;
        float ao        = 1.0;
    };

    struct GraphicsConfig
    {
        bool               vsync       = true;
        PolygonMode        polygonMode = PolygonMode::Shaded;
        LightSetting       lightSetting;
        PBRSetting         pbrSetting;
        PostprocessSetting postprocessSetting;

        float environmentMapResolution = 2048;
    };

    struct GraphicsContext
    {
        std::shared_ptr<GraphicsConfig> config = std::make_shared<GraphicsConfig>();
        Entity                          activeEntity;
    };

}  // namespace suplex