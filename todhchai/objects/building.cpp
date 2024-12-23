#include "building.h"
#include "../helpers/texture.h"         // for LoadTextureTileBox
#include <render/shader.h>   // your custom shader loader
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <cstdlib>



GLfloat vertex_buffer_data[72] = {
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

GLfloat color_buffer_data[72] = {
    // Front, red
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,

    // Back, yellow
    1.0f, 1.0f, 0.0f,
    1.0f, 1.0f, 0.0f,
    1.0f, 1.0f, 0.0f,
    1.0f, 1.0f, 0.0f,

    // Left, green
    0.0f, 1.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
    0.0f, 1.0f, 0.0f,

    // Right, cyan
    0.0f, 1.0f, 1.0f,
    0.0f, 1.0f, 1.0f,
    0.0f, 1.0f, 1.0f,
    0.0f, 1.0f, 1.0f,

    // Top, blue
    0.0f, 0.0f, 1.0f,
    0.0f, 0.0f, 1.0f,
    0.0f, 0.0f, 1.0f,
    0.0f, 0.0f, 1.0f,

    // Bottom, magenta
    1.0f, 0.0f, 1.0f,
    1.0f, 0.0f, 1.0f,
    1.0f, 0.0f, 1.0f,
    1.0f, 0.0f, 1.0f,
};

GLuint index_buffer_data[36] = {
    0, 1, 2,   0, 2, 3,
    4, 5, 6,   4, 6, 7,
    8, 9, 10,  8, 10, 11,
    12, 13, 14, 12, 14, 15,
    16, 17, 18, 16, 18, 19,
    20, 21, 22, 20, 22, 23,
};

// UV: front/back/left/right get real UVs. top/bottom are zero so no texture.
GLfloat uv_buffer_data[48] = {
    // Front
    0.0f, 1.0f,
    1.0f, 1.0f,
    1.0f, 0.0f,
    0.0f, 0.0f,

    // Back
    0.0f, 1.0f,
    1.0f, 1.0f,
    1.0f, 0.0f,
    0.0f, 0.0f,

    // Left
    0.0f, 1.0f,
    1.0f, 1.0f,
    1.0f, 0.0f,
    0.0f, 0.0f,

    // Right
    0.0f, 1.0f,
    1.0f, 1.0f,
    1.0f, 0.0f,
    0.0f, 0.0f,

    // Top
    0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    // Bottom
    0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
};

void Building::initialize(glm::vec3 pos, glm::vec3 scl) {
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

// --- UV buffer ---
glGenBuffers(1, &uvBufferID);
glBindBuffer(GL_ARRAY_BUFFER, uvBufferID);
glBufferData(GL_ARRAY_BUFFER, sizeof(uv_buffer_data), uv_buffer_data, GL_STATIC_DRAW);

// Enable attribute 1 for UV
glEnableVertexAttribArray(1);
glVertexAttribPointer(
    1,                  // Layout location = 1
    2,                  // 2 floats (u,v)
    GL_FLOAT,
    GL_FALSE,
    0,
    (void*)0
);

// (No color buffer at all)

// --- Index buffer ---
glGenBuffers(1, &indexBufferID);
glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);
glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index_buffer_data), index_buffer_data, GL_STATIC_DRAW);


    // Load and compile shaders
    programID = LoadShadersFromFile("../todhchai/shaders/box.vert", "../todhchai/shaders/box.frag");
    if (programID == 0) {
        std::cerr << "Failed to load building shaders." << std::endl;
    }

    // Get uniform locations
    mvpMatrixID     = glGetUniformLocation(programID, "MVP");
    textureSamplerID= glGetUniformLocation(programID, "textureSampler");

    // Define array of facade textures
    const char* facadeTextures[] = {
        "../assets/facade0.jpg",
        "../assets/facade1.jpg",
        "../assets/facade2.jpg",
        "../assets/facade3.jpg",
        "../assets/facade4.jpg",
        "../assets/facade5.jpg"
    };

    // Pick a random texture
textureID = LoadTextureTileBox(facadeTextures[rand() % 6], true);

glBindVertexArray(0);
}

void Building::render(const glm::mat4& vp) {
    glUseProgram(programID);
    glBindVertexArray(vertexArrayID);


    // Model transform
    glm::mat4 modelMatrix(1.0f);
    modelMatrix = glm::translate(modelMatrix, position);
    modelMatrix = glm::scale(modelMatrix, scale);

    glm::mat4 mvp = vp * modelMatrix;
    glUniformMatrix4fv(mvpMatrixID, 1, GL_FALSE, &mvp[0][0]);

    // Bind texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glUniform1i(textureSamplerID, 0);

    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

    // Cleanup
    glBindVertexArray(0);
}

void Building::cleanup() {
    glDeleteBuffers(1, &vertexBufferID);
    //glDeleteBuffers(1, &colorBufferID);
    glDeleteBuffers(1, &indexBufferID);
    glDeleteBuffers(1, &uvBufferID);
    glDeleteVertexArrays(1, &vertexArrayID);
    // glDeleteTextures(1, &textureID); // If you want to free texture as well
    glDeleteProgram(programID);
}
