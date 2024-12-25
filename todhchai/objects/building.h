// building.h
#ifndef BUILDING_H
#define BUILDING_H

#include <glm/glm.hpp>
#include <glad/gl.h>

class Building {
public:
    void initialize(glm::vec3 pos, glm::vec3 scl);
    void initialize(glm::vec3 pos, glm::vec3 scl, const char* texturePath);

    void render(const glm::mat4& vp, glm::vec3 lightPos, glm::vec3 viewPos);
    void cleanup();

private:
    glm::vec3 position;
    glm::vec3 scale;

    GLuint vertexArrayID;
    GLuint vertexBufferID;
    GLuint uvBufferID;
    GLuint indexBufferID;
    GLuint textureID;

    GLuint programID;
    GLuint mvpMatrixID;
    GLuint modelMatrixID;      // New
    GLuint textureSamplerID;   // New
    GLuint lightPosID;         // New
    GLuint viewPosID;         // New
    GLuint lightColorID;       // New
    GLuint objectColorID;      // New
};

#endif // BUILDING_H
