#include "tdas/extra.h"
#include "tdas/list.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

typedef struct Room {
  int id;
  char name[50];
  char description[256];
  List *items;
  int up, down, left, right;
  int is_final;
} Room;

typedef struct Graph {
  Room *Rooms[256];
  int roomCount;
} Graph;

typedef struct Item {
  char name[50];
  int value;
  int weight;
} Item;

typedef struct Player {
  int carriedWeight;
  int score;
  int remainingTime;
  List *items;
} Player;

Graph *labyrinth;
Graph *labyrinth_backup;

Graph *create_graph() {
  Graph *graph = malloc(sizeof(Graph));
  for (int i = 0; i < 256; i++) {
    graph->Rooms[i] = NULL;
  }
  graph->roomCount = 0;
  return graph;
}

void clear_inventory(Player* player) {
    Item* it = list_first(player->items);
    while (it != NULL) {
        free(it);
        it = list_next(player->items);
    }
    list_clean(player->items);
}

Graph* copy_graph(Graph* original) {
  Graph* copy = create_graph();
  for (int i = 0; i < 256; i++) {
      if (original->Rooms[i] != NULL) {
          Room* r = original->Rooms[i];
          Room* newRoom = malloc(sizeof(Room));
          *newRoom = *r;
          newRoom->items = list_create();
          for (Item* it = list_first(r->items); it != NULL; it = list_next(r->items)) {
              Item* newItem = malloc(sizeof(Item));
              *newItem = *it;
              list_pushBack(newRoom->items, newItem);
          }
          copy->Rooms[i] = newRoom;
      }
  }
  copy->roomCount = original->roomCount;
  return copy;
}

void restore_labyrinth() {
    for (int i = 0; i < 256; i++) {
        if (labyrinth->Rooms[i]) {
            Room* r = labyrinth->Rooms[i];
            for (Item* it = list_first(r->items); it != NULL; it = list_next(r->items)) {
                free(it);
            }
            list_clean(r->items);
            free(r->items);
            free(r);
            labyrinth->Rooms[i] = NULL;
        }
    }
    for (int i = 0; i < 256; i++) {
        if (labyrinth_backup->Rooms[i]) {
            Room* r = labyrinth_backup->Rooms[i];
            Room* newRoom = malloc(sizeof(Room));
            *newRoom = *r;
            newRoom->items = list_create();
            for (Item* it = list_first(r->items); it != NULL; it = list_next(r->items)) {
                Item* newItem = malloc(sizeof(Item));
                *newItem = *it;
                list_pushBack(newRoom->items, newItem);
            }
            labyrinth->Rooms[i] = newRoom;
        }
    }
    labyrinth->roomCount = labyrinth_backup->roomCount;
}

void show_main_menu() {
  printf("\n--- GraphQuest ---\n");
  printf("1. Cargar laberinto CSV\n");
  printf("2. Comenzar juego\n");
  printf("3. Salir\n");
  printf("Selccionar Opción: ");
}

int load_rooms() {
    FILE *file = fopen("graphquest.csv", "r");
    if (file == NULL) {
        perror("Error Abriendo el archivo");
        return 0;
    }

    char **fields;
    fields = read_line_csv(file, ',');

    while ((fields = read_line_csv(file, ',')) != NULL) {
        Room* r = malloc(sizeof(Room));
        r->id = atoi(fields[0]);
        strcpy(r->name, fields[1]);
        strcpy(r->description, fields[2]);
        r->items = list_create();

        List* items = split_string(fields[3], ";");
        for(char *item = list_first(items); item != NULL; item = list_next(items)){
            List* values = split_string(item, ",");
            Item* it = malloc(sizeof(Item));
            strcpy(it->name, list_first(values));
            it->value = atoi(list_next(values));
            it->weight = atoi(list_next(values));
            list_pushBack(r->items, it);
            list_clean(values);
            free(values);
        }
        list_clean(items);
        free(items);

        r->up = atoi(fields[4]);
        r->down = atoi(fields[5]);
        r->left = atoi(fields[6]);
        r->right = atoi(fields[7]);
        r->is_final = (strcmp(fields[8], "Si") == 0 || strcmp(fields[8], "si") == 0);

        labyrinth->Rooms[r->id] = r;
        labyrinth->roomCount++;
    }
    fclose(file);
    labyrinth_backup = copy_graph(labyrinth);
    return 1;
}

