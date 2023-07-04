#pragma once

#include "Render/Geometry/Mesh.hpp"
#include "Render/Geometry/Model.hpp"
#include "Render/Texture/Texture.hpp"
#include "Render/Texture/Texture2D.hpp"
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"
#include <assimp/types.h>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/trigonometric.hpp>
#include <intrin0.inl.h>
#include <spdlog/spdlog.h>
#include <stdint.h>
#include <utility>
#include <vector>
#include "imgui.h"
#include "ImGuizmo.h"
#include "Mesh.hpp"
#include "imgui_internal.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

using namespace glm;
using namespace spdlog;

namespace utils {
    static inline glm::mat4 ConvertMatrixToGLMFormat(const aiMatrix4x4& from)
    {
        glm::mat4 to;
        //the a,b,c,d in assimp is the row ; the 1,2,3,4 is the column
        to[0][0] = from.a1;
        to[1][0] = from.a2;
        to[2][0] = from.a3;
        to[3][0] = from.a4;
        to[0][1] = from.b1;
        to[1][1] = from.b2;
        to[2][1] = from.b3;
        to[3][1] = from.b4;
        to[0][2] = from.c1;
        to[1][2] = from.c2;
        to[2][2] = from.c3;
        to[3][2] = from.c4;
        to[0][3] = from.d1;
        to[1][3] = from.d2;
        to[2][3] = from.d3;
        to[3][3] = from.d4;
        return to;
    }

    static inline glm::vec3 GetGLMVec(const aiVector3D& vec) { return glm::vec3(vec.x, vec.y, vec.z); }

    static inline glm::quat GetGLMQuat(const aiQuaternion& pOrientation)
    {
        return glm::quat(pOrientation.w, pOrientation.x, pOrientation.y, pOrientation.z);
    }
}  // namespace utils

namespace suplex {

    struct BoneInfo
    {
        /*id is index in finalBoneMatrices*/
        int id;

        /*offset matrix transforms vertex from model space to bone space*/
        glm::mat4 offset;
    };

    class Model {
    public:
        Model() = default;

        Model(std::string const& path)
        {
            m_FilePath = path;
            auto res   = LoadModel();
            if (res)
                debug("Load model {} complete", path);
        }

        ~Model()
        {
            // for (auto& mesh : m_Meshes) { mesh.Unbind(); }
        }

        void OnUpdate(float ts) {}

        auto& GetMeshes() { return m_Meshes; }
        auto& GetMaterialIndex() { return m_MaterialIndex; }
        auto& GetFilePath() { return m_FilePath; }

        void SetMaterialIndex(uint32_t materialIndex) { m_MaterialIndex = materialIndex; }

        // void AddMesh(const Mesh& mesh) { m_Meshes.emplace_back(mesh); }

        bool LoadModel()
        {
            info("Load Model at path {}", m_FilePath);
            std::string path = m_FilePath;
            m_Meshes.clear();
            // read file via ASSIMP
            Assimp::Importer importer;
            const aiScene*   scene = importer.ReadFile(path.data(), aiProcess_Triangulate | aiProcess_GenSmoothNormals |
                                                                        aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
            // check for errors
            if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)  // if is Not Zero
            {
                spdlog::error("ERROR::ASSIMP:: {}", importer.GetErrorString());
                return false;
            }

            auto suffix = path.substr(path.find_last_of('.') + 1);
            m_Directory = path.substr(0, path.find_last_of('/'));
            // process ASSIMP's root node recursively
            ProcessNode(scene->mRootNode, scene);
            return true;
        }

        auto& GetBoneInfoMap() { return m_BoneInfoMap; }
        int&  GetBoneCount() { return m_BoneCounter; }

    private:
        // processes a node in a recursive fashion. Processes each individual
        // mesh located at the node and repeats this process on its children
        // nodes (if any).
        void ProcessNode(aiNode* node, const aiScene* scene)
        {
            // process each mesh located at the current node
            for (unsigned int i = 0; i < node->mNumMeshes; i++) {
                // the node object only contains indices to index the actual
                // objects in the scene. the scene contains all the data, node
                // is just to keep stuff organized (like relations between
                // nodes).
                aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
                m_Meshes.push_back(ProcessMesh(mesh, scene));
            }
            // after we've processed all of the meshes (if any) we then
            // recursively process each of the children nodes
            for (unsigned int i = 0; i < node->mNumChildren; i++) {
                ProcessNode(node->mChildren[i], scene);
            }
        }

        Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene)
        {
            // data to fill
            std::vector<Vertex>    vertices;
            std::vector<uint32_t>  indices;
            std::vector<Texture2D> textures;

            // walk through each of the mesh's vertices
            for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
                Vertex    vertex;
                glm::vec3 vector;
                vertex.Reset();

                vector.x        = mesh->mVertices[i].x;
                vector.y        = mesh->mVertices[i].y;
                vector.z        = mesh->mVertices[i].z;
                vertex.position = vector;
                if (mesh->HasNormals()) {
                    vector.x      = mesh->mNormals[i].x;
                    vector.y      = mesh->mNormals[i].y;
                    vector.z      = mesh->mNormals[i].z;
                    vertex.normal = vector;
                }

                if (mesh->mTextureCoords[0]) {
                    glm::vec2 uv;
                    uv.x            = mesh->mTextureCoords[0][i].x;
                    uv.y            = mesh->mTextureCoords[0][i].y;
                    vertex.texCoord = uv;
                    // tangent
                    vector.x       = mesh->mTangents[i].x;
                    vector.y       = mesh->mTangents[i].y;
                    vector.z       = mesh->mTangents[i].z;
                    vertex.tangent = vector;
                    // bitangent
                    vector.x         = mesh->mBitangents[i].x;
                    vector.y         = mesh->mBitangents[i].y;
                    vector.z         = mesh->mBitangents[i].z;
                    vertex.bitangent = vector;
                }
                else
                    vertex.texCoord = glm::vec2(0.0f, 0.0f);

                vertices.push_back(vertex);
            }

