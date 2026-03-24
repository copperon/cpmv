#include <stdlib.h>
#include <string.h>
#include <math.h> // for fabsf

#include <SDL3_ttf/SDL_ttf.h>

#include "node.h"

// text object to make
// drawing a lot of text simpler
typedef struct {
    float x; // relative to node x & y
    float y;
    int x_align; // -1 left, 0 centre, 1 right
    int y_align; // 1 top, 0 centre, -1 bottom
    char *text;
    SDL_Color color;
} TextObj_t;

size_t node_get_count_of_file(FILE *fptr) {
    char ch;
    size_t count = 0;
    int comma_count = 0;
    int in_quotes = 0;
    while((ch = fgetc(fptr)) != EOF) {
        switch (ch) {
            case '\n':
                if (comma_count == 4)
                    count++;
                comma_count = 0;
                in_quotes = 0;
                break;
            case ',':
                if (!in_quotes)
                    comma_count++;
                break;
            case '"':
                in_quotes = !in_quotes;
                break;
        }
    }
    if (comma_count == 4)
        count++;
    
    rewind(fptr);
    return count;
}

Node_t *node_create_from_file(FILE *fptr, size_t count) {
    // I'm replacing the given node IDs
    // with the smallest value possible so
    // I can look them up in a basic array.
    enum {
        ID,
        Outline,
        Name,
        Duration,
        Predecessor,
        STATE_COUNT
    };
    
    { // find nodes data section in file
    char nodes_tag[] = "[NODES]";
    int valid_index = 0;
    
    char ch;
    while((ch = fgetc(fptr)) != EOF) {
        if (ch == nodes_tag[valid_index]) {
            valid_index++;
            if (valid_index == (sizeof(nodes_tag) - 1)/sizeof(char)) {
                // string matches, continue to reading
                break;
            }
        } else {
            valid_index = 0;
        }
    }
    // skip next character (should be newline)
    ch = fgetc(fptr);
    }

    IntVector_t id_map = intv_create(count);
    Node_t *nodes = malloc(count * sizeof(Node_t));
    for (int i = 0; i < count; i++) {
        nodes->children = intv_create(4);
        nodes->parents = intv_create(4);
    }

    size_t node = 0;
    char str[256];
    int in_quotes = 0;
    int str_index = 0;
    int state = ID;
    
    char ch;
    while((ch = fgetc(fptr)) != EOF) {
        switch (ch) {
            case '\n':
                // Some lines are empty for formatting
                // reasons. They'll be skipped using this.
                // If the program accidentally started
                // reading from an invalid file, just
                // exit the program cause that's bad.
                if (state != Predecessor) {
                    if (state == ID) {
                        state = ID;
                        in_quotes = 0;
                        str_index = 0;
                        break;
                    } else {
                        printf("Invalid file.\n");
                        exit(1);
                    }
                }

            case ',':
                // Commas are treated as
                // regular characters when
                // encased in quotes.
                if (in_quotes)
                    goto default_char;

                // string null terminator
                str[str_index] = '\0';

                switch (state) {
                    case ID:
                        nodes[node].id = id_map.size;
                        intv_append(&id_map, atoi(str));
                        break;
                    case Outline:
                        nodes[node].outline = malloc((str_index + 1) * sizeof(char));
                        strcpy(nodes[node].outline, str);
                        break;
                    case Name:
                        nodes[node].name = malloc((str_index + 1) * sizeof(char));
                        strcpy(nodes[node].name, str);
                        break;
                    case Duration:
                        nodes[node].duration = atoi(str);
                        break;
                    case Predecessor: {
                        // checks for BEGIN node, meaning
                        // the start of the notetree.
                        if (str[0] == 'B') {
                            break;
                        }

                        char id_str[10];
                        int desti = 0;
                        for (int i = 0; i < str_index; i++) {
                            if (str[i] == ',') {
                                id_str[desti] = '\0';
                                intv_append(&nodes[node].parents, atoi(id_str));
                                desti = 0;
                            } else {
                                id_str[desti] = str[i];
                                desti++;
                            }
                        }
                        if (desti > 0) {
                            id_str[desti] = '\0';
                            intv_append(&nodes[node].parents, atoi(id_str));
                        }
                        break;
                    }
                }
                if (state == Predecessor) {
                    node++;
                    state = ID;
                } else {
                    state++;
                }
                
                // reset values for next list element
                str_index = 0;
                break;
            
            case '"':
                in_quotes = !in_quotes;
                break;
            
            default:
            default_char:
                // copy character to string for later
                str[str_index] = ch;
                str_index++;
        }
    }

    // remap parent nodes to the correct ids
    for (int i = 0; i < count; i++) {
        for (int ii = 0; ii < nodes[i].parents.size; ii++) {
            nodes[i].parents.data[ii] = intv_indexof(id_map, nodes[i].parents.data[ii]);
        }
    }

    // map parents back to children
    for (int i = 0; i < count; i++) {
        for (int ii = 0; ii < nodes[i].parents.size; ii++) {
            intv_append(&nodes[nodes[i].parents.data[ii]].children, i);
            nodes[nodes[i].parents.data[ii]].child_count++;
        }
    }

    intv_destroy(id_map);

    return nodes;
}

