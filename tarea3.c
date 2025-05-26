#include "tdas/extra.h"
#include "tdas/list.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// Estructura que representa una sala del laberinto.
// Cada sala tiene un ID, un nombre, una descripción, una lista de ítems y
// conexiones a otras salas (arriba, abajo, izquierda, derecha).
// También se indica si es la sala final del juego.
typedef struct Room {
  int id;
  char name[50];
  char description[256];
  List *items;
  int up, down, left, right;
  int is_final;
} Room;

// Representa todo el laberinto como un grafo compuesto por salas.
// Usa un arreglo para almacenar hasta 256 salas, y lleva la cuenta de cuántas hay.
typedef struct Graph {
  Room *Rooms[256];
  int roomCount;
} Graph;

// Estructura que representa un ítem dentro del juego.
// Cada ítem tiene un nombre, un valor (puntaje) y un peso (afecta el tiempo).
typedef struct Item {
  char name[50];
  int value;
  int weight;
} Item;

// Representa al jugador del juego.
// Guarda el peso que lleva encima, su puntaje total, el tiempo restante y su inventario de ítems.
typedef struct Player {
  int carriedWeight;
  int score;
  int remainingTime;
  List *items;
} Player;

// Declaración de variables globales: el laberinto actual y una copia de respaldo.
Graph *labyrinth;
Graph *labyrinth_backup;

// Función que crea un nuevo grafo vacío y lo inicializa.
// Inicializa todas las posiciones del arreglo a NULL y cuenta de salas en cero.
Graph *create_graph() {
  Graph *graph = malloc(sizeof(Graph));
  for (int i = 0; i < 256; i++) {
    graph->Rooms[i] = NULL;
  }
  graph->roomCount = 0;
  return graph;
}

// Limpia el inventario del jugador liberando la memoria de cada ítem y vaciando la lista.
void clear_inventory(Player* player) {
    Item* it = list_first(player->items);
    while (it != NULL) {
        free(it);
        it = list_next(player->items);
    }
    list_clean(player->items);
}

// Crea una copia exacta de un grafo (laberinto), incluyendo todas las salas y sus ítems.
// Esto se usa para poder reiniciar el juego desde el estado inicial.
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

// Restaura el laberinto original usando la copia de respaldo.
// Libera toda la memoria del laberinto actual y copia nuevamente los datos desde la versión guardada.
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
// Muestra el menú principal del juego en consola.
void show_main_menu() {
  printf("\n--- GraphQuest ---\n");
  printf("1. Cargar laberinto CSV\n");
  printf("2. Comenzar juego\n");
  printf("3. Salir\n");
  printf("Selccionar Opción: ");
}

// Esta función carga el archivo CSV que contiene la información del laberinto.
// Cada fila representa una sala con su descripción, ítems y conexiones a otras salas.
int load_rooms() {
    FILE *file = fopen("graphquest.csv", "r");  // Abre el archivo en modo lectura
    if (file == NULL) {
        perror("Error Abriendo el archivo");
        return 0;
    }

    char **fields;
    fields = read_line_csv(file, ',');  // Lee y descarta la cabecera

    // Lee cada línea del CSV y crea una sala
    while ((fields = read_line_csv(file, ',')) != NULL) {
        Room* r = malloc(sizeof(Room));
        r->id = atoi(fields[0]);
        strcpy(r->name, fields[1]);
        strcpy(r->description, fields[2]);
        r->items = list_create();

        // Procesa los ítems separados por ";", y cada ítem tiene nombre, valor y peso
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

        // Define las conexiones de la sala con otras
        r->up = atoi(fields[4]);
        r->down = atoi(fields[5]);
        r->left = atoi(fields[6]);
        r->right = atoi(fields[7]);
        r->is_final = (strcmp(fields[8], "Si") == 0 || strcmp(fields[8], "si") == 0);

        // Añade la sala al laberinto
        labyrinth->Rooms[r->id] = r;
        labyrinth->roomCount++;
    }

    fclose(file);

    // Crea una copia de seguridad del laberinto cargado para poder restaurarlo después si se desea
    labyrinth_backup = copy_graph(labyrinth);
    return 1;
}

