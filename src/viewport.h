#ifndef VIEWPORT_H
#define VIEWPORT_H

#include <SDL3/SDL.h>

typedef struct {
    float x;
    float y;
    float w;
    float h;
    float scale;
} Viewport_t;

void viewport_transform(SDL_FRect *rect, Viewport_t viewport);
void viewport_transformf(float *x, float *y, Viewport_t viewport);

#endif//VIEWPORT_H