// ground.h
#ifndef GROUND_H
#define GROUND_H

#include <glad/gl.h>
#include <glm/glm.hpp>

class Ground {
public:
    void initialize(glm::vec3 pos, glm::vec3 scl, const char* texturePath);
    void render(const glm::mat4& vp);
    void cleanup();
private:
    GLuint vertexArrayID;
    GLuint vertexBufferID;
    GLuint uvBufferID;
    GLuint textureID;
    GLuint programID;
    GLuint mvpMatrixID;
    GLuint modelMatrixID;
    GLuint textureSamplerID;
    GLuint lightPosID;
    GLuint lightColorID;
    GLuint objectColorID;
    glm::vec3 position;
    glm::vec3 scale;
};

#endif // GROUND_H
