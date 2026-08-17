#include "teViewport.h"
#include <iostream>

static void frameBufferSize_CB(GLFWwindow* w, int width, int height);
static void mouseButton_CB(GLFWwindow* w, int button, int action, int mods);
static void cursorPos_CB(GLFWwindow* w, double px, double py);
static void scroll_CB(GLFWwindow* w, double offsetX, double offsetY);

teViewport::teViewport()
{
    this->window = glfwCreateWindow(width, height, "Trident Editor X", nullptr, nullptr);
    if (!window)
    {
        glfwTerminate();    
    }

    glfwMakeContextCurrent(window);
    glfwSetWindowUserPointer(window, this);

    glfwSetFramebufferSizeCallback(window, frameBufferSize_CB);
    glfwSetMouseButtonCallback(window, mouseButton_CB);
    glfwSetCursorPosCallback(window, cursorPos_CB);
    glfwSetScrollCallback(window, scroll_CB);

    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);


    //GLuint program = createProgram(vertexShaderSrc, fragmentShaderSrc);
    //GLint resLoc  = glGetUniformLocation(program, "u_resolution");
    //GLint camLoc  = glGetUniformLocation(program, "u_camPos");
    //GLint zoomLoc = glGetUniformLocation(program, "u_zoom");

    std::cout << std::endl;
}

void teViewport::tick()
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }

    glClearColor(0.0f, 0.25f, 0.25f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glfwSwapBuffers(window);
    glfwPollEvents();

    //std::cout << "\r\033[K" << "x:" << cam.x << "  y:" << cam.y;
}

static void frameBufferSize_CB(GLFWwindow* w, int width, int height)
{
    teViewport* teVp = static_cast<teViewport*>(glfwGetWindowUserPointer(w));
    if (!teVp)
    {
        return;
    }

    teVp->width = width;
    teVp->height = height;
    glViewport(0, 0, width, height);
}

static void mouseButton_CB(GLFWwindow* w, int button, int action, int mods)
{
    teViewport* teVp = static_cast<teViewport*>(glfwGetWindowUserPointer(w));
    if (!teVp)
    {
        return;
    }

    if (button == GLFW_MOUSE_BUTTON_LEFT || button == GLFW_MOUSE_BUTTON_MIDDLE)
    {
        if (action == GLFW_PRESS)
        {
            teVp->cam.dragging = true;
        }
        else if (action == GLFW_RELEASE)
        {
            teVp->cam.dragging = false;
        }
    }
}

static void cursorPos_CB(GLFWwindow* w, double posX, double posY)
{
    teViewport* teVp = static_cast<teViewport*>(glfwGetWindowUserPointer(w));
    if (!teVp)
    {
        return;
    }

    if (teVp->cam.dragging)
    {
        double deltaX = posX - teVp->cam.lastX;
        double deltaY = posY - teVp->cam.lastY;

        teVp->cam.x -= (float)deltaX / teVp->cam.zoom;
        teVp->cam.y += (float)deltaY / teVp->cam.zoom;

        teVp->cam.lastX = posX;
        teVp->cam.lastY = posY;
    }
}

static void scroll_CB(GLFWwindow* w, double offsetX, double offsetY)
{
    teViewport* teVp = static_cast<teViewport*>(glfwGetWindowUserPointer(w));
    if (!teVp)
    {
        return;
    }

    double mouseX = 0;
    double mouseY = 0;

    glfwGetCursorPos(w, &mouseX, &mouseY);
}