// Muestra en consola el estado actual del jugador y la sala en la que se encuentra
void show_status(Player* player, Room* current_room) {
  printf("\n=== Estado Actual ===\n");
  printf("Sala: %s\n", current_room->name);
  printf("Descripción: %s\n", current_room->description);

  // Lista los ítems disponibles en la sala actual
  printf("\nÍtems en la sala:\n");
  int i = 1;
  for (Item* it = list_first(current_room->items); it != NULL; it = list_next(current_room->items)) {
      printf("  %d) %s (Valor: %d, Peso: %d)\n", i++, it->name, it->value, it->weight);
  }
  if (i == 1) printf("  (No hay ítems en esta sala)\n");

  // Muestra el tiempo que le queda al jugador
  printf("\nTiempo restante: %d\n", player->remainingTime);

  // Muestra el inventario del jugador
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

  // Muestra las posibles direcciones que el jugador puede tomar
  printf("\nDirecciones disponibles:\n");
  if (current_room->up != -1)    printf("  - W (arriba)\n");
  if (current_room->down != -1)  printf("  - S (abajo)\n");
  if (current_room->left != -1)  printf("  - A (izquierda)\n");
  if (current_room->right != -1) printf("  - D (derecha)\n");
}

// Esta función retorna la primera sala válida del laberinto.
// Se asume que es la sala inicial del juego.
Room* get_initial_room(Graph* labyrinth) {
  for (int i = 0; i < 256; i++) {
      if (labyrinth->Rooms[i] != NULL) {
          return labyrinth->Rooms[i];
      }
  }
  return NULL;
}

// Esta función permite al jugador moverse usando las teclas W, A, S, D.
// También calcula el costo en tiempo del movimiento en base al peso que lleva.
void move_forward(Player* player, Room** current_room, Graph* labyrinth, int* playing) {
    char direction[10];
    printf("¿A qué dirección quieres avanzar? (W = arriba, S = abajo, A = izquierda, D = derecha): ");
    scanf("%s", direction);
    getchar();  // Limpiar el buffer de entrada
    int new_id = -1;

    // Determina la nueva dirección según la tecla ingresada
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
        waitForKeyPress();
        return;
    }

    Room* new_room = labyrinth->Rooms[new_id];
    if (new_room == NULL) {
        printf("No hay una sala en esa dirección.\n");
        return;
    }

    // Calcula el peso total que el jugador está cargando
    int total_weight = 0;
    for (Item* it = list_first(player->items); it != NULL; it = list_next(player->items)) {
        total_weight += it->weight;
    }

    // El tiempo que se pierde al moverse depende del peso
    int time_penalty = (int)ceil((total_weight + 1) / 10.0);
    player->remainingTime -= time_penalty;

    if (player->remainingTime <= 0) {
        printf("¡Te has quedado sin tiempo! Fin del juego.\n");
        *playing = 0;
        return;
    }

    *current_room = new_room;
    printf("Avanzaste a la sala: %s\n", new_room->name);

    // Si se llega a la sala final, el juego termina
    if (new_room->is_final) {
        *playing = 0;
    }
}

