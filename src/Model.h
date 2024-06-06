#ifndef MODEL_H
#define MODEL_H

#include <glad/glad.h> 

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>


#include "Mesh.h"
#include "Program.h"
#include "stb_image.h"
#include "Animdata.h"
#include "assimp_glm_helpers.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
using namespace std;
using namespace glm;

class Model 
{
public:
    // model data 
    vector<shared_ptr<Texture>> textures_loaded;	// stores all the textures loaded so far, optimization to make sure textures aren't loaded more than once.
    map<string, vector<shared_ptr<Texture>>> originalTextures;
    shared_ptr<Texture> extTexture;
    map<string, Mesh> meshes;
    string directory;
    vec3 min = vec3(0);
    vec3 max = vec3(0);
    string filePath;

    // constructor, expects a filepath to a 3D model.
    Model(string const &path)
    {
        this->filePath = path;
        loadModel(path);
    }

    // draws the model, and thus all its meshes
    void Draw(const shared_ptr<Program> prog)
    {
        for (auto& pair : meshes) {
            pair.second.Draw(prog);
        }
    }

    void loadExtTexture(shared_ptr<Texture> texture)
    {
        extTexture = texture;

        originalTextures.clear();
        for (auto& pair : meshes) {
            originalTextures[pair.first] = pair.second.textures;
            pair.second.textures.push_back(texture);
        }
    }

    void unbindExternalTexture() {
        extTexture.reset();

        // Restore the original textures
        for (auto& pair : meshes) {
            if (originalTextures.find(pair.first) != originalTextures.end()) {
                pair.second.textures = originalTextures[pair.first];
            }
        }
    }

    auto& GetBoneInfoMap() { return m_BoneInfoMap; }
    int& GetBoneCount() { return m_BoneCounter; }
    
private:
    map<string, BoneInfo> m_BoneInfoMap;
    int m_BoneCounter = 0;

    // loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
    void loadModel(string const &path)
    {
        // read file via ASSIMP
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
        // check for errors
        if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
        {
            cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << endl;
            return;
        }
        // retrieve the directory path of the filepath
        directory = path.substr(0, path.find_last_of('/'));

        // process ASSIMP's root node recursively
        processNode(scene->mRootNode, scene);
        measure();
    }

    // processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
    void processNode(aiNode *node, const aiScene *scene)
    {
        //cout << "node name: " << node->mName.C_Str() << endl;
        // process each mesh located at the current node
        for(unsigned int i = 0; i < node->mNumMeshes; i++)
        {
            // the node object only contains indices to index the actual objects in the scene. 
            // the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            meshes[mesh->mName.C_Str()] = processMesh(mesh, scene);
        }
        // after we've processed all of the meshes (if any) we then recursively process each of the children nodes
        for(unsigned int i = 0; i < node->mNumChildren; i++)
        {
            processNode(node->mChildren[i], scene);
        }

    }

    void SetVertexBoneDataToDefault(Vertex& vertex)
    {
        for (int i = 0; i < MAX_BONE_INFLUENCE; i++)
        {
            vertex.m_BoneIDs[i] == -1;
            vertex.m_Weights[i] = 0.0f;
        }
    }

    Mesh processMesh(aiMesh *mesh, const aiScene *scene)
    {
        //cout << "mesh name: " << mesh->mName.C_Str() << endl;
        // data to fill
        vector<Vertex> vertices;
        vector<unsigned int> indices;
        vector<shared_ptr<Texture>>  textures;
        // walk through each of the mesh's vertices
        for(unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            Vertex vertex;
            SetVertexBoneDataToDefault(vertex);
            vertex.Position = AssimpGLMHelpers::GetGLMVec(mesh->mVertices[i]);
            vertex.Normal = AssimpGLMHelpers::GetGLMVec(mesh->mNormals[i]);
  
            if(mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
            {
                glm::vec2 vec;
                // a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't 
                // use models where a vertex can have multiple texture coordinates so we always take the first set (0).
                vec.x = mesh->mTextureCoords[0][i].x; 
                vec.y = mesh->mTextureCoords[0][i].y;
                vertex.TexCoords = vec;
            }
            else
                vertex.TexCoords = glm::vec2(0.0f, 0.0f);

            vertices.push_back(vertex);
        }
        // now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
        for(unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            // retrieve all indices of the face and store them in the indices vector
            for (unsigned int j = 0; j < face.mNumIndices; j++) { 
                indices.push_back(face.mIndices[j]);
            }                
        }

        // process materials
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];  
        // we assume a convention for sampler names in the shaders. Each diffuse texture should be named
        // as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER. 
        // Same applies to other texture as the following list summarizes:
        // diffuse: texture_diffuseN
        // specular: texture_specularN
        // normal: texture_normalN

        // 1. diffuse maps
        vector<shared_ptr<Texture>> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse", scene);
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
        // 2. specular maps
        vector<shared_ptr<Texture>> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular", scene);
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
        // 3. normal maps
        vector<shared_ptr<Texture>> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal", scene);
        textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
        // 4. height maps
        vector<shared_ptr<Texture>> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height", scene);
        textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());
        
