#include "viewport.h"

void viewport_transform(SDL_FRect *rect, Viewport_t viewport) {
    rect->x = (rect->x - viewport.x) * viewport.scale + viewport.w/2;
    rect->y = (rect->y - viewport.y) * viewport.scale + viewport.h/2;
    rect->w *= viewport.scale;
    rect->h *= viewport.scale;
}

void viewport_transformf(float *x, float *y, Viewport_t viewport) {
    *x = (*x - viewport.x) * viewport.scale + viewport.w/2;
    *y = (*y - viewport.y) * viewport.scale + viewport.h/2;
}