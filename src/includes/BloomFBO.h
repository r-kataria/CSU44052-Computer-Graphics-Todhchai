#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <glm/glm.hpp>

#include <iostream>
#include <vector>

struct BloomMip
{
	glm::vec2 size;
	glm::ivec2 intSize;
	unsigned int texture;
};

class BloomFBO
{
public:
	BloomFBO();
	~BloomFBO();
	bool Init(unsigned int windowWidth, unsigned int windowHeight, unsigned int mipChainLength);
	void Destroy();
	void BindForWriting();
	const std::vector<BloomMip>& MipChain() const;

private:
	bool mInit;
	unsigned int mFBO;
	std::vector<BloomMip> mMipChain;
};