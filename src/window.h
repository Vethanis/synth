#ifndef WINDOW_H
#define WINDOW_H

#include "pyglapi.h"

typedef struct GLFWwindow GLFWwindow;

PYGLAPI GLFWwindow* window_cons(int width, int height, int major, int minor, const char* title);
PYGLAPI void window_dest(GLFWwindow* w);
PYGLAPI int window_is_open(GLFWwindow* w);
PYGLAPI void window_swap(GLFWwindow* w);
PYGLAPI void window_close(GLFWwindow* w);
PYGLAPI void window_depth_test(int on);
PYGLAPI void window_cullface(int on);

#endif
