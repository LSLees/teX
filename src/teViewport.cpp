#include "teViewport.h"
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

static void frameBufferSize_CB(GLFWwindow* w, int width, int height);
static void mouseButton_CB(GLFWwindow* w, int button, int action, int mods);
static void cursorPos_CB(GLFWwindow* w, double px, double py);
static void scroll_CB(GLFWwindow* w, double offsetX, double offsetY);
static GLuint compileShader(GLenum type, const char* src);
static GLuint buildGridProgram();

static const char* gridVertSrc = R"(
#version 330 core

void main()
{
    vec2 pos = vec2((gl_VertexID << 1) & 2, gl_VertexID & 2);
    gl_Position = vec4(pos * 2.0 - 1.0, 0.0, 1.0);
}
)";

// Infinite, zoom/pan-aware grid drawn with screen-space derivatives (fwidth) so lines
// stay crisp at any zoom level. Draws a minor grid, a major grid every N cells, and axes.
static const char* gridFragSrc = R"(
#version 330 core

out vec4 FragColor;

uniform vec2  uResolution;
uniform vec2  uCamPos;
uniform float uZoom;

const float kMinorCell  = 32.0; // world units between minor lines at zoom = 1
const float kMajorEvery = 8.0;  // minor cells per major line

float gridLine(vec2 worldPos, float cellSize)
{
    vec2 coord = worldPos / cellSize;
    vec2 g = abs(fract(coord - 0.5) - 0.5) / fwidth(coord);
    float line = min(g.x, g.y);
    return 1.0 - clamp(line, 0.0, 1.0);
}

void main()
{
    vec2 screen = gl_FragCoord.xy - uResolution * 0.5;
    vec2 world  = screen / uZoom + uCamPos;

    float minor = gridLine(world, kMinorCell);
    float major = gridLine(world, kMinorCell * kMajorEvery);

    vec3  color = vec3(0.0);
    float alpha = 0.0;

    color = mix(color, vec3(1.0), minor * 0.06);
    alpha = max(alpha, minor * 0.06);

    color = mix(color, vec3(1.0), major * 0.16);
    alpha = max(alpha, major * 0.16);

    float axisWidth = 1.5 / uZoom;
    if (abs(world.x) < axisWidth)
    {
        color = mix(color, vec3(0.85, 0.25, 0.25), 0.85);
        alpha = max(alpha, 0.85);
    }
    if (abs(world.y) < axisWidth)
    {
        color = mix(color, vec3(0.25, 0.75, 0.30), 0.85);
        alpha = max(alpha, 0.85);
    }

    FragColor = vec4(color, alpha);
}
)";

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

    grid = buildGridProgram();
    glGenVertexArrays(1, &grid);
}

teViewport::~teViewport()
{
    glDeleteVertexArrays(1, &grid);
    glDeleteProgram(shader);
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

    /*
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();

    ImGui::NewFrame(); 
    ImGui::Begin("Debug");
    ImGui::Text("x:%d", cam.x);
    ImGui::Text("y:%d", cam.y);
    ImGui::End();
    ImGui::Render();
    */

    glfwPollEvents();
    glClearColor(0.0f, 0.25f, 0.25f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    renderGrid();

    //ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

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

void teViewport::renderGrid()
{
    int width = 0;
    int height = 0;
    glfwGetFramebufferSize(window, &width, &height);

    glUseProgram(shader);
    glUniform2f(glGetUniformLocation(shader, "uResolution"), (float)width, (float)height);
    glUniform2f(glGetUniformLocation(shader, "uCamPos"), cam.x, cam.y);
    glUniform1f(glGetUniformLocation(shader, "uZoom"), cam.zoom);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBindVertexArray(grid);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);

    glDisable(GL_BLEND);
    glUseProgram(0);

}

static GLuint buildGridProgram()
{
    GLuint vs = compileShader(GL_VERTEX_SHADER, gridVertSrc);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, gridFragSrc);

    GLuint program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    GLint ok = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &ok);
    if (!ok)
    {
        char log[1024];
        glGetProgramInfoLog(program, sizeof(log), nullptr, log);
    }

    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;
}

static GLuint compileShader(GLenum type, const char* src)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    GLint ok = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok)
    {
        char log[1024];
        glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
    }

    return shader;
}