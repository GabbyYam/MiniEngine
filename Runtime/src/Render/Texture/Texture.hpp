#pragma once
#include <assimp/texture.h>
#include <iostream>
#include <stdint.h>

namespace suplex {
    enum class ImageFormat { RGB, RGBA, RGBA32F };

    class Texture {
    public:
        Texture(){};
        virtual ~Texture() {}
        virtual void Allocate(){};
        virtual void Allocate(uint32_t w, uint32_t h){};

        virtual void LoadData(std::string const& path, ImageFormat format) {}
        virtual void LoadData(const aiTexture* aiTex, ImageFormat format) {}

        const uint8_t* data() const { return m_ImageData; }

        const int GetWidth() const { return m_Width; }

        const int GetHeight() const { return m_Height; }

        const int GetChannels() const { return m_Channels; }

        void SetType(const std::string& type) { m_Type = type; }

        void SetPath(const std::string& path) { m_Path = path; }

        const auto GetID() const { return m_TextureID; }

        const auto GetType() const { return m_Type; }

    protected:
        uint32_t    m_TextureID = 0;
        int         m_Width = 0, m_Height = 0, m_Channels = 0;
        uint8_t*    m_ImageData = nullptr;
        std::string m_Type;
        std::string m_Path;
    };
}  // namespace suplex