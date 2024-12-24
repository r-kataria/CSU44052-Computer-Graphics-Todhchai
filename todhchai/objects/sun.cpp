#include "sun.h"
#include <glad/gl.h>
#include <glm/gtc/matrix_transform.hpp>
#include <render/shader.h> // your shader loader
#include <iostream>

// A simple quad (two triangles):
static float quadVertices[] = {
    //  position      uv
    -1.0f, -1.0f,    0.0f, 0.0f,
     1.0f, -1.0f,    1.0f, 0.0f,
     1.0f,  1.0f,    1.0f, 1.0f,
    -1.0f,  1.0f,    0.0f, 1.0f
};

static unsigned int quadIndices[] = {
    0, 1, 2,  0, 2, 3
};

void Sun::initialize() 
{
    // Hardcode sun position in world space. 
    // Or you can move it with time-of-day logic:
    sunPositionWS = glm::vec3(0, 100.0f, -200.0f);

    // 1) Create VAO
    glGenVertexArrays(1, &vertexArrayID);
    glBindVertexArray(vertexArrayID);

    // 2) Create VBO
    glGenBuffers(1, &vertexBufferID);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    // Positions at layout=0, UVs at layout=1
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)(2*sizeof(float)));

    // 3) Create EBO (index buffer)
    glGenBuffers(1, &indexBufferID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quadIndices), quadIndices, GL_STATIC_DRAW);

    // 4) Load your sun texture (a bright disc with glow)
    textureID = LoadTexture("assets/sun_texture.png"); // or your method

    // 5) Load/compile sun shaders (vertex + fragment)
    programID = LoadShadersFromFile(
        "shaders/sun_billboard.vert", 
        "shaders/sun_billboard.frag"
    );

    // Get uniforms
    mvpMatrixID        = glGetUniformLocation(programID, "MVP");
    sunPositionWS_ID   = glGetUniformLocation(programID, "sunPositionWS");
    textureSamplerID   = glGetUniformLocation(programID, "textureSampler");

    // Unbind
    glBindVertexArray(0);
}

void Sun::render(const glm::mat4& view, const glm::mat4& projection, const glm::vec3& cameraPos)
{
    glUseProgram(programID);
    glBindVertexArray(vertexArrayID);

    // --- Billboard math: place the quad near the sun’s position,
    //     but rotate it so it always faces the camera.

    // Let’s assume we want the sun disc to be rendered at some world position 
    // (sunPositionWS). The simplest is we do not rotate the billboard, we 
    // just move it far away in front of the camera if you prefer.
    //
    // For demonstration, we'll do something like this:

    glm::vec3 toCam = cameraPos - sunPositionWS;
    float distance  = glm::length(toCam);
    // Optionally, clamp or scale the distance so the sun disc is always visible.

    // Simple approach: Build a model matrix that translates to `sunPositionWS`
    // and scales the quad so it appears suitably sized from your camera.
    // Then we remove the rotation from the view matrix
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, sunPositionWS);

    // scale so the disc is big enough
    float size = 10.0f;  // or scale with distance
    model = glm::scale(model, glm::vec3(size, size, size));

    // optional: remove rotation from view if you want a “true” billboard
    // but a simpler trick is to just let the sun quad always face the camera 
    // by using a geometry pass that is always unrotated relative to camera.

    glm::mat4 mvp = projection * view * model;
    glUniformMatrix4fv(mvpMatrixID, 1, GL_FALSE, &mvp[0][0]);

    // Optional: pass sun position if you want lens flare or something
    glUniform3fv(sunPositionWS_ID, 1, &sunPositionWS[0]);

    // Bind sun texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glUniform1i(textureSamplerID, 0);

    // Draw the quad
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
}

void Sun::cleanup()
{
    glDeleteBuffers(1, &vertexBufferID);
    glDeleteBuffers(1, &uvBufferID);
    glDeleteBuffers(1, &indexBufferID);
    glDeleteVertexArrays(1, &vertexArrayID);
    glDeleteProgram(programID);
    glDeleteTextures(1, &textureID);
}
