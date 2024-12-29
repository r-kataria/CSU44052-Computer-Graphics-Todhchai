#pragma once
#include <glm/glm.hpp>
#include <learnopengl/shader.h>
#include <learnopengl/camera.h>
#include <vector>
#include <string>

// Forward-declare classes you reference but define in .cpp
class Sun;
class Cube;
class Object;

/**
 * BaseScene
 * 
 * The abstract base that each derived scene (ParkScene, TowerScene, StructureScene) inherits from.
 */
class BaseScene
{
public:
    virtual ~BaseScene() = default;

    virtual void Init(Shader& shaderLight, Shader& shader) = 0;
    virtual void Update(float dt) = 0;
    virtual void RenderDepth(Shader &depthShader) = 0;
    virtual void Render(Shader &mainShader, Camera &camera) = 0;

    // Expose these so main can set uniforms for lighting
    virtual const std::vector<glm::vec3>& GetLightPositions() const = 0;
    virtual const std::vector<glm::vec3>& GetLightColors() const    = 0;
    virtual size_t GetLightCount() const = 0;
};


/**
 * ParkScene
 */
class ParkScene : public BaseScene
{
public:
    void Init(Shader& shaderLight, Shader& shader) override;
    void Update(float dt) override;
    void RenderDepth(Shader &depthShader) override;
    void Render(Shader &mainShader, Camera &camera) override;

    const std::vector<glm::vec3>& GetLightPositions() const override;
    const std::vector<glm::vec3>& GetLightColors() const override;
    size_t GetLightCount() const override;

private:
    // orbit data
    float m_orbitAngle = 0.f;
    std::vector<float> m_angleOffsetsDeg;
    std::vector<float> m_yValues;

    // scene data
    std::vector<glm::vec3> m_lightPositions;
    std::vector<glm::vec3> m_lightColors;
    std::vector<Sun>       m_suns;
    std::vector<Cube>      m_cubes;
    std::vector<Object>    m_objects;
};


/**
 * TowerScene
 */
class TowerScene : public BaseScene
{
public:
    void Init(Shader& shaderLight, Shader& shader) override;
    void Update(float dt) override;
    void RenderDepth(Shader &depthShader) override;
    void Render(Shader &mainShader, Camera &camera) override;

    const std::vector<glm::vec3>& GetLightPositions() const override;
    const std::vector<glm::vec3>& GetLightColors() const override;
    size_t GetLightCount() const override;

private:
    float m_orbitAngle = 0.f;
    std::vector<float> m_angleOffsetsDeg;
    std::vector<float> m_yValues;

    std::vector<glm::vec3> m_lightPositions;
    std::vector<glm::vec3> m_lightColors;
    std::vector<Sun>       m_suns;
    std::vector<Cube>      m_cubes;
    std::vector<Object>    m_objects;
};


/**
 * StructureScene
 */
class StructureScene : public BaseScene
{
public:
    void Init(Shader& shaderLight, Shader& shader) override;
    void Update(float dt) override;
    void RenderDepth(Shader &depthShader) override;
    void Render(Shader &mainShader, Camera &camera) override;

    const std::vector<glm::vec3>& GetLightPositions() const override;
    const std::vector<glm::vec3>& GetLightColors() const override;
    size_t GetLightCount() const override;

private:
    float m_orbitAngle = 0.f;
    std::vector<float> m_angleOffsetsDeg;
    std::vector<float> m_yValues;

    std::vector<glm::vec3> m_lightPositions;
    std::vector<glm::vec3> m_lightColors;
    std::vector<Sun>       m_suns;
    std::vector<Cube>      m_cubes;
    std::vector<Object>    m_objects;
};

