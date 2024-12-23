#ifndef BUILDING_H
#define BUILDING_H

#pragma once
#include <glm/glm.hpp>

struct Building
{
    glm::vec3 position;
    glm::vec3 scale;

    // OpenGL buffers
    unsigned int vertexArrayID;
    unsigned int vertexBufferID;
    unsigned int indexBufferID;
    unsigned int colorBufferID;
    unsigned int uvBufferID;
    unsigned int textureID;

    // Shader variable IDs
    unsigned int mvpMatrixID;
    unsigned int textureSamplerID;
    unsigned int programID;

    void initialize(glm::vec3 pos, glm::vec3 scl);
    void render(const glm::mat4& vp);
    void cleanup();
};


#endif