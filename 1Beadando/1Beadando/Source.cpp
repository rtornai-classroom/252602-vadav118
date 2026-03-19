#define _USE_MATH_DEFINES
#include <math.h>
#include <array>
#include <iostream>
#include <fstream>
#include <string>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

using namespace std;
using namespace glm;

// ---- KONSTANSOK ----
const float SCALE = 2.0f / 600.0f;
const int SEGMENTS = 100;
const int CIRCLE_VERTS = SEGMENTS + 2;
const int LINE_VERTS = 2;
const int TOTAL_VERTS = CIRCLE_VERTS + LINE_VERTS;

std::array<vec2, TOTAL_VERTS> vertices;

// ---- ABLAK ----
int window_width = 600;
int window_height = 600;
char window_title[] = "Beadando 1";

GLFWwindow* window = nullptr;

// ---- MOZGÁS (PIXELBEN!) ----
float cx = 0.0f;
float cy = 0.0f;

float vx = 10.0f * SCALE;
float vy = 0.0f;

float radius = 50.0f * SCALE;
bool angeld = false;

float lineY = 0.0f;
float Speed = 10.0f * SCALE;



// ---- OPENGL ----
#define numVBOs 1
#define numVAOs 1

GLuint VBO[numVBOs];
GLuint VAO[numVAOs];
GLuint renderingProgram;

// ---- SHADER ----
string readShaderSource(const char* filePath) {
    ifstream file(filePath);
    string content((istreambuf_iterator<char>(file)),
        istreambuf_iterator<char>());
    return content;
}

GLuint createShaderProgram() {
    string vert = readShaderSource("vertexShader.glsl");
    string frag = readShaderSource("fragmentShader.glsl");

    const char* vsrc = vert.c_str();
    const char* fsrc = frag.c_str();

    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(vs, 1, &vsrc, NULL);
    glShaderSource(fs, 1, &fsrc, NULL);

    glCompileShader(vs);
    glCompileShader(fs);

    GLuint prog = glCreateProgram();
    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    glLinkProgram(prog);

    glDeleteShader(vs);
    glDeleteShader(fs);

    return prog;
}

// ---- INIT ----
void init(GLFWwindow* window) {
    renderingProgram = createShaderProgram();

    // EGYSÉGKÖR
    vertices[0] = vec2(0.0f, 0.0f);
    for (int i = 0; i <= SEGMENTS; i++) {
        float angle = 2.0f * M_PI * i / SEGMENTS;
        vertices[i + 1] = vec2(cos(angle), sin(angle));
    }

    // VONAL
    vertices[CIRCLE_VERTS] = vec2(-0.3333f, 0.0f);
    vertices[CIRCLE_VERTS + 1] = vec2(0.3333f, 0.0f);

    glGenVertexArrays(1, VAO);
    glBindVertexArray(VAO[0]);

    glGenBuffers(1, VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);

    glBufferData(GL_ARRAY_BUFFER,
        vertices.size() * sizeof(vec2),
        vertices.data(),
        GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glUseProgram(renderingProgram);

    glClearColor(0.85f, 0.75f, 0.15f, 1.0f);
}

// ---- DISPLAY ----
void display(GLFWwindow* window, double currentTime) {
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(renderingProgram);
    glBindVertexArray(VAO[0]);
    
    // MOZGÁS ÉS PATTANÁS
    cx += vx;
    cy += vy;

    if (cx + radius > 1.0f || cx - radius < -1.0f)
        vx = -vx;

    if (cy + radius > 1.0f || cy - radius < -1.0f)
        vy = -vy;

    // ÜTKÖZÉS
    float startLine = -0.3333f;
    float endLine = 0.3333f;

    bool overlap = (cx >= startLine && cx <= endLine) && (abs(cy - lineY) <= radius);

    // KÖR
    glUniform1i(glGetUniformLocation(renderingProgram, "drawLine"), GL_FALSE);
    glUniform2f(glGetUniformLocation(renderingProgram, "circleCenter"), cx, cy);
    glUniform1f(glGetUniformLocation(renderingProgram, "radius"), radius);
    glUniform1i(glGetUniformLocation(renderingProgram, "colorSwap"), overlap);

    glDrawArrays(GL_TRIANGLE_FAN, 0, CIRCLE_VERTS);

    // VONAL
    glUniform1i(glGetUniformLocation(renderingProgram, "drawLine"), GL_TRUE);
    glUniform1f(glGetUniformLocation(renderingProgram, "lineY"), lineY);

    glLineWidth(3.0f);
    glDrawArrays(GL_LINES, CIRCLE_VERTS, LINE_VERTS);
}


void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void cursorPosCallback(GLFWwindow* window, double xPos, double yPos) {}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if ((action == GLFW_PRESS) && (key == GLFW_KEY_ESCAPE))
        glfwSetWindowShouldClose(window, GLFW_TRUE);

    if (key == GLFW_KEY_UP && (action == GLFW_PRESS || action == GLFW_REPEAT))
        lineY += Speed;

    if (key == GLFW_KEY_DOWN && (action == GLFW_PRESS || action == GLFW_REPEAT))
        lineY -= Speed;

    if (key == GLFW_KEY_S && action == GLFW_PRESS && !angeld) {
        float angle = 25.0f * M_PI / 180.0f;
        vx = cos(angle) * Speed;
        vy = sin(angle) * Speed;
        angeld = true;
    }
}

void cleanUpScene() {
    glDeleteVertexArrays(numVAOs, VAO);
    glDeleteBuffers(numVBOs, VBO);
    glDeleteProgram(renderingProgram);
    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}


int main(void) {
    if (!glfwInit())
        exit(EXIT_FAILURE);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);

    window = glfwCreateWindow(window_width, window_height, window_title, nullptr, nullptr);

    glfwMakeContextCurrent(window);

    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glfwSetKeyCallback(window, keyCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);

    if (glewInit() != GLEW_OK)
        exit(EXIT_FAILURE);

    glfwSwapInterval(1);

    glfwSetWindowSizeLimits(window, 600, 600, 3840, 2160);
    glfwSetWindowAspectRatio(window, 1, 1);

    init(window);

    cout << "Irányítás:" << endl;
    cout << "Fel/Le nyilak\tszakasz mozgatása" << endl;
    cout << "S\t\tKör elindítása 25 fokos szögben" << endl;
    cout << "ESC\t\tKilépés" << endl;

    framebufferSizeCallback(window, window_width, window_height);

    while (!glfwWindowShouldClose(window)) {
        display(window, glfwGetTime());
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    cleanUpScene();
    return EXIT_SUCCESS;
}