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

#include "includes/BloomFBO.h"
#include "includes/BloomRenderer.h"
#include "includes/Utils.h"
#include "includes/Input.h"
#include "includes/Cube.h"
#include "includes/Sun.h"
#include "includes/Object.h"

// Function declarations
void framebuffer_size_callback(GLFWwindow* window, int width, int height);

// settings
unsigned int SCR_WIDTH = 1280;
unsigned int SCR_HEIGHT = 720;
float exposure = 1.0f;
float bloomFilterRadius = 0.005f;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 5.0f));
float lastX = (float)SCR_WIDTH / 2.0;
float lastY = (float)SCR_HEIGHT / 2.0;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

float timeSinceLastPrint = 0.0f;
float printInterval = 1.0f; // 1 second


bool firstMouse = true;

// Shadow parameters
const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
float near_plane = 1.0f;
float far_plane = 1000.0f;

const unsigned int NUM_LIGHTS = 4;

bool showShadow = true;

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    // Request 4x MSAA
    glfwWindowHint(GLFW_SAMPLES, 8);

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL with Shadows", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);



    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }



    // Enable multisampling
    glEnable(GL_MULTISAMPLE);


    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    //glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


    // build and compile shaders
    // -------------------------
    Shader shader("shaders/bloom.vs", "shaders/bloom.fs");
    Shader shaderLight("shaders/bloom.vs", "shaders/light_box.fs");
    Shader shaderBloomFinal("shaders/bloom_final.vs", "shaders/bloom_final.fs");
    Shader simpleDepthShader("shaders/point_shadows_depth.vs", 
                            "shaders/point_shadows_depth.fs", 
                            "shaders/point_shadows_depth.gs"); // Depth shader for shadow mapping

    // load textures
    // -------------
    unsigned int woodTexture      = loadTexture(FileSystem::getPath("resources/textures/gray_concrete_powder.png").c_str(), true); // SRGB texture
    unsigned int containerTexture = loadTexture(FileSystem::getPath("resources/textures/barrel_side.png").c_str(), true); // SRGB texture


    // Configure shadow mapping resources
    // ----------------------------------
    // Generate depth cubemap texture
// Create depth cubemaps and framebuffers for each light
unsigned int depthCubemaps[NUM_LIGHTS];
unsigned int depthMapFBOs[NUM_LIGHTS];

glGenTextures(NUM_LIGHTS, depthCubemaps);
glGenFramebuffers(NUM_LIGHTS, depthMapFBOs);

