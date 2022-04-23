#define MAX_BONE_INFLUENCE 4

struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoords;
    glm::vec3 tangent;
    glm::vec3 bitangent;
    i32 boneIDs[MAX_BONE_INFLUENCE];
    f32 weights[MAX_BONE_INFLUENCE];
};

struct Texture
{
    u32 id;
    std::string type;
    std::string path;
};

struct BoneInfo
{
	i32 id;
	/*offset matrix transforms vertex from model space to bone space*/
	glm::mat4 offset;

};

inline glm::mat4 ConvertMatrixToGLMFormat(const aiMatrix4x4& from)
{
	glm::mat4 to;
	//the a,b,c,d in assimp is the row ; the 1,2,3,4 is the column
	to[0][0] = from.a1; to[1][0] = from.a2; to[2][0] = from.a3; to[3][0] = from.a4;
	to[0][1] = from.b1; to[1][1] = from.b2; to[2][1] = from.b3; to[3][1] = from.b4;
	to[0][2] = from.c1; to[1][2] = from.c2; to[2][2] = from.c3; to[3][2] = from.c4;
	to[0][3] = from.d1; to[1][3] = from.d2; to[2][3] = from.d3; to[3][3] = from.d4;
	return to;
}

inline glm::vec3 GetGLMVec(const aiVector3D& vec) 
{ 
	return glm::vec3(vec.x, vec.y, vec.z); 
}

inline glm::quat GetGLMQuat(const aiQuaternion& pOrientation)
{
	return glm::quat(pOrientation.w, pOrientation.x, pOrientation.y, pOrientation.z);
}

class Mesh
{
public:
    std::vector<Vertex> vertices;
    std::vector<u32> indices;
    std::vector<Texture> textures;
    u32 VAO;

    // constructor
    Mesh(std::vector<Vertex> vertices, std::vector<u32> indices, std::vector<Texture> textures)
    {
        this->vertices = vertices;
        this->indices = indices;
        this->textures = textures;

        setupMesh();
    }

    void Draw(u32 shaderProgram)
    {
        u32 diffuseNr  = 1;
        u32 specularNr = 1;
        u32 normalNr   = 1;
        u32 heightNr   = 1;
        for(u32 i = 0; i < textures.size(); ++i)
        {
            glActiveTexture(GL_TEXTURE0 + i);
            std::string number;
            std::string name = textures[i].type;
            if(name == "texture_diffuse")
            {
                number = std::to_string(diffuseNr++);
            }
            else if(name == "texture_specular")
            {
                number = std::to_string(specularNr++);
            }
            else if(name == "texture_normal")
            {
                number = std::to_string(normalNr++);
            }
            else if(name == "texture_height")
            {
                number = std::to_string(heightNr++);
            }
            glUniform1i(glGetUniformLocation(shaderProgram, (name + number).c_str()), i);
            glBindTexture(GL_TEXTURE_2D, textures[i].id);
        }

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, static_cast<u32>(indices.size()), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        glActiveTexture(GL_TEXTURE0);
    }

private:
    u32 VBO, EBO;

    void setupMesh()
    {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);
        
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(u32), &indices[0], GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, normal));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, texCoords));
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, tangent));
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, bitangent));
        glEnableVertexAttribArray(5);
        glVertexAttribIPointer(5, 4, GL_INT, sizeof(Vertex), (void *)offsetof(Vertex, boneIDs));
        glEnableVertexAttribArray(6);
        glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, weights));

        glBindVertexArray(0);
    }
};

class Model
{
public:
    std::vector<Texture> texturesLoaded;
    std::vector<Mesh> meshes;
    std::string directory;
    bool gammaCorrection;

    Model(std::string const &path, bool gamma = false)
        : gammaCorrection(gamma)
    {
        loadModel(path);
    }

    void Draw(u32 shaderProgram)
    {
        for(u32 i = 0; i < meshes.size(); ++i)
        {
            meshes[i].Draw(shaderProgram);
        }
    }

    auto &GetBoneInfoMap()
    {
        return boneInfoMap;
    }
    int &GetBoneCount()
    {
        return boneCounter;
    }

private:

    std::map<std::string, BoneInfo> boneInfoMap;
    i32 boneCounter = 0;

