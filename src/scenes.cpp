#include "scenes.h"
#include "includes/Sun.h"
#include "includes/Cube.h"
#include "includes/Object.h"
#include <learnopengl/filesystem.h> // For FileSystem::getPath
#include <learnopengl/shader.h>
#include <learnopengl/camera.h>
#include "includes/Utils.h" // For loadTexture

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


/* ================================
 * Implementation: ParkScene
 * ================================ */
void ParkScene::Init(Shader& shaderLight, Shader& shader)
{
    // 1) Hard-code some initial lights
    m_lightPositions = {
        glm::vec3(0.f, 10.f,  0.f),
        glm::vec3(-2.5f, 12.5f, -4.f),
        glm::vec3(3.f, 15.f, 1.f),
        glm::vec3(1.f, 17.5f, -6.f)
    };
    m_lightColors = {
        glm::vec3(5.f, 5.f, 5.f),
        glm::vec3(12.f,4.f,0.f),
        glm::vec3(3.f, 3.f, 18.f),
        glm::vec3(2.f, 10.f, 2.f)
    };

    // 2) Create suns
    for (size_t i=0; i<m_lightPositions.size(); i++)
    {
        Sun sun(shaderLight,
                m_lightColors[i],
                m_lightPositions[i],
                glm::vec3(0.f),
                glm::vec3(0.25f));
        m_suns.push_back(sun);
    }

    // 3) Example object: “park”
    Object park(
        shader,
        FileSystem::getPath("resources/objects/park/park.obj"),
        glm::vec3(0.f,5.f,0.f),
        glm::vec3(0.f),
        glm::vec3(0.1f)
    );
    m_objects.push_back(park);

    // 4) Floor
    unsigned int floorTex = loadTexture(FileSystem::getPath("resources/textures/gray_concrete_powder.png").c_str(), true);
    m_cubes.emplace_back(shader, floorTex,
        glm::vec3(0.f, -1.f, 0.f),
        glm::vec3(0.f),
        glm::vec3(25.f, 0.5f, 25.f));

    // 5) Basic orbit info
    m_angleOffsetsDeg = {0.f, 30.f, 60.f, 90.f};
    m_yValues         = {10.f, 12.5f, 15.f, 17.5f};
}

void ParkScene::Update(float dt)
{
    m_orbitAngle += dt * 0.5f;
    float radius = 7.f;
    for (size_t i=0; i<m_lightPositions.size(); i++)
    {
        float angle = glm::radians(m_angleOffsetsDeg[i]) + m_orbitAngle;
        float x = radius * cos(angle);
        float z = radius * sin(angle);
        float y = m_yValues[i];

        m_lightPositions[i] = glm::vec3(x,y,z);
        m_suns[i].SetPosition(m_lightPositions[i]);
    }
}

void ParkScene::RenderDepth(Shader &depthShader)
{
    // floor + cubes
    for (auto &c : m_cubes)
        c.RenderDepth(depthShader);
    // objects
    for (auto &o : m_objects)
        o.RenderDepth(depthShader);
}

void ParkScene::Render(Shader &mainShader, Camera &camera)
{
    std::vector<glm::vec3> dummyLP, dummyLC;
    for (auto &c : m_cubes)
        c.Render(camera, dummyLP, dummyLC);
    for (auto &o : m_objects)
        o.Render(camera, dummyLP, dummyLC);
    for (auto &sun : m_suns)
        sun.Render(camera, dummyLP, dummyLC);
}

// Light arrays
const std::vector<glm::vec3>& ParkScene::GetLightPositions() const { return m_lightPositions; }
const std::vector<glm::vec3>& ParkScene::GetLightColors()    const { return m_lightColors;    }
size_t ParkScene::GetLightCount() const { return m_lightPositions.size(); }


/* ================================
 * Implementation: TowerScene
 * ================================ */
void TowerScene::Init(Shader& shaderLight, Shader& shader)
{
    // 1) Hard-code some lights
    m_lightPositions = {
        glm::vec3(0.f, 10.f,  0.f),
        glm::vec3(-2.5f, 12.5f, -4.f),
        glm::vec3(3.f, 15.f, 1.f),
        glm::vec3(1.f, 17.5f, -6.f)
    };
    m_lightColors = {
        glm::vec3(5.f, 5.f, 5.f),
        glm::vec3(12.f,4.f,0.f),
        glm::vec3(3.f, 3.f, 18.f),
        glm::vec3(2.f, 10.f, 2.f)
    };

    // 2) Create suns
    for (size_t i=0; i<m_lightPositions.size(); i++)
    {
        Sun sun(shaderLight,
                m_lightColors[i],
                m_lightPositions[i],
                glm::vec3(0.f),
                glm::vec3(0.25f));
        m_suns.push_back(sun);
    }

    // 3) Example object: “tower”
    Object tower(
        shader,
        FileSystem::getPath("resources/objects/tower.obj"),
        glm::vec3(0.f,5.f,0.f),
        glm::vec3(0.f),
        glm::vec3(0.1f)
    );
    m_objects.push_back(tower);

    // 4) Floor
    unsigned int floorTex = loadTexture(FileSystem::getPath("resources/textures/gray_concrete_powder.png").c_str(), true);
    m_cubes.emplace_back(shader, floorTex,
        glm::vec3(0.f, -1.f, 0.f),
        glm::vec3(0.f),
        glm::vec3(25.f, 0.5f, 25.f));

    // 5) Orbit info
    m_angleOffsetsDeg = {0.f, 30.f, 60.f, 90.f};
    m_yValues         = {10.f, 12.5f, 15.f, 17.5f};
}

