#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "parser.h"
#include "tarea3.h"

// Separador CSV básico
#define DELIM ","

// Función que parsea la línea CSV y extrae tokens (simplificado)
static char **split_line(char *line, int *count) {
    int capacity = 16;
    char **tokens = malloc(capacity * sizeof(char*));
    *count = 0;

    char *token = strtok(line, DELIM);
    while(token) {
        if (*count >= capacity) {
            capacity *= 2;
            tokens = realloc(tokens, capacity * sizeof(char*));
        }
        tokens[*count] = strdup(token);
        (*count)++;
        token = strtok(NULL, DELIM);
    }
    return tokens;
}

// Limpia memoria tokens
static void free_tokens(char **tokens, int count) {
    for (int i = 0; i < count; i++) {
        free(tokens[i]);
    }
    free(tokens);
}

// Crea un escenario a partir de tokens del CSV (asume orden fijo)
static scenario_t *create_scenario_from_tokens(char **tokens, int count) {
    if (count < 10) return NULL; // mínimo columnas esperadas

    scenario_t *scen = malloc(sizeof(scenario_t));
    if (!scen) return NULL;

    scen->id = atoi(tokens[0]);
    scen->name = strdup(tokens[1]);
    scen->description = strdup(tokens[2]);

    // Parsear items separados por ';'
    scen->items = list_create();

    if (tokens[3][0] != '\0') {
        char *item_str = strdup(tokens[3]);
        char *item_token = strtok(item_str, ";");
        while (item_token) {
            // Solo guardamos el string, en el futuro podrías separar nombre, peso, valor
            list_pushBack(scen->items, strdup(item_token));
            item_token = strtok(NULL, ";");
        }
        free(item_str);
    }

    // Conexiones [up, down, left, right]
    scen->connections[0] = atoi(tokens[4]);
    scen->connections[1] = atoi(tokens[5]);
    scen->connections[2] = atoi(tokens[6]);
    scen->connections[3] = atoi(tokens[7]);

    scen->is_final = (strcmp(tokens[8], "Si") == 0 || strcmp(tokens[8], "si") == 0);

    return scen;
}

// Libera un escenario
void destroy_scenario(scenario_t *scen) {
    if (!scen) return;
    free(scen->name);
    free(scen->description);
    for (int i = 0; i < list_size(scen->items); i++) {
        free(list_get(scen->items, i));
    }
    list_destroy(scen->items);
    free(scen);
}

// Carga el archivo CSV y devuelve un grafo con todos los escenarios
world_graph_t *parse_game_data(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error al abrir archivo CSV");
        return NULL;
    }

    char buffer[4096];
    int line_num = 0;

    world_graph_t *graph = malloc(sizeof(world_graph_t));
    graph->capacity = 16;
    graph->count = 0;
    graph->scenarios = malloc(graph->capacity * sizeof(scenario_t*));

    // Lee la primera línea (header) y la ignora
    if (!fgets(buffer, sizeof(buffer), file)) {
        fclose(file);
        free(graph->scenarios);
        free(graph);
        return NULL;
    }

    while (fgets(buffer, sizeof(buffer), file)) {
        line_num++;

        int token_count = 0;
        char *line = strdup(buffer);
        char **tokens = split_line(line, &token_count);

        scenario_t *scen = create_scenario_from_tokens(tokens, token_count);

        if (scen) {
            if (graph->count == graph->capacity) {
                graph->capacity *= 2;
                graph->scenarios = realloc(graph->scenarios, graph->capacity * sizeof(scenario_t*));
            }
            graph->scenarios[graph->count++] = scen;
        }

        free_tokens(tokens, token_count);
        free(line);
    }

    fclose(file);
    return graph;
}
