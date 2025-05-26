#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <windows.h>

#include "tdas/list.h" 
#include "tdas/extra.h"

#define DELIM ","

// Structs del juego
typedef struct item {
    char *name;
    int weight;
    int value;
} item_t;

typedef struct scenario {
    int id;
    char *name;
    char *description;
    List *items;         // Lista de items*
    int connections[4];  // [up, down, left, right]
    bool is_final;
} scenario_t;

typedef struct world_graph {
    scenario_t **scenarios;
    int count;
    int capacity;
} world_graph_t;

// Variables globales
int current_scenario_id = 0;
int time_left = 30;
List *inventory = NULL;

 char **split_line(char *line, int *count) {
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

void free_tokens(char **tokens, int count) {
    for (int i = 0; i < count; i++) {
        free(tokens[i]);
    }
    free(tokens);
}

scenario_t *create_scenario_from_tokens(char **tokens, int count) {
    if (count < 10) return NULL;

    scenario_t *scen = malloc(sizeof(scenario_t));
    if (!scen) return NULL;

    scen->id = atoi(tokens[0]);
    scen->name = strdup(tokens[1]);
    scen->description = strdup(tokens[2]);

    scen->items = list_create();
    if (tokens[3][0] != '\0') {
        char *item_str = strdup(tokens[3]);
        char *item_token = strtok(item_str, ";");
        while (item_token) {
            list_pushBack(scen->items, strdup(item_token));
            item_token = strtok(NULL, ";");
        }
        free(item_str);
    }

    scen->connections[0] = atoi(tokens[4]);
    scen->connections[1] = atoi(tokens[5]);
    scen->connections[2] = atoi(tokens[6]);
    scen->connections[3] = atoi(tokens[7]);

    scen->is_final = (strcmp(tokens[8], "Si") == 0 || strcmp(tokens[8], "si") == 0);

    return scen;
}

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

// Función auxiliar para buscar escenario
scenario_t *get_scenario_by_id(world_graph_t *world, int id) {
    for (int i = 0; i < world->count; i++) {
        if (world->scenarios[i]->id == id) return world->scenarios[i];
    }
    return NULL;
}

// Mostrar info del escenario actual
void print_current_scenario(world_graph_t *world) {
    scenario_t *sc = get_scenario_by_id(world, current_scenario_id);
    if (!sc) {
        printf("Error: escenario no encontrado\n");
        return;
    }

    printf("\nEstas en: %s\n", sc->name);
    printf("%s\n", sc->description);
    printf("Items disponibles:\n");
    for (int i = 0; i < list_size(sc->items); i++) {
        item_t *it = list_get(sc->items, i);
        printf(" - %s (peso: %d, valor: %d)\n", it->name, it->weight, it->value);
    }

    printf("Conexiones:\n");
    char *directions[4] = {"Arriba", "Abajo", "Izquierda", "Derecha"};
    for (int d = 0; d < 4; d++) {
        int conn = sc->connections[d];
        if (conn != -1) {
            scenario_t *next = get_scenario_by_id(world, conn);
            if (next)
                printf(" %s: %s\n", directions[d], next->name);
        }
    }

    printf("Tiempo restante: %d\n", time_left);
}

// Moverse a otra dirección
void move_to_direction(world_graph_t *world, int direction) {
    scenario_t *sc = get_scenario_by_id(world, current_scenario_id);
    if (!sc) {
        printf("Escenario no válido\n");
        return;
    }
    int next_id = sc->connections[direction];
    if (next_id == -1) {
        printf("No puedes ir en esa dirección\n");
        return;
    }

    current_scenario_id = next_id;
    time_left--;
    printf("Te moviste a %s\n", get_scenario_by_id(world, current_scenario_id)->name);
}

// Agarrar un ítem
void pick_up_item(world_graph_t *world) {
    scenario_t *sc = get_scenario_by_id(world, current_scenario_id);
    if (!sc || list_size(sc->items) == 0) {
        printf("No hay items para agarrar\n");
        return;
    }

    printf("Items disponibles:\n");
    for (int i = 0; i < list_size(sc->items); i++) {
        item_t *it = list_get(sc->items, i);
        printf(" %d) %s (peso: %d, valor: %d)\n", i + 1, it->name, it->weight, it->value);
    }

    printf("Ingresa el número del item: ");
    int choice;
    scanf("%d", &choice);
    if (choice < 1 || choice > list_size(sc->items)) {
        printf("Opción inválida\n");
        return;
    }

    item_t *selected = list_get(sc->items, choice - 1);
    list_remove_at(sc->items, choice - 1);
    list_pushBack(inventory, selected);
    printf("Agarraste: %s\n", selected->name);
}

// Mostrar inventario
void show_inventory() {
    if (list_size(inventory) == 0) {
        printf("Inventario vacío\n");
        return;
    }
    printf("Inventario:\n");
    for (int i = 0; i < list_size(inventory); i++) {
        item_t *it = list_get(inventory, i);
        printf(" - %s (peso: %d, valor: %d)\n", it->name, it->weight, it->value);
    }
}

// Revisar si estás en el escenario final
bool is_final_scenario(world_graph_t *world) {
    scenario_t *sc = get_scenario_by_id(world, current_scenario_id);
    return sc && sc->is_final;
}

// Liberar memoria
void destroy_world_graph(world_graph_t *world) {
    for (int i = 0; i < world->count; i++) {
        scenario_t *sc = world->scenarios[i];
        free(sc->name);
        free(sc->description);
        for (int j = 0; j < list_size(sc->items); j++) {
            item_t *it = list_get(sc->items, j);
            free(it->name);
            free(it);
        }
        list_destroy(sc->items);
        free(sc);
    }
    free(world->scenarios);
    free(world);
}

void free_game_data(world_graph_t *world) {
    destroy_world_graph(world);
    for (int i = 0; i < list_size(inventory); i++) {
        item_t *it = list_get(inventory, i);
        free(it->name);
        free(it);
    }
    list_destroy(inventory);
}

// Menú principal
void show_menu() {
    printf("\nOpciones:\n");
    printf(" 1) Mostrar escenario actual\n");
    printf(" 2) Moverse (0=arriba, 1=abajo, 2=izquierda, 3=derecha)\n");
    printf(" 3) Agarrar item\n");
    printf(" 4) Mostrar inventario\n");
    printf(" 5) Salir\n");
    printf("Elige opción: ");
}

int main() {
    inventory = list_create();

    world_graph_t *world = parse_game_data("data/escenarios.csv");
    if (!world) {
        printf("No se pudo cargar el archivo de escenarios\n");
        return 1;
    }

    printf("Se cargaron %d escenarios correctamente\n", world->count);

    int running = 1;
    while (running && time_left > 0) {
        show_menu();
        int opt;
        scanf("%d", &opt);
        switch (opt) {
            case 1:
                print_current_scenario(world);
                break;
            case 2: {
                printf("Ingresa dirección (0=arriba,1=abajo,2=izquierda,3=derecha): ");
                int dir;
                scanf("%d", &dir);
                if (dir < 0 || dir > 3) {
                    printf("Dirección inválida\n");
                } else {
                    move_to_direction(world, dir);
                }
                break;
            }
            case 3:
                pick_up_item(world); 
                break;
            case 4:
                show_inventory();
                break;
            case 5:
                running = 0;
                break;
            default:
                printf("Opción inválida\n");
        }

        if (is_final_scenario(world)) {
            printf("Llegaste al escenario final, ganaste!\n");
            break;
        }

        if (time_left <= 0) {
            printf("Se acabó el tiempo, perdiste :(\n");
            break;
        }
    }

    free_game_data(world);
    return 0;
}
