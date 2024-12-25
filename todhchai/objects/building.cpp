#include "building.h"
#include "../helpers/texture.h"         // for LoadTextureTileBox
#include <render/shader.h>              // your custom shader loader
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <cstdlib>



// Example: Adding normals for each face of the cube
GLfloat vertex_buffer_data_with_normals[] = {
    // Positions          // Normals
    // Front face
    -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f,
     1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f,
     1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f,

    // Back face
     1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f,
     1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f,

    // Left face
    -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f,
    -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f,
    -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f,
    -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f,

    // Right face
     1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,
     1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,
     1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f,
     1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f,

    // Top face
    -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f,
     1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f,
     1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f,
    -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f,

    // Bottom face
    -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f,
     1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f,
     1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f,
    -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f,
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


// Existing vertex_buffer_data_with_normals and index_buffer_data remain unchanged

void Building::initialize(glm::vec3 pos, glm::vec3 scl, const char* texturePath) {
    this->position = pos;
    this->scale    = scl;

    // Create and bind the VAO
    glGenVertexArrays(1, &vertexArrayID);
    glBindVertexArray(vertexArrayID);

    // --- Position & Normal buffer ---
    glGenBuffers(1, &vertexBufferID);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_buffer_data_with_normals), vertex_buffer_data_with_normals, GL_STATIC_DRAW);

    // Enable attribute 0 for vertex position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0,                  // Layout location = 0
        3,                  // 3 floats (x,y,z)
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

    // --- Dynamic UV buffer ---
    GLfloat dynamic_uv_buffer_data[48]; // 24 vertices * 2 UVs

    // Front face UVs (U: x, V: y)
    dynamic_uv_buffer_data[0]  = 0.0f;
    dynamic_uv_buffer_data[1]  = 0.0f;
    dynamic_uv_buffer_data[2]  = 2.0f * scl.x; // Repeat texture every 1 unit
    dynamic_uv_buffer_data[3]  = 0.0f;
    dynamic_uv_buffer_data[4]  = 2.0f * scl.x;
    dynamic_uv_buffer_data[5]  = 2.0f * scl.y;
    dynamic_uv_buffer_data[6]  = 0.0f;
    dynamic_uv_buffer_data[7]  = 2.0f * scl.y;

    // Back face UVs (U: x, V: y)
    dynamic_uv_buffer_data[8]  = 0.0f;
    dynamic_uv_buffer_data[9]  = 0.0f;
    dynamic_uv_buffer_data[10] = 2.0f * scl.x;
    dynamic_uv_buffer_data[11] = 0.0f;
    dynamic_uv_buffer_data[12] = 2.0f * scl.x;
    dynamic_uv_buffer_data[13] = 2.0f * scl.y;
    dynamic_uv_buffer_data[14] = 0.0f;
    dynamic_uv_buffer_data[15] = 2.0f * scl.y;

    // Left face UVs (U: z, V: y)
    dynamic_uv_buffer_data[16] = 0.0f;
    dynamic_uv_buffer_data[17] = 0.0f;
    dynamic_uv_buffer_data[18] = 2.0f * scl.z;
    dynamic_uv_buffer_data[19] = 0.0f;
    dynamic_uv_buffer_data[20] = 2.0f * scl.z;
    dynamic_uv_buffer_data[21] = 2.0f * scl.y;
    dynamic_uv_buffer_data[22] = 0.0f;
    dynamic_uv_buffer_data[23] = 2.0f * scl.y;

    // Right face UVs (U: z, V: y)
    dynamic_uv_buffer_data[24] = 0.0f;
    dynamic_uv_buffer_data[25] = 0.0f;
    dynamic_uv_buffer_data[26] = 2.0f * scl.z;
    dynamic_uv_buffer_data[27] = 0.0f;
    dynamic_uv_buffer_data[28] = 2.0f * scl.z;
    dynamic_uv_buffer_data[29] = 2.0f * scl.y;
    dynamic_uv_buffer_data[30] = 0.0f;
    dynamic_uv_buffer_data[31] = 2.0f * scl.y;

    // Top face UVs (U: x, V: z)
    dynamic_uv_buffer_data[32] = 0.0f;
    dynamic_uv_buffer_data[33] = 0.0f;
    dynamic_uv_buffer_data[34] = 2.0f * scl.x; // Adjust based on X scale
    dynamic_uv_buffer_data[35] = 0.0f;
    dynamic_uv_buffer_data[36] = 2.0f * scl.x;
    dynamic_uv_buffer_data[37] = 2.0f * scl.z; // Adjust based on Z scale
    dynamic_uv_buffer_data[38] = 0.0f;
    dynamic_uv_buffer_data[39] = 2.0f * scl.z;

    // Bottom face UVs (U: x, V: z)
    dynamic_uv_buffer_data[40] = 0.0f;
    dynamic_uv_buffer_data[41] = 0.0f;
    dynamic_uv_buffer_data[42] = 2.0f * scl.x; // Adjust based on X scale
    dynamic_uv_buffer_data[43] = 0.0f;
    dynamic_uv_buffer_data[44] = 2.0f * scl.x;
    dynamic_uv_buffer_data[45] = 2.0f * scl.z; // Adjust based on Z scale
    dynamic_uv_buffer_data[46] = 0.0f;
    dynamic_uv_buffer_data[47] = 2.0f * scl.z;

    // Load UV buffer
    glGenBuffers(1, &uvBufferID);
    glBindBuffer(GL_ARRAY_BUFFER, uvBufferID);
    glBufferData(GL_ARRAY_BUFFER, sizeof(dynamic_uv_buffer_data), dynamic_uv_buffer_data, GL_STATIC_DRAW);

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
    mvpMatrixID      = glGetUniformLocation(programID, "MVP");
    modelMatrixID    = glGetUniformLocation(programID, "Model");
    textureSamplerID = glGetUniformLocation(programID, "textureSampler");
    lightPosID       = glGetUniformLocation(programID, "lightPos");
    viewPosID       = glGetUniformLocation(programID, "viewPos");
    lightColorID     = glGetUniformLocation(programID, "lightColor");
    objectColorID    = glGetUniformLocation(programID, "objectColor");

    // Pick a random texture
    textureID = LoadTextureTileBox(texturePath, false, true);
    if (textureID == 0) {
        std::cerr << "Error: Failed to load texture at path: " << texturePath << std::endl;
    }

    glBindVertexArray(0);
}




