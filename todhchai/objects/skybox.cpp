#include "skybox.h"
#include "../helpers/texture.h"          // For LoadTextureTileBox
#include <render/shader.h>    // For your custom LoadShadersFromFile
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>


#include <glad/gl.h>
#include <GLFW/glfw3.h>

GLfloat skybox_vertex_buffer_data[72] = {
    // Front face
    -1.0f, -1.0f,  1.0f,
        1.0f, -1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,

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

GLuint skybox_index_buffer_data[36] = {
    0,1,2,   0,2,3,    // front
    4,5,6,   4,6,7,    // back
    8,9,10,  8,10,11,  // left
    12,13,14,12,14,15, // right
    16,17,18,16,18,19, // top
    20,21,22,20,22,23  // bottom
};

// Typical cross layout for a single texture approach:
GLfloat skybox_uv_buffer_data[48] = {
    // Front face  (Column 1, Row 1)
    0.25f, 0.33333f,
    0.50f, 0.33333f,
    0.50f, 0.66667f,
    0.25f, 0.66667f,

    // Back face   (Column 3, Row 1)
    0.75f, 0.33333f,
    1.00f, 0.33333f,
    1.00f, 0.66667f,
    0.75f, 0.66667f,

    // Left face   (Column 0, Row 1)
    0.00f, 0.33333f,
    0.25f, 0.33333f,
    0.25f, 0.66667f,
    0.00f, 0.66667f,

    // Right face  (Column 2, Row 1)
    0.50f, 0.33333f,
    0.75f, 0.33333f,
    0.75f, 0.66667f,
    0.50f, 0.66667f,

    // Top face    (Column 1, Row 0)
    0.25f, 0.66667f,
    0.50f, 0.66667f,
    0.50f, 1.00000f,
    0.25f, 1.00000f,

    // Bottom face (Column 1, Row 2)
    0.25f, 0.00f,
    0.50f, 0.00f,
    0.50f, 0.33333f,
    0.25f, 0.33333f,
};

// initialization
void Skybox::initialize(const glm::vec3& pos, const glm::vec3& scl)
{
    position = pos;
    scale    = scl;

    // Create VAO
    glGenVertexArrays(1, &vertexArrayID);
    glBindVertexArray(vertexArrayID);

    // Vertex buffer
    glGenBuffers(1, &vertexBufferID);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(skybox_vertex_buffer_data),
                 skybox_vertex_buffer_data,
                 GL_STATIC_DRAW);

    // UV buffer
    glGenBuffers(1, &uvBufferID);
    glBindBuffer(GL_ARRAY_BUFFER, uvBufferID);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(skybox_uv_buffer_data),
                 skybox_uv_buffer_data,
                 GL_STATIC_DRAW);

    // Index buffer
    glGenBuffers(1, &indexBufferID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 sizeof(skybox_index_buffer_data),
                 skybox_index_buffer_data,
                 GL_STATIC_DRAW);

    // Load/compile skybox shaders
    programID = LoadShadersFromFile("../todhchai/shaders/skybox.vert",
                                    "../todhchai/shaders/skybox.frag");
    if (programID == 0) {
        std::cerr << "Failed to load skybox shaders." << std::endl;
    }

    // Get uniform locations
    mvpMatrixID      = glGetUniformLocation(programID, "MVP");
    textureSamplerID = glGetUniformLocation(programID, "textureSampler");

    // Load all skybox textures
    TextureIds[0] = LoadTextureTileBox("../assets/sky.png", false);
    TextureIds[1] = LoadTextureTileBox("../assets/sky_debug.png", false);
    TextureIds[2] = LoadTextureTileBox("../assets/studio_garden.png", false);
    TextureIds[3] = LoadTextureTileBox("../assets/studio_garden_debug.png", false);

    // Make sure we set the sampler to texture unit 0
    glUseProgram(programID);
    glUniform1i(textureSamplerID, 0);

    glBindVertexArray(0);
}

// render
void Skybox::render(const glm::mat4& vp, int flip)
{
    // Disable depth writes so skybox doesn't obscure everything
    glDepthMask(GL_FALSE);

    // Use the skybox shader
    glUseProgram(programID);
    glBindVertexArray(vertexArrayID);

    // Positions
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    // UVs
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, uvBufferID);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);

    // Model transform (often identity for skybox, or a big scale)
    glm::mat4 model = glm::translate(glm::mat4(1.0f), position);
    model = glm::scale(model, scale);

    // MVP
    glm::mat4 mvp = vp * model;
    glUniformMatrix4fv(mvpMatrixID, 1, GL_FALSE, &mvp[0][0]);

    // Flip uniform in the fragment shader (if needed)
    glUniform1i(glGetUniformLocation(programID, "Flip"), flip);

    // Bind the chosen texture (e.g. from a global or local variable)
    extern int UsedTextureIndex; // or if you have it declared in some global .h
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, TextureIds[UsedTextureIndex]);

    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

    // Cleanup
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(2);
    glBindVertexArray(0);

    // Re-enable depth writes
    glDepthMask(GL_TRUE);
}

// cleanup
void Skybox::cleanup()
{
    glDeleteBuffers(1, &vertexBufferID);
    glDeleteBuffers(1, &uvBufferID);
    glDeleteBuffers(1, &indexBufferID);
    glDeleteVertexArrays(1, &vertexArrayID);

    glDeleteProgram(programID);
}
