struct KeyPosition
{
    glm::vec3 position;
    f32 timeStamp; 
};

struct KeyRotation
{
    glm::quat orientation;
    f32 timeStamp;
};

struct KeyScale
{
    glm::vec3 scale;
    f32 timeStamp;
};

class Bone
{
public:
    Bone(const std::string &name, i32 ID, const aiNodeAnim *channel)
        : mName(name), mID(ID), mLocalTransform(1.0f)
    {
        mNumPositions = channel->mNumPositionKeys;
        for(i32 positionIndex = 0; positionIndex < mNumPositions; ++positionIndex)
        {
            aiVector3D aiPosition = channel->mPositionKeys[positionIndex].mValue;
            f32 timeStamp = (f32)channel->mPositionKeys[positionIndex].mTime;
            KeyPosition data;
            data.position = GetGLMVec(aiPosition);
            data.timeStamp = timeStamp;
            mPositions.push_back(data);
        }

        mNumRotations = channel->mNumPositionKeys;
        for(i32 rotationIndex = 0; rotationIndex < mNumRotations; ++rotationIndex)
        {
            aiQuaternion aiOrientation = channel->mRotationKeys[rotationIndex].mValue;
            f32 timeStamp = (f32)channel->mRotationKeys[rotationIndex].mTime;
            KeyRotation data;
            data.orientation = GetGLMQuat(aiOrientation);
            data.timeStamp = timeStamp;
            mRotations.push_back(data);
        }

        mNumScalings = channel->mNumScalingKeys;
        for(i32 scaleIndex = 0; scaleIndex < mNumScalings; ++scaleIndex)
        {
            aiVector3D scale = channel->mScalingKeys[scaleIndex].mValue;
            f32 timeStamp = (f32)channel->mScalingKeys[scaleIndex].mTime;
            KeyScale data;
            data.scale = GetGLMVec(scale);
            data.timeStamp = timeStamp;
            mScales.push_back(data);
        }
    }

    void Update(f32 animationTime)
    {
        glm::mat4 translation = InterpolatePosition(animationTime);
        glm::mat4 rotation = InterpolateRotation(animationTime);
        glm::mat4 scale = InterpolateScale(animationTime);
        mLocalTransform = translation * rotation * scale; 
    }

    glm::mat4 GetLocalTransform()
    {
        return mLocalTransform;
    }

    std::string GetBoneName() const
    {
        return mName;
    }

    i32 GetBoneID()
    {
        return mID;
    }

    i32 GetPositionIndex(f32 animationTime)
    {
        for(i32 index = 0; index < mNumPositions - 1; ++index)
        {
            if(animationTime < mPositions[index + 1].timeStamp)
            {
                return index;
            }
        }
        Assert(0);
        return -1;
    }

    i32 GetRotationIndex(f32 animationTime)
    {
        for(i32 index = 0; index < mNumRotations - 1; ++index)
        {
            if(animationTime < mRotations[index + 1].timeStamp)
            {
                return index;
            }
        }
        Assert(0);
        return -1;
    }

    i32 GetScaleIndex(f32 animationTime)
    {
        for(i32 index = 0; index < mNumScalings - 1; ++index)
        {
            if(animationTime < mScales[index + 1].timeStamp)
            {
                return index;
            }
        }
        Assert(0);
        return -1;
    }
private:
    f32 GetScaleFactor(f32 lastTimeStamp, f32 nextTimeStamp, f32 animationTime)
    {
        f32 scaleFactor = 0.0f;
        f32 midWayLength = animationTime - lastTimeStamp;
        f32 framesDiff = nextTimeStamp - lastTimeStamp;
        scaleFactor = midWayLength / framesDiff;
        return scaleFactor;
    }

    glm::mat4 InterpolatePosition(f32 animationTime)
    {
        if(1 == mNumPositions)
           return glm::translate(glm::mat4(1.0f), mPositions[0].position);
        i32 p0Index = GetPositionIndex(animationTime);
        i32 p1Index = p0Index + 1;
        f32 scaleFactor = GetScaleFactor(mPositions[p0Index].timeStamp, 
                                         mPositions[p1Index].timeStamp,
                                         animationTime);
        glm::vec3 finalPosition = glm::mix(mPositions[p0Index].position, mPositions[p1Index].position, scaleFactor);
        return glm::translate(glm::mat4(1.0f), finalPosition); 
    }

    glm::mat4 InterpolateRotation(f32 animationTime)
    {
        if(1 == mNumRotations)
        {
            auto rotation = glm::normalize(mRotations[0].orientation);
            return glm::toMat4(rotation);
        }

        i32 p0Index = GetRotationIndex(animationTime);
        i32 p1Index = p0Index + 1;
        f32 scaleFactor = GetScaleFactor(mRotations[p0Index].timeStamp,
                                         mRotations[p1Index].timeStamp,
                                         animationTime);
        glm::quat finalRotation = glm::slerp(mRotations[p0Index].orientation, mRotations[p1Index].orientation, scaleFactor);
        finalRotation = glm::normalize(finalRotation);
        return glm::toMat4(finalRotation);
    }

    glm::mat4 InterpolateScale(f32 animationTime)
    {
        if(1 == mNumScalings)
           return glm::scale(glm::mat4(1.0f), mScales[0].scale);
        
        i32 p0Index = GetScaleIndex(animationTime);
        i32 p1Index = p0Index + 1;
        f32 scaleFactor = GetScaleFactor(mScales[p0Index].timeStamp,
                                         mScales[p1Index].timeStamp,
                                         animationTime);
        glm::vec3 finalScale = glm::mix(mScales[p0Index].scale, mScales[p1Index].scale, scaleFactor);
        return glm::scale(glm::mat4(1.0f), finalScale);
    }

    std::vector<KeyPosition> mPositions;
    std::vector<KeyRotation> mRotations;
    std::vector<KeyScale> mScales;

    i32 mNumPositions;
    i32 mNumRotations;
    i32 mNumScalings;

    glm::mat4 mLocalTransform;
    std::string mName;
    i32 mID;
};
