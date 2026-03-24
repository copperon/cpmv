#ifndef NODE_H
#define NODE_H

#include <stdio.h>
#include <stdbool.h>
#include <SDL3/SDL.h>

#include "vector.h"
#include "viewport.h"

#define NODE_WIDTH 96
#define NODE_HEIGHT 64
#define NODE_FILL_COLOR 255, 255, 255, 255
#define NODE_OUTLINE_COLOR 0, 0, 0, 255

#define NODE_GRID_SIZE 70

#define NODE_TEXT_WIDTH 80
#define NODE_MAX_TEXT_HEIGHT 40

typedef struct {
    float x;
    float y;

    char *name;
    char *outline;
    int duration;
    int total_duration;
    bool on_critical_path;
    int id;
    IntVector_t parents;
    IntVector_t children;
    int child_count;
} Node_t;

size_t node_get_count_of_file(FILE *fptr);
Node_t *node_create_from_file(FILE *fptr, size_t count);
void node_destroy_all(Node_t *nodes, size_t count);

float node_grid_snap(float pos);

void node_load_positions_from_file(FILE *fptr, Node_t *nodes, size_t count);
void node_save_positions_to_file(FILE *fptr, Node_t *nodes, size_t count);
void node_set_positions(Node_t *nodes, size_t count);
void node_set_durations(Node_t *nodes, size_t count);
void node_set_critical_path(Node_t *nodes, size_t count);
size_t node_get_root(Node_t *nodes, size_t count);
size_t node_get_end(Node_t *nodes, size_t count);
size_t node_get_colliding(Node_t *nodes, size_t count, float x, float y);

void node_render_critical_path(Node_t *nodes, size_t count, Viewport_t viewport, SDL_Renderer *renderer);
void node_render(Node_t node, Viewport_t viewport, TTF_Font *font, SDL_Renderer *renderer);
void node_render_all(Node_t *nodes, size_t count, Viewport_t viewport, TTF_Font *font, SDL_Renderer *renderer);

#endif//NODE_H