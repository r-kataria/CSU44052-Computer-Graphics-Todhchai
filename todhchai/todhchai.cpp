#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <render/shader.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <iostream>
#include <cstdlib> // For srand and rand
#include <ctime>   // For time
#define _USE_MATH_DEFINES
#include <math.h>

// Forward declarations
static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode);
static void mouse_callback(GLFWwindow* window, double xpos, double ypos);

// --------------------------------------------------------
// 1) Minecraft-style camera globals
// --------------------------------------------------------
static glm::vec3 cameraPos   = glm::vec3(0.0f,  1.0f,  5.0f);
static glm::vec3 cameraFront = glm::vec3(0.0f,  0.0f, -1.0f);
static glm::vec3 cameraUp    = glm::vec3(0.0f,  1.0f,  0.0f);

static float yaw   = -90.0f;   // Start facing down -Z
static float pitch = 0.0f;

static float lastX = 1024.0f / 2.0f; // half window width
static float lastY =  768.0f / 2.0f; // half window height
static bool  firstMouse = true;

static float cameraSpeed = 0.2f; // Movement speed

// Window pointer
static GLFWwindow *window;

// For selecting which texture to use
int UsedTextureIndex = 0;

// --------------------------------------------------------
// Utility: Load a single 2D texture (for skybox faces)
// --------------------------------------------------------
static GLuint LoadTextureTileBox(const char *texture_file_path) {
    stbi_set_flip_vertically_on_load(true);
    int w, h, channels;
    uint8_t *img = stbi_load(texture_file_path, &w, &h, &channels, 4);
    if (!img) {
        std::cout << "Failed to load texture " << texture_file_path << std::endl;
        return 0;
    }

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    // For skybox, clamp to edge
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    // We'll use linear filter, no mipmap
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    GLenum format = (channels == 4) ? GL_RGBA : GL_RGB;
    glTexImage2D(GL_TEXTURE_2D, 0, format, w, h, 0, format, GL_UNSIGNED_BYTE, img);

    stbi_image_free(img);
    return texture;
}

// --------------------------------------------------------
// Building struct - Here used as our "Skybox" geometry
// --------------------------------------------------------
struct Building {
    glm::vec3 position;
    glm::vec3 scale;

    // Cube data
    GLfloat vertex_buffer_data[72] = { // 24 vertices (6 faces, 4 corners each)
        // Front face
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,

        // Back face
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,

        // Left face
        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        // Right face
         1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,

        // Top face
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        // Bottom face
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,
    };

    GLuint index_buffer_data[36] = { // 6 faces * 2 triangles * 3 indices
        0,1,2,   0,2,3,    // front
        4,5,6,   4,6,7,    // back
        8,9,10,  8,10,11,  // left
        12,13,14,12,14,15, // right
        16,17,18,16,18,19, // top
        20,21,22,20,22,23  // bottom
    };

    // UV layout (typical cross layout for a skybox texture)
    GLfloat uv_buffer_data[48] = {
        // Front face (Column 1, Row 1)
        0.25f, 0.33333f,
        0.50f, 0.33333f,
        0.50f, 0.66667f,
        0.25f, 0.66667f,

        // Back face (Column 3, Row 1)
        0.75f, 0.33333f,
        1.00f, 0.33333f,
        1.00f, 0.66667f,
        0.75f, 0.66667f,

        // Left face (Column 0, Row 1)
        0.00f, 0.33333f,
        0.25f, 0.33333f,
        0.25f, 0.66667f,
        0.00f, 0.66667f,

        // Right face (Column 2, Row 1)
        0.50f, 0.33333f,
        0.75f, 0.33333f,
        0.75f, 0.66667f,
        0.50f, 0.66667f,

        // Top face (Column 1, Row 0)
        0.25f, 0.66667f,
        0.50f, 0.66667f,
        0.50f, 1.00000f,
        0.25f, 1.00000f,

        // Bottom face (Column 1, Row 2)
        0.25f, 0.00f,
        0.50f, 0.00f,
        0.50f, 0.33333f,
        0.25f, 0.33333f,
    };

    // OpenGL handles
    GLuint vertexArrayID;
    GLuint vertexBufferID;
    GLuint indexBufferID;
    GLuint uvBufferID;
    GLuint TextureIds[4];

    // Shader uniform locations
    GLuint mvpMatrixID;
    GLuint textureSamplerID;
    GLuint programID;

