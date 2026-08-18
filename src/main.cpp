#include "teViewport.h"

int main()
{
    if (!glfwInit())
    {
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    teViewport vp;

    if (!vp.window)
    {
        return 1;
    }
    
    while (!glfwWindowShouldClose(vp.window))
    {
        vp.tick();
    }
    
    return 0;
}