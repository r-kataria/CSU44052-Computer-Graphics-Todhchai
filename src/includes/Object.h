#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include <vector>

#include <learnopengl/shader_m.h>
#include <learnopengl/camera.h>
#include "model.h"


/**
 * Object class that loads and renders a 3D model using Assimp + the LearnOpenGL Model class.
 * 
 * Similar to the Cube class, but instead of manually specifying vertices,
 * we rely on the Model class to manage geometry and textures.
 */
class Object
{
public:
    // Constructor: accepts the shader that will be used to render,
    // the path to the model file, and optional transforms.
    Object(Shader& shader,
           const std::string& modelPath,
           const glm::vec3& position  = glm::vec3(0.0f),
           const glm::vec3& rotation  = glm::vec3(0.0f),
           const glm::vec3& scale     = glm::vec3(1.0f));

    // Destructor
    ~Object();

    // Setters for transformation
    void SetPosition(const glm::vec3& pos);
    void SetRotation(const glm::vec3& rot);
    void SetScale   (const glm::vec3& scl);

    // Render the model with lighting
    void Render(Camera& camera,
                const std::vector<glm::vec3>& lightPositions,
                const std::vector<glm::vec3>& lightColors);

    // Render only the depth information (for shadow mapping)
    void RenderDepth(Shader& depthShader);

private:
    // Reference to the shader used for normal drawing
    Shader&       m_Shader;

    // The loaded model (via Assimp / Model from LearnOpenGL)
    Model         m_Model;

    // Transform data
    glm::vec3     m_Position;
    glm::vec3     m_Rotation;
    glm::vec3     m_Scale;
};