    void initialize(const glm::vec3& pos, const glm::vec3& scl)
    {
        this->position = pos;
        this->scale    = scl;

        // Create VAO
        glGenVertexArrays(1, &vertexArrayID);
        glBindVertexArray(vertexArrayID);

        // Vertex buffer
        glGenBuffers(1, &vertexBufferID);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_buffer_data), vertex_buffer_data, GL_STATIC_DRAW);

        // UV buffer
        glGenBuffers(1, &uvBufferID);
        glBindBuffer(GL_ARRAY_BUFFER, uvBufferID);
        glBufferData(GL_ARRAY_BUFFER, sizeof(uv_buffer_data), uv_buffer_data, GL_STATIC_DRAW);

        // Index buffer
        glGenBuffers(1, &indexBufferID);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index_buffer_data), index_buffer_data, GL_STATIC_DRAW);

        // Load/compile skybox shaders
        programID = LoadShadersFromFile("../todhchai/skybox.vert", "../todhchai/skybox.frag");
        if (programID == 0) {
            std::cerr << "Failed to load skybox shaders." << std::endl;
        }

        // Get uniform locations
        mvpMatrixID      = glGetUniformLocation(programID, "MVP");
        textureSamplerID = glGetUniformLocation(programID, "textureSampler");

        // Load all skybox textures
        TextureIds[0] = LoadTextureTileBox("../assets/sky.png");
        TextureIds[1] = LoadTextureTileBox("../assets/sky_debug.png");
        TextureIds[2] = LoadTextureTileBox("../assets/studio_garden.png");
        TextureIds[3] = LoadTextureTileBox("../assets/studio_garden_debug.png");

        // Make sure we set the sampler to texture unit 0
        glUseProgram(programID);
        glUniform1i(textureSamplerID, 0);
    }

    void render(const glm::mat4& vp, int flip)
    {
        glUseProgram(programID);

        // Bind our VAO
        glBindVertexArray(vertexArrayID);

        // Positions
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

        // UVs
        glEnableVertexAttribArray(2);
        glBindBuffer(GL_ARRAY_BUFFER, uvBufferID);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

        // Indices
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);

        // Model transform
        glm::mat4 model = glm::translate(glm::mat4(1.0f), position);
        model = glm::scale(model, scale);

        // MVP
        glm::mat4 mvp = vp * model;
        glUniformMatrix4fv(mvpMatrixID, 1, GL_FALSE, &mvp[0][0]);

        // Flip?
        glUniform1i(glGetUniformLocation(programID, "Flip"), flip);

        // Bind the chosen texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, TextureIds[UsedTextureIndex]);

        // Draw
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

        // Cleanup
        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(2);
    }

    void cleanup()
    {
        glDeleteBuffers(1, &vertexBufferID);
        glDeleteBuffers(1, &uvBufferID);
        glDeleteBuffers(1, &indexBufferID);
        glDeleteVertexArrays(1, &vertexArrayID);

        // In this example, TextureIds array was used; if you re-used the same
        // building for multiple, handle carefully. For demo, we won't do a full
        // texture free here. In a real project, you'd call glDeleteTextures if needed.

        glDeleteProgram(programID);
    }
};

// --------------------------------------------------------
// Mouse callback: updates yaw/pitch based on mouse movement
// --------------------------------------------------------
static void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse) {
        lastX = float(xpos);
        lastY = float(ypos);
        firstMouse = false;
    }
    float xoffset = float(xpos) - lastX;
    float yoffset = lastY - float(ypos);  // reversed
    lastX = float(xpos);
    lastY = float(ypos);

    // Sensitivity
    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    // Update yaw & pitch
    yaw   += xoffset;
    pitch += yoffset;

    // Constrain pitch
    if (pitch > 89.0f)  pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    // Recalculate cameraFront
    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(direction);
}

// --------------------------------------------------------
// Key callback: arrow keys for movement
// --------------------------------------------------------
static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    // Move if pressed or repeated
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {

        // XZ-plane movement
        glm::vec3 rightVec = glm::normalize(glm::cross(cameraFront, cameraUp));
        switch (key)
        {
            case GLFW_KEY_UP:    // Forward
                cameraPos += glm::vec3(cameraFront.x, 0.0f, cameraFront.z) * cameraSpeed;
                break;
            case GLFW_KEY_DOWN:  // Backward
                cameraPos -= glm::vec3(cameraFront.x, 0.0f, cameraFront.z) * cameraSpeed;
                break;
            case GLFW_KEY_LEFT:  // Strafe left
                cameraPos -= rightVec * cameraSpeed;
                break;
            case GLFW_KEY_RIGHT: // Strafe right
                cameraPos += rightVec * cameraSpeed;
                break;

            case GLFW_KEY_ESCAPE:
                glfwSetWindowShouldClose(window, GL_TRUE);
                break;

            default:
                break;
        }
    }
}

// --------------------------------------------------------
// Main
// --------------------------------------------------------
int main(void) 
{
    // Init GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW.\n";
        return -1;
    }

    // OpenGL version hints
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // For Mac
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create window
    window = glfwCreateWindow(1024, 768, "Lab 2 - Skybox", nullptr, nullptr);
    if (window == nullptr) {
        std::cerr << "Failed to open a GLFW window.\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Capture mouse, hide cursor
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Set callbacks
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, mouse_callback);

    // Load GL with glad
    int version = gladLoadGL(glfwGetProcAddress);
    if (version == 0) {
        std::cerr << "Failed to initialize OpenGL context.\n";
        return -1;
    }

    // Basic GL config
    glClearColor(0.2f, 0.2f, 0.25f, 0.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // Cull front faces so interior of cube is shown
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);

    // Create the skybox
    Building skybox;
    skybox.initialize(glm::vec3(0.0f), glm::vec3(50.0f)); // big box

    // Projection matrix
    float FoV = 45.0f;
    float zNear = 0.1f;
    float zFar = 1000.0f;
    glm::mat4 projectionMatrix = glm::perspective(glm::radians(FoV),
                                                  4.0f/3.0f,
                                                  zNear,
                                                  zFar);

    // We'll toggle this depending on the texture
    int flip = 0;

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        // Clear
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Handle texture selection
        if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
            UsedTextureIndex = 0;
            flip = 0;
        }
        else if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) {
            UsedTextureIndex = 1;
            flip = 0;
        }
        else if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) {
            UsedTextureIndex = 2;
            flip = 1;
        }
        else if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS) {
            UsedTextureIndex = 3;
            flip = 1;
        }

        // Build the FPS-style view matrix
        glm::mat4 viewMatrix = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        glm::mat4 vp = projectionMatrix * viewMatrix;

        // Draw skybox
        skybox.render(vp, flip);

        // Swap + Poll
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    skybox.cleanup();
    glfwTerminate();
    return 0;
}
