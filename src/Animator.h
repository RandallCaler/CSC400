#pragma once

#include <glm/glm.hpp>
#include <map>
#include <vector>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <memory>

#include "Animation.h"
#include "Bone.h"
#include "assimp_glm_helpers.h"

class Animator
{
public:
	Animator()
	{

		m_FinalBoneMatrices.reserve(200);

		for (int i = 0; i < 200; i++)
			m_FinalBoneMatrices.push_back(glm::mat4(1.0f));
	}

	void UpdateAnimation(float dt)
	{
		m_DeltaTime = dt;
		if (m_CurrentAnimation)
		{
			float previousTime = m_CurrentTime;
			m_CurrentTime += m_CurrentAnimation->GetTicksPerSecond() * dt;
			m_CurrentTime = fmod(m_CurrentTime, m_CurrentAnimation->GetDuration());

			if (previousTime > m_CurrentTime)
			{
				m_AnimationCompletedOnce = true;
			}

			CalculateBoneTransform(&m_CurrentAnimation->GetRootNode(), m_CurrentAnimation->GetGlobalInverseTransformation());
		}
	}

	void PlayAnimation(shared_ptr<Animation> pAnimation)
	{
		m_CurrentAnimation = pAnimation;
		m_CurrentTime = 0.0f;
		m_AnimationCompletedOnce = false;
	}

	void CalculateBoneTransform(const AssimpNodeData* node, glm::mat4 parentTransform)
	{

		std::string nodeName = node->name;
		glm::mat4 nodeTransform = node->transformation;

		Bone* Bone = m_CurrentAnimation->FindBone(nodeName);

		if (Bone)
		{
			Bone->Update(m_CurrentTime);
			nodeTransform = Bone->GetLocalTransform();
		}

		glm::mat4 globalTransformation = parentTransform * nodeTransform;

		auto boneInfoMap = m_CurrentAnimation->GetBoneIDMap();	
		if (boneInfoMap.find(nodeName) != boneInfoMap.end())
		{
			int index = boneInfoMap[nodeName].id;
			glm::mat4 offset = boneInfoMap[nodeName].offset;
			m_FinalBoneMatrices[index] = globalTransformation * offset;
		}

		for (int i = 0; i < node->childrenCount; i++)
			CalculateBoneTransform(&node->children[i], globalTransformation);
	}

	std::vector<glm::mat4> GetFinalBoneMatrices()
	{
		return m_FinalBoneMatrices;
	}

	void setCurrentAnimation(shared_ptr<Animation> animation) {
		m_CurrentAnimation = animation;
	}

	shared_ptr<Animation> getCurrentAnimation() {
		return m_CurrentAnimation;
	}

	bool m_AnimationCompletedOnce;


private:
	std::vector<glm::mat4> m_FinalBoneMatrices;
	shared_ptr<Animation> m_CurrentAnimation;
	float m_CurrentTime;
	float m_DeltaTime;

};