// Esta función permite al jugador recoger ítems de la sala actual.
// Los ítems recogidos se agregan al inventario del jugador y se descuentan del tiempo disponible.
void collect_items(Player* player, Room* current_room) {
    if (list_size(current_room->items) == 0) {
        printf("No hay ítems para recoger en esta sala.\n");
        waitForKeyPress();
        return;
    }

    printf("Ítems disponibles para recoger:\n");
    int i = 1;
    for (Item* it = list_first(current_room->items); it != NULL; it = list_next(current_room->items)) {
        printf("  %d) %s (Valor: %d, Peso: %d)\n", i++, it->name, it->value, it->weight);
    }
    printf("Ingrese el número del ítem a recoger (0 para terminar): ");
    int choice;

    // Bucle para permitir recoger múltiples ítems
    while (1) {
        scanf("%d", &choice);
        getchar();  // Limpiar el buffer de entrada
        if (choice == 0) break;
        if (choice < 1 || choice > list_size(current_room->items)) {
            printf("Opción inválida. Intente nuevamente: ");
            continue;
        }

        // Encuentra el ítem seleccionado por el jugador
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
            // Duplica el ítem para añadirlo al inventario del jugador
            Item* newItem = malloc(sizeof(Item));
            strcpy(newItem->name, selected->name);
            newItem->value = selected->value;
            newItem->weight = selected->weight;
            list_pushBack(player->items, newItem);
            player->score += newItem->value;

            // Elimina el ítem de la sala
            list_popCurrent(current_room->items);

            // Se descuenta tiempo por recoger el ítem
            player->remainingTime -= 1;
            if (player->remainingTime <= 0) {
                printf("¡Te has quedado sin tiempo! Fin del juego.\n");
                break;
            }

            printf("Recogiste: %s\n", newItem->name);
        }

        // Si ya no quedan ítems, se informa al jugador
        if (list_size(current_room->items) == 0) {
            printf("No quedan más ítems en la sala.\n");
            break;
        }

        // Muestra los ítems restantes en la sala
        printf("Ítems restantes:\n");
        idx = 1;
        for (Item* it = list_first(current_room->items); it != NULL; it = list_next(current_room->items)) {
            printf("  %d) %s (Valor: %d, Peso: %d)\n", idx++, it->name, it->value, it->weight);
        }
        printf("Ingrese el número del ítem a recoger (0 para terminar): ");
    }
}

// Esta función permite al jugador descartar ítems de su inventario.
void discard_items(Player* player) {
    if (list_size(player->items) == 0) {
        printf("No tienes ítems para descartar.\n");
        waitForKeyPress();
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
        getchar();  // Limpiar el buffer de entrada

        if (choice == 0) break;
        if (choice < 1 || choice > list_size(player->items)) {
            printf("Opción inválida. Intente nuevamente.\n");
            waitForKeyPress();
            continue;
        }

        // Encuentra el ítem seleccionado por el jugador
        // y lo elimina de su inventario.
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

// Esta función maneja el menú del juego, donde el jugador puede interactuar con el laberinto.
void game_menu() {
    int option;
    int playing = 1;

    Player player;
    player.items = list_create();
    player.remainingTime  = 10;
    player.carriedWeight = 0;
    player.score = 0;

    // Se obtiene la sala inicial del laberinto
    Room* current_room = get_initial_room(labyrinth);
    if (current_room == NULL) {
        printf("No se encontró la sala inicial.\n");
        return;
    }


    while (playing) {
        if (current_room->is_final) {
            break;
        }
        // Muestra el estado actual del jugador y la sala
        show_status(&player, current_room);
        printf("\n--- Menú del Juego ---\n");
        printf("1. Recoger ítem(s)\n");
        printf("2. Descartar ítem(s)\n");
        printf("3. Avanzar en una dirección (WASD)\n");
        printf("4. Reiniciar partida\n");
        printf("5. Salir del juego\n");
        printf("Seleccione una opción: ");
        scanf("%d", &option);
        getchar();  // Limpiar el buffer de entrada

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
                waitForKeyPress();
                
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

    // Si el jugador llegó a la sala final, muestra el estado final del juego
    if (current_room->is_final) {
        show_status(&player, current_room);
        printf("\n¡Felicidades! Has llegado a la sala final.\n");
        printf("Puntaje final: %d\n", player.score);
        printf("Ítems recolectados:\n");
        int i = 1;
        for (Item* it = list_first(player.items); it != NULL; it = list_next(player.items)) {
            printf("  %d) %s (Valor: %d, Peso: %d)\n", i++, it->name, it->value, it->weight);
        }
        if (i == 1) { 
            printf("  (No recogiste ningún ítem)\n");
            waitForKeyPress();
        }
    }
}

// Función principal que inicia el juego y muestra el menú principal.
// Permite al usuario cargar un laberinto desde un archivo CSV y comenzar a jugar.
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
