#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <learnopengl/shader.h>
#include <learnopengl/camera.h> // Ensure Camera is included
#include <string>
#include <vector>

class Sun {
public:
    // Constructors
    Sun(Shader& shader, glm::vec3 color = glm::vec3(1.0f), 
         glm::vec3 position = glm::vec3(0.0f), 
         glm::vec3 rotation = glm::vec3(0.0f), 
         glm::vec3 scale = glm::vec3(1.0f));

    // Destructor
    ~Sun();

    // Setters for transformations
    void SetPosition(const glm::vec3& position);
    void SetRotation(const glm::vec3& rotation);
    void SetScale(const glm::vec3& scale);

    // Render the Sun
    // Changed from const Camera& to Camera&
    void Render(Camera& camera, const std::vector<glm::vec3>& lightPositions, 
               const std::vector<glm::vec3>& lightColors);

private:
    // Render data
    unsigned int VAO, VBO;

    // Sun properties
    Shader& shader;
    glm::vec3 color;
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;

    // Initialization
    void InitRenderData();
};
