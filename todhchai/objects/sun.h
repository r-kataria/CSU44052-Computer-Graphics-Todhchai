#ifndef SUN_H
#define SUN_H

#include <glm/glm.hpp>

class Sun {
public:
    void initialize();
    void render(const glm::mat4& view, const glm::mat4& projection, 
                const glm::vec3& cameraPos);
    void cleanup();

private:
    unsigned int programID;
    unsigned int vertexArrayID;
    unsigned int vertexBufferID;
    unsigned int uvBufferID;
    unsigned int indexBufferID;
    unsigned int textureID;

    unsigned int mvpMatrixID;
    unsigned int sunPositionWS_ID;  // Uniform for sun world-space position
    unsigned int textureSamplerID;

    glm::vec3 sunPositionWS; // The sun’s world-space position
};

#endif
