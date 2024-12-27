#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <learnopengl/shader.h>
#include <learnopengl/camera.h> // Ensure Camera is included
#include <string>
#include <vector>

class Cube {
public:
    // Constructors
    Cube(Shader& shader, unsigned int texture, 
         glm::vec3 position = glm::vec3(0.0f), 
         glm::vec3 rotation = glm::vec3(0.0f), 
         glm::vec3 scale = glm::vec3(1.0f));

    // Destructor
    ~Cube();

    // Setters for transformations
    void SetPosition(const glm::vec3& position);
    void SetRotation(const glm::vec3& rotation);
    void SetScale(const glm::vec3& scale);

    // Render the cube
    // Changed from const Camera& to Camera&
    void Render(Camera& camera, const std::vector<glm::vec3>& lightPositions, 
               const std::vector<glm::vec3>& lightColors);

private:
    // Render data
    unsigned int VAO, VBO;

    // Cube properties
    Shader& shader;
    unsigned int texture;
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;

    // Initialization
    void InitRenderData();
};
