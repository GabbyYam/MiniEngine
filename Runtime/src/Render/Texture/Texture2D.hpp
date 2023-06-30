#pragma once
#include "Render/Texture/Texture.hpp"
#include <algorithm>
#include <assimp/texture.h>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/fwd.hpp>
#include <iostream>

#include <stb_image.h>
#include <stdint.h>

#include <glm/glm.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <spdlog/spdlog.h>

using namespace glm;
using namespace spdlog;

namespace suplex {
    class Texture2D : public Texture {
    public:
        Texture2D() = default;

        Texture2D(std::string const& path, ImageFormat format = ImageFormat::RGB) { LoadData(path, format); }

        virtual void Allocate(uint32_t w, uint32_t h) override
        {
            glGenTextures(1, &m_TextureID);

            m_Width = w, m_Height = h;

            // pre-allocate enough memory for the LUT texture.
            glBindTexture(GL_TEXTURE_2D, m_TextureID);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, w, h, 0, GL_RG, GL_FLOAT, 0);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        }

        void SolidColor()
        {
            auto data = new uint8_t[m_Width * m_Height * 3];
            std::fill(data, data + (m_Width * m_Height * 3), 255);
            glBindTexture(GL_TEXTURE_2D, m_TextureID);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, m_Width, m_Height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            delete[] data;
            info("Create Solid Texture");
        }

        virtual ~Texture2D()
        {
            if (m_ImageData) stbi_image_free(m_ImageData);
        }

        virtual void LoadData(std::string const& path, ImageFormat format) override
        {
            stbi_set_flip_vertically_on_load(true);

            LoadTextureFromFile(path.data(), format);

            stbi_set_flip_vertically_on_load(false);
        }
        virtual void LoadData(const aiTexture* aiTex, ImageFormat format) override { LoadTextureFromMemory(aiTex); }

    private:
        void LoadTextureFromFile(std::string_view path, ImageFormat format)
        {
            if (format == ImageFormat::RGB) {
                glGenTextures(1, &m_TextureID);
                glBindTexture(GL_TEXTURE_2D, m_TextureID);
                // set the texture wrapping parameters
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                                GL_REPEAT);  // set texture wrapping to GL_REPEAT (default wrapping method)
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                // set texture filtering parameters
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                // load image, create texture and generate mipmaps
                // stbi_set_flip_vertically_on_load(true);
                // tell stb_image.h to flip loaded texture's on the y-axis.
                // The FileSystem::getPath(...) is part of the GitHub repository so we can find files on any IDE/platform; replace it with your own image path.
                uint8_t* data = stbi_load(path.data(), &m_Width, &m_Height, &m_Channels, STBI_rgb);
                if (data) {
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_Width, m_Height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
                    glGenerateMipmap(GL_TEXTURE_2D);
                    info("Load Texture from path {}", path.data());
                }
                else {
                    error("Failed to load texture");
                }
                stbi_image_free(data);
            }
            else if (format == ImageFormat::RGBA) {
                glGenTextures(1, &m_TextureID);
                glBindTexture(GL_TEXTURE_2D, m_TextureID);
                // set the texture wrapping parameters
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                                GL_REPEAT);  // set texture wrapping to GL_REPEAT (default wrapping method)
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                // set texture filtering parameters
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                // load image, create texture and generate mipmaps
                // stbi_set_flip_vertically_on_load(true);
                // tell stb_image.h to flip loaded texture's on the y-axis.
                // The FileSystem::getPath(...) is part of the GitHub repository so we can find files on any IDE/platform; replace it with your own image path.
                uint8_t* data = stbi_load(path.data(), &m_Width, &m_Height, &m_Channels, STBI_rgb_alpha);
                if (data) {
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_Width, m_Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
                    glGenerateMipmap(GL_TEXTURE_2D);
                    info("Load Texture from path {}", path.data());
                }
                else {
                    error("Failed to load texture");
                }
                stbi_image_free(data);
            }

            else {
                int   width, height, nrComponents;
                void* data = stbi_loadf(path.data(), &width, &height, &nrComponents, 0);
                if (data) {
                    glGenTextures(1, &m_TextureID);
                    glBindTexture(GL_TEXTURE_2D, m_TextureID);
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT,
                                 data);  // note how we specify the texture's data value to be float

                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

                    info("Load HDR Image for cube map successful.");
                }
                else {
                    error("Failed to load HDR image.");
                }
                stbi_image_free(data);
            }
            // stbi_set_flip_vertically_on_load(false);
        }

        void LoadTextureFromMemory(const aiTexture* aiTex)
        {
            glGenTextures(1, &m_TextureID);
            glBindTexture(GL_TEXTURE_2D, m_TextureID);
            // set the texture wrapping parameters
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            // set texture wrapping to GL_REPEAT (default wrapping method)
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            // set texture filtering parameters
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            // load image, create texture and generate mipmaps

            uint8_t* data = nullptr;

            // Read texture from memory when processing FBX format
            if (aiTex->mHeight == 0) {
                data = stbi_load_from_memory(reinterpret_cast<uint8_t*>(aiTex->pcData), aiTex->mWidth, &m_Width, &m_Height,
                                             &m_Channels, STBI_rgb);
            }
            else {
                data = stbi_load_from_memory(reinterpret_cast<uint8_t*>(aiTex->pcData), aiTex->mWidth * aiTex->mHeight, &m_Width,
                                             &m_Height, &m_Channels, STBI_rgb);
            }

            if (data) {
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_Width, m_Height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
                glGenerateMipmap(GL_TEXTURE_2D);
            }
            else {
                error("Failed to load texture");
            }
            stbi_image_free(data);
        }
    };
}  // namespace suplex