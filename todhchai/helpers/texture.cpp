#include "texture.h"
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

// texture.cpp

GLuint LoadTextureTileBox(const char *texture_file_path, bool doRepeat)
{
    // Flip image vertically if needed (adjust based on your UV mapping)
    stbi_set_flip_vertically_on_load(true);

    int w, h, channels;
    // Force loading the image with 3 channels (RGB)
    unsigned char* img = stbi_load(texture_file_path, &w, &h, &channels, 3);
    if (!img) {
        std::cerr << "Failed to load texture: " << texture_file_path 
                  << ". Reason: " << stbi_failure_reason() << std::endl;
        return 0;
    }

    std::cout << "Loaded texture: " << texture_file_path 
              << " (" << w << "x" << h << "), Channels: " << channels << std::endl;

    // Create and bind a texture object
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    // Set wrap mode (repeat vs. clamp)
    if (doRepeat) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    } else {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        float borderColor[] = { 0.2f, 0.2f, 0.25f, 1.0f };
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    }

    // Basic filtering (before mipmaps)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Since we forced 3 channels, set format accordingly
    GLenum format = GL_RGB;

    // Upload the texture to the GPU
    glTexImage2D(GL_TEXTURE_2D, 0, format, w, h, 0, format, GL_UNSIGNED_BYTE, img);

    // Generate mipmaps for better scaling
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    // Cleanup
    stbi_image_free(img);

    // Return the OpenGL texture ID
    return texture;
}
