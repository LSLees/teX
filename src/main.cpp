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
    
    while (!glfwWindowShouldClose(vp.window))
    {
        vp.tick();
    }

    glfwTerminate();
    return 0;
}