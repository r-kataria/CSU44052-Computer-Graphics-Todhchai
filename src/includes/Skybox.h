// skybox.h

#ifndef SKYBOX_H
#define SKYBOX_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>
#include <memory>

// Adjust the include paths based on your project structure
#include <learnopengl/shader.h>
#include <learnopengl/camera.h>



/**
 * @brief Skybox class responsible for loading, rendering, and managing a cubemap-based skybox.
 */
class Skybox
{
public:
    /**
     * @brief Constructor that initializes the Skybox with a shader and cubemap textures.
     * 
     * @param shader Reference to the Shader object used for rendering the skybox.
     * @param faces Vector containing file paths to the six cubemap textures.
     */
    Skybox(Shader& shader, const std::vector<std::string>& faces);

    /**
     * @brief Destructor that cleans up allocated resources.
     */
    ~Skybox();

    /**
     * @brief Renders the skybox.
     * 
     * @param camera Reference to the Camera object providing view and projection matrices.
     */
    void Render(Camera& camera);

    /**
     * @brief Cleans up OpenGL resources manually if needed.
     */
    void Cleanup();

private:
    Shader& m_Shader;            ///< Reference to the shader used for rendering the skybox.
    unsigned int VAO, VBO;       ///< Vertex Array Object and Vertex Buffer Object.
    unsigned int cubemapTexture; ///< ID of the loaded cubemap texture.

    /**
     * @brief Loads a cubemap texture from provided file paths.
     * 
     * @param faces Vector containing file paths to the six cubemap textures.
     * @return The OpenGL texture ID of the loaded cubemap.
     */
    unsigned int loadCubemap(const std::vector<std::string>& faces);
};

#endif // SKYBOX_H
