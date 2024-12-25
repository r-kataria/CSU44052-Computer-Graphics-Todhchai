#include "sun.h"
#include "../helpers/texture.h"         // for LoadTextureTileBox
#include <render/shader.h>              // your custom shader loader
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <cstdlib>

// Definition of static member arrays

const GLfloat Sun::vertex_buffer_data[72] = {
    // Front face
    -1.0f, -1.0f, 1.0f,
     1.0f, -1.0f, 1.0f,
     1.0f,  1.0f, 1.0f,
    -1.0f,  1.0f, 1.0f,

    // Back face
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,

    // Left face
    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    // Right face
     1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,

    // Top face
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    // Bottom face
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,
};

const GLuint Sun::index_buffer_data[36] = {
    0, 1, 2,   0, 2, 3,
    4, 5, 6,   4, 6, 7,
    8, 9, 10,  8, 10, 11,
    12, 13, 14, 12, 14, 15,
    16, 17, 18, 16, 18, 19,
    20, 21, 22, 20, 22, 23,
};

// Implementation of Sun methods

void Sun::initialize(glm::vec3 pos, glm::vec3 scl) {
    this->position = pos;
    this->scale    = scl;

    // Create and bind the VAO
    glGenVertexArrays(1, &vertexArrayID);
    glBindVertexArray(vertexArrayID);

    // --- Position buffer ---
    glGenBuffers(1, &vertexBufferID);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_buffer_data), vertex_buffer_data, GL_STATIC_DRAW);

    // Enable attribute 0 for vertex position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0,                  // Layout location = 0
        3,                  // 3 floats (x,y,z)
        GL_FLOAT,
        GL_FALSE,
        0,
        (void*)0
    );


    // --- Index buffer ---
    glGenBuffers(1, &indexBufferID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index_buffer_data), index_buffer_data, GL_STATIC_DRAW);

    // Load and compile shaders
    programID = LoadShadersFromFile("../todhchai/shaders/sun.vert", "../todhchai/shaders/sun.frag");
    if (programID == 0) {
        std::cerr << "Failed to load Sun shaders." << std::endl;
    }

    // Get uniform locations
    mvpMatrixID      = glGetUniformLocation(programID, "MVP");

    // Unbind VAO to prevent accidental modifications
    glBindVertexArray(0);
}

void Sun::render(const glm::mat4& vp) {
    glUseProgram(programID);
    glBindVertexArray(vertexArrayID);

    // Model transform
    glm::mat4 modelMatrix(1.0f);
    modelMatrix = glm::translate(modelMatrix, position);
    modelMatrix = glm::scale(modelMatrix, scale);

    glm::mat4 mvp = vp * modelMatrix;
    glUniformMatrix4fv(mvpMatrixID, 1, GL_FALSE, &mvp[0][0]);


    // Draw the cube using element array
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

    // Cleanup
    glBindVertexArray(0);
    glUseProgram(0);
}

void Sun::cleanup() {
    glDeleteBuffers(1, &vertexBufferID);
    glDeleteBuffers(1, &indexBufferID);
    glDeleteVertexArrays(1, &vertexArrayID);
    glDeleteProgram(programID);
}
