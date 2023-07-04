#pragma once

#include "Random/Random.hpp"
#include <glm/fwd.hpp>
#include <vector>
namespace suplex {

    class Noise {
    public:
        static std::vector<glm::vec3> SSAOKernel()
        {
            std::vector<glm::vec3> kernel;

            for (int i = 0; i < 64; ++i) {
                glm::vec3 sample(Random::Float() * 2.0 - 1.0, Random::Float() * 2.0 - 1.0, Random::Float());
                sample = glm::normalize(sample);
                sample *= Random::Float();
                float scale = (float)i / 64.0;

                // Scale samples s.t. they're more aligned to center of kernel

                auto Lerp = [](float a, float b, float f) { return a + f * (b - a); };
                // scale     = Lerp(0.1f, 1.0f, scale * scale);
                sample *= scale;
                kernel.push_back(sample);
            }
            return kernel;
        }

        static std::vector<glm::vec3> SSAONoise()
        {
            // Noise texture
            std::vector<glm::vec3> noise;
            for (int i = 0; i < 16; i++) {
                glm::vec3 noiseSample(Random::Float() * 2.0 - 1.0, Random::Float() * 2.0 - 1.0,
                                      0.0f);  // rotate around z-axis (in tangent space)
                noise.push_back(noiseSample);
            }
            return noise;
        }
    };
}  // namespace suplex