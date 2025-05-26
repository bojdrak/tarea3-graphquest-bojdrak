#ifndef TAREA3_H
#define TAREA3_H

typedef struct item item_t;
typedef struct scenario scenario_t;
typedef struct world_graph world_graph_t;

scenario_t *get_scenario_by_id(world_graph_t *world, int id);
void print_current_scenario(world_graph_t *world);
void move_to_direction(world_graph_t *world, int direction);
void pick_up_item(world_graph_t *world);
void show_inventory();
void free_game_data(world_graph_t *world);
void destroy_world_graph(world_graph_t *world);
void destroy_scenario(scenario_t *scen);
void free_tokens(char **tokens, int count);
void show_menu();
int main();

#endif // TAREA3_H

