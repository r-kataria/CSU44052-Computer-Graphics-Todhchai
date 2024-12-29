#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "./helpers/filesystem.h"
#include "./helpers/shader.h"
#include "./helpers/camera.h"

#include <iostream>
#include <vector>
#include <cmath>

// Custom class includes
#include "includes/BloomFBO.h"
#include "includes/BloomRenderer.h"
#include "includes/Utils.h"
#include "includes/Input.h"
#include "includes/Cube.h"
#include "includes/Sun.h"
#include "includes/Object.h"

// Scene management
#include "scenes.h"

// Forward declaration for callbacks
void framebuffer_size_callback(GLFWwindow*, int, int);

// Global variables
unsigned int SCR_WIDTH  = 1280;
unsigned int SCR_HEIGHT = 720;
float exposure          = 1.0f;
float bloomFilterRadius = 0.005f;

// Camera setup
Camera camera(glm::vec3(30.f, 30.f, 100.f));
float lastX      = static_cast<float>(SCR_WIDTH) / 2.0f;
float lastY      = static_cast<float>(SCR_HEIGHT) / 2.0f;
bool  firstMouse = true;

// Timing variables
float deltaTime           = 0.0f;
float lastFrame           = 0.0f;
float timeSinceLastPrint  = 0.0f;
float printInterval       = 1.0f;
int   framesCount         = 0;

// Scene management
int currentSceneIndex = 1; // Default to scene #1

// Shadow settings
const unsigned int SHADOW_WIDTH = 512, SHADOW_HEIGHT = 512;
float near_plane  = 1.0f;
float far_plane   = 1000.0f;
bool  showShadow  = true;

float control_y = 0.0f;

