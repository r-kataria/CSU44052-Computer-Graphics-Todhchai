#include "texture.h"
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

GLuint LoadTextureTileBox(const char *texture_file_path, bool doRepeat)
{
    // Flip image if you want textures to match typical UV coordinate systems.
    // If your model's UVs are already correct, you can keep this as false.
    stbi_set_flip_vertically_on_load(false);

    int w, h, channels;
    // Ask STB to return the *original* channels (3 or 4) so we can pick the correct format below.
    // If you always want RGBA, replace the last parameter (0) with 4.
    unsigned char* img = stbi_load(texture_file_path, &w, &h, &channels, 0);
    if (!img) {
        std::cerr << "Failed to load texture: " << texture_file_path << std::endl;
        return 0;
    }

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

    // Determine the correct format
    GLenum format;
    if (channels == 3) {
        format = GL_RGB;
    } else if (channels == 4) {
        format = GL_RGBA;
    } else {
        // Fallback, though very rare if not 3 or 4
        format = GL_RGB;
        std::cerr << "Warning: texture has unexpected channel count = " << channels << ". Defaulting to GL_RGB.\n";
    }

    // Upload the texture to the GPU
    glTexImage2D(GL_TEXTURE_2D, 0, format, w, h, 0, format, GL_UNSIGNED_BYTE, img);

    // If you want mipmaps:
    glGenerateMipmap(GL_TEXTURE_2D);
    // And set the filtering to use the mipmaps
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    // Cleanup
    stbi_image_free(img);

    // Return the OpenGL texture ID
    return texture;
}