void node_destroy_all(Node_t *nodes, size_t count) {
    for (int i = 0; i < count; i++) {
        intv_destroy(nodes[i].parents);
        intv_destroy(nodes[i].children);
        free(nodes[i].name);
        free(nodes[i].outline);
    }
    free(nodes);
}

float node_grid_snap(float pos) {
    return roundf(pos / NODE_GRID_SIZE) * NODE_GRID_SIZE;
}

void node_load_positions_from_file(FILE *fptr, Node_t *nodes, size_t count) {
    for (size_t i = 0; i < count; i++) {
        fread(&nodes[i].x, sizeof(float), 1, fptr);
        fread(&nodes[i].y, sizeof(float), 1, fptr);
    }
}

void node_save_positions_to_file(FILE *fptr, Node_t *nodes, size_t count) {
    for (size_t i = 0; i < count; i++) {
        fwrite(&nodes[i].x, sizeof(float), 1, fptr);
        fwrite(&nodes[i].y, sizeof(float), 1, fptr);
    }
}

size_t node_get_colliding(Node_t *nodes, size_t count, float x, float y) {
    for (size_t i = 0; i < count; i++) {
        if (fabsf(nodes[i].x - x) < NODE_WIDTH / 2
         && fabsf(nodes[i].y - y) < NODE_HEIGHT / 2)
            return i;
    }
    return count;
}

size_t node_get_root(Node_t *nodes, size_t count) {
    // find root node
    for (size_t i = 0; i < count; i++) {
        // root node has no parents
        if (nodes[i].parents.size == 0)
            return i;
    }
    return count;
}

size_t node_get_end(Node_t *nodes, size_t count) {
    for (size_t i = count - 1; i > 0; i--) {
        // root node has no children
        if (nodes[i].children.size == 0)
            return i;
    }
    return count;
}

void node_set_positions(Node_t *nodes, size_t count) {
    // try to load from save file if it exists
    FILE *node_pos = fopen("node_pos.bin", "rb");
    if (node_pos) {
        node_load_positions_from_file(node_pos, nodes, count);
        return;
    }

    size_t root = node_get_root(nodes, count);

    IntVector_t current_nodes = intv_create(32);
    IntVector_t next_nodes = intv_create(32);

    for (size_t ii = 0; ii < nodes[root].children.size; ii++) {
        intv_append(&current_nodes, nodes[root].children.data[ii]);
    }
    intv_print(current_nodes, 1);

    size_t node_x = 1;
    while (current_nodes.size > 0) {
        for (size_t i = 0; i < current_nodes.size; i++) {
            Node_t *node = &nodes[current_nodes.data[i]];
            node->x = node_x * NODE_GRID_SIZE * 2;
            node->y = i * NODE_GRID_SIZE * 2;
            for (size_t ii = 0; ii < node->children.size; ii++) {
                int already_exists = 0;
                for (size_t iii = 0; iii < current_nodes.size; iii++) {
                    if (node->children.data[ii] == current_nodes.data[iii]) {
                        already_exists = 1;
                        break;
                    }
                }
                if (!already_exists) {
                    intv_append(&next_nodes, node->children.data[ii]);
                }
            }
        }
        intv_print(next_nodes, 1);
        int *curr_data = current_nodes.data;
        current_nodes.size = next_nodes.size;
        current_nodes.data = next_nodes.data;
        next_nodes.data = curr_data;
        intv_erase(&next_nodes);

        node_x++;
    }

    intv_destroy(current_nodes);
    intv_destroy(next_nodes);
}

