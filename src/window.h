#pragma once

typedef struct GLFWwindow GLFWwindow;

namespace window{

GLFWwindow* init(int width, int height, int major, int minor, const char* title);
void destroy(GLFWwindow* w);
int is_open(GLFWwindow* w);
void swap(GLFWwindow* w);
void close(GLFWwindow* w);
void depth_test(int on);
void cull_face(int on);
float get_width(GLFWwindow* w);
float get_height(GLFWwindow* w);

};