        ExtractBoneWeightForVertices(vertices, mesh, scene);
        // return a mesh object created from the extracted mesh data
        return Mesh(vertices, indices, textures, mesh->mName.C_Str());
    }

    void SetVertexBoneData(Vertex& vertex, int boneID, float weight)
    {
        for (int i = 0; i < MAX_BONE_INFLUENCE; ++i)
        {
            if (vertex.m_BoneIDs[i] < 0)
            {
                vertex.m_Weights[i] = weight;
                vertex.m_BoneIDs[i] = boneID;
                break;
            }
        }
    }

    void ExtractBoneWeightForVertices(vector<Vertex>& vertices, aiMesh* mesh, const aiScene* scene)
    {
        auto& boneInfoMap = m_BoneInfoMap;
        int& boneCount = m_BoneCounter;

        cout << "number of bones: " << mesh->mNumBones << endl;
        for (int boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex)
        {
            int boneID = -1;
            string boneName = mesh->mBones[boneIndex]->mName.C_Str();
            if (boneInfoMap.find(boneName) == boneInfoMap.end())
            {
                BoneInfo newBoneInfo;
                newBoneInfo.id = boneCount;
                newBoneInfo.offset = AssimpGLMHelpers::ConvertMatrixToGLMFormat(mesh->mBones[boneIndex]->mOffsetMatrix);
                boneInfoMap[boneName] = newBoneInfo;
                boneID = boneCount;
                boneCount++;
            }
            else
            {
                boneID = boneInfoMap[boneName].id;
            }
            assert(boneID != -1);
            auto weights = mesh->mBones[boneIndex]->mWeights;
            int numWeights = mesh->mBones[boneIndex]->mNumWeights;

            for (int weightIndex = 0; weightIndex < numWeights; ++weightIndex)
            {
                int vertexId = weights[weightIndex].mVertexId;
                float weight = weights[weightIndex].mWeight;
                assert(vertexId <= vertices.size());
                SetVertexBoneData(vertices[vertexId], boneID, weight);
            }
        }
    }

    // checks all material textures of a given type and loads the textures if they're not loaded yet.
    // the required info is returned as a Texture struct.
    vector<shared_ptr<Texture>> loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName, const aiScene* scene)
    {
        vector<shared_ptr<Texture>> textures;
        shared_ptr<Texture> texture = make_shared<Texture>();
        for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
        {
            aiString str;
            mat->GetTexture(type, i, &str);
            // Check if the texture is embedded
            if (str.data[0] == '*') {  
                // Embedded texture
                string indexStr = string(str.data + 1);
                int index = stoi(indexStr);
                aiTexture* aiTex = scene->mTextures[index];
                
                if (aiTex->mHeight == 0) {
                    texture->setType(typeName);
                    texture->setBuffer(reinterpret_cast<unsigned char*>(aiTex->pcData), aiTex->mWidth);
                    texture->init(true);
                    texture->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

                }
                else {
                    cout << "uncompressed data" << endl;
                    GLenum format = GL_RGBA; // Example format
                    // Other formats include GL_RGB, GL_RGBA, GL_LUMINANCE based on your data
                    GLuint id;
                    glGenTextures(1, &id);
                    glBindTexture(GL_TEXTURE_2D, id);
                    texture->setType(typeName);
                    texture->setID(id);
                    glTexImage2D(GL_TEXTURE_2D, 0, format, aiTex->mWidth, aiTex->mHeight, 0, format, GL_UNSIGNED_BYTE, aiTex->pcData);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

                    glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture
                }

                textures.push_back(texture);
            }
            else {
                // External texture
                bool skip = false;
                for (unsigned int j = 0; j < textures_loaded.size(); j++)
                {
                    if (strcmp(textures_loaded[j]->getFilename().data(), str.C_Str()) == 0)
                    {
                        textures.push_back(textures_loaded[j]);
                        skip = true;
                        break;
                    }
                }
                if (!skip)
                {
                    texture->setFilename(directory+'/'+str.C_Str());
                    texture->setType(typeName);
                    texture->init(false); 
                    texture->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
                    textures.push_back(texture);
                    textures_loaded.push_back(texture);

                }
            }
        }
        
        return textures;
    }

    void measure()
    {
        float minX, minY, minZ;
        float maxX, maxY, maxZ;

        minX = minY = minZ = (numeric_limits<float>::max)();
        maxX = maxY = maxZ = -(numeric_limits<float>::max)();
        for (auto& pair : meshes)
        {
            if (pair.second.min.x < minX) minX = pair.second.min.x;
            if (pair.second.max.x > maxX) maxX = pair.second.max.x;

            if (pair.second.min.y < minY) minY = pair.second.min.y;
            if (pair.second.max.y > maxY) maxY = pair.second.max.y;

            if (pair.second.min.z < minZ) minZ = pair.second.min.z;
            if (pair.second.max.z > maxZ) maxZ = pair.second.max.z;
        }

        min.x = minX;
        min.y = minY;
        min.z = minZ;
        max.x = maxX;
        max.y = maxY;
        max.z = maxZ;
    }

};

#endif
