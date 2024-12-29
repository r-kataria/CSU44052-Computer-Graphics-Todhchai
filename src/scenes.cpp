#include "scenes.h"
#include "includes/Sun.h"
#include "includes/Cube.h"
#include "includes/Object.h"
#include "helpers/filesystem.h"
#include "helpers/shader.h"
#include "helpers/camera.h"
#include "includes/Utils.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <random>

// External variables from main.cpp
extern unsigned int SCR_WIDTH;
extern unsigned int SCR_HEIGHT;
extern float far_plane;
extern float control_y;

// ParkScene Implementation
void ParkScene::Init(Shader& shaderLight, Shader& shader)
{
    // Initialize light positions and colors
    m_lightPositions = { glm::vec3(0.f, 10.f,  0.f) };
    m_lightColors = { glm::vec3(5.f, 5.f, 5.f) };

    // Create suns
    for (size_t i = 0; i < m_lightPositions.size(); i++)
    {
        Sun sun(shaderLight, m_lightColors[i], m_lightPositions[i], glm::vec3(0.f), glm::vec3(0.25f));
        m_suns.push_back(sun);
    }

    // Add park object
    m_objects.emplace_back(shader, FileSystem::getPath("resources/objects/park/park.obj"),
                           glm::vec3(0.f, 2.f, 0.f), glm::vec3(0.f), glm::vec3(0.3f));

    // Add floor
    unsigned int floorTex = loadTexture(FileSystem::getPath("resources/textures/gray_concrete_powder.png").c_str(), true);
    m_cubes.emplace_back(shader, floorTex, glm::vec3(0.f, -1.f, 0.f), glm::vec3(0.f), glm::vec3(50.f, 1.0f, 50.f));

    // Set orbit parameters
    m_angleOffsetsDeg = { 0.f, 30.f, 60.f, 90.f };
    m_yValues = { 10.f, 12.5f, 15.f, 17.5f };
}

void ParkScene::Update(float dt)
{
    m_orbitAngle += dt * 0.5f;
    float radius = 7.f;
    for (size_t i = 0; i < m_lightPositions.size(); i++)
    {
        float angle = glm::radians(m_angleOffsetsDeg[i]) + m_orbitAngle;
        float x = radius * cos(angle);
        float z = radius * sin(angle);
        float y = m_yValues[i];

        m_lightPositions[i] = glm::vec3(x, y, z);
        m_suns[i].SetPosition(m_lightPositions[i]);
    }
}

void ParkScene::RenderDepth(Shader& depthShader)
{
    for (auto& c : m_cubes)
        c.RenderDepth(depthShader);
    for (auto& o : m_objects)
        o.RenderDepth(depthShader);
}

void ParkScene::Render(Shader& mainShader, Camera& camera)
{
    std::vector<glm::vec3> dummyLP, dummyLC;
    for (auto& c : m_cubes)
        c.Render(camera, dummyLP, dummyLC);
    for (auto& o : m_objects)
        o.Render(camera, dummyLP, dummyLC);
    for (auto& sun : m_suns)
        sun.Render(camera, dummyLP, dummyLC);
}

const std::vector<glm::vec3>& ParkScene::GetLightPositions() const { return m_lightPositions; }
const std::vector<glm::vec3>& ParkScene::GetLightColors() const { return m_lightColors; }
size_t ParkScene::GetLightCount() const { return m_lightPositions.size(); }

