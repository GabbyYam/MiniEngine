#pragma once
#include "Render/Shader/Shader.hpp"
#include "Render/Texture/Texture.hpp"
#include "glad/glad.h"
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <memory>
#include <spdlog/spdlog.h>
#include <stdint.h>
#include <array>
#include <vector>
#include "stb_image.h"
#include "glm/glm.hpp"

const static std::string texture_prefix = "../Assets/CubeMap/";

namespace suplex {
    class CubeMap : public Texture {
    public:
        CubeMap() {}

        virtual void Allocate() override
        {
            glGenTextures(1, &m_TextureID);
            glBindTexture(GL_TEXTURE_CUBE_MAP, m_TextureID);
        };

        void AllocateCubeMap(float resolution)
        {
            glBindTexture(GL_TEXTURE_CUBE_MAP, m_TextureID);

            for (unsigned int i = 0; i < 6; ++i) {
                // note that we store each face with 16 bit floating point values
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, resolution, resolution, 0, GL_RGB, GL_FLOAT, nullptr);
            }
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        }

        void AllocateMipCubeMap(float resolution)
        {
            glGenTextures(1, &m_TextureID);
            glBindTexture(GL_TEXTURE_CUBE_MAP, m_TextureID);
            for (unsigned int i = 0; i < 6; ++i) {
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, resolution, resolution, 0, GL_RGB, GL_FLOAT, nullptr);
            }
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER,
                            GL_LINEAR_MIPMAP_LINEAR);  // enable pre-filter mipmap sampling (combatting visible dots artifact)
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
        }

        virtual void LoadData(std::string const& path, ImageFormat format) override {}
        virtual void LoadData(const aiTexture* aiTex, ImageFormat format) override {}

        void LoadData(std::vector<std::string> const& paths, ImageFormat format)
        {
            stbi_set_flip_vertically_on_load(true);
            for (unsigned int i = 0; i < paths.size(); i++) {
                void* data;
                switch (format) {
                    case ImageFormat::RGB:
                        data = stbi_load((texture_prefix + paths[i]).c_str(), &m_Width, &m_Height, &m_Channels, 0);
                        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, m_Width, m_Height, 0, GL_RGB, GL_UNSIGNED_BYTE,
                                     data);
                        break;
                    case ImageFormat::RGBA:
                        data = stbi_load((texture_prefix + paths[i]).c_str(), &m_Width, &m_Height, &m_Channels, 0);
                        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, m_Width, m_Height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                                     data);
                        break;

                    case ImageFormat::RGBA32F:
                        data = stbi_loadf((texture_prefix + paths[i]).c_str(), &m_Width, &m_Height, &m_Channels, 0);
                        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, m_Width, m_Height, 0, GL_RGBA, GL_FLOAT, data);
                        break;
                }

                if (data == nullptr) spdlog::error("Cubemap texture failed to load at path: {}", paths[i]);
                stbi_image_free(data);
            }
            stbi_set_flip_vertically_on_load(false);

            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
            spdlog::info("Load CubeMap complete");
        }
    };
}  // namespace suplex