#include "input.h"

#include "myglheaders.h"

PYGLAPI void input_capture_cursor(GLFWwindow* window, int on){
    if(on)
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    else
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}
static double x0=0.0, x1=0.0, y0=0.0, y1=0.0;
PYGLAPI void input_poll(GLFWwindow* window){
    glfwPollEvents();
    x1 = x0;
    y1 = y0;
    glfwGetCursorPos(window, &x0, &y0);
    x1 = x0 - x1;
    y1 = y0 - y1;
}
PYGLAPI int input_getKey(GLFWwindow* window, int key){
    return glfwGetKey(window, key);
}
PYGLAPI double input_relative_x(GLFWwindow* window){
    return x1;
}
PYGLAPI double input_relative_y(GLFWwindow* window){
    return y1;
}
PYGLAPI double input_x(GLFWwindow* window){
    return x0;
}
PYGLAPI double input_y(GLFWwindow* window){
    return y0;
}
PYGLAPI int input_mouseleft(GLFWwindow* window){
    return glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
}
PYGLAPI int input_mouseright(GLFWwindow* window){
    return glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
}
PYGLAPI double getTime(){
    return glfwGetTime();
}
PYGLAPI void setTime(double t){
    glfwSetTime(t);
}