for (unsigned int n = 0; n < NUM_LIGHTS; ++n)
{
    // Generate depth cubemap texture
    glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemaps[n]);
    for (unsigned int i = 0; i < 6; ++i)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT,
                     SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    // Create framebuffer object for shadow mapping
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBOs[n]);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubemaps[n], 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

    // configure (floating point) framebuffers
    // ---------------------------------------
    unsigned int hdrFBO;
    glGenFramebuffers(1, &hdrFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);

    // create 2 floating point multisampled color buffers
    unsigned int colorBuffersMSAA[2];
    glGenTextures(2, colorBuffersMSAA);
    for (unsigned int i = 0; i < 2; i++)
    {
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, colorBuffersMSAA[i]);
        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, GL_TRUE);
        glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        // attach multisampled texture to framebuffer
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D_MULTISAMPLE, colorBuffersMSAA[i], 0);
    }

    // create and attach a multisampled depth buffer (renderbuffer)
    unsigned int rboDepthMSAA;
    glGenRenderbuffers(1, &rboDepthMSAA);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepthMSAA);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepthMSAA);

    // set the draw buffers
    unsigned int attachmentsMSAA[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    glDrawBuffers(2, attachmentsMSAA);

    // check framebuffer completeness
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Multisampled HDR Framebuffer not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);



    // configure a resolved framebuffer (single-sample)
    unsigned int resolvedFBO;
    glGenFramebuffers(1, &resolvedFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, resolvedFBO);

    // create 2 normal (non-multisampled) floating point color buffers
    unsigned int resolvedColorBuffers[2];
    glGenTextures(2, resolvedColorBuffers);
    for (unsigned int i = 0; i < 2; i++)
    {
        glBindTexture(GL_TEXTURE_2D, resolvedColorBuffers[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        // attach texture to framebuffer
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, resolvedColorBuffers[i], 0);
    }

    // create and attach a single-sample depth renderbuffer
    unsigned int resolvedDepthRBO;
    glGenRenderbuffers(1, &resolvedDepthRBO);
    glBindRenderbuffer(GL_RENDERBUFFER, resolvedDepthRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, resolvedDepthRBO);

    // set the draw buffers
    unsigned int resolvedAttachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    glDrawBuffers(2, resolvedAttachments);

    // check framebuffer completeness
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Resolved Framebuffer not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);



    // ping-pong-framebuffer for blurring
    unsigned int pingpongFBO[2];
    unsigned int pingpongColorbuffers[2];
    glGenFramebuffers(2, pingpongFBO);
    glGenTextures(2, pingpongColorbuffers);
    for (unsigned int i = 0; i < 2; i++)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
        glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // Clamp to edge to prevent artifacts
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongColorbuffers[i], 0);
        // check framebuffer completeness
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "Ping-pong Framebuffer not complete!" << std::endl;
    }

    // lighting info
    // -------------
    // positions
    std::vector<glm::vec3> lightPositions;
    lightPositions.push_back(glm::vec3( 0.0f, 0.5f,  1.5f));
    lightPositions.push_back(glm::vec3(-2.465, 18.8801, -4.00198));
    lightPositions.push_back(glm::vec3( 3.0f, 0.5f,  1.0f));
    lightPositions.push_back(glm::vec3(0.465, 13.8801, -6.00198));
    // colors
    std::vector<glm::vec3> lightColors;
    lightColors.push_back(glm::vec3(5.0f, 5.0f, 5.0f));       // Brighter White/Yellow (Midday)
    lightColors.push_back(glm::vec3(12.0f, 4.0f, 0.0f));      // Orange/Red (Sunset/Sunrise) - Increased intensity
    lightColors.push_back(glm::vec3(3.0f, 3.0f, 18.0f));      // Bluish (Twilight/Dusk)
    lightColors.push_back(glm::vec3(2.0f, 10.0f, 2.0f));      // Greenish (Abstract/Stylized)


    // shader configuration
    // --------------------
    shader.use();
    shader.setInt("diffuseTexture", 0);
    shaderBloomFinal.use();
    shaderBloomFinal.setInt("scene", 0);
    shaderBloomFinal.setInt("bloomBlur", 1);

    // Configure depth shader
    simpleDepthShader.use();
    simpleDepthShader.setFloat("far_plane", far_plane);
    // Assuming the depth shader uses texture unit 1 for the depth cubemap
    shader.use();
    shader.setInt("depthMap", 1); // depthMap sampler in the main shader will use texture unit 1

    // Retrieve actual framebuffer size
    glfwMakeContextCurrent(window);
    int framebufferWidth, framebufferHeight;
    glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);
    SCR_WIDTH = framebufferWidth;
    SCR_HEIGHT = framebufferHeight;

    // Initialize bloom renderer with actual framebuffer size
    BloomRenderer bloomRenderer;
    bloomRenderer.Init(SCR_WIDTH, SCR_HEIGHT);


    Object myModelObject(
        shader, 
        FileSystem::getPath("resources/objects/tower.obj"),
        glm::vec3(0.0f, 10.0f, 0.0f),   // position
        glm::vec3(0.0f, 0.0f, 0.0f),   // rotation
        glm::vec3(0.10f, 0.10f, 0.10f)    // scale
    );


    // Create Cube instances
    // ---------------------
    std::vector<Cube> cubes;
    
    // Floor cube
    cubes.emplace_back(shader, woodTexture, 
        glm::vec3(0.0f, -1.0f, 0.0f), 
        glm::vec3(0.0f), 
        glm::vec3(25.0f, 0.5f, 25.0f));

    // Scenery cubes
    cubes.emplace_back(shader, containerTexture, 
        glm::vec3(0.0f, 1.5f, 0.0f), 
        glm::vec3(0.0f, 0.0f, 0.0f), 
        glm::vec3(0.5f));

    cubes.emplace_back(shader, containerTexture, 
        glm::vec3(2.0f, 0.0f, 1.0f), 
        glm::vec3(0.0f, 0.0f, 0.0f), 
        glm::vec3(0.5f));

    cubes.emplace_back(shader, containerTexture, 
        glm::vec3(-1.0f, -1.0f, 2.0f), 
        glm::vec3(0.0f, 60.0f, 0.0f), 
        glm::vec3(1.0f));

    cubes.emplace_back(shader, containerTexture, 
        glm::vec3(0.0f, 2.7f, 4.0f), 
        glm::vec3(23.0f, 0.0f, 0.0f), 
        glm::vec3(1.25f));

    cubes.emplace_back(shader, containerTexture, 
        glm::vec3(-2.0f, 1.0f, -3.0f), 
        glm::vec3(124.0f, 0.0f, 0.0f), 
        glm::vec3(1.0f));

    cubes.emplace_back(shader, containerTexture, 
        glm::vec3(-3.0f, 0.0f, 0.0f), 
        glm::vec3(0.0f, 0.0f, 0.0f), 
        glm::vec3(0.5f));



    // Initialize light cubes
    std::vector<Sun> suns;
    for (unsigned int i = 0; i < lightPositions.size(); i++)
    {
        suns.emplace_back(shaderLight, lightColors[i], 
            lightPositions[i], 
            glm::vec3(0.25f * i), 
            glm::vec3(0.25f));
    }


    // Define shadow transformation matrices as a lambda function
    auto GetShadowTransforms = [&](const glm::vec3& lightPos) -> std::vector<glm::mat4> {
        std::vector<glm::mat4> shadowTransforms;
        glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), 
                                            (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT, 
                                            near_plane, far_plane);
        shadowTransforms.push_back(shadowProj * 
            glm::lookAt(lightPos, lightPos + glm::vec3( 1.0,  0.0,  0.0), glm::vec3(0.0,-1.0,  0.0)));
        shadowTransforms.push_back(shadowProj * 
            glm::lookAt(lightPos, lightPos + glm::vec3(-1.0,  0.0,  0.0), glm::vec3(0.0,-1.0,  0.0)));
        shadowTransforms.push_back(shadowProj * 
            glm::lookAt(lightPos, lightPos + glm::vec3( 0.0,  1.0,  0.0), glm::vec3(0.0, 0.0,  1.0)));
        shadowTransforms.push_back(shadowProj * 
            glm::lookAt(lightPos, lightPos + glm::vec3( 0.0, -1.0,  0.0), glm::vec3(0.0, 0.0, -1.0)));
        shadowTransforms.push_back(shadowProj * 
            glm::lookAt(lightPos, lightPos + glm::vec3( 0.0,  0.0,  1.0), glm::vec3(0.0,-1.0,  0.0)));
        shadowTransforms.push_back(shadowProj * 
            glm::lookAt(lightPos, lightPos + glm::vec3( 0.0,  0.0, -1.0), glm::vec3(0.0,-1.0,  0.0)));
        return shadowTransforms;
    };


    // Render loop
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        // --------------------
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);

            // --- Print camera position every 1 second ---
    timeSinceLastPrint += deltaTime;
    if (timeSinceLastPrint >= printInterval)
    {
        timeSinceLastPrint = 0.0f;
        std::cout << "Camera position: ("
                  << camera.Position.x << ", "
                  << camera.Position.y << ", "
                  << camera.Position.z << ")"
                  << std::endl;
    }

