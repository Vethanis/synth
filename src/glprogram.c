#include "glprogram.h"
#include "shader.h"
#include "myglheaders.h"
#include "stdlib.h"
#include "stdio.h"
#include "assert.h"

PYGLAPI unsigned glprogram_cons(const char* vertsrc, const char* fragsrc){
    unsigned vert = shader_cons(vertsrc, 0);
    unsigned frag = shader_cons(fragsrc, 1);

    unsigned handle = glCreateProgram();
    glAttachShader(handle, vert);
    glAttachShader(handle, frag);
    glLinkProgram(handle);

    int status = 0;
    glGetProgramiv(handle, GL_LINK_STATUS, &status);

    if(!status){
        int loglen = 1;
        glGetProgramiv(handle, GL_INFO_LOG_LENGTH, &loglen);
        char* log = malloc(loglen + 1);
        assert(log);
        glGetProgramInfoLog(handle, loglen, NULL, log);
        log[loglen] = 0;
        puts(log);
        free(log);
        handle = 0;
    }

    shader_dest(frag);
    shader_dest(vert);

    return handle;
}

PYGLAPI void glprogram_dest(unsigned id){
    glDeleteProgram(id);
}

PYGLAPI void glprogram_bind(unsigned id){
    glUseProgram(id);
}

PYGLAPI void setUniformInt(unsigned id, const char* name, int value){
    glUniform1i(glGetUniformLocation(id, name), value);
}
PYGLAPI void setUniformFloat(unsigned id, const char* name, float value){
    glUniform1f(glGetUniformLocation(id, name), value);
}
PYGLAPI void setUniformVec2(unsigned id, const char* name, float* value){
    glUniform2fv(glGetUniformLocation(id, name), 1, value);
}
PYGLAPI void setUniformVec3(unsigned id, const char* name, float* value){
    glUniform3fv(glGetUniformLocation(id, name), 1, value);
}
PYGLAPI void setUniformVec4(unsigned id, const char* name, float* value){
    glUniform4fv(glGetUniformLocation(id, name), 1, value);
}
PYGLAPI void setUniformMat3(unsigned id, const char* name, float* value){
    glUniformMatrix3fv(glGetUniformLocation(id, name), 1, 0, value);
}
PYGLAPI void setUniformMat4(unsigned id, const char* name, float* value){
    glUniformMatrix4fv(glGetUniformLocation(id, name), 1, 0, value);
}
