// cloud.cpp

#include "cloud.h"
#include "../helpers/objectLoader.h"
#include "../helpers/texture.h"
#include <render/shader.h>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <string>


// Example: Recompute normals for each face
void RecomputeNormals(
    const std::vector<glm::vec3>& vertices,
    const std::vector<unsigned int>& indices,
    std::vector<glm::vec3>& normals)
{
    normals.resize(vertices.size(), glm::vec3(0.0f));
    
    // Iterate over each triangle
    for (size_t i = 0; i < indices.size(); i += 3) {
        unsigned int idx0 = indices[i];
        unsigned int idx1 = indices[i + 1];
        unsigned int idx2 = indices[i + 2];
        
        glm::vec3 v0 = vertices[idx0];
        glm::vec3 v1 = vertices[idx1];
        glm::vec3 v2 = vertices[idx2];
        
        // Compute the face normal
        glm::vec3 edge1 = v1 - v0;
        glm::vec3 edge2 = v2 - v0;
        glm::vec3 faceNormal = glm::normalize(glm::cross(edge1, edge2));
        
        // Accumulate the face normal to each vertex normal
        normals[idx0] += faceNormal;
        normals[idx1] += faceNormal;
        normals[idx2] += faceNormal;
    }
    
    // Normalize the accumulated normals
    for (auto& normal : normals) {
        normal = glm::normalize(normal);
    }
}

// ----------------------------------------------
// Initialize the Cloud
// ----------------------------------------------
void Cloud::initialize(const glm::vec3& pos, const glm::vec3& scl)
{
    position = pos;
    scale    = scl;

    // 1. Load the OBJ file (minecraft.obj) from your assets folder
    ObjectLoader loader;
    bool res = loader.LoadOBJ("../assets/trees.obj", vertices, uvs, normals, indices, materials, materialToIndices);
    if(!res)
    {
        std::cerr << "Failed to load ../assets/minecraft.obj" << std::endl;
        return;
    }

    if (normals.empty()) {
        std::cout << "No normals found. Recomputing normals." << std::endl;
    }

    RecomputeNormals(vertices, indices, normals);

    // 2. Load all required textures based on materials
    textures.resize(materials.size(), 0); // Initialize texture vector

    for (size_t m = 0; m < materials.size(); ++m) {
        std::string texturePath = "../assets/" + materials[m].diffuse_texname;
        textures[m] = LoadTextureTileBox(texturePath.c_str(), true, true); // Enable repeat

        if (textures[m] == 0) {
            std::cerr << "Failed to load texture: " << materials[m].diffuse_texname 
                      << ". Using default white texture." << std::endl;
            // Optionally, load a default texture or handle as needed
            // Example: Load a default white texture
            // textures[m] = LoadTextureTileBox("../assets/default_white.png", false);
        }
    }

    // 3. Compile/link the cloud shaders (cloud.vert, cloud.frag)
    programID = LoadShadersFromFile("../todhchai/shaders/cloud.vert",
                                    "../todhchai/shaders/cloud.frag");
    if(programID == 0)
    {
        std::cerr << "Failed to load cloud shaders." << std::endl;
        return;
    }

    // 4. Create a VAO
    glGenVertexArrays(1, &vertexArrayID);
    glBindVertexArray(vertexArrayID);

    // 5. Create VBO for vertices
    glGenBuffers(1, &vertexBufferID);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
    glBufferData(GL_ARRAY_BUFFER,
                 vertices.size() * sizeof(glm::vec3),
                 &vertices[0],
                 GL_STATIC_DRAW);

    // 6. Create VBO for UVs
    glGenBuffers(1, &uvBufferID);
    glBindBuffer(GL_ARRAY_BUFFER, uvBufferID);
    glBufferData(GL_ARRAY_BUFFER,
                 uvs.size() * sizeof(glm::vec2),
                 &uvs[0],
                 GL_STATIC_DRAW);

    // 7. (Optional) Create VBO for normals if you want lighting
    glGenBuffers(1, &normalBufferID);
    glBindBuffer(GL_ARRAY_BUFFER, normalBufferID);
    glBufferData(GL_ARRAY_BUFFER,
                 normals.size() * sizeof(glm::vec3),
                 &normals[0],
                 GL_STATIC_DRAW);

    // 8. Create separate index buffers for each material
    materialIndexBuffers.resize(materials.size(), 0);
    for (size_t m = 0; m < materials.size(); ++m) {
        if (materialToIndices.find(m) == materialToIndices.end()) {
            // No indices for this material
            continue;
        }

        glGenBuffers(1, &materialIndexBuffers[m]);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, materialIndexBuffers[m]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     materialToIndices[m].size() * sizeof(unsigned int),
                     materialToIndices[m].data(),
                     GL_STATIC_DRAW);
    }

    // 9. Get uniform IDs
    mvpMatrixID      = glGetUniformLocation(programID, "MVP");
    textureSamplerID = glGetUniformLocation(programID, "textureSampler");
    lightPosID       = glGetUniformLocation(programID, "lightPos");
    lightColorID     = glGetUniformLocation(programID, "lightColor");
    objectColorID    = glGetUniformLocation(programID, "objectColor");

    // 10. Set the sampler to texture unit 0
    glUseProgram(programID);
    glUniform1i(textureSamplerID, 0);

    // Unbind VAO and ELEMENT_ARRAY_BUFFER
    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

