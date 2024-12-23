#pragma once
#include <glad/gl.h>

// Load a 2D texture from file, optionally repeating or clamping
GLuint LoadTextureTileBox(const char *texture_file_path, bool doRepeat);
