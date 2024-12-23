#ifndef SKYBOX_H
#define SKYBOX_H

#pragma once

#include <glm/glm.hpp>
#include <glad/gl.h>

struct Skybox
{
    glm::vec3 position;
    glm::vec3 scale;

    GLuint vertexArrayID;
    GLuint vertexBufferID;
    GLuint indexBufferID;
    GLuint uvBufferID;
    GLuint TextureIds[4];

    GLuint mvpMatrixID;
    GLuint textureSamplerID;
    GLuint programID;

    void initialize(const glm::vec3& pos, const glm::vec3& scl);
    void render(const glm::mat4& vp, int flip);
    void cleanup();
};

#endif
