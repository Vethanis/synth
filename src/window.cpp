#include "window.h"

#include "myglheaders.h"
#include "stdio.h"

namespace window{

static void error_callback(int error, const char* description){
    puts(description);
}

static void onWindowResize(GLFWwindow* window, int x, int y){
    glViewport(0, 0, x, y);
}

GLFWwindow* init(int width, int height, int major_ver, int minor_ver, const char* title){
    glfwSetErrorCallback(error_callback);
    if(!glfwInit()){
        puts("Failed to init glfw");
        return NULL;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, major_ver);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, minor_ver);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(width, height, title, NULL, NULL);
    if (!window){
        puts("Failed to get window from glfw");
        glfwTerminate();
        return NULL;
    }
    glfwMakeContextCurrent(window);
    glewExperimental=1;
    glViewport(0, 0, width, height);
    if(glewInit() != GLEW_OK){
        puts("Failed to init glew");
        glfwTerminate();
        return NULL;
    }
    glGetError();    // invalid enumerant shows up here, just part of glew being itself.
    glfwSwapInterval(1);
    glfwSetWindowSizeCallback(window, onWindowResize);
    return window;
}
void destroy(GLFWwindow* w){
    glfwDestroyWindow(w);
    glfwTerminate();
}
int is_open(GLFWwindow* w){
    return !glfwWindowShouldClose(w);
}
void close(GLFWwindow* w){
    glfwSetWindowShouldClose(w, GL_TRUE);
}
void swap(GLFWwindow* w){
    glfwSwapBuffers(w);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
void depth_test(int on){
    if(on)
        glEnable(GL_DEPTH_TEST);
    else
        glDisable(GL_DEPTH_TEST);
}
void cull_face(int on){
    if(on)
        glEnable(GL_CULL_FACE);
    else
        glDisable(GL_CULL_FACE);
}

float get_width(GLFWwindow* w){
    int x, y;
    glfwGetWindowSize(w, &x, &y);
    return float(x);
}
float get_height(GLFWwindow* w){
    int x, y;
    glfwGetWindowSize(w, &x, &y);
    return float(y);
}

};
