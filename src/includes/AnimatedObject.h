#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include <vector>

#include <learnopengl/shader_m.h>           // or #include <learnopengl/shader.h> if appropriate
#include <learnopengl/camera.h>
#include <learnopengl/model_animation.h>    // bone-capable model
#include <learnopengl/animator.h>

/**
 * AnimatedObject: parallels your "Object" class,
 * but holds Model, Animation, Animator for skeletal animation.
 */
class AnimatedObject
{
public:
    AnimatedObject(
        Shader&           shader,
        const std::string modelPath,
        const glm::vec3&  position  = glm::vec3(0.f),
        const glm::vec3&  rotation  = glm::vec3(0.f),
        const glm::vec3&  scale     = glm::vec3(1.f)
    );

    ~AnimatedObject();

    // Transform setters, parallel to Object
    void SetPosition(const glm::vec3& pos);
    void SetRotation(const glm::vec3& rot);
    void SetScale   (const glm::vec3& scl);

    // (Optional) We add Update so we can do m_Animator->UpdateAnimation(dt)
    void Update(float dt);

    // Render the animated model with the given camera & lights
    void Render(Camera& camera,
                const std::vector<glm::vec3>& lightPositions,
                const std::vector<glm::vec3>& lightColors);

    // Render a depth-only pass if you want the animated model to cast shadows
    void RenderDepth(Shader& depthShader);

private:
    Shader&       m_Shader;      // The render shader to use (like your Object uses m_Shader)
    Model*        m_Model;       // The bone-capable model
    Animation*    m_Animation;   // The animation data
    Animator*     m_Animator;    // Updates bone transforms each frame

    // Transforms
    glm::vec3     m_Position;
    glm::vec3     m_Rotation;
    glm::vec3     m_Scale;
};

