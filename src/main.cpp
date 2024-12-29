#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader.h>
#include <learnopengl/camera.h>

#include <iostream>
#include <vector>
#include <cmath>

// Includes for your custom classes
#include "includes/BloomFBO.h"
#include "includes/BloomRenderer.h"
#include "includes/Utils.h"
#include "includes/Input.h"
#include "includes/Cube.h"
#include "includes/Sun.h"
#include "includes/Object.h"

// The new Scene header:
#include "scenes.h"

// Forward declarations
void framebuffer_size_callback(GLFWwindow*, int, int);
// Globals
unsigned int SCR_WIDTH  = 1280;
unsigned int SCR_HEIGHT = 720;
float exposure          = 1.0f;
float bloomFilterRadius = 0.005f;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 5.0f));
float lastX      = (float)SCR_WIDTH / 2.0f;
float lastY      = (float)SCR_HEIGHT / 2.0f;
bool  firstMouse = true;

// timing
float deltaTime        = 0.0f;
float lastFrame        = 0.0f;
float timeSinceLastPrint = 0.0f;
float printInterval      = 1.0f;
int   framesCount        = 0;

// scene switching
int currentSceneIndex = 1; // default scene #1

// shadows
const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
float near_plane  = 1.0f;
float far_plane   = 1000.0f;
bool  showShadow  = true;

