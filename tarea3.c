#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "tdas/list.h"
#include "parser.h"

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
    List *items;         // Lista de item_t*
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
        printf("No podís ir en esa dirección\n");
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
    list_push_back(inventory, selected);
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
