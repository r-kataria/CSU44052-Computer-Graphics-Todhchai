#include "camera.h"
#include <iostream>
#include <cmath>      // for sin/cos
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Define the globals declared in camera.h
glm::vec3 cameraPos   = glm::vec3(0.0f, 1.0f, 5.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f, 0.0f);

float yaw   = -90.0f;
float pitch = 0.0f;
float lastX = 1024.0f / 2.0f;
float lastY = 768.0f  / 2.0f;
bool  firstMouse = true;
float cameraSpeed = 0.2f;

// Mouse callback
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    static bool s_firstMouseCaptured = true;
    if (s_firstMouseCaptured) {
        lastX = float(xpos);
        lastY = float(ypos);
        s_firstMouseCaptured = false;
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

// Key callback
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {

        glm::vec3 rightVec = glm::normalize(glm::cross(cameraFront, cameraUp));
        switch (key)
        {
            case GLFW_KEY_UP:
                cameraPos += cameraFront * cameraSpeed;
                break;
            case GLFW_KEY_DOWN:
                cameraPos -= cameraFront * cameraSpeed;
                break;
            case GLFW_KEY_LEFT:
                cameraPos -= rightVec * cameraSpeed;
                break;
            case GLFW_KEY_RIGHT:
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

// Print camera position every second (console)
void ShowCameraPosEverySecond() {
    static double lastTime = 0.0;
    double currentTime = glfwGetTime();
    if (currentTime - lastTime >= 1.0) {
        std::cout << "CameraPos = ("
                  << cameraPos.x << ", "
                  << cameraPos.y << ", "
                  << cameraPos.z << ")\n";
        lastTime = currentTime;
    }
}
