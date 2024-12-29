#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "../helpers/camera.h"

#include "Input.h"

extern float exposure;
extern Camera camera;

extern float deltaTime;

extern bool firstMouse;

extern float lastX;
extern float lastY;

extern bool showShadow;

extern int currentSceneIndex;

extern float control_y;


void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);

    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
    {
        if (exposure > 0.0f)
            exposure -= 0.001f;
        else
            exposure = 0.0f;
    }
    else if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
    {
        exposure += 0.001f;
    }



if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
    {
        control_y +=0.01;
    }
    else if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
    {
        control_y -=0.01;
    }

    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
        showShadow = !showShadow;
    }

    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) currentSceneIndex = 1;
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) currentSceneIndex = 2;
    if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) currentSceneIndex = 3;
    if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS) currentSceneIndex = 4;

}


// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}
