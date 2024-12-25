// main.cpp

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "helpers/camera.h"
#include "objects/skybox.h"
#include "objects/sun.h"
#include "objects/building.h"
#include "objects/cloud.h"   // Include the Cloud class
#include "objects/ground.h" // Include the Ground class

#include <vector>
#include <cstdlib>
#include <ctime>
#include <iostream>

// Callback functions (mouse_callback, key_callback) should be defined or implemented elsewhere

GLFWwindow* window = nullptr; 
int UsedTextureIndex = 0;


// Camera variables (assuming these are defined in "helpers/camera.h")
extern glm::vec3 cameraPos;
extern glm::vec3 cameraFront;
extern glm::vec3 cameraUp;

// Function to display camera position every second (assuming it's defined elsewhere)
extern void ShowCameraPosEverySecond();

int main()
{
    srand(static_cast<unsigned int>(time(nullptr)));

    if (!glfwInit()) {
        std::cerr << "Failed to init GLFW\n";
        return -1;
    }

    // Setup OpenGL version and profile
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // For macOS
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create window
    window = glfwCreateWindow(1024, 768, "Skybox + Building + Cloud", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to open GLFW window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Setup input modes and callbacks
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback); // Implemented elsewhere
    glfwSetKeyCallback(window, key_callback);         // Implemented elsewhere

    // Initialize GLAD
    if (!gladLoadGL(glfwGetProcAddress)) {
        std::cerr << "Failed to load GL\n";
        return -1;
    }

    // Set OpenGL state
    glClearColor(0.2f, 0.2f, 0.25f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);

    // 1. Create Skybox
    Skybox skybox;
    skybox.initialize(glm::vec3(0.0f), glm::vec3(50.0f));


    glm::vec3 lightPos(50.0f, 50.0f, 50.0f);


    Sun sunOne;
    sunOne.initialize(lightPos, glm::vec3(2.0f));


    // Define array of facade textures
    const char* facadeTextures[] = {
        "../assets/tex/minecraft/block/grass_block_top.png"
    };
    // 2. Create Buildings
    std::vector<Building> buildings;
    {
        const float spacing = 8.0f;
        const float size = 2.0f;
        const int rows = 6;
        const int cols = 6;
        const float heightMid = 2.0f;
        float xOffset = (rows - 1) * (size + spacing) / 2.0f;
        float zOffset = (cols - 1) * (size + spacing) / 2.0f;

        for(int i = 0; i < rows; ++i) {
            for(int j = 0; j < cols; ++j) {
                float x = i * (size + spacing) - xOffset;
                float z = j * (size + spacing) - zOffset;
                float height = heightMid + (rand() / (float)RAND_MAX) * heightMid;
                glm::vec3 position(x, height, z);
                glm::vec3 bscale(size, height, size);

                Building b;
                b.initialize(position, bscale, facadeTextures[0]);
                buildings.push_back(b);
            }
        }
    }

        // 3. Create Grounds
        std::vector<Building> grounds;
        const char* groundTextures[] = {
            "../assets/tex/minecraft/block/dirt.png",
            "../assets/tex/minecraft/block/grass_block_top.png",
            "../assets/tex/minecraft/block/stone.png",
            "../assets/tex/minecraft/block/gravel.png"
        };

        const int groundCountX = 2;
        const int groundCountZ = 2;
        const int groundSizeX = 50;
        const int groundSizeZ = 50;
        for(int i = 0; i < groundCountX; ++i) {
            for(int j = 0; j < groundCountZ; ++j) {
                Building g;
                // Arrange them in a 2x2 grid
                float x = i * groundSizeX * 2;
                float z = j * groundSizeZ * 2;
                glm::vec3 position(x, -1.0f, z);
                glm::vec3 scale(groundSizeX, 0.1f, groundSizeZ); // No scaling needed
                g.initialize(position, scale, groundTextures[i * groundCountZ + j]); // Repeat texture 10 times
                grounds.push_back(g);
            }
        }

    Cloud myCloud;
    // minecraft
     myCloud.initialize(glm::vec3(100.0f, -50.0f, 30.0f), glm::vec3(1.0f));

    //myCloud.initialize(glm::vec3(75.0f, 75.0f, 75.0f), glm::vec3(50.0f));

    // Setup projection matrix
    float FoV   = 45.0f;
    float zNear = 0.1f;
    float zFar  = 1000.0f;
    glm::mat4 projection = glm::perspective(glm::radians(FoV), 4.0f/3.0f, zNear, zFar);

    int flip = 0;

    while (!glfwWindowShouldClose(window)) {
        // Clear buffers
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Handle texture selection for skybox
        if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) { UsedTextureIndex=0; flip=0; }
        if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) { UsedTextureIndex=1; flip=0; }
        if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) { UsedTextureIndex=2; flip=1; }
        if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS) { UsedTextureIndex=3; flip=1; }

        // Build camera's view matrix
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

        // Skybox: remove translation from view
        glm::mat4 skyboxView = glm::mat4(glm::mat3(view));
        glCullFace(GL_FRONT);
        glm::mat4 skyboxVP = projection * skyboxView;
        skybox.render(skyboxVP, flip);

        // Buildings
        glCullFace(GL_BACK);
        glm::mat4 buildingVP = projection * view;
        for(auto &b : buildings) {
            // Uncomment if you have buildings to render
            b.render(buildingVP, lightPos);
        }
        // Render Grounds
        for(auto &g : grounds) {
            g.render(buildingVP, lightPos); // Use the same View-Projection matrix as buildings
        }


        sunOne.render(buildingVP);


        // Disable face culling temporarily for clouds if needed
        glDisable(GL_CULL_FACE);

        // Render Clouds
        myCloud.render(buildingVP, lightPos);

        // Re-enable face culling
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);

        // Print camera once per second in console
        ShowCameraPosEverySecond();

        // Swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    skybox.cleanup();
    for (auto &b : buildings) {
        b.cleanup();
    }

            for(auto &g : grounds) {
            g.cleanup(); // Use the same View-Projection matrix as buildings
        }

    myCloud.cleanup();

    glfwTerminate();
    return 0;
}
