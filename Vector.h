#ifndef VECTOR_H
#define VECTOR_H

#include "Core.h"

class Vector {
public:
    Vector()
    {
        steps = 0;
        skip = 0;
    }

    Vector(s32 steps, us32 skip)
    {
        Vector::steps = steps;
        Vector::skip = skip;
    }

    s32 steps;
    us32 skip;
    us32 currentSkip;
};

#endif
