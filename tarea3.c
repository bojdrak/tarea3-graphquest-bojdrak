#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "tdas/list.h"

#define MAX_LINE_LENGTH 1024
#define MAX_ESCENARIOS 100

// ------------------ ESTRUCTURAS ------------------
typedef struct Item {
    char nombre[50];
    int peso;
    int valor;
} Item;

typedef struct Escenario {
    int id;
    char nombre[100];
    char descripcion[300];
    List* items;
    int arriba, abajo, izquierda, derecha;
    int esFinal;
} Escenario;

typedef struct Jugador {
    Escenario* ubicacion;
    List* inventario;
    int tiempo;
    int puntaje;
} Jugador;

// ------------------ VARIABLES GLOBALES ------------------

Escenario* escenarios[MAX_ESCENARIOS];
int total_escenarios = 0;
Jugador jugador;

// ------------------ FUNCIONES AUXILIARES ------------------

Escenario* crearEscenario(int id, char* nombre, char* descripcion, int arriba, int abajo, int izquierda, int derecha, int esFinal) {
    Escenario* e = (Escenario*)malloc(sizeof(Escenario));
    e->id = id;
    strcpy(e->nombre, nombre);
    strcpy(e->descripcion, descripcion);
    e->arriba = arriba;
    e->abajo = abajo;
    e->izquierda = izquierda;
    e->derecha = derecha;
    e->esFinal = esFinal;
    e->items = list_create();
    return e;
}

Item* crearItem(char* nombre, int peso, int valor) {
    Item* i = (Item*)malloc(sizeof(Item));
    strcpy(i->nombre, nombre);
    i->peso = peso;
    i->valor = valor;
    return i;
}

Escenario* buscarEscenarioPorID(int id) {
    for (int i = 0; i < total_escenarios; i++) {
        if (escenarios[i]->id == id) return escenarios[i];
    }
    return NULL;
}

void cargarItems(Escenario* escenario, char* str) {
    char* token = strtok(str, ";");
    while (token != NULL) {
        char nombre[50];
        int peso, valor;
        sscanf(token, "%[^,],%d,%d", nombre, &peso, &valor);
        list_pushBack(escenario->items, crearItem(nombre, peso, valor));
        token = strtok(NULL, ";");
    }
}

// ------------------ CARGA CSV ------------------

void cargarLaberinto() {
    FILE* file = fopen("graphquest.csv", "r");
    if (!file) {
        printf("No se pudo abrir el archivo.\n");
        return;
    }
    char line[MAX_LINE_LENGTH];
    fgets(line, MAX_LINE_LENGTH, file); // Saltar cabecera

    while (fgets(line, MAX_LINE_LENGTH, file)) {
        int id, arriba, abajo, izquierda, derecha, esFinal = 0;
        char nombre[100], descripcion[300], items_str[300], final_str[10];

        sscanf(line, "%d,%[^,],\"%[^\"]\",%[^,],%d,%d,%d,%d,%[^\n]",
               &id, nombre, descripcion, items_str, &arriba, &abajo, &izquierda, &derecha, final_str);

        if (strcmp(final_str, "Si") == 0) esFinal = 1;

        Escenario* e = crearEscenario(id, nombre, descripcion, arriba, abajo, izquierda, derecha, esFinal);
        if (strlen(items_str) > 1) cargarItems(e, items_str);
        escenarios[total_escenarios++] = e;
    }
    fclose(file);
    jugador.ubicacion = buscarEscenarioPorID(1); // Entrada principal
    jugador.inventario = list_create();
    jugador.tiempo = 10;
    jugador.puntaje = 0;
    printf("Laberinto cargado con %d escenarios.\n", total_escenarios);
}

// ------------------ JUEGO ------------------

int calcularTiempoMovimiento() {
    int peso = 0;
    Item* item = list_first(jugador.inventario);
    while (item) {
        peso += item->peso;
        item = list_next(jugador.inventario);
    }
    return (peso + 1 + 9) / 10; // ceil
}

