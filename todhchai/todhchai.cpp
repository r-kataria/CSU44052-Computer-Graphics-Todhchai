// main.cpp

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "helpers/camera.h"
#include "objects/skybox.h"
#include "objects/sun.h"
#include "objects/building.h"
#include "objects/cloud.h"
#include "objects/ground.h"
#include "render/shader.h"
#include <vector>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include "helpers/postprocess.h" // For HDR/Bloom pipeline
#include <string>

// Callback functions (mouse_callback, key_callback) should be defined or implemented elsewhere
GLFWwindow* window = nullptr;
int UsedTextureIndex = 0;
// Camera variables (assuming these are defined in "helpers/camera.h")
extern glm::vec3 cameraPos;
extern glm::vec3 cameraFront;
extern glm::vec3 cameraUp;
// Function to display camera position every second (assuming it's defined elsewhere)
extern void ShowCameraPosEverySecond();

// Shadow mapping variables
const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
unsigned int depthMapFBO;
unsigned int depthCubemap;
float near_plane = 1.0f;
float far_plane = 100.0f; // Adjust based on your scene's depth

// Shader program IDs
GLuint depthShaderProgram;

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
    window = glfwCreateWindow(1024, 768, "Skybox + Building + Cloud + HDR + Bloom + Shadows", nullptr, nullptr);
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

    // Initialize HDR/Bloom
    int screenWidth, screenHeight;
    glfwGetFramebufferSize(window, &screenWidth, &screenHeight);
    InitHDRBloom(screenWidth, screenHeight);

    // Create Skybox
    Skybox skybox;
    skybox.initialize(glm::vec3(0.0f), glm::vec3(50.0f));
    glm::vec3 lightPos(50.0f, 50.0f, 50.0f);

    // Create Sun
    Sun sunOne;
    sunOne.initialize(lightPos, glm::vec3(5.0f));

    // Create Buildings
    const char* facadeTextures[] = {
        "../assets/tex/minecraft/block/grass_block_top.png"
    };
    std::vector<Building> buildings;

    {
        const float spacing = 8.0f;
        const float size = 2.0f;
        const int rows = 6;
        const int cols = 6;
        const float heightMid = 2.0f;
        float xOffset = (rows - 1) * (size + spacing) / 2.0f;
        float zOffset = (cols - 1) * (size + spacing) / 2.0f;
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
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

    // Create Grounds
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
    for (int i = 0; i < groundCountX; ++i) {
        for (int j = 0; j < groundCountZ; ++j) {
            Building g;
            // Arrange them in a 2x2 grid
            float x = i * groundSizeX * 2;
            float z = j * groundSizeZ * 2;
            glm::vec3 position(x, -1.0f, z);
            glm::vec3 scale(groundSizeX, 0.1f, groundSizeZ);
            g.initialize(position, scale, groundTextures[i * groundCountZ + j]);
            grounds.push_back(g);
        }
    }

    // Create Clouds
    Cloud myCloud;
    myCloud.initialize(glm::vec3(50.0f, -30.0f, 50.0f), glm::vec3(1.0f));

    // Setup projection matrix
    float FoV = 45.0f;
    float zNear = 0.1f;
    float zFar = 1000.0f;
    glm::mat4 projection = glm::perspective(glm::radians(FoV), (float)screenWidth / (float)screenHeight, zNear, zFar);

    int flip = 0;

    // Load shaders
    GLuint depthShaderProgram = LoadShadersFromFile("../todhchai/shaders/depth_cube.vert",
                                                    "../todhchai/shaders/depth_cube.frag",
                                                    "../todhchai/shaders/depth_cube.geom");
    if (depthShaderProgram == 0) {
        std::cerr << "Failed to load depthShaderProgram" << std::endl;
    }

    // Main render loop
    while (!glfwWindowShouldClose(window))
    {
        // 0. Render scene to depth cubemap for point shadows
        float aspect = (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT;
        glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), aspect, near_plane, far_plane);
        std::vector<glm::mat4> shadowTransforms;
        shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(1.0,  0.0,  0.0), glm::vec3(0.0, -1.0,  0.0)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(-1.0,  0.0,  0.0), glm::vec3(0.0, -1.0,  0.0)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0,  1.0,  0.0), glm::vec3(0.0,  0.0,  1.0)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0, -1.0,  0.0), glm::vec3(0.0,  0.0, -1.0)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0,  0.0,  1.0), glm::vec3(0.0, -1.0,  0.0)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0,  0.0, -1.0), glm::vec3(0.0, -1.0,  0.0)));

        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);

        // Use the depth shader
        glUseProgram(depthShaderProgram);
        for (unsigned int i = 0; i < 6; ++i)
            glUniformMatrix4fv(glGetUniformLocation(depthShaderProgram, ("shadowMatrices[" + std::to_string(i) + "]").c_str()), 1, GL_FALSE, &shadowTransforms[i][0][0]);
        glUniform1f(glGetUniformLocation(depthShaderProgram, "far_plane"), far_plane);
        glUniform3fv(glGetUniformLocation(depthShaderProgram, "lightPos"), 1, &lightPos[0]);

        // Render the scene from the light's point of view
        // Modify your render functions to accept the shader program as a parameter

        // myCloud.render(depthShaderProgram);
        // sunOne.renderDepth(depthShaderProgram); // Sun is the light source, no need to render it to depth map

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // Restore viewport
        glfwGetFramebufferSize(window, &screenWidth, &screenHeight);
        glViewport(0, 0, screenWidth, screenHeight);

        // 1. Bind HDR framebuffer & clear
        glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 2. Render the scene as usual
        // Handle texture selection for skybox
        if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) { UsedTextureIndex = 0; flip = 0; }
        if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) { UsedTextureIndex = 1; flip = 0; }
        if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) { UsedTextureIndex = 2; flip = 1; }
        if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS) { UsedTextureIndex = 3; flip = 1; }

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

        // Bind depth map (cubemap)
        glActiveTexture(GL_TEXTURE3); // Use texture unit 3
        glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);

        for (auto& b : buildings) {
            b.render(buildingVP, lightPos, cameraPos);
        }
        // Grounds
        for (auto& g : grounds) {
            g.render(buildingVP, lightPos, cameraPos);
        }

        // Sun
        sunOne.render(buildingVP);

        // Clouds
        glDisable(GL_CULL_FACE);
        myCloud.render(buildingVP, lightPos, cameraPos);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);

        // Done rendering scene to HDR FBO
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // 3. Bloom Passes
        // a) Extract bright areas
        ExtractBright(hdrColorBuffer);
        // b) Blur the bright areas
        BlurBrightTexture();
        // c) Final Pass: Combine + Tone Map
        RenderHDRBloomFinal(hdrColorBuffer, 1.5f /* exposure */); // Increased exposure

        // 4. Print camera once per second in console
        ShowCameraPosEverySecond();

        // Swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    skybox.cleanup();
    for (auto& b : buildings) {
        b.cleanup();
    }
    for (auto& g : grounds) {
        g.cleanup();
    }
    myCloud.cleanup();
    sunOne.cleanup();
    CleanupPostProcess(); // Cleanup post-processing resources
    glfwTerminate();
    return 0;
}