void TowerScene::Update(float dt)
{
    m_orbitAngle += dt * 0.5f;
    float radius = 7.f;
    for (size_t i=0; i<m_lightPositions.size(); i++)
    {
        float angle = glm::radians(m_angleOffsetsDeg[i]) + m_orbitAngle;
        float x = radius * cos(angle);
        float z = radius * sin(angle);
        float y = m_yValues[i];

        m_lightPositions[i] = glm::vec3(x,y,z);
        m_suns[i].SetPosition(m_lightPositions[i]);
    }
}

void TowerScene::RenderDepth(Shader &depthShader)
{
    for (auto &c : m_cubes)
        c.RenderDepth(depthShader);
    for (auto &o : m_objects)
        o.RenderDepth(depthShader);
}

void TowerScene::Render(Shader &mainShader, Camera &camera)
{
    std::vector<glm::vec3> dummyLP, dummyLC;
    for (auto &c : m_cubes)
        c.Render(camera, dummyLP, dummyLC);
    for (auto &o : m_objects)
        o.Render(camera, dummyLP, dummyLC);
    for (auto &sun : m_suns)
        sun.Render(camera, dummyLP, dummyLC);
}

const std::vector<glm::vec3>& TowerScene::GetLightPositions() const { return m_lightPositions; }
const std::vector<glm::vec3>& TowerScene::GetLightColors()    const { return m_lightColors;    }
size_t TowerScene::GetLightCount() const { return m_lightPositions.size(); }


/* ================================
 * Implementation: StructureScene
 * ================================ */
void StructureScene::Init(Shader& shaderLight, Shader& shader)
{
    m_lightPositions = {
        glm::vec3(0.f, 10.f,  0.f),
        glm::vec3(-2.5f, 12.5f, -4.f),
        glm::vec3(3.f, 15.f, 1.f),
        glm::vec3(1.f, 17.5f, -6.f)
    };
    m_lightColors = {
        glm::vec3(5.f, 5.f, 5.f),
        glm::vec3(12.f,4.f,0.f),
        glm::vec3(3.f, 3.f, 18.f),
        glm::vec3(2.f, 10.f, 2.f)
    };

    for (size_t i=0; i<m_lightPositions.size(); i++)
    {
        Sun sun(shaderLight,
                m_lightColors[i],
                m_lightPositions[i],
                glm::vec3(0.f),
                glm::vec3(0.25f));
        m_suns.push_back(sun);
    }

    Object structure(
        shader,
        FileSystem::getPath("resources/objects/structure.obj"),
        glm::vec3(0.f,5.f,0.f),
        glm::vec3(0.f),
        glm::vec3(0.1f)
    );
    m_objects.push_back(structure);

    unsigned int floorTex = loadTexture(FileSystem::getPath("resources/textures/gray_concrete_powder.png").c_str(), true);
    m_cubes.emplace_back(shader, floorTex,
        glm::vec3(0.f, -1.f, 0.f),
        glm::vec3(0.f),
        glm::vec3(25.f, 0.5f, 25.f));

    m_angleOffsetsDeg = {0.f, 30.f, 60.f, 90.f};
    m_yValues         = {10.f, 12.5f, 15.f, 17.5f};
}

void StructureScene::Update(float dt)
{
    m_orbitAngle += dt * 0.5f;
    float radius = 7.f;
    for (size_t i=0; i<m_lightPositions.size(); i++)
    {
        float angle = glm::radians(m_angleOffsetsDeg[i]) + m_orbitAngle;
        float x = radius * cos(angle);
        float z = radius * sin(angle);
        float y = m_yValues[i];

        m_lightPositions[i] = glm::vec3(x,y,z);
        m_suns[i].SetPosition(m_lightPositions[i]);
    }
}

void StructureScene::RenderDepth(Shader &depthShader)
{
    for (auto &c : m_cubes)
        c.RenderDepth(depthShader);
    for (auto &o : m_objects)
        o.RenderDepth(depthShader);
}

void StructureScene::Render(Shader &mainShader, Camera &camera)
{
    std::vector<glm::vec3> dummyLP, dummyLC;
    for (auto &c : m_cubes)
        c.Render(camera, dummyLP, dummyLC);
    for (auto &o : m_objects)
        o.Render(camera, dummyLP, dummyLC);
    for (auto &sun : m_suns)
        sun.Render(camera, dummyLP, dummyLC);
}

const std::vector<glm::vec3>& StructureScene::GetLightPositions() const { return m_lightPositions; }
const std::vector<glm::vec3>& StructureScene::GetLightColors()    const { return m_lightColors;    }
size_t StructureScene::GetLightCount() const { return m_lightPositions.size(); }

