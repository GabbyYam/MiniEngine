#pragma once

#include "Render/Shader/Shader.hpp"
#include "Render/Texture/Texture2D.hpp"
#include "glm/fwd.hpp"
#include "imgui.h"

#include <array>
#include <cstddef>
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>
#include <memory>
#include <spdlog/spdlog.h>
#include <stdint.h>
#include <tiny_obj_loader.h>
#include <vector>

namespace suplex {

    constexpr int MAX_BONE_INFLUENCE = 4;

    struct Vertex
    {
        glm::vec3 position  = {0, 0, 0};
        glm::vec3 normal    = {0, 0, 0};
        glm::vec2 texCoord  = {0, 0};
        glm::vec3 tangent   = {0, 0, 0};
        glm::vec3 bitangent = {0, 0, 0};

        int   boneIDs[MAX_BONE_INFLUENCE]{-1};
        float weights[MAX_BONE_INFLUENCE]{0.0f};

        Vertex()
        {
            for (int i = 0; i < MAX_BONE_INFLUENCE; i++) {
                boneIDs[i] = -1;
                weights[i] = 0.0f;
            }
        }

        void Reset()
        {
            for (int i = 0; i < MAX_BONE_INFLUENCE; i++) {
                boneIDs[i] = -1;
                weights[i] = 0.0f;
            }
        }

        void SetBoneData(int boneID, float weight)
        {
            for (int i = 0; i < MAX_BONE_INFLUENCE; ++i) {
                boneIDs[i] = boneID;
                weights[i] = weight;
            }
        }
    };

    class Mesh {
    public:
        Mesh() = default;

        Mesh(std::vector<Vertex>&& vs, std::vector<uint32_t>&& ids, std::vector<Texture2D>&& texs)
        {
            m_Vertices = vs;
            m_Indices  = ids;
            m_Textures = texs;

            BindBuffer();
        }

        virtual ~Mesh() {}

        virtual void Unbind()
        {
            glDeleteVertexArrays(1, &m_VAO);
            glDeleteBuffers(1, &m_VBO);
            glDeleteBuffers(1, &m_EBO);
        }

        virtual void BindBuffer()
        {
            // create buffers/arrays
            glGenVertexArrays(1, &m_VAO);
            glGenBuffers(1, &m_VBO);
            glGenBuffers(1, &m_EBO);

            glBindVertexArray(m_VAO);
            // load data into vertex buffers
            glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
            // A great thing about structs is that their memory layout is sequential for all its items.
            // The effect is that we can simply pass a pointer to the struct and it translates perfectly to a glm::vec3/2 array which
            // again translates to 3/2 floats which translates to a byte array.
            glBufferData(GL_ARRAY_BUFFER, m_Vertices.size() * sizeof(Vertex), &m_Vertices[0], GL_STATIC_DRAW);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_Indices.size() * sizeof(unsigned int), &m_Indices[0], GL_STATIC_DRAW);

            // set the vertex attribute pointers
            // vertex Positions
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
            // vertex normals
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
            // vertex texture coords
            glEnableVertexAttribArray(2);
            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));
            // vertex tangent
            glEnableVertexAttribArray(3);
            glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tangent));
            // vertex bitangent
            glEnableVertexAttribArray(4);
            glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, bitangent));

            // Bone Id & weights
            glEnableVertexAttribArray(5);
            glVertexAttribIPointer(5, 4, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, boneIDs));
            glEnableVertexAttribArray(6);
            glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, weights));

            glBindVertexArray(0);
        }

        void Log()
        {
            debug("Model vertex num = {}, index number = {}, texture num = {}", m_Vertices.size(), m_Indices.size(),
                  m_Textures.size());
        }

        const auto GetVertexNum() const { return m_Vertices.size(); }

        virtual void Render(const std::shared_ptr<Shader> shader)
        {
            if (m_VAO == 0) { BindBuffer(); }

            // bind appropriate textures
            unsigned int diffuseNr  = 1;
            unsigned int specularNr = 1;
            unsigned int normalNr   = 1;
            unsigned int heightNr   = 1;
            for (unsigned int i = 0; i < m_Textures.size(); i++) {
                // retrieve texture number (the N in diffuse_textureN)
                std::string number;
                std::string name = m_Textures[i].GetType();
                if (name == "DiffuseMap")
                    number = std::to_string(diffuseNr++);
                else if (name == "SpecularMap")
                    number = std::to_string(specularNr++);  // transfer unsigned int to string
                else if (name == "NormalMap")
                    number = std::to_string(normalNr++);  // transfer unsigned int to string
                else if (name == "HeightMap")
                    number = std::to_string(heightNr++);  // transfer unsigned int to string

                // glActiveTexture(GL_TEXTURE0 + i);  // active proper texture unit before binding
                // glUniform1i(glGetUniformLocation(shader->GetID(), (name + number).c_str()), i);
                // glBindTexture(GL_TEXTURE_2D, m_Textures[i].GetID());
                debug("Bind Texture, Type is {}, Slot = {}", name, i);
                shader->Bind();
                shader->BindTexture(name, m_Textures[i].GetID(), i, SamplerType::Texture2D);
            }

            glBindVertexArray(m_VAO);
            glDrawElements(GL_TRIANGLES, static_cast<uint32_t>(m_Indices.size()), GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);

            glActiveTexture(GL_TEXTURE0);
        }

    protected:
        std::vector<Vertex>    m_Vertices;
        std::vector<uint32_t>  m_Indices;
        std::vector<Texture2D> m_Textures;
        uint32_t               m_VAO = 0, m_VBO = 0, m_EBO = 0;
    };

}  // namespace suplex
