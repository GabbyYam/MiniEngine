#pragma once
#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include <stdint.h>
#include <string>
#include <string_view>
#include <fstream>
#include <sstream>

#include "spdlog/spdlog.h"

#include <iostream>

// #ifdef _MSC_VER
// static const std::string prefix = "../../Assets/Shaders/";
// #else
// #endif
static const std::string prefix = "../Assets/Shaders/";

using namespace spdlog;
namespace suplex {
    enum class SamplerType { Texture2D, CubeMap };

    class Shader {
    public:
        Shader(std::string const& vert, std::string const& frag) { LoadFromFile(vert, frag); }

        void LoadFromFile(std::string const& vert, std::string const& frag, std::string const& shaderName = "")
        {
            auto vert_path = prefix + vert;
            auto frag_path = prefix + frag;

            // 1. retrieve the vertex/fragment source code from filePath
            std::string   vertexCode;
            std::string   fragmentCode;
            std::ifstream vShaderFile;
            std::ifstream fShaderFile;

            // ensure ifstream objects can throw exceptions:
            vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
            fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

            try {
                // open files
                vShaderFile.open(vert_path.data());
                fShaderFile.open(frag_path.data());
                std::stringstream vShaderStream, fShaderStream;

                // read file's buffer contents into streams
                vShaderStream << vShaderFile.rdbuf();
                fShaderStream << fShaderFile.rdbuf();

                // close file handlers
                vShaderFile.close();
                fShaderFile.close();

                // convert stream into string
                vertexCode   = vShaderStream.str();
                fragmentCode = fShaderStream.str();
            }
            catch (std::ifstream::failure& e) {
                error("ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: {}", e.what());
            }
            const char* vShaderCode = vertexCode.c_str();
            const char* fShaderCode = fragmentCode.c_str();
            // 2. compile shaders
            unsigned int vertex, fragment;

            // vertex shader
            vertex = glCreateShader(GL_VERTEX_SHADER);
            glShaderSource(vertex, 1, &vShaderCode, NULL);
            glCompileShader(vertex);
            checkCompileErrors(vertex, "VERTEX");

            // fragment Shader
            fragment = glCreateShader(GL_FRAGMENT_SHADER);
            glShaderSource(fragment, 1, &fShaderCode, NULL);
            glCompileShader(fragment);
            checkCompileErrors(fragment, "FRAGMENT");

            // shader Program
            m_ShaderID = glCreateProgram();
            glAttachShader(m_ShaderID, vertex);
            glAttachShader(m_ShaderID, fragment);
            glLinkProgram(m_ShaderID);
            debug("Create shader id = {}", m_ShaderID);
            checkCompileErrors(m_ShaderID, "PROGRAM");

            // delete the shaders as they're linked into our program now and no longer necessary
            glDeleteShader(vertex);
            glDeleteShader(fragment);

            m_ShaderName = shaderName.empty() ? frag.substr(0, frag.find_last_of('.')) : shaderName;
        }

        void  Bind() { glUseProgram(m_ShaderID); }
        void  Unbind() { glUseProgram(0); }
        auto  GetID() { return m_ShaderID; }
        auto& GetShaderName() { return m_ShaderName; }

    public:
        void BindTexture(std::string const& samplerName, const int textureID, const int index, SamplerType samplerType)
        {
            glActiveTexture(GL_TEXTURE0 + index);
            glBindTexture(samplerType == SamplerType::Texture2D ? GL_TEXTURE_2D : GL_TEXTURE_CUBE_MAP, textureID);
            glUniform1i(glGetUniformLocation(m_ShaderID, samplerName.data()), index);
        }

        void SetInt(std::string const& uniformName, const int value)
        {
            auto id = glGetUniformLocation(m_ShaderID, uniformName.data());
            glUniform1i(id, value);
        }

        void SetFloat(std::string const& uniformName, const float* value_ptr)
        {
            auto id = glGetUniformLocation(m_ShaderID, uniformName.data());
            glUniform1fv(id, 1, value_ptr);
        }

        void SetFloat2(std::string const& uniformName, const float* value_ptr)
        {
            auto id = glGetUniformLocation(m_ShaderID, uniformName.data());
            glUniform2fv(id, 1, value_ptr);
        }

        void SetFloat3(std::string const& uniformName, const float* value_ptr)
        {
            auto id = glGetUniformLocation(m_ShaderID, uniformName.data());
            glUniform3fv(id, 1, value_ptr);
        }

        void SetMaterix4(std::string const& uniformName, const float* value_ptr)
        {
            auto id = glGetUniformLocation(m_ShaderID, uniformName.data());
            glUniformMatrix4fv(id, 1, GL_FALSE, value_ptr);
        }

    private:
        uint32_t    m_ShaderID   = 0;
        std::string m_ShaderName = "New Material";

    private:
        // utility function for checking shader compilation/linking errors.
        // ------------------------------------------------------------------------
        void checkCompileErrors(unsigned int shader, std::string type)
        {
            int  success;
            char infoLog[1024];
            if (type != "PROGRAM") {
                glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
                if (!success) {
                    glGetShaderInfoLog(shader, 1024, NULL, infoLog);
                    std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n"
                              << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
                }
            }
            else {
                glGetProgramiv(shader, GL_LINK_STATUS, &success);
                if (!success) {
                    glGetProgramInfoLog(shader, 1024, NULL, infoLog);
                    std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n"
                              << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
                }
            }
        }
    };
}  // namespace suplex