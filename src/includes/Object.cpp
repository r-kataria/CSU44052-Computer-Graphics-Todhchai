#include "Object.h"

// Forward-declare any globals you need from your main app if you follow
// the same pattern as your Cube.cpp (e.g., extern int SCR_WIDTH, SCR_HEIGHT; etc.)
extern int SCR_WIDTH;
extern int SCR_HEIGHT;
extern float far_plane;

// Constructor
Object::Object(Shader& shader,
               const std::string& modelPath,
               const glm::vec3& position,
               const glm::vec3& rotation,
               const glm::vec3& scale)
    : m_Shader(shader),
      m_Model(modelPath),   // Load the model once
      m_Position(position),
      m_Rotation(rotation),
      m_Scale(scale)
{
    // If you need to flip textures or do other one-time config, do it here
    // e.g.: stbi_set_flip_vertically_on_load(true);
}

// Destructor
Object::~Object()
{
    // Typically nothing special to do here,
    // Model's destructor and GPU buffers are handled automatically
}

// Setters
void Object::SetPosition(const glm::vec3& pos)
{
    m_Position = pos;
}

void Object::SetRotation(const glm::vec3& rot)
{
    m_Rotation = rot;
}

void Object::SetScale(const glm::vec3& scl)
{
    m_Scale = scl;
}

/**
 * Renders the object normally, with your main scene shader that includes
 * lighting and possibly shadows (you'll bind shadow maps externally).
 */
void Object::Render(Camera& camera,
                    const std::vector<glm::vec3>& lightPositions,
                    const std::vector<glm::vec3>& lightColors)
{
    // 1. Activate the shader
    m_Shader.use();

    // 2. Compute view/projection
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom),
                                            (float)SCR_WIDTH / (float)SCR_HEIGHT,
                                            0.1f, far_plane);
    glm::mat4 view = camera.GetViewMatrix();

    m_Shader.setMat4("projection", projection);
    m_Shader.setMat4("view", view);

    // 3. Compute the model transform
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, m_Position);
    model = glm::rotate(model, glm::radians(m_Rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, glm::radians(m_Rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, glm::radians(m_Rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::scale(model, m_Scale);

    m_Shader.setMat4("model", model);

    // 4. Update lighting uniforms (mimic what you did in your Cube class).
    //    If you have an array of Lights in your shader, update them here:
    for (unsigned int i = 0; i < lightPositions.size(); i++)
    {
        m_Shader.setVec3("lights[" + std::to_string(i) + "].Position", lightPositions[i]);
        m_Shader.setVec3("lights[" + std::to_string(i) + "].Color",    lightColors[i]);
    }
    
    // View position, and far_plane if your fragment shader uses it
    m_Shader.setVec3("viewPos", camera.Position);
    m_Shader.setFloat("far_plane", far_plane); // relevant for point shadows

    // 5. Draw the model
    m_Model.Draw(m_Shader);
}

/**
 * Renders the object to a depth buffer for shadow mapping
 * (similar to your Cube::RenderDepth). 
 * Typically used with a simpler depth-only shader.
 */
void Object::RenderDepth(Shader& depthShader)
{
    depthShader.use();

    // Compute the model matrix with the same position/rotation/scale
    glm::mat4 modelMat = glm::mat4(1.0f);
    modelMat = glm::translate(modelMat, m_Position);
    modelMat = glm::rotate(modelMat, glm::radians(m_Rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    modelMat = glm::rotate(modelMat, glm::radians(m_Rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    modelMat = glm::rotate(modelMat, glm::radians(m_Rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
    modelMat = glm::scale(modelMat, m_Scale);

    depthShader.setMat4("model", modelMat);

    // If your depth shader also needs other uniforms (like "far_plane", "lightPos", etc.), set them too

    // Draw the model with the depth shader
    m_Model.Draw(depthShader);
}
