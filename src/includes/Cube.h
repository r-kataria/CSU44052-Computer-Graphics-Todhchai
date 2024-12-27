// Cube.h
#ifndef CUBE_H
#define CUBE_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <learnopengl/shader.h>
#include <learnopengl/camera.h>

class Cube
{
public:
    // Constructor
    Cube(Shader& shader, unsigned int texture, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale);

    // Destructor
    ~Cube();

    // Setters
    void SetPosition(const glm::vec3& pos);
    void SetRotation(const glm::vec3& rot);
    void SetScale(const glm::vec3& scl);

    // Render methods
    void Render(Camera& camera, const std::vector<glm::vec3>& lightPositions, 
                const std::vector<glm::vec3>& lightColors);

    // Add this function
    void RenderDepth(Shader& depthShader);

private:
    // Render data
    unsigned int VAO, VBO;

    // Shader and texture
    Shader& shader;
    unsigned int texture;

    // Transformations
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;

    // Initializes the cube's buffer and vertex attributes
    void InitRenderData();
};

#endif