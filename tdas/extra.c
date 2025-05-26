#include "extra.h"


#define MAX_LINE_LENGTH 4096
#define MAX_FIELDS      128

char **read_line_csv(FILE *file, char separator) {
    static char line[MAX_LINE_LENGTH];
    static char *field[MAX_FIELDS];
    int idx = 0;

    if (fgets(line, MAX_LINE_LENGTH, file) == NULL)
        return NULL;  // fin de fichero

    // quitar salto de línea
    line[strcspn(line, "\r\n")] = '\0';

    char *ptr = line;
    while (*ptr && idx < MAX_FIELDS - 1) {
        char *start;

        if (*ptr == '\"') {
            // campo entrecomillado
            ptr++;              // saltar la comilla inicial
            start = ptr;

            // compactar contenido: convertir "" → " y copiar el resto
            char *dest = ptr;
            while (*ptr) {
                if (*ptr == '\"' && *(ptr + 1) == '\"') {
                    *dest++ = '\"';  // una comilla literal
                    ptr += 2;        // saltar ambas
                }
                else if (*ptr == '\"') {
                    ptr++;           // fin del campo
                    break;
                }
                else {
                    *dest++ = *ptr++;
                }
            }
            *dest = '\0';        // terminar cadena

            // ahora ptr apunta justo después de la comilla de cierre
            if (*ptr == separator) ptr++;
        }
        else {
            // campo sin comillas
            start = ptr;
            while (*ptr && *ptr != separator)
                ptr++;
            if (*ptr == separator) {
                *ptr = '\0';
                ptr++;
            }
        }

        field[idx++] = start;
    }

    field[idx] = NULL;
    return field;
}


List *split_string(const char *str, const char *delim) {
  List *result = list_create();
  char *token = strtok((char *)str, delim);

  while (token != NULL) {
    // Eliminar espacios en blanco al inicio del token
    while (*token == ' ') {
      token++;
    }

    // Eliminar espacios en blanco al final del token
    char *end = token + strlen(token) - 1;
    while (*end == ' ' && end > token) {
      *end = '\0';
      end--;
    }

    // Copiar el token en un nuevo string
    char *new_token = strdup(token);

    // Agregar el nuevo string a la lista
    list_pushBack(result, new_token);

    // Obtener el siguiente token
    token = strtok(NULL, delim);
  }

  return result;
}

// Función para limpiar la pantalla
void clearScreen() {
  printf("\033[H\033[J");
}

void waitForKeyPress() {
  printf("Presione Enter para continuar...");
  while (getchar() != '\n');
}