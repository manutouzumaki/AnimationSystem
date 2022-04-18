struct AssimpNodeData
{
    glm::mat4 transformation;
    std::string name;
    i32 childrenCount;
    std::vector<AssimpNodeData> children;
};

class Animation
{
public:
    Animation() = default;

    Animation(const std::string &animationPath, Model *model)
    {
        Assimp::Importer importer;
        const aiScene *scene = importer.ReadFile(animationPath, aiProcess_Triangulate);
        Assert(scene && scene->mRootNode);
        auto animation = scene->mAnimations[0];
        mDuration = (f32)animation->mDuration;
        mTicksPerSecond = (f32)animation->mTicksPerSecond;
        aiMatrix4x4 globalTransformation = scene->mRootNode->mTransformation;
        globalTransformation = globalTransformation.Inverse();
        ReadHeirarchyData(mRootNode, scene->mRootNode);
        ReadMissingBones(animation, *model);
    }

    ~Animation()
    {
    }

    Bone *FindBone(const std::string &name)
    {
        auto iter = std::find_if(mBones.begin(), mBones.end(),
                [&](const Bone &bone)
                {
                    return bone.GetBoneName() == name;
                }
        );
        if(iter == mBones.end()) return nullptr;
        else return &(*iter);
    }

    inline f32 GetTicksPerSecond()
    {
        return mTicksPerSecond;
    }

    inline f32 GetDuration()
    {
        return mDuration;
    }

    inline const AssimpNodeData &GetRootNode()
    {
        return mRootNode;
    }

    inline const std::map<std::string, BoneInfo> &GetBoneIDMap()
    {
        return mBoneInfoMap;
    }

private:

    void ReadMissingBones(const aiAnimation *animation, Model &model)
    {
        i32 size = animation->mNumChannels;
        auto &boneInfoMap = model.GetBoneInfoMap();
        i32 &boneCount = model.GetBoneCount();

        for(i32 i = 0; i < size; ++i)
        {
            auto channel = animation->mChannels[i];
            std::string boneName = channel->mNodeName.data;

            if(boneInfoMap.find(boneName) == boneInfoMap.end())
            {
                boneInfoMap[boneName].id = boneCount;
                boneCount++;
            }
            mBones.push_back(Bone(channel->mNodeName.data,
                                  boneInfoMap[channel->mNodeName.data].id,
                                  channel));
        }

        mBoneInfoMap = boneInfoMap;
    }

    void ReadHeirarchyData(AssimpNodeData &dest, const aiNode *src)
    {
        Assert(src);

        dest.name = src->mName.data;
        dest.transformation = ConvertMatrixToGLMFormat(src->mTransformation);
        dest.childrenCount = src->mNumChildren;

        for(u32 i = 0; i < src->mNumChildren; ++i)
        {
            AssimpNodeData newData;
            ReadHeirarchyData(newData, src->mChildren[i]);
            dest.children.push_back(newData);
        }
    }

    f32 mDuration;
    f32 mTicksPerSecond;
    std::vector<Bone> mBones;
    AssimpNodeData mRootNode;
    std::map<std::string, BoneInfo> mBoneInfoMap;

};
