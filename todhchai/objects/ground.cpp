// ground.cpp
#include "ground.h"
#include "../helpers/texture.h" // For LoadTextureTileBox
#include "../render/shader.h"   // For LoadShadersFromFile
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

void Ground::initialize(glm::vec3 pos, glm::vec3 scl, const char* texturePath) {
    position = pos;
    scale = scl;

    GLfloat repeat = 10;

    // Create and bind the VAO
    glGenVertexArrays(1, &vertexArrayID);
    glBindVertexArray(vertexArrayID);

    // Define vertex data for a flat plane (two triangles)
    GLfloat vertex_buffer_data_with_normals[] = {
        // Positions          // Normals
        // First triangle
        -5.0f, 0.0f, -5.0f,   0.0f, 1.0f, 0.0f,
         5.0f, 0.0f, -5.0f,   0.0f, 1.0f, 0.0f,
         5.0f, 0.0f,  5.0f,   0.0f, 1.0f, 0.0f,
        // Second triangle
        -5.0f, 0.0f, -5.0f,   0.0f, 1.0f, 0.0f,
         5.0f, 0.0f,  5.0f,   0.0f, 1.0f, 0.0f,
        -5.0f, 0.0f,  5.0f,   0.0f, 1.0f, 0.0f,
    };

    // Load vertex buffer
    glGenBuffers(1, &vertexBufferID);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_buffer_data_with_normals), vertex_buffer_data_with_normals, GL_STATIC_DRAW);

    // Enable attribute 0 for vertex position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0,                  // Layout location = 0
        3,                  // 3 floats (x, y, z)
        GL_FLOAT,
        GL_FALSE,
        6 * sizeof(GLfloat),// Stride (positions + normals)
        (void*)0            // Offset
    );

    // Enable attribute 2 for vertex normals
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(
        2,                                  // Layout location = 2
        3,                                  // 3 floats (nx, ny, nz)
        GL_FLOAT,
        GL_FALSE,
        6 * sizeof(GLfloat),                // Stride
        (void*)(3 * sizeof(GLfloat))        // Offset
    );

    // Define UVs with tiling
    GLfloat uv_buffer_data[] = {
        // UV coordinates (tiling based on 'repeat')
        0.0f, 0.0f,
        repeat, 0.0f,
        repeat, repeat,
        0.0f, 0.0f,
        repeat, repeat,
        0.0f, repeat
    };

    // Load UV buffer
    glGenBuffers(1, &uvBufferID);
    glBindBuffer(GL_ARRAY_BUFFER, uvBufferID);
    glBufferData(GL_ARRAY_BUFFER, sizeof(uv_buffer_data), uv_buffer_data, GL_STATIC_DRAW);

    // Enable attribute 1 for UV
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        1,                  // Layout location = 1
        2,                  // 2 floats (u, v)
        GL_FLOAT,
        GL_FALSE,
        0,
        (void*)0
    );

    // Load and compile shaders (reuse existing shaders)
    programID = LoadShadersFromFile("../todhchai/shaders/box.vert", "../todhchai/shaders/box.frag");
    if (programID == 0) {
        std::cerr << "Failed to load ground shaders." << std::endl;
    }

    // Get uniform locations
    mvpMatrixID      = glGetUniformLocation(programID, "MVP");
    modelMatrixID    = glGetUniformLocation(programID, "Model");
    textureSamplerID = glGetUniformLocation(programID, "textureSampler");
    lightPosID       = glGetUniformLocation(programID, "lightPos");
    lightColorID     = glGetUniformLocation(programID, "lightColor");
    objectColorID    = glGetUniformLocation(programID, "objectColor");

    // Load texture
    textureID = LoadTextureTileBox(texturePath, true, true);

    // Unbind VAO
    glBindVertexArray(0);
}

void Ground::render(const glm::mat4& vp) {
    glUseProgram(programID);
    glBindVertexArray(vertexArrayID);

    // Model transformation
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, position);
    modelMatrix = glm::scale(modelMatrix, scale);

    // Calculate MVP matrix
    glm::mat4 mvp = vp * modelMatrix;

    // Set uniform values
    glUniformMatrix4fv(mvpMatrixID, 1, GL_FALSE, &mvp[0][0]);
    glUniformMatrix4fv(modelMatrixID, 1, GL_FALSE, &modelMatrix[0][0]);

    // Set light properties
    glm::vec3 lightPos(0.0f, 4.0f, 0.0f); // Fixed light position
    glm::vec3 lightColor(1.0f, 1.0f, 1.0f); // White light
    glm::vec3 objectColor(1.0f, 1.0f, 1.0f); // Base color

    glUniform3fv(lightPosID, 1, &lightPos[0]);
    glUniform3fv(lightColorID, 1, &lightColor[0]);
    glUniform3fv(objectColorID, 1, &objectColor[0]);

    // Bind texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glUniform1i(textureSamplerID, 0);

    // Draw the plane (6 vertices)
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // Cleanup
    glBindVertexArray(0);
    glUseProgram(0);
}

void Ground::cleanup() {
    glDeleteBuffers(1, &vertexBufferID);
    glDeleteBuffers(1, &uvBufferID);
    glDeleteVertexArrays(1, &vertexArrayID);
    glDeleteTextures(1, &textureID);
    glDeleteProgram(programID);
}
