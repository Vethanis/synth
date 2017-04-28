#ifndef GLPROGRAM_H
#define GLPROGRAM_H

#include "pyglapi.h"

PYGLAPI unsigned glprogram_cons(const char* vertsrc, const char* fragsrc);
PYGLAPI void glprogram_dest(unsigned id);

PYGLAPI void glprogram_bind(unsigned id);

PYGLAPI void setUniformInt(unsigned id, const char* name, int value);
PYGLAPI void setUniformFloat(unsigned id, const char* name, float value);
PYGLAPI void setUniformVec2(unsigned id, const char* name, float* value);
PYGLAPI void setUniformVec3(unsigned id, const char* name, float* value);
PYGLAPI void setUniformVec4(unsigned id, const char* name, float* value);
PYGLAPI void setUniformMat3(unsigned id, const char* name, float* value);
PYGLAPI void setUniformMat4(unsigned id, const char* name, float* value);

#endif