void show_status(Player* player, Room* current_room) {
  printf("\n=== Estado Actual ===\n");
  printf("Sala: %s\n", current_room->name);
  printf("Descripción: %s\n", current_room->description);

  printf("\nÍtems en la sala:\n");
  int i = 1;
  for (Item* it = list_first(current_room->items); it != NULL; it = list_next(current_room->items)) {
      printf("  %d) %s (Valor: %d, Peso: %d)\n", i++, it->name, it->value, it->weight);
  }
  if (i == 1) printf("  (No hay ítems en esta sala)\n");

  printf("\nTiempo restante: %d\n", player->remainingTime);

  printf("\nInventario del jugador:\n");
  int total_weight = 0, total_score = 0, j = 1;
  for (Item* it = list_first(player->items); it != NULL; it = list_next(player->items)) {
      printf("  %d) %s (Valor: %d, Peso: %d)\n", j++, it->name, it->value, it->weight);
      total_weight += it->weight;
      total_score += it->value;
  }
  if (j == 1) printf("  (Inventario vacío)\n");
  printf("Peso total: %d\n", total_weight);
  printf("Puntaje acumulado: %d\n", total_score);

  printf("\nDirecciones disponibles:\n");
  if (current_room->up != -1)    printf("  - W (arriba)\n");
  if (current_room->down != -1)  printf("  - S (abajo)\n");
  if (current_room->left != -1)  printf("  - A (izquierda)\n");
  if (current_room->right != -1) printf("  - D (derecha)\n");
}


Room* get_initial_room(Graph* labyrinth) {
  for (int i = 0; i < 256; i++) {
      if (labyrinth->Rooms[i] != NULL) {
          return labyrinth->Rooms[i];
      }
  }
  return NULL;
}

void move_forward(Player* player, Room** current_room, Graph* labyrinth, int* playing) {
    char direction[10];
    printf("¿A qué dirección quieres avanzar? (W = arriba, S = abajo, A = izquierda, D = derecha): ");
    scanf("%s", direction);

    int new_id = -1;
    if ((strcmp(direction, "W") == 0 || strcmp(direction, "w") == 0) && (*current_room)->up != -1)
        new_id = (*current_room)->up;
    else if ((strcmp(direction, "S") == 0 || strcmp(direction, "s") == 0) && (*current_room)->down != -1)
        new_id = (*current_room)->down;
    else if ((strcmp(direction, "A") == 0 || strcmp(direction, "a") == 0) && (*current_room)->left != -1)
        new_id = (*current_room)->left;
    else if ((strcmp(direction, "D") == 0 || strcmp(direction, "d") == 0) && (*current_room)->right != -1)
        new_id = (*current_room)->right;
    else {
        printf("Dirección inválida o no disponible.\n");
        return;
    }

    Room* new_room = labyrinth->Rooms[new_id];
    if (new_room == NULL) {
        printf("No hay una sala en esa dirección.\n");
        return;
    }

    int total_weight = 0;
    for (Item* it = list_first(player->items); it != NULL; it = list_next(player->items)) {
        total_weight += it->weight;
    }

    int time_penalty = (int)ceil((total_weight + 1) / 10.0);
    player->remainingTime -= time_penalty;

    if (player->remainingTime <= 0) {
        printf("¡Te has quedado sin tiempo! Fin del juego.\n");
        *playing = 0;
        return;
    }

    *current_room = new_room;
    printf("Avanzaste a la sala: %s\n", new_room->name);

    if (new_room->is_final) {
        *playing = 0;
    }
}

void collect_items(Player* player, Room* current_room) {
    if (list_size(current_room->items) == 0) {
        printf("No hay ítems para recoger en esta sala.\n");
        return;
    }

    printf("Ítems disponibles para recoger:\n");
    int i = 1;
    for (Item* it = list_first(current_room->items); it != NULL; it = list_next(current_room->items)) {
        printf("  %d) %s (Valor: %d, Peso: %d)\n", i++, it->name, it->value, it->weight);
    }
    printf("Ingrese el número del ítem a recoger (0 para terminar): ");
    int choice;
    while (1) {
        scanf("%d", &choice);
        if (choice == 0) break;
        if (choice < 1 || choice > list_size(current_room->items)) {
            printf("Opción inválida. Intente nuevamente: ");
            continue;
        }
        int idx = 1;
        Item* selected = NULL;
        for (Item* it = list_first(current_room->items); it != NULL; it = list_next(current_room->items)) {
            if (idx == choice) {
                selected = it;
                break;
            }
            idx++;
        }
        if (selected) {
            Item* newItem = malloc(sizeof(Item));
            strcpy(newItem->name, selected->name);
            newItem->value = selected->value;
            newItem->weight = selected->weight;
            list_pushBack(player->items, newItem);
            player->score += newItem->value;

            list_popCurrent(current_room->items);

            player->remainingTime -= 1;
            if (player->remainingTime <= 0) {
                printf("¡Te has quedado sin tiempo! Fin del juego.\n");
                break;
            }

            printf("Recogiste: %s\n", newItem->name);
        }
        if (list_size(current_room->items) == 0) {
            printf("No quedan más ítems en la sala.\n");
            break;
        }
        printf("Ítems restantes:\n");
        idx = 1;
        for (Item* it = list_first(current_room->items); it != NULL; it = list_next(current_room->items)) {
            printf("  %d) %s (Valor: %d, Peso: %d)\n", idx++, it->name, it->value, it->weight);
        }
        printf("Ingrese el número del ítem a recoger (0 para terminar): ");
    }
}