// ----------------------------------------------
// Render the Cloud
// ----------------------------------------------
void Cloud::render(const glm::mat4& vp, glm::vec3 lightPos)
{
    // Use the cloud shader
    glUseProgram(programID);
    glBindVertexArray(vertexArrayID);

    // Positions -> layout location = 0
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    // UVs -> layout location = 1
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, uvBufferID);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

    // Normals -> layout location = 2
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, normalBufferID);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    // Compute model matrix
    glm::mat4 model = glm::translate(glm::mat4(1.0f), position);
    model = glm::scale(model, scale);

    // Compute final MVP
    glm::mat4 mvp = vp * model;
    glUniformMatrix4fv(mvpMatrixID, 1, GL_FALSE, &mvp[0][0]);





            // Set light properties
    glm::vec3 lightColor(1.0f, 1.0f, 1.0f); // White light
    glm::vec3 objectColor(1.0f, 1.0f, 1.0f); // Base color (can be adjusted)






    // Iterate over each material and render corresponding indices
    for (size_t m = 0; m < materials.size(); ++m) {
        if (materialToIndices.find(m) == materialToIndices.end()) {
            // No indices for this material
            continue;
        }


        glUniform3fv(lightPosID, 1, &lightPos[0]);
        glUniform3fv(lightColorID, 1, &lightColor[0]);
        glUniform3fv(objectColorID, 1, &objectColor[0]);


        // Bind the corresponding texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textures[m]);

        // Bind the index buffer for this material
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, materialIndexBuffers[m]);

        // Determine the number of indices
        GLsizei indexCount = static_cast<GLsizei>(materialToIndices[m].size());

        // Draw the elements
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    }

    // Cleanup
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);

    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

// ----------------------------------------------
// Cleanup the Cloud
// ----------------------------------------------
void Cloud::cleanup()
{
    glDeleteBuffers(1, &vertexBufferID);
    glDeleteBuffers(1, &uvBufferID);
    glDeleteBuffers(1, &normalBufferID);
    for (auto& ib : materialIndexBuffers) {
        if (ib != 0) {
            glDeleteBuffers(1, &ib);
        }
    }
    glDeleteVertexArrays(1, &vertexArrayID);

    glDeleteProgram(programID);
    for (auto& tex : textures) {
        if (tex != 0) {
            glDeleteTextures(1, &tex);
        }
    }
}
