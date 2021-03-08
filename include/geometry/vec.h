#ifndef VEC_H
#define VEC_H

// vga.cpp
class vec2f
{
public:

    vec2f();
    vec2f(float x, float y);
    vec2f(vec2f* pCpy);

    float crossproduct(vec2f V); 

public:
    
    float x, y;
};

// vec3f..

#endif