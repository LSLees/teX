#include "teViewport.h"
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include <iostream>

static void frameBufferSize_CB(GLFWwindow* w, int width, int height);
static void mouseButton_CB(GLFWwindow* w, int button, int action, int mods);
static void cursorPos_CB(GLFWwindow* w, double px, double py);
static void scroll_CB(GLFWwindow* w, double offsetX, double offsetY);

teViewport::teViewport()
{
    this->window = glfwCreateWindow(2250, 1350, "Trident Editor X", nullptr, nullptr);
    if (!window)
    {
        glfwTerminate();    
    }

    glfwMakeContextCurrent(window);
    glfwSetWindowUserPointer(window, this);
    glfwSwapInterval(1);
    glfwSetFramebufferSizeCallback(window, frameBufferSize_CB);
    glfwSetMouseButtonCallback(window, mouseButton_CB);
    glfwSetCursorPosCallback(window, cursorPos_CB);
    glfwSetScrollCallback(window, scroll_CB);
    gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress));

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
    ImGui::GetIO().FontGlobalScale = 2.0f;
}

teViewport::~teViewport()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
}

void teViewport::tick()
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();

    ImGui::NewFrame(); 
    ImGui::Begin("Debug");
    ImGui::Text("x:%d", cam.x);
    ImGui::Text("y:%d", cam.y);
    ImGui::End();
    ImGui::Render();

    //std::cout << cam.x << cam.y << std::endl;

    glfwPollEvents();
    glClearColor(0.0f, 0.25f, 0.25f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(window);
}

static void frameBufferSize_CB(GLFWwindow* w, int width, int height)
{
    teViewport* teVp = static_cast<teViewport*>(glfwGetWindowUserPointer(w));
    if (!teVp)
    {
        return;
    }

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