void discard_items(Player* player) {
    if (list_size(player->items) == 0) {
        printf("No tienes ítems para descartar.\n");
        return;
    }

    int choice;
    while (1) {
        printf("\nÍtems en tu inventario:\n");
        int i = 1;
        for (Item* it = list_first(player->items); it != NULL; it = list_next(player->items)) {
            printf("  %d) %s (Valor: %d, Peso: %d)\n", i++, it->name, it->value, it->weight);
        }
        printf("Ingrese el número del ítem a descartar (0 para terminar): ");
        scanf("%d", &choice);

        if (choice == 0) break;
        if (choice < 1 || choice > list_size(player->items)) {
            printf("Opción inválida. Intente nuevamente.\n");
            continue;
        }

        int idx = 1;
        Item* selected = NULL;
        for (Item* it = list_first(player->items); it != NULL; it = list_next(player->items)) {
            if (idx == choice) {
                selected = it;
                break;
            }
            idx++;
        }
        if (selected) {
            printf("Descartaste: %s\n", selected->name);
            list_popCurrent(player->items);
            free(selected);

            player->remainingTime -= 1;
            if (player->remainingTime <= 0) {
                printf("¡Te has quedado sin tiempo! Fin del juego.\n");
                break;
            }
        }
        if (list_size(player->items) == 0) {
            printf("Ya no tienes más ítems en tu inventario.\n");
            break;
        }
    }
}

void game_menu() {
    int option;
    int playing = 1;

    Player player;
    player.items = list_create();
    player.remainingTime  = 10;
    player.carriedWeight = 0;
    player.score = 0;

    Room* current_room = get_initial_room(labyrinth);
    if (current_room == NULL) {
        printf("No se encontró la sala inicial.\n");
        return;
    }

    while (playing) {
        if (current_room->is_final) {
            break;
        }
        show_status(&player, current_room);
        printf("\n--- Menú del Juego ---\n");
        printf("1. Recoger ítem(s)\n");
        printf("2. Descartar ítem(s)\n");
        printf("3. Avanzar en una dirección (WASD)\n");
        printf("4. Reiniciar partida\n");
        printf("5. Salir del juego\n");
        printf("Seleccione una opción: ");
        scanf("%d", &option);

        switch(option) {
            case 1:
                collect_items(&player, current_room);
                break;
            case 2:
                discard_items(&player);
                break;
            case 3:
                move_forward(&player, &current_room, labyrinth, &playing);
                if (!playing) continue;
                break;
            case 4:
                printf("Reiniciando partida...\n");
                clear_inventory(&player);
                restore_labyrinth();
                player.remainingTime = 10;
                player.carriedWeight = 0;
                player.score = 0;
                current_room = get_initial_room(labyrinth);
                break;
            case 5:
                printf("Saliendo del juego...\n");
                playing = 0;
                break;
            default:
                printf("Opción inválida.\n");
        }
    }

    if (current_room->is_final) {
        show_status(&player, current_room);
        printf("\n¡Felicidades! Has llegado a la sala final.\n");
        printf("Puntaje final: %d\n", player.score);
        printf("Ítems recolectados:\n");
        int i = 1;
        for (Item* it = list_first(player.items); it != NULL; it = list_next(player.items)) {
            printf("  %d) %s (Valor: %d, Peso: %d)\n", i++, it->name, it->value, it->weight);
        }
        if (i == 1) printf("  (No recogiste ningún ítem)\n");
        printf("Presiona ENTER para volver al menú principal...");
        getchar();
        getchar();
    }
}

int main() {
    labyrinth = create_graph();

    int option;
    int is_ready = 0;

    do {
        show_main_menu();
        char buffer[32];
        if (!fgets(buffer, sizeof(buffer), stdin)) break;
        char *endptr;
        option = strtol(buffer, &endptr, 10);
        if (endptr == buffer || *endptr != '\n') {
            printf("Opción inválida.\n");
            continue;
        }

        switch(option) {
            case 1:
                if (load_rooms()) {
                    is_ready = 1;
                    printf("Laberinto cargado exitosamente.\n");
                }
                break;
            case 2:
                if (!is_ready) {
                    printf("Primero debes cargar un laberinto.\n");
                } else {
                    game_menu();
                    int c;
                    while ((c = getchar()) != '\n' && c != EOF);
                }
                break;
            case 3:
                printf("¡Hasta luego!\n");
                break;
        }
    } while (option != 3);

    return 0;
}
