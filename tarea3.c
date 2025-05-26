#include "tdas/extra.h"
#include "tdas/list.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <windows.h>

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
  printf("1. Load Labyrinth from CSV\n");
  printf("2. Start Game\n");
  printf("3. Exit\n");
  printf("Select an option: ");
}

int load_rooms() {
    char fileName[256];
    printf("Enter the name of the CSV file containing your labyrinth: ");
    scanf("%s", fileName);
    int c;
    while ((c = getchar()) != '\n' && c != EOF);

    FILE *file = fopen(fileName, "r");
    if (file == NULL) {
        perror("Error opening file");
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
  printf("\n=== Current Status ===\n");
  printf("Room: %s\n", current_room->name);
  printf("Description: %s\n", current_room->description);

  printf("\nItems in the room:\n");
  int i = 1;
  for (Item* it = list_first(current_room->items); it != NULL; it = list_next(current_room->items)) {
      printf("  %d) %s (Value: %d, Weight: %d)\n", i++, it->name, it->value, it->weight);
  }
  if (i == 1) printf("  (No items in this room)\n");

  printf("\nRemaining time: %d\n", player->remainingTime);

  printf("\nPlayer's inventory:\n");
  int total_weight = 0, total_score = 0, j = 1;
  for (Item* it = list_first(player->items); it != NULL; it = list_next(player->items)) {
      printf("  %d) %s (Value: %d, Weight: %d)\n", j++, it->name, it->value, it->weight);
      total_weight += it->weight;
      total_score += it->value;
  }
  if (j == 1) printf("  (Empty inventory)\n");
  printf("Total weight: %d\n", total_weight);
  printf("Accumulated score: %d\n", total_score);

  printf("\nAvailable directions:\n");
  if (current_room->up != -1)    printf("  - Up\n");
  if (current_room->down != -1)     printf("  - Down\n");
  if (current_room->left != -1) printf("  - Left\n");
  if (current_room->right != -1)   printf("  - Right\n");
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
    char direction[20];
    printf("Which direction do you want to move? (up/down/left/right): ");
    scanf("%s", direction);

    int new_id = -1;
    if (strcmp(direction, "up") == 0 && (*current_room)->up != -1)
        new_id = (*current_room)->up;
    else if (strcmp(direction, "down") == 0 && (*current_room)->down != -1)
        new_id = (*current_room)->down;
    else if (strcmp(direction, "left") == 0 && (*current_room)->left != -1)
        new_id = (*current_room)->left;
    else if (strcmp(direction, "right") == 0 && (*current_room)->right != -1)
        new_id = (*current_room)->right;
    else {
        printf("Invalid or unavailable direction.\n");
        return;
    }

    Room* new_room = labyrinth->Rooms[new_id];
    if (new_room == NULL) {
        printf("No room in that direction.\n");
        return;
    }

    int total_weight = 0;
    for (Item* it = list_first(player->items); it != NULL; it = list_next(player->items)) {
        total_weight += it->weight;
    }

    int time_penalty = (int)ceil((total_weight + 1) / 10.0);
    player->remainingTime -= time_penalty;

    if (player->remainingTime <= 0) {
        printf("You've run out of time! Game over.\n");
        *playing = 0;
        return;
    }

    *current_room = new_room;
    printf("You moved to the room: %s\n", new_room->name);

    if (new_room->is_final) {
        *playing = 0;
        return;
    }
}

void collect_items(Player* player, Room* current_room) {
    if (list_size(current_room->items) == 0) {
        printf("There are no items to collect in this room.\n");
        return;
    }

    printf("Items available for collection:\n");
    int i = 1;
    for (Item* it = list_first(current_room->items); it != NULL; it = list_next(current_room->items)) {
        printf("  %d) %s (Value: %d, Weight: %d)\n", i++, it->name, it->value, it->weight);
    }
    printf("Enter the number of the item to collect (0 to finish): ");
    int choice;
    while (1) {
        scanf("%d", &choice);
        if (choice == 0) break;
        if (choice < 1 || choice > list_size(current_room->items)) {
            printf("Invalid option. Try again: ");
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
                printf("You have run out of time! Game over.\n");
                break;
            }

            printf("You collected: %s\n", newItem->name);
        }
        if (list_size(current_room->items) == 0) {
            printf("There are no more items in the room.\n");
            break;
        }
        printf("Remaining items:\n");
        idx = 1;
        for (Item* it = list_first(current_room->items); it != NULL; it = list_next(current_room->items)) {
            printf("  %d) %s (Value: %d, Weight: %d)\n", idx++, it->name, it->value, it->weight);
        }
        printf("Enter the number of the item to collect (0 to finish): ");
    }
}

void discard_items(Player* player) {
    if (list_size(player->items) == 0) {
        printf("You have no items to discard.\n");
        return;
    }

    int choice;
    while (1) {
        printf("\nItems in your inventory:\n");
        int i = 1;
        for (Item* it = list_first(player->items); it != NULL; it = list_next(player->items)) {
            printf("  %d) %s (Value: %d, Weight: %d)\n", i++, it->name, it->value, it->weight);
        }
        printf("Enter the number of the item to discard (0 to finish): ");
        scanf("%d", &choice);

        if (choice == 0) break;
        if (choice < 1 || choice > list_size(player->items)) {
            printf("Invalid option. Try again.\n");
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
            printf("You discarded: %s\n", selected->name);
            list_popCurrent(player->items);
            free(selected);

            player->remainingTime -= 1;
            if (player->remainingTime <= 0) {
                printf("You're out of time! Game over.\n");
                break;
            }
        }
        if (list_size(player->items) == 0) {
            printf("You have no more items in your inventory.\n");
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
        printf("Initial room not found.\n");
        return;
    }

    while (playing) {
        if (current_room->is_final) {
            break;
        }
        show_status(&player, current_room);
        printf("\n--- Game Menu ---\n");
        printf("1. Pick up item(s)\n");
        printf("2. Discard item(s)\n");
        printf("3. Move in a direction\n");
        printf("4. Restart game\n");
        printf("5. Exit game\n");
        printf("Choose an option: ");
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
                printf("Restarting game...\n");
                clear_inventory(&player);
                restore_labyrinth();
                player.remainingTime = 10;
                player.carriedWeight = 0;
                player.score = 0;
                current_room = get_initial_room(labyrinth);
                break;
            case 5:
                printf("Exiting game...\n");
                playing = 0;
                break;
            default:
                printf("Invalid option.\n");
        }
    }

    if (current_room->is_final) {
        show_status(&player, current_room);
        printf("\nCongratulations! You've reached the final room.\n");
        printf("Final score: %d\n", player.score);
        printf("Collected items:\n");
        int i = 1;
        for (Item* it = list_first(player.items); it != NULL; it = list_next(player.items)) {
            printf("  %d) %s (Value: %d, Weight: %d)\n", i++, it->name, it->value, it->weight);
        }
        if (i == 1) printf("  (You didn't collect any items)\n");
        printf("Press ENTER to return to the main menu...");
        getchar();
        getchar();
    }
}

int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
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
            printf("Invalid option.\n");
            continue;
        }

        switch(option) {
            case 1:
                if (load_rooms()) {
                    is_ready = 1;
                    printf("Labyrinth successfully loaded.\n");
                }
                break;
            case 2:
                if (!is_ready) {
                    printf("You must first load a labyrinth.\n");
                } else {
                    game_menu();
                    int c;
                    while ((c = getchar()) != '\n' && c != EOF);
                }
                break;
            case 3:
                printf("Goodbye!\n");
                break;
        }
    } while (option != 3);

    return 0;
}