void Building::render(const glm::mat4& vp, glm::vec3 lightPos, glm::vec3 viewPos) {
    if (programID == 0 || textureID == 0) {
        std::cerr << "Error: Shader program or texture not initialized." << std::endl;
        return;
    }

    glUseProgram(programID);
    glBindVertexArray(vertexArrayID);

    // Model transform
    glm::mat4 modelMatrix(1.0f);
    modelMatrix = glm::translate(modelMatrix, position);
    modelMatrix = glm::scale(modelMatrix, scale);

    // Calculate MVP matrix
    glm::mat4 mvp = vp * modelMatrix;

    // Set uniform values
    glUniformMatrix4fv(mvpMatrixID, 1, GL_FALSE, &mvp[0][0]);
    glUniformMatrix4fv(modelMatrixID, 1, GL_FALSE, &modelMatrix[0][0]);

    // 6. Set light/uniforms
    glm::vec3 lightColor(1.0f, 1.0f, 1.0f);
    glm::vec3 objectColor(1.0f, 1.0f, 1.0f);

    glUniform3fv(lightPosID, 1, &lightPos[0]);
    glUniform3fv(viewPosID, 1, &viewPos[0]);
    glUniform3fv(lightColorID, 1, &lightColor[0]);
    glUniform3fv(objectColorID, 1, &objectColor[0]);

    // Bind texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glUniform1i(textureSamplerID, 0);

    // Draw the cube using element array
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

    // Cleanup
    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Building::cleanup() {
    if (vertexBufferID != 0)
        glDeleteBuffers(1, &vertexBufferID);
    if (uvBufferID != 0)
        glDeleteBuffers(1, &uvBufferID);
    if (indexBufferID != 0)
        glDeleteBuffers(1, &indexBufferID);
    if (vertexArrayID != 0)
        glDeleteVertexArrays(1, &vertexArrayID);
    if (textureID != 0)
        glDeleteTextures(1, &textureID);
    if (programID != 0)
        glDeleteProgram(programID);
}