// TowerScene Implementation
void TowerScene::Init(Shader& shaderLight, Shader& shader)
{
    // Initialize light positions and colors
    m_lightPositions = {
        glm::vec3(0.f, 10.f,  0.f),
        glm::vec3(-2.5f, 12.5f, -4.f),
        glm::vec3(3.f, 15.f, 1.f),
        glm::vec3(1.f, 17.5f, -6.f),
        glm::vec3(-1.f, 20.f, -8.f),
    };
    m_lightColors = {
        glm::vec3(5.f, 5.f, 5.f),
        glm::vec3(12.f, 4.f, 0.f),
        glm::vec3(3.f, 3.f, 18.f),
        glm::vec3(2.f, 10.f, 2.f),
        glm::vec3(10.f, 2.f, 2.f)
    };

    // Create suns
    for (size_t i = 0; i < m_lightPositions.size(); i++)
    {
        Sun sun(shaderLight, m_lightColors[i], m_lightPositions[i], glm::vec3(0.f), glm::vec3(0.25f));
        m_suns.push_back(sun);
    }

    // Add tower object
    m_objects.emplace_back(shader, FileSystem::getPath("resources/objects/tower.obj"),
                           glm::vec3(0.f, 6.3f, 0.f), glm::vec3(0.f), glm::vec3(0.5f));

    // Add floor
    unsigned int floorTex = loadTexture(FileSystem::getPath("resources/textures/gray_concrete_powder.png").c_str(), true);
    m_cubes.emplace_back(shader, floorTex, glm::vec3(0.f, -1.f, 0.f), glm::vec3(0.f), glm::vec3(50.f, 1.0f, 50.f));

    // Set orbit parameters
    m_angleOffsetsDeg = { 0.f, 30.f, 60.f, 90.f, 120.f};
    m_yValues = { 10.f, 20.f, 30.f, 40.f, 50.f };
}

void TowerScene::Update(float dt)
{
    m_orbitAngle += dt * 0.5f;
    float radius = 15.f;
    for (size_t i = 0; i < m_lightPositions.size(); i++)
    {
        float angle = glm::radians(m_angleOffsetsDeg[i]) + m_orbitAngle;
        float x = radius * cos(angle);
        float z = radius * sin(angle);
        float y = m_yValues[i];

        m_lightPositions[i] = glm::vec3(x, y, z);
        m_suns[i].SetPosition(m_lightPositions[i]);
    }
}

void TowerScene::RenderDepth(Shader& depthShader)
{
    for (auto& c : m_cubes)
        c.RenderDepth(depthShader);
    for (auto& o : m_objects)
        o.RenderDepth(depthShader);
}

void TowerScene::Render(Shader& mainShader, Camera& camera)
{
    std::vector<glm::vec3> dummyLP, dummyLC;
    for (auto& c : m_cubes)
        c.Render(camera, dummyLP, dummyLC);
    for (auto& o : m_objects)
        o.Render(camera, dummyLP, dummyLC);
    for (auto& sun : m_suns)
        sun.Render(camera, dummyLP, dummyLC);
}

const std::vector<glm::vec3>& TowerScene::GetLightPositions() const { return m_lightPositions; }
const std::vector<glm::vec3>& TowerScene::GetLightColors() const { return m_lightColors; }
size_t TowerScene::GetLightCount() const { return m_lightPositions.size(); }

// StructureScene Implementation
void StructureScene::Init(Shader& shaderLight, Shader& shader)
{

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> colorDist(0.0f, 15.0f);

    m_lightPositions = {
        glm::vec3(7.5f, 7.5f,  7.5f),
        glm::vec3(7.5f, 7.5f, -7.5f),
        glm::vec3(-7.5f, 7.5f, 7.5f),
        glm::vec3(-7.5f, 7.5f, -7.5f)
    };
    m_lightColors = {
        glm::vec3(colorDist(gen),colorDist(gen),colorDist(gen)),
        glm::vec3(colorDist(gen),colorDist(gen),colorDist(gen)),
        glm::vec3(colorDist(gen),colorDist(gen),colorDist(gen)),
        glm::vec3(colorDist(gen),colorDist(gen),colorDist(gen))
    };

    // Create suns
    for (size_t i = 0; i < m_lightPositions.size(); i++)
    {
        Sun sun(shaderLight, m_lightColors[i], m_lightPositions[i], glm::vec3(0.f), glm::vec3(0.25f));
        m_suns.push_back(sun);
    }



    // Add structure object
    m_objects.emplace_back(shader, FileSystem::getPath("resources/objects/trinity.obj"),
                           glm::vec3(0.f, 1.f, 0.f), glm::vec3(0.f), glm::vec3(0.1f));

    // Add floor
    unsigned int floorTex = loadTexture(FileSystem::getPath("resources/textures/gray_concrete_powder.png").c_str(), true);
    m_cubes.emplace_back(shader, floorTex, glm::vec3(0.f, -1.f, 0.f), glm::vec3(0.f), glm::vec3(50.f, 1.0f, 50.f));

    // Clear orbit parameters as they are not needed
    m_angleOffsetsDeg.clear();
    m_yValues.clear();
}

