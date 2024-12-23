#pragma once
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>

// Global camera variables
extern glm::vec3 cameraPos;
extern glm::vec3 cameraFront;
extern glm::vec3 cameraUp;

extern float yaw;
extern float pitch;
extern float lastX;
extern float lastY;
extern bool firstMouse;
extern float cameraSpeed;

// Callback prototypes
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);

void ShowCameraPosEverySecond();