void mostrarEstado() {
    Escenario* e = jugador.ubicacion;
    printf("\n=== %s ===\n%s\n\n", e->nombre, e->descripcion);
    printf("Items en este escenario:\n");
    Item* item = list_first(e->items);
    while (item) {
        printf("- %s (Peso: %d, Valor: %d)\n", item->nombre, item->peso, item->valor);
        item = list_next(e->items);
    }
    printf("\nInventario:\n");
    item = list_first(jugador.inventario);
    int peso_total = 0;
    while (item) {
        printf("* %s (Peso: %d, Valor: %d)\n", item->nombre, item->peso, item->valor);
        peso_total += item->peso;
        item = list_next(jugador.inventario);
    }
    printf("Peso total: %d | Puntaje: %d | Tiempo restante: %d\n", peso_total, jugador.puntaje, jugador.tiempo);
}

void iniciarPartida(); // Declaración anticipada

void menuJuego() {
    int opcion;
    while (1) {
        mostrarEstado();
        printf("\nOpciones:\n");
        printf("1. Recoger ítem\n2. Descartar ítem\n3. Avanzar\n4. Reiniciar partida\n5. Salir\n");
        scanf("%d", &opcion);

        if (jugador.tiempo <= 0) {
            printf("Se acabó el tiempo. Has perdido.\n");
            break;
        }

        if (opcion == 1) {
            char nombre[50];
            printf("Nombre del ítem a recoger: ");
            scanf("%s", nombre);
            Item* item = list_first(jugador.ubicacion->items);
            while (item) {
                if (strcmp(item->nombre, nombre) == 0) {
                    list_pushBack(jugador.inventario, item);
                    jugador.puntaje += item->valor;
                    list_popFront(jugador.ubicacion->items);
                    jugador.tiempo--;
                    break;
                }
                item = list_next(jugador.ubicacion->items);
            }
        } else if (opcion == 2) {
            char nombre[50];
            printf("Nombre del ítem a descartar: ");
            scanf("%s", nombre);
            Item* item = list_first(jugador.inventario);
            while (item) {
                if (strcmp(item->nombre, nombre) == 0) {
                    list_popFront(jugador.inventario);
                    jugador.puntaje -= item->valor;
                    jugador.tiempo--;
                    break;
                }
                item = list_next(jugador.inventario);
            }
        } else if (opcion == 3) {
            char dir;
            printf("Dirección (W=Arriba, S=Abajo, A=Izq, D=Derecha): ");
            scanf(" %c", &dir);
            int id_dest = -1;
            if (dir == 'W') id_dest = jugador.ubicacion->arriba;
            else if (dir == 'S') id_dest = jugador.ubicacion->abajo;
            else if (dir == 'A') id_dest = jugador.ubicacion->izquierda;
            else if (dir == 'D') id_dest = jugador.ubicacion->derecha;

            if (id_dest == -1) {
                printf("No hay camino en esa dirección.\n");
            } else {
                jugador.ubicacion = buscarEscenarioPorID(id_dest);
                jugador.tiempo -= calcularTiempoMovimiento();
                if (jugador.ubicacion->esFinal) {
                    printf("¡Llegaste al final! Puntaje final: %d\n", jugador.puntaje);
                    break;
                }
            }
        } else if (opcion == 4) {
            iniciarPartida();
            break;
        } else if (opcion == 5) {
            break;
        }
    }
}

void iniciarPartida() {
    jugador.ubicacion = buscarEscenarioPorID(1);
    jugador.inventario = list_create();
    jugador.tiempo = 10;
    jugador.puntaje = 0;
    menuJuego();
}

// ------------------ MENÚ PRINCIPAL ------------------

int main() {
    int opcion;
    while (1) {
        printf("\n=== GRAPHQUEST ===\n1. Cargar laberinto\n2. Iniciar partida\n3. Salir\n");
        scanf("%d", &opcion);
        if (opcion == 1) cargarLaberinto();
        else if (opcion == 2) iniciarPartida();
        else if (opcion == 3) break;
    }
    return 0;
}