void node_set_durations(Node_t *nodes, size_t count) {
    size_t root = node_get_root(nodes, count);
    
    IntVector_t current_duration = intv_create(32);
    IntVector_t current = intv_create(32);
    for (size_t i = 0; i < nodes[root].child_count; i++) {
        intv_append(&current, nodes[root].children.data[i]);
        intv_append(&current_duration, nodes[root].duration);
    }

    IntVector_t next_duration = intv_create(32);
    IntVector_t next = intv_create(32);

    while (current.size != 0) {
        for (size_t i = 0; i < current.size; i++) {
            Node_t *node = &nodes[current.data[i]];

            if (current_duration.data[i] > node->total_duration)
                node->total_duration = current_duration.data[i];
            
            for (size_t ii = 0; ii < node->child_count; ii++) {
                intv_append(&next, node->children.data[ii]);
                intv_append(&next_duration, node->total_duration + node->duration);
            }
        }
        // swap current and next vectors
        IntVector_t cur = current;
        IntVector_t cur_d = current_duration;
        current = next;
        current_duration = next_duration;
        next = cur;
        next_duration = cur_d;
        next.size = 0;
        next_duration.size = 0;
    }
    intv_destroy(current);
    intv_destroy(current_duration);
    intv_destroy(next);
    intv_destroy(next_duration);
}

void node_set_critical_path(Node_t *nodes, size_t count) {
    size_t node = node_get_end(nodes, count);

    while (nodes[node].parents.size != 0) {
        size_t largest_node = nodes[node].parents.data[0];
        for (size_t i = 1; i < nodes[node].parents.size; i++) {
            if (nodes[nodes[node].parents.data[i]].total_duration > nodes[largest_node].total_duration) {
                largest_node = nodes[node].parents.data[i];
            }
        }
        nodes[largest_node].on_critical_path = true;
        node = largest_node;
    }
}

void node_render_critical_path(Node_t *nodes, size_t count, Viewport_t viewport, SDL_Renderer *renderer) {
    size_t node = node_get_end(nodes, count);

    while (nodes[node].parents.size != 0) {
        int has_parent = 0;
        for (size_t i = 0; i < nodes[node].parents.size; i++) {
            if (nodes[nodes[node].parents.data[i]].on_critical_path) {
                size_t next = nodes[node].parents.data[i];
                // draw path
                float x = nodes[node].x;
                float y = nodes[node].y;
                float next_x = nodes[next].x;
                float next_y = nodes[next].y;
                viewport_transformf(&x, &y, viewport);
                viewport_transformf(&next_x, &next_y, viewport);
                SDL_RenderLine(renderer, x, y-1, next_x, next_y-1);
                SDL_RenderLine(renderer, x, y+1, next_x, next_y+1);
                SDL_RenderLine(renderer, x-1, y, next_x-1, next_y);
                SDL_RenderLine(renderer, x+1, y, next_x+1, next_y);
                has_parent = 1;

                node = next;
            }
        }
        if (!has_parent) {
            printf("Failed to render critical path at node %zu\n", node);
            break;
        }
    }
}