int main()
{
    // GLFW init
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Request 4x MSAA
    glfwWindowHint(GLFW_SAMPLES, 8);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Todhchai - Ireland in Future", nullptr, nullptr);
    if (!window)
    {
        std::cerr << "Failed to create GLFW Window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window,   mouse_callback);
    glfwSetScrollCallback(window,      scroll_callback);

    // Capture mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // GLAD init
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to init GLAD" << std::endl;
        return -1;
    }

    // GL states
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_DEPTH_TEST);

    // *** CUBE RENDER FIX *** 
    glDisable(GL_CULL_FACE);

    // Shaders
    Shader shader("shaders/bloom.vs", "shaders/bloom.fs");
    Shader shaderLight("shaders/bloom.vs", "shaders/light_box.fs");
    Shader shaderBloomFinal("shaders/bloom_final.vs", "shaders/bloom_final.fs");
    Shader simpleDepthShader("shaders/point_shadows_depth.vs",
                             "shaders/point_shadows_depth.fs",
                             "shaders/point_shadows_depth.gs");

    // Shadow maps
    const unsigned int MAX_SUNS = 4;
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

    // HDR MSAA FBO
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

    // Resolved FBO
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

    // Ping-pong FBO
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

    // Shader config
    shader.use();
    shader.setInt("diffuseTexture", 0);

    shaderBloomFinal.use();
    shaderBloomFinal.setInt("scene", 0);
    shaderBloomFinal.setInt("bloomBlur", 1);

    simpleDepthShader.use();
    simpleDepthShader.setFloat("far_plane", far_plane);

    shader.use();
    shader.setInt("depthMap", 1);

    // Adjust to real FB size
    glfwMakeContextCurrent(window);
    int fbWidth, fbHeight;
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
    SCR_WIDTH  = fbWidth;
    SCR_HEIGHT = fbHeight;

    // Bloom renderer
    BloomRenderer bloomRenderer;
    bloomRenderer.Init(SCR_WIDTH, SCR_HEIGHT);

    // Prepare Scenes
    ParkScene      parkScene;
    TowerScene     towerScene;
    StructureScene structureScene;

    parkScene.Init(shaderLight, shader);
    towerScene.Init(shaderLight, shader);
    structureScene.Init(shaderLight, shader);

    std::vector<BaseScene*> allScenes = { &parkScene, &towerScene, &structureScene };

    // Helper to build 6 shadow transforms
    auto GetShadowTransforms = [&](const glm::vec3& lightPos)
    {
        std::vector<glm::mat4> mats;
        glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f),
                                                (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT,
                                                near_plane, far_plane);

        mats.push_back(shadowProj * glm::lookAt(lightPos, lightPos+glm::vec3(1,0,0),  glm::vec3(0,-1,0)));
        mats.push_back(shadowProj * glm::lookAt(lightPos, lightPos+glm::vec3(-1,0,0), glm::vec3(0,-1,0)));
        mats.push_back(shadowProj * glm::lookAt(lightPos, lightPos+glm::vec3(0,1,0),  glm::vec3(0,0,1)));
        mats.push_back(shadowProj * glm::lookAt(lightPos, lightPos+glm::vec3(0,-1,0), glm::vec3(0,0,-1)));
        mats.push_back(shadowProj * glm::lookAt(lightPos, lightPos+glm::vec3(0,0,1),  glm::vec3(0,-1,0)));
        mats.push_back(shadowProj * glm::lookAt(lightPos, lightPos+glm::vec3(0,0,-1), glm::vec3(0,-1,0)));
        return mats;
    };

    // Render loop
    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        timeSinceLastPrint += deltaTime;
        framesCount++;
        if (timeSinceLastPrint >= printInterval)
        {
            float fps = (float)framesCount / timeSinceLastPrint;
            std::cout << "Camera position: ("
                      << camera.Position.x << ", "
                      << camera.Position.y << ", "
                      << camera.Position.z << ")"
                      << " | FPS: " << fps << std::endl;

            timeSinceLastPrint = 0.0f;
            framesCount = 0;
        }

        processInput(window);

        // Switch scenes with 1,2,3 if you want (just an example)
        // if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) currentSceneIndex = 1;
        // if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) currentSceneIndex = 2;
        // if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) currentSceneIndex = 3;

        BaseScene* currentScene = allScenes[currentSceneIndex - 1];
        currentScene->Update(deltaTime);

        // Shadow pass
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

        // Normal pass
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
        glClearColor(0.02f, 0.02f, 0.02f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Bind shadow maps
        shader.use();
        for (size_t i = 0; i < sunCount; ++i)
        {
            std::string uniformName = "depthMaps[" + std::to_string(i) + "]";
            shader.setInt(uniformName.c_str(), 1 + (int)i);
        }

        for (size_t i = 0; i < sunCount; ++i)
        {
            glActiveTexture(GL_TEXTURE1 + i);
            glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemaps[i]);
        }

        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom),
                                                (float)SCR_WIDTH / (float)SCR_HEIGHT,
                                                0.1f, far_plane);
        glm::mat4 view = camera.GetViewMatrix();
        shader.setMat4("projection", projection);
        shader.setMat4("view",       view);
        shader.setVec3("viewPos",    camera.Position);
        shader.setFloat("far_plane", far_plane);
        shader.setInt("shadows",     showShadow ? 1 : 0);

        // The crucial part for your bloom.fs (struct Light)
        for (int i = 0; i < (int)currentScene->GetLightCount(); i++)
        {
            const glm::vec3& pos = currentScene->GetLightPositions()[i];
            const glm::vec3& col = currentScene->GetLightColors()[i];

            shader.setVec3("lights[" + std::to_string(i) + "].Position", pos);
            shader.setVec3("lights[" + std::to_string(i) + "].Color",    col);
        }

        currentScene->Render(shader, camera);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // 3) Resolve MSAA
        glBindFramebuffer(GL_READ_FRAMEBUFFER, hdrFBO);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolvedFBO);
        glBlitFramebuffer(0, 0, SCR_WIDTH, SCR_HEIGHT,
                          0, 0, SCR_WIDTH, SCR_HEIGHT,
                          GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT,
                          GL_NEAREST);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // 4) Bloom
        bloomRenderer.RenderBloomTexture(resolvedColorBuffers[1], bloomFilterRadius);

        // 5) Final pass
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        shaderBloomFinal.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, resolvedColorBuffers[0]);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, bloomRenderer.BloomTexture());
        shaderBloomFinal.setFloat("exposure", exposure);
        bloomRenderer.renderQuad();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    bloomRenderer.Destroy();
    glfwTerminate();
    return 0;
}


// ============== CALLBACKS =========================
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    SCR_WIDTH  = width;
    SCR_HEIGHT = height;
    glViewport(0, 0, width, height);
}
