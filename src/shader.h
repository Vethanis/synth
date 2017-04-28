#ifndef SHADER_H
#define SHADER_H

#include "myglheaders.h"
#include "pyglapi.h"

PYGLAPI unsigned shader_cons(const char* src, int fragment);
PYGLAPI void shader_dest(unsigned id);

#endif
