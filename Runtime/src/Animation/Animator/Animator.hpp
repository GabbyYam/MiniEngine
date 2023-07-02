#pragma once

#include <Animation/Animation/Animation.hpp>
#include <memory>

namespace suplex {
    class Animator {
    public:
        Animator(const std::shared_ptr<Animation> animation)
        {
            m_CurrentTime      = 0.0f;
            m_CurrentAnimation = animation;

            m_FinalBoneMatrices.reserve(100);

            for (int i = 0; i < 100; i++) m_FinalBoneMatrices.push_back(glm::mat4(1.0f));
        }

        void UpdateAnimation(float dt)
        {
            m_DeltaTime = dt;
            if (m_CurrentAnimation) {
                m_CurrentTime += m_CurrentAnimation->GetTicksPerSecond() * dt;
                m_CurrentTime = fmod(m_CurrentTime, m_CurrentAnimation->GetDuration());
                CalculateBoneTransform(std::make_shared<AssimpNodeData>(&m_CurrentAnimation->GetRootNode()), glm::mat4(1.0f));
            }
        }

        void PlayAnimation(std::shared_ptr<Animation> animation)
        {
            m_CurrentAnimation = animation;
            m_CurrentTime      = 0.0f;
        }

        void CalculateBoneTransform(const std::shared_ptr<AssimpNodeData> node, glm::mat4 parentTransform)
        {
            std::string nodeName      = node->name;
            glm::mat4   nodeTransform = node->transformation;

            Bone* Bone = m_CurrentAnimation->FindBone(nodeName);

            if (Bone) {
                Bone->Update(m_CurrentTime);
                nodeTransform = Bone->GetLocalTransform();
            }

            glm::mat4 globalTransformation = parentTransform * nodeTransform;

            auto boneInfoMap = m_CurrentAnimation->GetBoneIDMap();
            if (boneInfoMap.find(nodeName) != boneInfoMap.end()) {
                int       index            = boneInfoMap[nodeName].id;
                glm::mat4 offset           = boneInfoMap[nodeName].offset;
                m_FinalBoneMatrices[index] = globalTransformation * offset;
            }

            for (int i = 0; i < node->childrenCount; i++)
                CalculateBoneTransform(std::make_shared<AssimpNodeData>(&node->children[i]), globalTransformation);
        }

        std::vector<glm::mat4> GetFinalBoneMatrices() { return m_FinalBoneMatrices; }

    private:
        std::vector<glm::mat4>     m_FinalBoneMatrices;
        std::shared_ptr<Animation> m_CurrentAnimation;
        float                      m_CurrentTime;
        float                      m_DeltaTime;
    };
}  // namespace suplex