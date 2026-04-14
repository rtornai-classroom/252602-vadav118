#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

enum eVertexArrayObject { VAO_Bezier, VAOCount };
enum eBufferObject { VBO_Bezier, BOCount };
enum eProgram { Program_Bezier, ProgramCount };

GLuint vaoAr[VAOCount];
GLuint vboAr[BOCount];
GLuint progAr[ProgramCount];

int windowWidth = 800;
int windowHeight = 600;

glm::vec2 controlPoints[100];
int pointCount = 0;
int draggedPointIdx = -1;

GLuint LoadShaders(const char* vsPath, const char* fsPath) {
    auto readCode = [](const char* path) {
        std::ifstream file(path);
        std::stringstream buffer;
        if (file.is_open()) buffer << file.rdbuf();
        return buffer.str();
        };
    std::string vsCode = readCode(vsPath);
    std::string fsCode = readCode(fsPath);
    const char* vsSrc = vsCode.c_str();
    const char* fsSrc = fsCode.c_str();

    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vsSrc, NULL);
    glCompileShader(vs);

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fsSrc, NULL);
    glCompileShader(fs);

    GLuint prog = glCreateProgram();
    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    glLinkProgram(prog);

    glDeleteShader(vs);
    glDeleteShader(fs);
    return prog;
}

glm::vec2 getBezierPoint(float t, glm::vec2* pts, int n) {
    if (n == 1) return pts[0];
    std::vector<glm::vec2> nextPts;
    for (int i = 0; i < n - 1; i++) {
        nextPts.push_back(glm::mix(pts[i], pts[i + 1], t));
    }
    return getBezierPoint(t, nextPts.data(), n - 1);
}

glm::vec2 getMousePos(GLFWwindow* w) {
    double x, y;
    glfwGetCursorPos(w, &x, &y);
    float aspect = (float)windowWidth / (float)windowHeight;
    return glm::vec2((x / windowWidth * 2.0f - 1.0f) * aspect, 1.0f - y / windowHeight * 2.0f);
}

void display() {
    glClearColor(1.0f, 1.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(progAr[Program_Bezier]);

    float aspect = (float)windowWidth / (float)windowHeight;
    glm::mat4 proj = glm::ortho(-aspect, aspect, -1.0f, 1.0f);
    glm::mat4 mv = glm::mat4(1.0f);

    glUniformMatrix4fv(glGetUniformLocation(progAr[Program_Bezier], "matProjection"), 1, GL_FALSE, glm::value_ptr(proj));
    glUniformMatrix4fv(glGetUniformLocation(progAr[Program_Bezier], "matModelView"), 1, GL_FALSE, glm::value_ptr(mv));
    GLint colorLoc = glGetUniformLocation(progAr[Program_Bezier], "uColor");

    if (pointCount == 0) return;

    glBindVertexArray(vaoAr[VAO_Bezier]);
    glBindBuffer(GL_ARRAY_BUFFER, vboAr[VBO_Bezier]);

    glLineWidth(3.0f);         
    glEnable(GL_LINE_SMOOTH);   
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

    glUniform3f(colorLoc, 0.0f, 0.8f, 0.0f);
    glBufferData(GL_ARRAY_BUFFER, pointCount * sizeof(glm::vec2), controlPoints, GL_DYNAMIC_DRAW);
    glDrawArrays(GL_LINE_STRIP, 0, pointCount);

    if (pointCount >= 2) {
        std::vector<glm::vec2> curve;
        for (int i = 0; i <= 200; i++) curve.push_back(getBezierPoint(i / 200.0f, controlPoints, pointCount));
        glUniform3f(colorLoc, 1.0f, 0.0f, 0.0f);
        glBufferData(GL_ARRAY_BUFFER, curve.size() * sizeof(glm::vec2), curve.data(), GL_DYNAMIC_DRAW);
        glDrawArrays(GL_LINE_STRIP, 0, (GLsizei)curve.size());
    }

    glEnable(GL_POINT_SMOOTH);
    glPointSize(9.0f); 
    glUniform3f(colorLoc, 0.0f, 0.0f, 1.0f);
    glBufferData(GL_ARRAY_BUFFER, pointCount * sizeof(glm::vec2), controlPoints, GL_DYNAMIC_DRAW);
    glDrawArrays(GL_POINTS, 0, pointCount);

    glDisable(GL_POINT_SMOOTH);
    glDisable(GL_LINE_SMOOTH);
}

void mouseButtonCallback(GLFWwindow* w, int button, int action, int mods) {
    glm::vec2 m = getMousePos(w);
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            for (int i = 0; i < pointCount; i++) {
                if (glm::distance(m, controlPoints[i]) < 0.06f) {
                    draggedPointIdx = i; return;
                }
            }
            if (pointCount < 100) controlPoints[pointCount++] = m;
        }
        else if (action == GLFW_RELEASE) {
            draggedPointIdx = -1;
        }
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
        for (int i = 0; i < pointCount; i++) {
            if (glm::distance(m, controlPoints[i]) < 0.06f) {
                for (int j = i; j < pointCount - 1; j++) controlPoints[j] = controlPoints[j + 1];
                pointCount--; break;
            }
        }
    }
}

void cursorPosCallback(GLFWwindow* w, double x, double y) {
    if (draggedPointIdx != -1) controlPoints[draggedPointIdx] = getMousePos(w);
}

int main() {
    if (!glfwInit()) return -1;
    GLFWwindow* window = glfwCreateWindow(800, 600, "Bead 2", NULL, NULL);
    glfwMakeContextCurrent(window);
    glewInit();

    progAr[Program_Bezier] = LoadShaders("vertexShader.glsl", "fragmentShader.glsl");
    glGenVertexArrays(VAOCount, vaoAr);
    glGenBuffers(BOCount, vboAr);

    glBindVertexArray(vaoAr[VAO_Bezier]);
    glBindBuffer(GL_ARRAY_BUFFER, vboAr[VBO_Bezier]);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    controlPoints[0] = glm::vec2(-0.6f, -0.4f);
    controlPoints[1] = glm::vec2(-0.2f, 0.5f);
    controlPoints[2] = glm::vec2(0.2f, 0.5f);
    controlPoints[3] = glm::vec2(0.6f, -0.4f);
    pointCount = 4;

    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);

    while (!glfwWindowShouldClose(window)) {
        glfwGetFramebufferSize(window, &windowWidth, &windowHeight);
        glViewport(0, 0, windowWidth, windowHeight);
        display();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}