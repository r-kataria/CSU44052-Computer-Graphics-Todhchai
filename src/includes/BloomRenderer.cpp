#include "BloomRenderer.h"
#include "../helpers/shader.h"
#include <vector>





// renderQuad() renders a 1x1 XY quad in NDC
// -----------------------------------------
unsigned int quadVAO = 0;
unsigned int quadVBO;
void BloomRenderer::renderQuad()
{
    if (quadVAO == 0)
    {
        float quadVertices[] = {
            // positions        // texture Coords
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
             1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
             1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        // setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}




BloomRenderer::BloomRenderer() : mInit(false) {}
BloomRenderer::~BloomRenderer() {}

bool BloomRenderer::Init(unsigned int windowWidth, unsigned int windowHeight)
{
    if (mInit) return true;
    mSrcViewportSize = glm::ivec2(windowWidth, windowHeight);
    mSrcViewportSizeFloat = glm::vec2((float)windowWidth, (float)windowHeight);

    // Framebuffer initialization remains the same
    const unsigned int num_bloom_mips = 12;
    bool status = mFBO.Init(windowWidth, windowHeight, num_bloom_mips);
    if (!status) {
        std::cerr << "Failed to initialize bloom FBO - cannot create bloom renderer!\n";
        return false;
    }

    // Shaders setup remains the same
    mDownsampleShader = new Shader("shaders/downsample.vs", "shaders/downsample.fs");
    mUpsampleShader = new Shader("shaders/upsample.vs", "shaders/upsample.fs");

    // Configure shader uniforms
    mDownsampleShader->use();
    mDownsampleShader->setInt("srcTexture", 0);
    glUseProgram(0);

    mUpsampleShader->use();
    mUpsampleShader->setInt("srcTexture", 0);
    glUseProgram(0);

    return true;
}

void BloomRenderer::Destroy()
{
	mFBO.Destroy();
	delete mDownsampleShader;
	delete mUpsampleShader;
}
void BloomRenderer::RenderDownsamples(unsigned int srcTexture)
{
    const std::vector<BloomMip>& mipChain = mFBO.MipChain();

    mDownsampleShader->use();
    mDownsampleShader->setVec2("srcResolution", mSrcViewportSizeFloat);
    if (mKarisAverageOnDownsample) {
        mDownsampleShader->setInt("mipLevel", 0);
    }

    // Bind srcTexture as initial texture input
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, srcTexture);

    // Progressively downsample through the mip chain
    for (int i = 0; i < (int)mipChain.size(); i++)
    {
        const BloomMip& mip = mipChain[i];
        glViewport(0, 0, mip.size.x, mip.size.y);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_2D, mip.texture, 0);

        // Render screen-filled quad
        renderQuad();

        // Update shader uniforms for next mip
        mDownsampleShader->setVec2("srcResolution", mip.size);
        glBindTexture(GL_TEXTURE_2D, mip.texture);

        // Disable Karis average for subsequent mips
        if (i == 0) { mDownsampleShader->setInt("mipLevel", 1); }
    }

    glUseProgram(0);
}

void BloomRenderer::RenderUpsamples(float filterRadius)
{
    const std::vector<BloomMip>& mipChain = mFBO.MipChain();

    mUpsampleShader->use();
    mUpsampleShader->setFloat("filterRadius", filterRadius);

    // Enable additive blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    glBlendEquation(GL_FUNC_ADD);

    for (int i = (int)mipChain.size() - 1; i > 0; i--)
    {
        const BloomMip& mip = mipChain[i];
        const BloomMip& nextMip = mipChain[i - 1];

        // Bind texture to read from
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mip.texture);

        // Set framebuffer to write to the next mip
        glViewport(0, 0, nextMip.size.x, nextMip.size.y);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_2D, nextMip.texture, 0);

        // Render screen-filled quad
        renderQuad();
    }

    // Disable additive blending
    glDisable(GL_BLEND);

    glUseProgram(0);
}

void BloomRenderer::RenderBloomTexture(unsigned int srcTexture, float filterRadius)
{
	mFBO.BindForWriting();

	this->RenderDownsamples(srcTexture);
	this->RenderUpsamples(filterRadius);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	// Restore viewport
	glViewport(0, 0, mSrcViewportSize.x, mSrcViewportSize.y);
}

GLuint BloomRenderer::BloomTexture()
{
	return mFBO.MipChain()[0].texture;
}

GLuint BloomRenderer::BloomMip_i(int index)
{
	const std::vector<BloomMip>& mipChain = mFBO.MipChain();
	int size = (int)mipChain.size();
	return mipChain[(index > size-1) ? size-1 : (index < 0) ? 0 : index].texture;
}
