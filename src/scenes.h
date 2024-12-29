#pragma once
#include <glm/glm.hpp>
#include "helpers/shader.h"
#include "helpers/camera.h"
#include <vector>
#include <string>

// Fixes import errors
class Sun;
class Cube;
class Object;

// base class for all scenes.
class BaseScene
{
public:
    virtual ~BaseScene() = default;

    virtual void Init(Shader& shaderLight, Shader& shader) = 0;
    virtual void Update(float dt) = 0;
    virtual void RenderDepth(Shader& depthShader) = 0;
    virtual void Render(Shader& mainShader, Camera& camera) = 0;

    virtual const std::vector<glm::vec3>& GetLightPositions() const = 0;
    virtual const std::vector<glm::vec3>& GetLightColors() const = 0;
    virtual size_t GetLightCount() const = 0;
};

// Scene representing a park environment.
class ParkScene : public BaseScene
{
public:
    void Init(Shader& shaderLight, Shader& shader) override;
    void Update(float dt) override;
    void RenderDepth(Shader& depthShader) override;
    void Render(Shader& mainShader, Camera& camera) override;

    const std::vector<glm::vec3>& GetLightPositions() const override;
    const std::vector<glm::vec3>& GetLightColors() const override;
    size_t GetLightCount() const override;

private:
    float m_orbitAngle = 0.f;
    std::vector<float> m_angleOffsetsDeg;
    std::vector<float> m_yValues;

    std::vector<glm::vec3> m_lightPositions;
    std::vector<glm::vec3> m_lightColors;
    std::vector<Sun> m_suns;
    std::vector<Cube> m_cubes;
    std::vector<Object> m_objects;
};

// Scene representing a tower environment.
class TowerScene : public BaseScene
{
public:
    void Init(Shader& shaderLight, Shader& shader) override;
    void Update(float dt) override;
    void RenderDepth(Shader& depthShader) override;
    void Render(Shader& mainShader, Camera& camera) override;

    const std::vector<glm::vec3>& GetLightPositions() const override;
    const std::vector<glm::vec3>& GetLightColors() const override;
    size_t GetLightCount() const override;

private:
    float m_orbitAngle = 0.f;
    std::vector<float> m_angleOffsetsDeg;
    std::vector<float> m_yValues;

    std::vector<glm::vec3> m_lightPositions;
    std::vector<glm::vec3> m_lightColors;
    std::vector<Sun> m_suns;
    std::vector<Cube> m_cubes;
    std::vector<Object> m_objects;
};

// Scene representing structural elements.
class StructureScene : public BaseScene
{
public:
    void Init(Shader& shaderLight, Shader& shader) override;
    void Update(float dt) override;
    void RenderDepth(Shader& depthShader) override;
    void Render(Shader& mainShader, Camera& camera) override;

    const std::vector<glm::vec3>& GetLightPositions() const override;
    const std::vector<glm::vec3>& GetLightColors() const override;
    size_t GetLightCount() const override;

private:
    float m_orbitAngle = 0.f;
    std::vector<float> m_angleOffsetsDeg;
    std::vector<float> m_yValues;

    std::vector<glm::vec3> m_lightPositions;
    std::vector<glm::vec3> m_lightColors;
    std::vector<Sun> m_suns;
    std::vector<Cube> m_cubes;
    std::vector<Object> m_objects;
};

// Scene representing a forest with trees.
class TreesScene : public BaseScene
{
public:
    void Init(Shader& shaderLight, Shader& shader) override;
    void Update(float dt) override;
    void RenderDepth(Shader& depthShader) override;
    void Render(Shader& mainShader, Camera& camera) override;

    const std::vector<glm::vec3>& GetLightPositions() const override;
    const std::vector<glm::vec3>& GetLightColors() const override;
    size_t GetLightCount() const override;

private:
    std::vector<glm::vec3> m_lightPositions;
    std::vector<glm::vec3> m_lightColors;
    std::vector<Sun> m_suns;
    std::vector<Cube> m_cubes;
    std::vector<Object> m_objects;
    std::vector<Object> m_trees;

    int m_numTrees = 100;
    float m_treeAreaSize = 100.0f;
};
