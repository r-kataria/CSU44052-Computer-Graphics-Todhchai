// postprocess.h

#ifndef POSTPROCESS_H
#define POSTPROCESS_H

#include <glad/gl.h>

// HDR Framebuffer and attachments
extern unsigned int hdrFBO;
extern unsigned int hdrColorBuffer;
extern unsigned int rboDepth;

// Bright Extraction Framebuffer
extern unsigned int brightFBO;
extern unsigned int brightTexture;

// Ping-Pong Framebuffers for Gaussian blur
extern unsigned int pingpongFBO[2];
extern unsigned int pingpongColorbuffers[2];

// Fullscreen Quad VAO/VBO
extern unsigned int quadVAO;
extern unsigned int quadVBO;

// Shader program IDs
extern GLuint brightExtractProgram;
extern GLuint blurProgram;
extern GLuint bloomFinalProgram;

// Initialize HDR and Bloom pipeline
void InitHDRBloom(unsigned int width, unsigned int height);

// Extract bright areas from the scene
void ExtractBright(unsigned int sceneTexture);

// Apply Gaussian blur to the bright areas
void BlurBrightTexture();

// Combine the blurred bright areas with the original scene and apply tone mapping
void RenderHDRBloomFinal(unsigned int sceneTex, float exposure);

// Cleanup function
void CleanupPostProcess();

#endif // POSTPROCESS_H
