#ifndef VECTOR_H
#define VECTOR_H

class Vector {
public:
    Vector()
    {
        steps = 0;
        skip = 0;
    }

    Vector(int steps, int skip)
    {
        Vector::steps = steps;
        Vector::skip = skip;
    }

    bool direction;
    int steps;
    int skip;
    int currentSkip;
};

#endif
