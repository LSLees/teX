#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>

class teCamera
{
public:
    float x = 0.0f;
    float y = 0.0f;
    float lastX = 0.0f;
    float lastY = 0.0f;
    float zoom = 1.0f;

    bool dragging = false;
};

class teViewport
{
public:
    GLFWwindow* window;
    teCamera cam;

    teViewport();
    ~teViewport();
    void tick();
};