#include "shader.h"

#include "myglheaders.h"

#include "stdlib.h"
#include "assert.h"
#include "stdio.h"

PYGLAPI unsigned shader_cons(const char* src, int fragment){
    unsigned handle = glCreateShader(fragment ? GL_FRAGMENT_SHADER : GL_VERTEX_SHADER);
    glShaderSource(handle, 1, &src, NULL);
    glCompileShader(handle);
    int status = 0;
    glGetShaderiv(handle, GL_COMPILE_STATUS, &status);
    if(!status){
        int loglen = 1;
        glGetShaderiv(handle, GL_INFO_LOG_LENGTH, &loglen);
        char* log = malloc(loglen + 1);
        assert(log);
        glGetShaderInfoLog(handle, loglen, NULL, log);
        log[loglen] = 0;
        puts(log);
        free(log);
        glDeleteShader(handle);
        handle = 0;
    }
    return handle;
}
PYGLAPI void shader_dest(unsigned id){
    glDeleteShader(id);
}