int main()
{
    // Initialize GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 8); // 8x MSAA

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Todhchai - Ireland in Future", nullptr, nullptr);
    if (!window)
    {
        std::cerr << "Failed to create GLFW Window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Set callbacks
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window,   mouse_callback);
    glfwSetScrollCallback(window,      scroll_callback);

    // Capture mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Configure OpenGL state
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE); // Disable face culling for cube rendering

    // Load shaders
    Shader shader("shaders/bloom.vs", "shaders/bloom.fs");
    Shader shaderLight("shaders/bloom.vs", "shaders/light_box.fs");
    Shader shaderBloomFinal("shaders/bloom_final.vs", "shaders/bloom_final.fs");
    Shader simpleDepthShader("shaders/point_shadows_depth.vs",
                             "shaders/point_shadows_depth.fs",
                             "shaders/point_shadows_depth.gs");

    // Configure shadow maps
    const unsigned int MAX_SUNS = 16;
    unsigned int depthCubemaps[MAX_SUNS];
    unsigned int depthMapFBOs[MAX_SUNS];
    glGenTextures(MAX_SUNS, depthCubemaps);
    glGenFramebuffers(MAX_SUNS, depthMapFBOs);

    for (unsigned int n = 0; n < MAX_SUNS; ++n)
    {
        glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemaps[n]);
        for (unsigned int i = 0; i < 6; ++i)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT,
                         SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
        }
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBOs[n]);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubemaps[n], 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    // Configure HDR MSAA Framebuffer
    unsigned int hdrFBO;
    glGenFramebuffers(1, &hdrFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);

    unsigned int colorBuffersMSAA[2];
    glGenTextures(2, colorBuffersMSAA);
    for (unsigned int i = 0; i < 2; i++)
    {
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, colorBuffersMSAA[i]);
        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA16F,
                                SCR_WIDTH, SCR_HEIGHT, GL_TRUE);
        glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i,
                               GL_TEXTURE_2D_MULTISAMPLE, colorBuffersMSAA[i], 0);
    }

    unsigned int rboDepthMSAA;
    glGenRenderbuffers(1, &rboDepthMSAA);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepthMSAA);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH_COMPONENT,
                                     SCR_WIDTH, SCR_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                              GL_RENDERBUFFER, rboDepthMSAA);

    unsigned int attachmentsMSAA[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, attachmentsMSAA);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Multisampled HDR Framebuffer not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Configure Resolved Framebuffer
    unsigned int resolvedFBO;
    glGenFramebuffers(1, &resolvedFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, resolvedFBO);

    unsigned int resolvedColorBuffers[2];
    glGenTextures(2, resolvedColorBuffers);
    for (unsigned int i = 0; i < 2; i++)
    {
        glBindTexture(GL_TEXTURE_2D, resolvedColorBuffers[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F,
                     SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i,
                               GL_TEXTURE_2D, resolvedColorBuffers[i], 0);
    }

    unsigned int resolvedDepthRBO;
    glGenRenderbuffers(1, &resolvedDepthRBO);
    glBindRenderbuffer(GL_RENDERBUFFER, resolvedDepthRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT,
                          SCR_WIDTH, SCR_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                              GL_RENDERBUFFER, resolvedDepthRBO);

    unsigned int resolvedAttachments[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, resolvedAttachments);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Resolved Framebuffer not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Configure Ping-pong Framebuffers for Bloom
    unsigned int pingpongFBO[2];
    unsigned int pingpongColorbuffers[2];
    glGenFramebuffers(2, pingpongFBO);
    glGenTextures(2, pingpongColorbuffers);
    for (unsigned int i = 0; i < 2; i++)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
        glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F,
                     SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_2D, pingpongColorbuffers[i], 0);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "Ping-pong Framebuffer not complete!" << std::endl;
    }

    // Configure shaders
    shader.use();
    shader.setInt("diffuseTexture", 0);

    shaderBloomFinal.use();
    shaderBloomFinal.setInt("scene", 0);
    shaderBloomFinal.setInt("bloomBlur", 1);

    simpleDepthShader.use();
    simpleDepthShader.setFloat("far_plane", far_plane);

    shader.use();
    shader.setInt("depthMap", 1);

    // Update framebuffer size
    glfwMakeContextCurrent(window);
    int fbWidth, fbHeight;
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
    SCR_WIDTH  = fbWidth;
    SCR_HEIGHT = fbHeight;

    // Initialize Bloom Renderer
    BloomRenderer bloomRenderer;
    bloomRenderer.Init(SCR_WIDTH, SCR_HEIGHT);

    // Initialize Scenes
    ParkScene      parkScene;
    TreesScene     treesScene;
    TowerScene     towerScene;
    StructureScene structureScene;

    parkScene.Init(shaderLight, shader);
    treesScene.Init(shaderLight, shader);
    towerScene.Init(shaderLight, shader);
    structureScene.Init(shaderLight, shader);

    std::vector<BaseScene*> allScenes = { &towerScene, &parkScene, &structureScene, &treesScene };

    // Lambda to generate shadow transformation matrices
    auto GetShadowTransforms = [&](const glm::vec3& lightPos) -> std::vector<glm::mat4>
    {
        std::vector<glm::mat4> mats;
        glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f),
                                                static_cast<float>(SHADOW_WIDTH) / SHADOW_HEIGHT,
                                                near_plane, far_plane);

        mats.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(1, 0, 0),  glm::vec3(0, -1, 0)));
        mats.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(-1, 0, 0), glm::vec3(0, -1, 0)));
        mats.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0, 1, 0),  glm::vec3(0, 0, 1)));
        mats.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0, -1, 0), glm::vec3(0, 0, -1)));
        mats.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0, 0, 1),  glm::vec3(0, -1, 0)));
        mats.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0, 0, -1), glm::vec3(0, -1, 0)));
        return mats;
    };

    // Render loop
    while (!glfwWindowShouldClose(window))
    {
        // Calculate delta time
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // FPS and camera position logging
        timeSinceLastPrint += deltaTime;
        framesCount++;
        if (timeSinceLastPrint >= printInterval)
        {
            float fps = static_cast<float>(framesCount) / timeSinceLastPrint;
            std::cout << "Camera position: ("
                      << camera.Position.x << ", "
                      << camera.Position.y << ", "
                      << camera.Position.z << ")"
                      << " | FPS: " << fps << std::endl;
            std::cout << "Control Y: " << control_y << std::endl;

            timeSinceLastPrint = 0.0f;
            framesCount = 0;
        }

        // Process input
        processInput(window);

        // Update current scene
        BaseScene* currentScene = allScenes[currentSceneIndex - 1];
        currentScene->Update(deltaTime);

        // Shadow pass for each sun
        size_t sunCount = currentScene->GetLightCount();
        for (size_t n = 0; n < sunCount; ++n)
        {
            glm::vec3 lightPos = currentScene->GetLightPositions()[n];

            glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
            glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBOs[n]);
            glClear(GL_DEPTH_BUFFER_BIT);
            simpleDepthShader.use();

            std::vector<glm::mat4> shadowMats = GetShadowTransforms(lightPos);
            for (unsigned int i = 0; i < 6; ++i)
            {
                std::string name = "shadowMatrices[" + std::to_string(i) + "]";
                simpleDepthShader.setMat4(name.c_str(), shadowMats[i]);
            }
            simpleDepthShader.setVec3("lightPos", lightPos);
            simpleDepthShader.setFloat("far_plane", far_plane);

            currentScene->RenderDepth(simpleDepthShader);

            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

        // Normal rendering pass
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
        glClearColor(0.f, 0.f, 0.f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Bind shadow maps
        shader.use();
        for (size_t i = 0; i < sunCount; ++i)
        {
            std::string uniformName = "depthMaps[" + std::to_string(i) + "]";
            shader.setInt(uniformName.c_str(), 1 + static_cast<int>(i));
        }

        for (size_t i = 0; i < sunCount; ++i)
        {
            glActiveTexture(GL_TEXTURE1 + i);
            glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemaps[i]);
        }

        // Set projection and view matrices
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom),
                                                static_cast<float>(SCR_WIDTH) / SCR_HEIGHT,
                                                0.1f, far_plane);
        glm::mat4 view = camera.GetViewMatrix();
        shader.setMat4("projection", projection);
        shader.setMat4("view",       view);
        shader.setVec3("viewPos",    camera.Position);
        shader.setFloat("far_plane", far_plane);
        shader.setInt("shadows",     showShadow ? 1 : 0);
        shader.setInt("lightCount",  static_cast<int>(currentScene->GetLightCount()));
        shader.setFloat("ambientS", control_y);

        // Configure light properties
        for (int i = 0; i < static_cast<int>(currentScene->GetLightCount()); i++)
        {
            const glm::vec3& pos = currentScene->GetLightPositions()[i];
            const glm::vec3& col = currentScene->GetLightColors()[i];

            shader.setVec3("lights[" + std::to_string(i) + "].Position", pos);
            shader.setVec3("lights[" + std::to_string(i) + "].Color",    col);
        }

        // Render current scene
        currentScene->Render(shader, camera);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // Resolve MSAA framebuffer
        glBindFramebuffer(GL_READ_FRAMEBUFFER, hdrFBO);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolvedFBO);
        glBlitFramebuffer(0, 0, SCR_WIDTH, SCR_HEIGHT,
                          0, 0, SCR_WIDTH, SCR_HEIGHT,
                          GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT,
                          GL_NEAREST);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // Apply bloom effect
        bloomRenderer.RenderBloomTexture(resolvedColorBuffers[1], bloomFilterRadius);

        // Final rendering pass
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        shaderBloomFinal.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, resolvedColorBuffers[0]);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, bloomRenderer.BloomTexture());
        shaderBloomFinal.setFloat("exposure", exposure);
        bloomRenderer.renderQuad();

        // Swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    bloomRenderer.Destroy();
    glfwTerminate();
    return 0;
}

// Callback function for framebuffer resizing
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    SCR_WIDTH  = width;
    SCR_HEIGHT = height;
    glViewport(0, 0, width, height);
}
