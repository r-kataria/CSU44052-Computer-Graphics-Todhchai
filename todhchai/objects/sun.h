#ifndef SUN_H
#define SUN_H

#pragma once
#include <glad/gl.h>
#include <glm/gtc/matrix_transform.hpp>


struct Sun
{
    // Public members
    glm::vec3 position;
    glm::vec3 scale;

    // Constructor and Destructor
    Sun() : position(0.0f), scale(1.0f) {}
    ~Sun() {}

    // Public methods
    void initialize(glm::vec3 pos, glm::vec3 scl);
    void render(const glm::mat4& vp);
    void cleanup();

private:
    // OpenGL buffers
    GLuint vertexArrayID;
    GLuint vertexBufferID;
    GLuint indexBufferID;
    GLuint uvBufferID;
    GLuint textureID;

    // Shader variable IDs
    GLuint mvpMatrixID;
    GLuint textureSamplerID;
    GLuint programID;

    // Static data arrays
    static const GLfloat vertex_buffer_data[72];
    static const GLfloat color_buffer_data[72];
    static const GLuint index_buffer_data[36];
    static const GLfloat uv_buffer_data[48];
};

#endif // SUN_H