    void loadModel(std::string const &path)
    {
        Assimp::Importer importer;
        const aiScene *scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace);
        if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            printf("Error Loading Model\n");
            printf(importer.GetErrorString());
            printf("\n");
            return; 
        }

        directory = path.substr(0, path.find_last_of('/'));

        processNode(scene->mRootNode, scene);
    }

    void processNode(aiNode *node, const aiScene *scene)
    {
        for(u32 i = 0; i < node->mNumMeshes; ++i)
        {
            aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
            meshes.push_back(processMesh(mesh, scene));
        }

        for(u32 i = 0; i < node->mNumChildren; ++i)
        {
            processNode(node->mChildren[i], scene);
        }
    }

    void SetVertexBoneDataToDefault(Vertex &vertex)
    {
        for(i32 i = 0; i < MAX_BONE_INFLUENCE; ++i)
        {
            vertex.boneIDs[i] = -1;
            vertex.weights[i] = 0.0f; 
        } 
    }

    Mesh processMesh(aiMesh *mesh, const aiScene *scene)
    {
        std::vector<Vertex> vertices;
        std::vector<u32> indices;
        std::vector<Texture> textures;

        for(u32 i = 0; i < mesh->mNumVertices; ++i)
        {
            Vertex vertex = {};
            SetVertexBoneDataToDefault(vertex);
            
            vertex.position = GetGLMVec(mesh->mVertices[i]);
            vertex.normal = GetGLMVec(mesh->mNormals[i]);
            vertex.tangent = GetGLMVec(mesh->mTangents[i]);
            vertex.bitangent = GetGLMVec(mesh->mBitangents[i]);

            if(mesh->mTextureCoords[0])
            {
                glm::vec2 texCoords;
                texCoords.x = mesh->mTextureCoords[0][i].x;
                texCoords.y = mesh->mTextureCoords[0][i].y;
                vertex.texCoords = texCoords;
            }
            vertices.push_back(vertex);
        }

        for(u32 i = 0; i < mesh->mNumFaces; ++i)
        {
            aiFace face = mesh->mFaces[i];
            for(u32 j = 0; j < face.mNumIndices; ++j)
            {
                indices.push_back(face.mIndices[j]);
            } 
        }
        aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];

        std::vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
        std::vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
		textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
		std::vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
		textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
		std::vector<Texture> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
		textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());

        ExtractBoneWeightForVertices(vertices, mesh, scene);

        return Mesh(vertices, indices, textures);
    }

    void SetVertexBoneData(Vertex &vertex, i32 boneID, f32 weight)
    {
        for(i32 i = 0; i < MAX_BONE_INFLUENCE; ++i)
        {
            if(vertex.boneIDs[i] < 0)
            {
                vertex.weights[i] = weight;
                vertex.boneIDs[i] = boneID;
                break; 
            } 
        } 
    }

    void ExtractBoneWeightForVertices(std::vector<Vertex> &vertices, aiMesh *mesh, const aiScene *scene)
    {
        auto &boneInfoMapRef = boneInfoMap;
        i32 &boneCounterRef = boneCounter;

        for(u32 boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex)
        {
            i32 boneID = -1;
            std::string boneName = mesh->mBones[boneIndex]->mName.C_Str();
            if(boneInfoMapRef.find(boneName) == boneInfoMapRef.end())
            {
                BoneInfo newBoneInfo;
                newBoneInfo.id = boneCounterRef;
                newBoneInfo.offset = ConvertMatrixToGLMFormat(mesh->mBones[boneIndex]->mOffsetMatrix);
                boneInfoMapRef[boneName] = newBoneInfo;
                boneID = boneCounterRef;
                boneCounterRef++;
            }
            else
            {
                boneID = boneInfoMapRef[boneName].id;
            }
            Assert(boneID != -1);
            auto weights = mesh->mBones[boneIndex]->mWeights;
            i32 numWeights = mesh->mBones[boneIndex]->mNumWeights;

            for(i32 weightIndex = 0; weightIndex < numWeights; ++weightIndex)
            {
                i32 vertexId = weights[weightIndex].mVertexId;
                f32 weight = weights[weightIndex].mWeight;
                Assert(vertexId <= vertices.size());
                SetVertexBoneData(vertices[vertexId], boneID, weight);
            }
        }
    }

    u32 TextureFromFile(const char *path, const std::string directory, bool gamma = false)
    {
        std::string filename = std::string(path);
        filename = directory + '/' + filename;

        u32 textureID;
        glGenTextures(1, &textureID);

        i32 width, height, nrComponents;
        unsigned char *data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
        if(data)
        {
            GLenum format;
            if(nrComponents == 1)
                format = GL_RED;
            else if(nrComponents == 3)
                format = GL_RGB;
            else if(nrComponents == 4)
                format = GL_RGBA;

            glBindTexture(GL_TEXTURE_2D, textureID);
            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            stbi_image_free(data);
        }
        else
        {
            printf("Error Loading Texture\n");
            stbi_image_free(data);
        }
        return textureID;
    }

    std::vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName)
    {
        std::vector<Texture> textures;
        for(u32 i = 0; i < mat->GetTextureCount(type); i++)
        {
            aiString str;
            mat->GetTexture(type, i, &str);

            bool skip = false;
            for(u32 j = 0; j < texturesLoaded.size(); j++)
            {
                if(std::strcmp(texturesLoaded[j].path.data(), str.C_Str()) == 0)
                {
                    textures.push_back(texturesLoaded[j]);
                    skip = true;
                    break;
                }
            }
            if(!skip)
            {
                Texture texture;
                texture.id = TextureFromFile(str.C_Str(), this->directory);
                texture.type = typeName;
                texture.path = str.C_Str();
                textures.push_back(texture);
                texturesLoaded.push_back(texture); 
            }
        }
        return textures;
    }

};
