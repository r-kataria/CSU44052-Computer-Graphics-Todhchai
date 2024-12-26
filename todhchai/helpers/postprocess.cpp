// postprocess.cpp

#include "postprocess.h"
#include <iostream>
#include <glm/glm.hpp>
#include "../render/shader.h" // For LoadShadersFromFile

// Define extern variables
unsigned int hdrFBO         = 0;
unsigned int hdrColorBuffer = 0;
unsigned int rboDepth       = 0;

// Bright Extraction Framebuffer and Texture
unsigned int brightFBO     = 0;
unsigned int brightTexture = 0;

// Ping-Pong Framebuffers for Gaussian blur
unsigned int pingpongFBO[2]         = {0, 0};
unsigned int pingpongColorbuffers[2] = {0, 0};

// Fullscreen Quad VAO/VBO
unsigned int quadVAO = 0;
unsigned int quadVBO = 0;

// Shader program IDs
GLuint brightExtractProgram = 0;
GLuint blurProgram          = 0;
GLuint bloomFinalProgram    = 0;

// Quad vertices
static float quadVertices[] = {
    // positions   // texCoords
    -1.0f,  1.0f,  0.0f, 1.0f,
    -1.0f, -1.0f,  0.0f, 0.0f,
     1.0f,  1.0f,  1.0f, 1.0f,
     1.0f, -1.0f,  1.0f, 0.0f,
};

// Declare lastPingPong as a static global variable
static unsigned int lastPingPong = 0; // Tracks the last ping-pong buffer used

// Render a fullscreen quad
static void RenderQuad()
{
    if (quadVAO == 0)
    {
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);

        // Position attribute
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(
            0, 
            2, 
            GL_FLOAT, 
            GL_FALSE, 
            4 * sizeof(float), 
            (void*)0
        );

        // TexCoords attribute
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(
            1, 
            2, 
            GL_FLOAT, 
            GL_FALSE, 
            4 * sizeof(float), 
            (void*)(2 * sizeof(float))
        );
    }

    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

void InitHDRBloom(unsigned int width, unsigned int height)
{
    // 1. HDR Framebuffer
    glGenFramebuffers(1, &hdrFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);

    // Create floating point color buffer
    glGenTextures(1, &hdrColorBuffer);
    glBindTexture(GL_TEXTURE_2D, hdrColorBuffer);
    glTexImage2D(
        GL_TEXTURE_2D, 0, GL_RGBA16F, 
        width, height, 
        0, GL_RGBA, GL_FLOAT, NULL
    );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    // Attach to FBO
    glFramebufferTexture2D(
        GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
        GL_TEXTURE_2D, hdrColorBuffer, 0
    );

    // Create renderbuffer for depth
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "HDR Framebuffer not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // 2. Bright-extraction Framebuffer
    glGenFramebuffers(1, &brightFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, brightFBO);

    // Create bright texture
    glGenTextures(1, &brightTexture);
    glBindTexture(GL_TEXTURE_2D, brightTexture);
    glTexImage2D(
        GL_TEXTURE_2D, 0, GL_RGBA16F, 
        width, height, 
        0, GL_RGBA, GL_FLOAT, NULL
    );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    // Attach to FBO
    glFramebufferTexture2D(
        GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
        GL_TEXTURE_2D, brightTexture, 0
    );

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Bright Extraction Framebuffer not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // 3. Ping-Pong Framebuffers for Gaussian blur
    glGenFramebuffers(2, pingpongFBO);
    glGenTextures(2, pingpongColorbuffers);
    for (unsigned int i = 0; i < 2; i++)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
        glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[i]);
        glTexImage2D(
            GL_TEXTURE_2D, 0, GL_RGBA16F, 
            width, height, 
            0, GL_RGBA, GL_FLOAT, NULL
        );
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(
            GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
            GL_TEXTURE_2D, pingpongColorbuffers[i], 0
        );
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "Ping-Pong Framebuffer " << i << " not complete!" << std::endl;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // 4. Load the three post-processing shaders
    brightExtractProgram = LoadShadersFromFile(
        "../todhchai/shaders/screenQuad.vert", 
        "../todhchai/shaders/brightExtract.frag"
    );
    if (brightExtractProgram == 0) {
        std::cout << "Failed to load brightExtractProgram" << std::endl;
    }

    blurProgram = LoadShadersFromFile(
        "../todhchai/shaders/screenQuad.vert", 
        "../todhchai/shaders/blur.frag"
    );
    if (blurProgram == 0) {
        std::cout << "Failed to load blurProgram" << std::endl;
    }

    bloomFinalProgram = LoadShadersFromFile(
        "../todhchai/shaders/screenQuad.vert", 
        "../todhchai/shaders/bloomFinal.frag"
    );
    if (bloomFinalProgram == 0) {
        std::cout << "Failed to load bloomFinalProgram" << std::endl;
    }
}

void ExtractBright(unsigned int sceneTexture)
{
    // Use brightExtractProgram
    glUseProgram(brightExtractProgram);

    // Set uniforms
    GLint locScene = glGetUniformLocation(brightExtractProgram, "scene");
    glUniform1i(locScene, 0);

    // Bind scene texture to texture unit 0
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, sceneTexture);

    // Bind to brightFBO
    glBindFramebuffer(GL_FRAMEBUFFER, brightFBO);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Render quad
    RenderQuad();

    // Unbind framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void BlurBrightTexture()
{
    bool horizontal = true;
    bool first_iteration = true;
    unsigned int amount = 50; // Number of blur passes

    glUseProgram(blurProgram);
    GLint locHorizontal = glGetUniformLocation(blurProgram, "horizontal");

    for (unsigned int i = 0; i < amount; i++)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
        glUniform1i(locHorizontal, horizontal ? 1 : 0);

        // Bind the appropriate texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, first_iteration ? brightTexture 
                                                    : pingpongColorbuffers[!horizontal]);

        // Render quad
        RenderQuad();

        horizontal = !horizontal;
        if (first_iteration)
            first_iteration = false;
        lastPingPong = !horizontal; // Track the last used buffer
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void RenderHDRBloomFinal(unsigned int sceneTex, float exposure)
{
    glUseProgram(bloomFinalProgram);
    // Set uniforms
    GLint locScene = glGetUniformLocation(bloomFinalProgram, "scene");
    GLint locBloom = glGetUniformLocation(bloomFinalProgram, "bloomBlur");
    GLint locExposure = glGetUniformLocation(bloomFinalProgram, "exposure");
    glUniform1i(locScene, 0);
    glUniform1i(locBloom, 1);
    glUniform1f(locExposure, exposure);

    // Bind textures
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, sceneTex);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[lastPingPong]);

    // Render to default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Render quad
    RenderQuad();
}

void CleanupPostProcess()
{
    // Delete framebuffers and textures
    glDeleteFramebuffers(1, &hdrFBO);
    glDeleteTextures(1, &hdrColorBuffer);
    glDeleteRenderbuffers(1, &rboDepth);

    glDeleteFramebuffers(1, &brightFBO);
    glDeleteTextures(1, &brightTexture);

    glDeleteFramebuffers(2, pingpongFBO);
    glDeleteTextures(2, pingpongColorbuffers);

    // Delete quad VAO/VBO
    glDeleteVertexArrays(1, &quadVAO);
    glDeleteBuffers(1, &quadVBO);

    // Delete shader programs
    glDeleteProgram(brightExtractProgram);
    glDeleteProgram(blurProgram);
    glDeleteProgram(bloomFinalProgram);
}