void node_render(Node_t node, Viewport_t viewport, TTF_Font *font, SDL_Renderer *renderer) {
    SDL_FRect dest;
    dest.x = node.x - NODE_WIDTH/2;
    dest.y = node.y - NODE_HEIGHT/2;
    dest.w = NODE_WIDTH;
    dest.h = NODE_HEIGHT;
    viewport_transform(&dest, viewport);

    SDL_SetRenderDrawColor(renderer, NODE_FILL_COLOR);
    SDL_RenderFillRect(renderer, &dest);

    // draw node border
    SDL_SetRenderDrawColor(renderer, NODE_OUTLINE_COLOR);
    SDL_RenderRect(renderer, &dest);

    SDL_Color black = {0, 0, 0};
    SDL_Color green = {40, 180, 80};
    SDL_Color pink = {180, 50, 100};

    TextObj_t text[6] = {
        {50, -35, -1, 1, "", black}, // id
        {44, 10, 1, 1, node.outline, black},
        {0, -10, 0, 0, node.name, black},
        {-44, 10, -1, 1, "", black}, // duration
        {-44, -55, -1, 1, "", green}, // pre duration
        {45, -55, 1, 1, "", pink} // total duration
    };
    char id_buffer[10];
    sprintf(id_buffer, "%d", node.id);
    text[0].text = id_buffer;

    char duration_buffer[10];
    sprintf(duration_buffer, "%d", node.duration);
    text[3].text = duration_buffer;

    char pre_duration_buffer[10];
    sprintf(pre_duration_buffer, "%d", node.total_duration);
    text[4].text = pre_duration_buffer;

    char total_duration_buffer[10];
    sprintf(total_duration_buffer, "%d", node.total_duration + node.duration);
    text[5].text = total_duration_buffer;

    for (int i = 0; i < sizeof(text)/sizeof(TextObj_t); i++) {
        SDL_Surface *text_surface = TTF_RenderText_Blended_Wrapped(font, text[i].text, 0, text[i].color, NODE_TEXT_WIDTH);
        SDL_Texture *text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);
        SDL_DestroySurface(text_surface);

        SDL_FRect text_rect;
        text_rect.x = node.x + text[i].x;
        text_rect.y = node.y + text[i].y;
        SDL_GetTextureSize(text_texture, &text_rect.w, &text_rect.h);
        if (text_rect.h > NODE_MAX_TEXT_HEIGHT) {
            text_rect.w *= NODE_MAX_TEXT_HEIGHT / text_rect.h;
            text_rect.h *= NODE_MAX_TEXT_HEIGHT / text_rect.h;
        }
        
        if (text[i].x_align == 0) {
            text_rect.x -= text_rect.w / 2;
        } else if (text[i].x_align == 1) {
            text_rect.x -= text_rect.w;
        }
        if (text[i].y_align == 0) {
            text_rect.y -= text_rect.h / 2;
        } else if (text[i].y_align == -1) {
            text_rect.y -= text_rect.h;
        }

        viewport_transform(&text_rect, viewport);

        SDL_RenderTexture(renderer, text_texture, NULL, &text_rect);
        SDL_DestroyTexture(text_texture);
    }

    
}

void node_render_all(Node_t *nodes, size_t count, Viewport_t viewport, TTF_Font *font, SDL_Renderer *renderer) {
    SDL_SetRenderDrawColor(renderer, NODE_OUTLINE_COLOR);
    // draw lines
    for (size_t i = 0; i < count; i++) {
        for (int ii = 0; ii < nodes[i].children.size; ii++) {
            float x = nodes[i].x;
            float y = nodes[i].y;
            Node_t child = nodes[nodes[i].children.data[ii]];
            viewport_transformf(&x, &y, viewport);
            viewport_transformf(&child.x, &child.y, viewport);
            SDL_RenderLine(renderer, x, y, child.x, child.y);
        }
    }

    node_render_critical_path(nodes, count, viewport, renderer);

    for (size_t i = 0; i < count; i++) {
        node_render(nodes[i], viewport, font, renderer);
    }
}