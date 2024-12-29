#include "AnimatedObject.h"

// Suppose we rely on these externs as you do for Object:
extern int SCR_WIDTH;
extern int SCR_HEIGHT;
extern float far_plane;

AnimatedObject::AnimatedObject(
    Shader&           shader,
    const std::string modelPath,
    const glm::vec3&  position,
    const glm::vec3&  rotation,
    const glm::vec3&  scale
)
    : m_Shader(shader)
    , m_Model(nullptr)
    , m_Animation(nullptr)
    , m_Animator(nullptr)
    , m_Position(position)
    , m_Rotation(rotation)
    , m_Scale(scale)
{
    // Load the bone-capable Model
    m_Model = new Model(modelPath);

    // Create the Animation and Animator from the same path
    // (You could pass a separate .dae for animation if desired.)
    m_Animation = new Animation(modelPath, m_Model);
    m_Animator  = new Animator(m_Animation);
}

AnimatedObject::~AnimatedObject()
{
    // Clean up dynamically allocated data
    if (m_Animator)   delete m_Animator;
    if (m_Animation)  delete m_Animation;
    if (m_Model)      delete m_Model;
}

void AnimatedObject::SetPosition(const glm::vec3& pos)
{
    m_Position = pos;
}

void AnimatedObject::SetRotation(const glm::vec3& rot)
{
    m_Rotation = rot;
}

void AnimatedObject::SetScale(const glm::vec3& scl)
{
    m_Scale = scl;
}

// Optionally call in your Scene::Update to step animation with deltaTime
void AnimatedObject::Update(float dt)
{
    if (m_Animator)
        m_Animator->UpdateAnimation(dt);
}

void AnimatedObject::Render(Camera& camera,
                            const std::vector<glm::vec3>& lightPositions,
                            const std::vector<glm::vec3>& lightColors)
{
    // 1) Use the assigned shader
    m_Shader.use();

    // 2) Camera projection & view (like your Object)
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom),
                                            (float)SCR_WIDTH / (float)SCR_HEIGHT,
                                            0.1f, far_plane);
    glm::mat4 view = camera.GetViewMatrix();
    m_Shader.setMat4("projection", projection);
    m_Shader.setMat4("view",       view);

    // 3) Model matrix
    glm::mat4 modelMat(1.0f);
    modelMat = glm::translate(modelMat, m_Position);

    modelMat = glm::rotate(modelMat, glm::radians(m_Rotation.x), glm::vec3(1,0,0));
    modelMat = glm::rotate(modelMat, glm::radians(m_Rotation.y), glm::vec3(0,1,0));
    modelMat = glm::rotate(modelMat, glm::radians(m_Rotation.z), glm::vec3(0,0,1));

    modelMat = glm::scale(modelMat, m_Scale);
    m_Shader.setMat4("model", modelMat);

    // 4) Update lighting uniforms (like your Object)
    for (unsigned int i = 0; i < lightPositions.size(); i++)
    {
        m_Shader.setVec3("lights[" + std::to_string(i) + "].Position", lightPositions[i]);
        m_Shader.setVec3("lights[" + std::to_string(i) + "].Color",    lightColors[i]);
    }
    m_Shader.setVec3("viewPos", camera.Position);
    m_Shader.setFloat("far_plane", far_plane);

    // 5) Pass bone transforms from m_Animator
    if (m_Animator)
    {
        auto finalBones = m_Animator->GetFinalBoneMatrices();
        for (int i = 0; i < (int)finalBones.size(); i++)
        {
            std::string uniformName = "finalBonesMatrices[" + std::to_string(i) + "]";
            m_Shader.setMat4(uniformName.c_str(), finalBones[i]);
        }
    }

    // 6) Draw the model
    if (m_Model)
        m_Model->Draw(m_Shader);
}

void AnimatedObject::RenderDepth(Shader& depthShader)
{
    // If you want the animated object to cast shadows in a depth pass,
    // do the same approach, possibly with a skeletal depth shader:
    depthShader.use();

    // Build model matrix
    glm::mat4 modelMat(1.0f);
    modelMat = glm::translate(modelMat, m_Position);

    modelMat = glm::rotate(modelMat, glm::radians(m_Rotation.x), glm::vec3(1,0,0));
    modelMat = glm::rotate(modelMat, glm::radians(m_Rotation.y), glm::vec3(0,1,0));
    modelMat = glm::rotate(modelMat, glm::radians(m_Rotation.z), glm::vec3(0,0,1));

    modelMat = glm::scale(modelMat, m_Scale);
    depthShader.setMat4("model", modelMat);

    // If your depth shader also has finalBonesMatrices[], set them here similarly:
    // if (m_Animator)
    // {
    //     auto finalBones = m_Animator->GetFinalBoneMatrices();
    //     for (int i = 0; i < (int)finalBones.size(); i++)
    //     {
    //         std::string uniformName = "finalBonesMatrices[" + std::to_string(i) + "]";
    //         depthShader.setMat4(uniformName.c_str(), finalBones[i]);
    //     }
    // }

    if (m_Model)
        m_Model->Draw(depthShader);
}
