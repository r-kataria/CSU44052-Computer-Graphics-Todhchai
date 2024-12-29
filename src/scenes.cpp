// -------------------- scenes.cpp --------------------
#include "scenes.h"
#include "includes/Sun.h"
#include "includes/Cube.h"
#include "includes/Object.h"
#include <learnopengl/filesystem.h>
#include <learnopengl/shader.h>
#include <learnopengl/camera.h>
#include "includes/Utils.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// We'll use these externs so we can access them in TowerScene::Render:
extern unsigned int SCR_WIDTH;      // from main.cpp
extern unsigned int SCR_HEIGHT;     // from main.cpp
extern float far_plane;             // from main.cpp

extern float control_y;

/* ================================
 * Implementation: ParkScene
 * ================================ */
void ParkScene::Init(Shader& shaderLight, Shader& shader)
{
    // 1) Hard-code some initial lights
    m_lightPositions = {
        glm::vec3(0.f, 10.f,  0.f),
    };
    m_lightColors = {
        glm::vec3(5.f, 5.f, 5.f),
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
        glm::vec3(0.f,1.6f,0.f),
        glm::vec3(0.f),
        glm::vec3(0.3f)
    );
    m_objects.push_back(park);

    // 4) Floor
    unsigned int floorTex = loadTexture(FileSystem::getPath("resources/textures/gray_concrete_powder.png").c_str(), true);
    m_cubes.emplace_back(shader, floorTex,
        glm::vec3(0.f, -1.f, 0.f),
        glm::vec3(0.f),
        glm::vec3(50.f, 1.0f, 50.f));

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
        glm::vec3(0.f,6.3f,0.f),
        glm::vec3(0.f),
        glm::vec3(0.5f)
    );
    m_objects.push_back(tower);

    // 5) Floor
    unsigned int floorTex = loadTexture(FileSystem::getPath("resources/textures/gray_concrete_powder.png").c_str(), true);
    m_cubes.emplace_back(shader, floorTex,
        glm::vec3(0.f, -1.f, 0.f),
        glm::vec3(0.f),
        glm::vec3(50.f, 1.0f, 50.f));

    // 6) Orbit info
    m_angleOffsetsDeg = {0.f, 30.f, 60.f, 90.f};
    m_yValues         = {10.f, 12.5f, 15.f, 17.5f};
}

void TowerScene::Update(float dt)
{
    // Orbit logic
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

void TowerScene::RenderDepth(Shader &depthShader)
{
    // Render floor & tower to depth
    for (auto &c : m_cubes)
        c.RenderDepth(depthShader);
    for (auto &o : m_objects)
        o.RenderDepth(depthShader);

    // (Optional) If you want the vampire to cast shadows, you'd need a skeletal Depth VS.
    // Here we omit it for minimal code. 
}

void TowerScene::Render(Shader &mainShader, Camera &camera)
{
    // Render the tower/floor/suns as before
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

#include <random> // Include this at the top of your file

void StructureScene::Init(Shader& shaderLight, Shader& shader)
{
    // Configuration for the grid
    const int gridSize = 3;
    const float spacing = 10.0f; // Distance between lights
    const float startOffset = -((gridSize - 1) * spacing) / 2.0f; // To center the grid at (0,0)

    // Random number generation for colors
    std::random_device rd;
    std::mt19937 gen(rd());
    // Assuming light color intensities similar to original code
    std::uniform_real_distribution<float> colorDist(0.0f, 15.0f); 

    // Generate grid positions and random colors
    for(int i = 0; i < gridSize; ++i)
    {
        for(int j = 0; j < gridSize; ++j)
        {
            // Calculate X and Z positions to form a grid on the XZ-plane
            float x = startOffset + i * spacing;
            float z = startOffset + j * spacing;
            float y = 15.0f; // You can adjust the Y position as needed

            m_lightPositions.emplace_back(glm::vec3(x, y, z));

            // Generate a random color
            glm::vec3 randomColor(colorDist(gen), colorDist(gen), colorDist(gen));
            m_lightColors.emplace_back(randomColor);
        }
    }

    // Create Sun objects for each light
    for (size_t i = 0; i < m_lightPositions.size(); ++i)
    {
        Sun sun(shaderLight,
                m_lightColors[i],
                m_lightPositions[i],
                glm::vec3(0.f),
                glm::vec3(0.25f)); // Adjust the last parameter as needed
        m_suns.push_back(sun);
    }

    // Initialize other scene objects as before
    Object structure(
        shader,
        FileSystem::getPath("resources/objects/trinity.obj"),
        glm::vec3(0.f, 9.6f, 0.f),
        glm::vec3(0.f),
        glm::vec3(0.1f)
    );
    m_objects.push_back(structure);

    unsigned int floorTex = loadTexture(FileSystem::getPath("resources/textures/grass.png").c_str(), true);
    m_cubes.emplace_back(shader, floorTex,
        glm::vec3(0.f, 8.7f, 0.f),
        glm::vec3(0.f),
        glm::vec3(25.f, 0.5f, 25.f));

    // If m_angleOffsetsDeg and m_yValues are still needed, you might want to adjust them
    // For example, you could generate corresponding offsets for the grid
    // Here, we'll clear them since they were originally for 4 lights
    m_angleOffsetsDeg.clear();
    m_yValues.clear();

    // Optionally, generate new offsets based on grid if required
    // For now, they are left empty
}


void StructureScene::Update(float dt)
{
   // m_objects[0].SetPosition( glm::vec3(0.f, control_y, 0.f) );
 
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