            ProcessBoneWeight(vertices, mesh, scene);

            for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
                aiFace face = mesh->mFaces[i];

                for (unsigned int j = 0; j < face.mNumIndices; j++)
                    indices.push_back(face.mIndices[j]);
            }
            // process materials
            aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

            // 1. diffuse maps
            std::vector<Texture2D> diffuseMaps = LoadMaterialTextures(material, aiTextureType_DIFFUSE, "DiffuseMap", scene);
            textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
            // 2. specular maps
            std::vector<Texture2D> specularMaps = LoadMaterialTextures(material, aiTextureType_SPECULAR, "SpecularMap", scene);
            textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
            // 3. normal maps
            std::vector<Texture2D> normalMaps = LoadMaterialTextures(material, aiTextureType_HEIGHT, "NormalMap", scene);
            textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
            // 4. height maps
            std::vector<Texture2D> heightMaps = LoadMaterialTextures(material, aiTextureType_AMBIENT, "HeightMap", scene);
            textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());

            // return a mesh object created from the extracted mesh data
            return Mesh(std::move(vertices), std::move(indices), std::move(textures));
        }

        void ProcessBoneWeight(std::vector<Vertex>& vertices, aiMesh* mesh, const aiScene* scene)
        {
            for (int boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex) {
                int         boneID   = -1;
                std::string boneName = mesh->mBones[boneIndex]->mName.C_Str();
                if (m_BoneInfoMap.find(boneName) == m_BoneInfoMap.end()) {
                    BoneInfo newBoneInfo;
                    newBoneInfo.id          = m_BoneCounter;
                    newBoneInfo.offset      = utils::ConvertMatrixToGLMFormat(mesh->mBones[boneIndex]->mOffsetMatrix);
                    m_BoneInfoMap[boneName] = newBoneInfo;
                    boneID                  = m_BoneCounter;
                    m_BoneCounter++;
                }
                else {
                    boneID = m_BoneInfoMap[boneName].id;
                }
                assert(boneID != -1);
                auto weights    = mesh->mBones[boneIndex]->mWeights;
                int  numWeights = mesh->mBones[boneIndex]->mNumWeights;

                for (int weightIndex = 0; weightIndex < numWeights; ++weightIndex) {
                    int   vertexId = weights[weightIndex].mVertexId;
                    float weight   = weights[weightIndex].mWeight;
                    assert(vertexId <= vertices.size());
                    vertices[vertexId].SetBoneData(boneID, weight);
                }
            }
        }

        // checks all material textures of a given type and loads the textures
        // if they're not loaded yet. the required info is returned as a Texture2D struct.
        std::vector<Texture2D> LoadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName, const aiScene* scene)
        {
            std::vector<Texture2D> textures;
            for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
                aiString str;
                mat->GetTexture(type, i, &str);

                auto      embeddedTexture = scene->GetEmbeddedTexture(str.C_Str());
                Texture2D texture;

                if (embeddedTexture) {
                    texture.LoadData(embeddedTexture, TextureFormat::RGB);
                }
                else {
                    // To prevent temporary object been constructed
                    // use emplace_back here instead of push_back(Texture2D(str.C_Str())))
                    auto textureFilename = m_Directory + "/" + std::string(str.C_Str());
                    debug("Try to find {} {}", typeName, textureFilename);
                    texture.LoadData(textureFilename, TextureFormat::RGB);
                }

                texture.SetType(typeName);
                textures.push_back(texture);
            }

            return textures;
        }

    public:
        std::map<std::string, BoneInfo> m_BoneInfoMap;
        int                             m_BoneCounter = 0;

        std::vector<Mesh> m_Meshes;
        std::string       m_Directory;
        std::string       m_FilePath;

        uint32_t m_MaterialIndex = 0;
    };
}  // namespace suplex