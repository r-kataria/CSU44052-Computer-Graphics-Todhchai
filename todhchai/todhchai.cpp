#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "helpers/camera.h"
#include "objects/skybox.h"
#include "objects/building.h"

#include <vector>
#include <cstdlib>  // rand
#include <ctime>    // time
#include <iostream>


GLFWwindow* window = nullptr; // define it here
int UsedTextureIndex = 0;


int main()
{
    srand((unsigned int)time(nullptr));

    if (!glfwInit()) {
        std::cerr << "Failed to init GLFW\n";
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(1024, 768, "Skybox + Building", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to open GLFW window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetKeyCallback(window, key_callback);

    if (!gladLoadGL(glfwGetProcAddress)) {
        std::cerr << "Failed to load GL\n";
        return -1;
    }

    glClearColor(0.2f, 0.2f, 0.25f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glEnable(GL_CULL_FACE);

    // Create Skybox
    Skybox skybox;
    skybox.initialize(glm::vec3(0.0f), glm::vec3(50.0f));

    // Create buildings
    std::vector<Building> buildings;
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
            b.initialize(position, bscale);
            buildings.push_back(b);
        }
    }

    // Setup projection
    float FoV   = 45.0f;
    float zNear = 0.1f;
    float zFar  = 1000.0f;
    glm::mat4 projection = glm::perspective(glm::radians(FoV), 4.0f/3.0f, zNear, zFar);

    int flip = 0;

    while (!glfwWindowShouldClose(window)) {
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

        // Render skybox first
        glCullFace(GL_FRONT);
        glm::mat4 skyboxVP = projection * skyboxView;
        skybox.render(skyboxVP, flip);

        // Render buildings
        glCullFace(GL_BACK);
        glm::mat4 buildingVP = projection * view;
        for(auto &b : buildings) {
            b.render(buildingVP);
        }

        // Print camera once per second in console
        ShowCameraPosEverySecond();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    skybox.cleanup();
    for (auto &b : buildings) {
        b.cleanup();
    }

    glfwTerminate();
    return 0;
}
