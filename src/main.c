#include <stdio.h>
#include <math.h> // roundf

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include "node.h"
#include "viewport.h"

#define VIEWPORT_SCALE_MAX 3
#define VIEWPORT_SCALE_MIN 0.25

static const int base_win_width = 1280;
static const int base_win_height = 720;
static float win_scale = 1.0;

static SDL_Renderer *renderer;
static SDL_Window *window;
static Viewport_t viewport;

static size_t node_count;
static Node_t *nodes;

static int init_graphics() {
    if(!SDL_Init(SDL_INIT_EVENTS)) {
        printf("SDL_Init failed: %s\n", SDL_GetError());
        return 0;
    }

    if (!TTF_Init()) {
        printf("TTF_Init failed: %s\n", SDL_GetError());
        return 0;
    }

    window = SDL_CreateWindow("Critical Path Method Visualiser",
                base_win_width, base_win_height, SDL_WINDOW_RESIZABLE);
    if (!window) {
        printf("SDL_CreateWindow failed: %s\n", SDL_GetError());
    }

    renderer = SDL_CreateRenderer(window, NULL);
    if (!renderer) {
        printf("SDL_CreateRenderer failed: %s\n", SDL_GetError());
    }
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    if (!SDL_SetRenderVSync(renderer, 1)) {
        printf("SDL_SetRenderVSync() failed: %s\n", SDL_GetError());
    }

    viewport.x = 0.0;
    viewport.y = 0.0;
    viewport.w = base_win_width;
    viewport.h = base_win_height;
    viewport.scale = 1.0;

    return 1;
}

static int init_nodes() {
    // load nodes from file
    FILE *fptr = fopen("raw_data.csv", "r");

    node_count = node_get_count_of_file(fptr);
    if (!node_count) {
        printf("Failed to get node count.\n");
        return 0;
    }

    #ifdef DEBUG
        printf("Node count: %zu\n", node_count);
    #endif
    
    nodes = node_create_from_file(fptr, node_count);
    if (!nodes) {
        printf("Failed to create nodes.\n");
        return 0;
    }
    #ifdef DEBUG
        for (int i = 0; i < node_count; i++) {
            printf("Node {\n  ID: %d\n  Outline: \"%s\"\n  Name: \"%s\"\n  Duration: %d\n  Parents: ", nodes[i].id, nodes[i].outline, nodes[i].name, nodes[i].duration);
            intv_print(nodes[i].parents, 0);
            printf("  Children: ");
            intv_print(nodes[i].children, 0);
            printf("}\n");
        }
    #endif

    return 1;
}

int main(void) {
    if (!init_nodes()) {
        return 1;
    }

    if (!init_graphics()) {
        return 1;
    }

    TTF_Font *font = TTF_OpenFont("gg_sans.ttf", 16);
    TTF_SetFontLineSkip(font, 16);
    if (!font) {
        printf("TTF_Font() failed: %s\n", SDL_GetError());
        return 1;
    }

    node_set_positions(nodes, node_count);
    node_set_durations(nodes, node_count);
    node_set_critical_path(nodes, node_count);

    float mouse_last_x, mouse_last_y;
    SDL_GetMouseState(&mouse_last_x, &mouse_last_y);

    // used for grabbing nodes
    int shift_pressed = 0;
    size_t selected_node = node_count;
    float mouse_node_offset_x, mouse_node_offset_y;

    int running = 1;
    while (running) {
        // get mouse state
        float mouse_x, mouse_y;
        SDL_MouseButtonFlags mouse_buttons = SDL_GetMouseState(&mouse_x, &mouse_y);

        float mouse_world_x = (mouse_x - viewport.w / 2) / viewport.scale + viewport.x;
        float mouse_world_y = (mouse_y - viewport.h / 2) / viewport.scale + viewport.y;
        
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            switch (e.type) {
                case SDL_EVENT_QUIT:
                    running = 0;
                    break;
                case SDL_EVENT_MOUSE_WHEEL: {
                    float last_viewport_x = viewport.x;
                    float last_viewport_y = viewport.y;

                    viewport.x += mouse_world_x;
                    viewport.y += mouse_world_y;
                    viewport.scale += (e.wheel.y * viewport.scale) * 0.1;
                    if (viewport.scale > VIEWPORT_SCALE_MAX * win_scale) {
                        viewport.scale = VIEWPORT_SCALE_MAX * win_scale;
                    }
                    if (viewport.scale < VIEWPORT_SCALE_MIN * win_scale) {
                        viewport.scale = VIEWPORT_SCALE_MIN * win_scale;
                    }
                    mouse_world_x = (mouse_x - viewport.w / 2) / viewport.scale + last_viewport_x;
                    mouse_world_y = (mouse_y - viewport.h / 2) / viewport.scale + last_viewport_y;
                    viewport.x -= mouse_world_x;
                    viewport.y -= mouse_world_y;
                    break;
                }
                case SDL_EVENT_WINDOW_RESIZED: {
                    int new_x, new_y;
                    SDL_GetWindowSize(window, &new_x, &new_y);
                    
                    win_scale = (float)new_x / base_win_width;
                    if (viewport.scale > VIEWPORT_SCALE_MAX * win_scale) {
                        viewport.scale = VIEWPORT_SCALE_MAX * win_scale;
                    }
                    if (viewport.scale < VIEWPORT_SCALE_MIN * win_scale) {
                        viewport.scale = VIEWPORT_SCALE_MIN * win_scale;
                    }
                    viewport.w = new_x;
                    viewport.h = new_y;
                    printf("+AHHHH %d, %f\n", new_x, win_scale);
                    break;
                }
                case SDL_EVENT_KEY_DOWN:
                    if (e.key.scancode == SDL_SCANCODE_LSHIFT)
                        shift_pressed = 1;
                    break;
                case SDL_EVENT_KEY_UP:
                    if (e.key.scancode == SDL_SCANCODE_LSHIFT)
                        shift_pressed = 0;
                    break;
            }
        }

        // grab node at mouse pos
        if (mouse_buttons & SDL_BUTTON_LMASK) {
            if (shift_pressed && selected_node == node_count) {
                selected_node = node_get_colliding(nodes, node_count,
                                mouse_world_x, mouse_world_y);
                mouse_node_offset_x = nodes[selected_node].x - mouse_world_x;
                mouse_node_offset_y = nodes[selected_node].y - mouse_world_y;
            }
        } else {
            selected_node = node_count;
        }

        if (selected_node != node_count) {
            // drag node
            nodes[selected_node].x = node_grid_snap(mouse_world_x + mouse_node_offset_x);
            nodes[selected_node].y = node_grid_snap(mouse_world_y + mouse_node_offset_y);
        } else {
            // pan camera
            if (!shift_pressed && mouse_buttons & SDL_BUTTON_LMASK) {
                viewport.x -= (mouse_x - mouse_last_x) / viewport.scale;
                viewport.y -= (mouse_y - mouse_last_y) / viewport.scale;
            }
        }

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);

        node_render_all(nodes, node_count, viewport, font, renderer);

        SDL_RenderPresent(renderer);

        // set last mouse pos for next frame
        mouse_last_x = mouse_x;
        mouse_last_y = mouse_y;
    }

    FILE *node_pos = fopen("node_pos.bin", "wb");
    if (node_pos) {
        node_save_positions_to_file(node_pos, nodes, node_count);
    }
    
    SDL_Quit();
    node_destroy_all(nodes, node_count);
    return 0;
}