void StructureScene::Update(float dt)
{
    // Currently no updates required
}

void StructureScene::RenderDepth(Shader& depthShader)
{
    for (auto& c : m_cubes)
        c.RenderDepth(depthShader);
    for (auto& o : m_objects)
        o.RenderDepth(depthShader);
}

void StructureScene::Render(Shader& mainShader, Camera& camera)
{
    std::vector<glm::vec3> dummyLP, dummyLC;
    for (auto& c : m_cubes)
        c.Render(camera, dummyLP, dummyLC);
    for (auto& o : m_objects)
        o.Render(camera, dummyLP, dummyLC);
    for (auto& sun : m_suns)
        sun.Render(camera, dummyLP, dummyLC);
}

const std::vector<glm::vec3>& StructureScene::GetLightPositions() const { return m_lightPositions; }
const std::vector<glm::vec3>& StructureScene::GetLightColors() const { return m_lightColors; }
size_t StructureScene::GetLightCount() const { return m_lightPositions.size(); }

// TreesScene Implementation
void TreesScene::Init(Shader& shaderLight, Shader& shader)
{
    // Initialize light positions and colors
    m_lightPositions = {
        glm::vec3(5.f, 10.f,  5.f),
        glm::vec3(5.f, 10.f, -5.f),
        glm::vec3(-5.f, 10.f, 5.f),
        glm::vec3(-5.f, 10.f, -5.f)
    };
    m_lightColors = {
        glm::vec3(5.f, 5.f, 5.f),
        glm::vec3(5.f, 5.f, 5.f),
        glm::vec3(5.f, 5.f, 5.f),
        glm::vec3(5.f, 5.f, 5.f)
    };

    // Create suns
    for (size_t i = 0; i < m_lightPositions.size(); i++)
    {
        Sun sun(shaderLight, m_lightColors[i], m_lightPositions[i], glm::vec3(0.f), glm::vec3(0.25f));
        m_suns.push_back(sun);
    }

    // Load tree models
    std::string tree1Path = FileSystem::getPath("resources/objects/trees/tree1.obj");
    std::string tree2Path = FileSystem::getPath("resources/objects/trees/tree2.obj");

    // Random placement of trees
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> distPos(-m_treeAreaSize / 2.0f, m_treeAreaSize / 2.0f);
    std::uniform_int_distribution<int> distTreeType(1, 2);
    std::uniform_real_distribution<float> distScale(0.8f, 1.2f);

    for(int i = 0; i < m_numTrees; ++i)
    {
        float x = distPos(gen);
        float z = distPos(gen);
        float y = 4.f;

        int treeType = distTreeType(gen);
        std::string treePath = (treeType == 1) ? tree1Path : tree2Path;

        float scaleFactor = distScale(gen);

        m_trees.emplace_back(shader, treePath, glm::vec3(x, y, z), glm::vec3(0.f), glm::vec3(scaleFactor));
    }

    // Add floor
    unsigned int floorTex = loadTexture(FileSystem::getPath("resources/textures/grass_block_top.png").c_str(), true);
    m_cubes.emplace_back(shader, floorTex, glm::vec3(0.f, -0.5f, 0.f), glm::vec3(0.f), glm::vec3(100.f, 1.0f, 100.f));
}

void TreesScene::Update(float dt)
{
    // No updates required as suns are linked to the camera
}

void TreesScene::RenderDepth(Shader& depthShader)
{
    for (auto& c : m_cubes)
        c.RenderDepth(depthShader);
    for (auto& t : m_trees)
        t.RenderDepth(depthShader);
}

void TreesScene::Render(Shader& mainShader, Camera& camera)
{
    m_cubes[0].Render(camera, m_lightPositions, m_lightColors);
    for (auto& t : m_trees)
        t.Render(camera, m_lightPositions, m_lightColors);
    for (auto& sun : m_suns)
        sun.Render(camera, m_lightPositions, m_lightColors);
}

const std::vector<glm::vec3>& TreesScene::GetLightPositions() const { return m_lightPositions; }
const std::vector<glm::vec3>& TreesScene::GetLightColors() const { return m_lightColors; }
size_t TreesScene::GetLightCount() const { return m_lightPositions.size(); }
