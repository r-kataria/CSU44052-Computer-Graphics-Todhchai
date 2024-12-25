// cloud.h

#ifndef CLOUD_H
#define CLOUD_H

#pragma once

#include <glm/glm.hpp>
#include <glad/gl.h>
#include <vector>
#include <unordered_map>
#include "../helpers/objectLoader.h" // Include to access Material

struct Cloud
{
    // Transform
    glm::vec3 position;
    glm::vec3 scale;

    // GL buffers
    GLuint vertexArrayID;
    GLuint vertexBufferID;
    GLuint uvBufferID;
    GLuint normalBufferID;
    // Separate index buffers for each material
    std::vector<GLuint> materialIndexBuffers;

    // Shaders, texture, uniforms
    GLuint programID;
    GLuint mvpMatrixID;
    GLuint textureSamplerID;

    // CPU-side geometry data
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec2> uvs;
    std::vector<glm::vec3> normals;
    std::vector<unsigned int> indices;

    // Materials
    std::vector<Material> materials;
    std::unordered_map<int, std::vector<unsigned int>> materialToIndices;

    // Textures corresponding to each material
    std::vector<GLuint> textures;

    void initialize(const glm::vec3& pos, const glm::vec3& scl);
    void render(const glm::mat4 &vp, glm::vec3 lightPos);
    void cleanup();


    GLuint lightPosID;         // New
    GLuint lightColorID;       // New
    GLuint objectColorID;      // New

};

#endif // CLOUD_H