for (unsigned int n = 0; n < NUM_LIGHTS; ++n)
{
    // 1. Render scene to depth cubemap
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBOs[n]);
    glClear(GL_DEPTH_BUFFER_BIT);
    simpleDepthShader.use();

    std::vector<glm::mat4> shadowTransformsMat = GetShadowTransforms(lightPositions[n]);
    for(unsigned int i = 0; i < 6; ++i)
        simpleDepthShader.setMat4("shadowMatrices[" + std::to_string(i) + "]", shadowTransformsMat[i]);

    simpleDepthShader.setVec3("lightPos", lightPositions[n]);
    simpleDepthShader.setFloat("far_plane", far_plane);

    // Render all cubes with depth shader
    for(auto& cube : cubes)
    {
        cube.RenderDepth(simpleDepthShader);
    }
    myModelObject.RenderDepth(simpleDepthShader);
 
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

        // 2. Render scene as normal with shadows into HDR framebuffer
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);





shader.use();
for (unsigned int i = 0; i < NUM_LIGHTS; ++i)
{
    shader.setInt("depthMaps[" + std::to_string(i) + "]", 1 + i); // Assuming texture unit 0 is for diffuse texture
}

// Bind each depth cubemap to texture units starting from 1
for (unsigned int i = 0; i < NUM_LIGHTS; ++i)
{
    glActiveTexture(GL_TEXTURE1 + i);
    glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemaps[i]);
}






        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom),
                                (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, far_plane);
        glm::mat4 view = camera.GetViewMatrix();
        shader.setMat4("projection", projection);
        shader.setMat4("view", view);
        // Set lighting uniforms
        shader.setVec3("viewPos", camera.Position);
        shader.setFloat("far_plane", far_plane);
        shader.setInt("shadows", showShadow); // Enable shadows


        for(auto& cube : cubes)
        {
            cube.Render(camera, lightPositions, lightColors);
        }

        // Render all suns
        for(auto& s : suns)
        {
            s.Render(camera, lightPositions, lightColors);
        }

        myModelObject.Render(camera, lightPositions, lightColors);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // 3. Resolve multisampled framebuffer to resolvedFBO
        glBindFramebuffer(GL_READ_FRAMEBUFFER, hdrFBO);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolvedFBO);
        glBlitFramebuffer(0, 0, SCR_WIDTH, SCR_HEIGHT, 0, 0, SCR_WIDTH, SCR_HEIGHT,
                          GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // 4. Perform Bloom on the resolved color buffer
        bloomRenderer.RenderBloomTexture(resolvedColorBuffers[1], bloomFilterRadius);

        // 5. Final render pass to screen using resolved color buffer and bloom texture
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        shaderBloomFinal.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, resolvedColorBuffers[0]); // Resolved scene color
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, bloomRenderer.BloomTexture()); // Bloom texture
        shaderBloomFinal.setFloat("exposure", exposure);
        bloomRenderer.renderQuad();

        // Swap buffers and poll IO events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }



    bloomRenderer.Destroy();
    glfwTerminate();
    return 0;
}


// Update the framebuffer_size_callback to modify SCR_WIDTH and SCR_HEIGHT
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    SCR_WIDTH = width;
    SCR_HEIGHT = height;
    glViewport(0, 0, width, height);
}


