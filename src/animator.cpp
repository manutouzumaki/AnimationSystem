class Animator
{
public:
    Animator(Animation *animation)
    {
        mCurrentTime = 0.0f;
        mCurrentAnimation = animation;

        mFinalBoneMatrices.reserve(100);

        for(i32 i = 0; i < 100; ++i)
        {
            mFinalBoneMatrices.push_back(glm::mat4(1.0f));
        }
    }

    void UpdateAnimation(f32 dt)
    {
        mDeltaTime = dt;
        if(mCurrentAnimation)
        {
            mCurrentTime += mCurrentAnimation->GetTicksPerSecond() * dt;
            mCurrentTime = fmodf(mCurrentTime, mCurrentAnimation->GetDuration());
            CalculateBoneTransform(&mCurrentAnimation->GetRootNode(), glm::mat4(1.0f));
        }
    }

    void PlayAnimation(Animation *animation)
    {
        mCurrentAnimation = animation;
        mCurrentTime = 0.0f;
    }

    void CalculateBoneTransform(const AssimpNodeData* node, glm::mat4 parentTransform)
    {
        std::string nodeName = node->name;
        glm::mat4 nodeTransform = node->transformation;

        Bone *bone = mCurrentAnimation->FindBone(nodeName);

        if(bone)
        {
            bone->Update(mCurrentTime);
            nodeTransform = bone->GetLocalTransform();
        }

        glm::mat4 globalTransformation = parentTransform * nodeTransform;

        auto boneInfoMap = mCurrentAnimation->GetBoneIDMap();
        if(boneInfoMap.find(nodeName) != boneInfoMap.end())
        {
            i32 index = boneInfoMap[nodeName].id;
            glm::mat4 offset = boneInfoMap[nodeName].offset;
            mFinalBoneMatrices[index] = globalTransformation * offset;
        }

        for(i32 i = 0; i < node->childrenCount; ++i)
        {
            CalculateBoneTransform(&node->children[i], globalTransformation);
        }
    }

    std::vector<glm::mat4> GetFinalBoneMatrices()
    {
        return mFinalBoneMatrices;
    }
private:
    std::vector<glm::mat4> mFinalBoneMatrices;
    Animation *mCurrentAnimation;
    f32 mCurrentTime;
    f32 mDeltaTime;
};
