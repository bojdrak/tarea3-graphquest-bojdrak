# GraphQuest  
**GraphQuest** es un juego tipo laberinto donde el jugador debe avanzar entre salas, recolectar ítems y llegar a una sala final antes de quedarse sin tiempo. Las salas están representadas como nodos de un grafo cargado desde un archivo CSV, y el jugador puede moverse usando las teclas **W, A, S, D**.

## Funcionalidades

### El programa incluye las siguientes funcionalidades:

**1. Cargar laberinto desde un archivo CSV**  
Carga la estructura del laberinto desde un archivo `graphquest.csv`. Cada sala contiene información como nombre, descripción, ítems disponibles y conexiones con otras salas.

**2. Navegación con WASD**  
El jugador puede moverse por el laberinto usando:
- `W` para arriba
- `A` para izquierda
- `S` para abajo
- `D` para derecha  
Cada movimiento consume tiempo dependiendo del peso del inventario.

**3. Recolección de ítems**  
En cada sala, el jugador puede recoger ítems, los cuales tienen un valor (puntaje) y un peso (afecta el tiempo de movimiento).

**4. Inventario del jugador**  
El jugador puede ver y administrar su inventario:
- Ver ítems actuales
- Descartar ítems para reducir peso

**5. Reinicio de partida**  
El jugador puede reiniciar el juego en cualquier momento, lo que restablece el laberinto y el estado inicial del jugador.

## Requisitos

### Software necesario
- **gcc** para compilar el código.
- **MSYS2** o cualquier terminal compatible con compilación C en Windows.

## Archivos del proyecto

Asegúrate de tener los siguientes archivos:

- `tarea3.c` – Archivo principal del juego
- `tdas/extra.c`, `tdas/extra.h` – Funciones auxiliares como `read_line_csv`
- `tdas/list.c`, `tdas/list.h` – Implementación de lista enlazada
- `graphquest.csv` – Archivo con la estructura del laberinto

## Instrucciones de Ejecución

### Paso 1: Compilación

Desde la carpeta del proyecto, compila usando:

```bash
gcc tarea3.c tdas/*.c -o graphquest
```

### Paso 2: Ejecución

```bash
./graphquest
```

## Menú de Opciones

```
--- GraphQuest ---
1. Cargar laberinto CSV
2. Comenzar juego
3. Salir
Seleccionar Opción:
```

## Menú del Juego

Una vez iniciado el juego:

```
--- Menú del Juego ---
1. Recoger ítem(s)
2. Descartar ítem(s)
3. Avanzar en una dirección (WASD)
4. Reiniciar partida
5. Salir del juego
Seleccione una opción:
```

## Detalles Técnicos

### Estructura de las Salas

Cada sala contiene:
- ID
- Nombre
- Descripción
- Ítems (lista enlazada)
- Conexiones (arriba, abajo, izquierda, derecha)
- Indicador si es la sala final

### Estructura del Jugador

El jugador tiene:
- Tiempo restante
- Puntaje acumulado
- Peso cargado
- Inventario (lista enlazada de ítems)

### Tiempo y Movimiento

Cada movimiento consume tiempo en base a la fórmula:
```
ceil((peso total + 1) / 10.0)
```

Recoger o descartar ítems también consume 1 unidad de tiempo.

## Formato del CSV

El archivo `graphquest.csv` debe tener el siguiente formato por línea (sin encabezado obligatorio):
```
ID,Nombre,Descripción,"nombre,valor,peso;nombre2,valor2,peso2",Arriba,Abajo,Izquierda,Derecha,Final
```
Ejemplo:
```
0,Entrada,Una sala fría,"espada,10,3;escudo,5,2",-1,1,-1,-1,no
```

## Errores Conocidos

- Si el CSV no tiene el formato adecuado, puede generar errores o saltarse salas.
- El input espera texto o números según el contexto. Ingresar valores inesperados puede producir comportamientos no deseados.

## Ejemplo de Uso

1. Cargar el laberinto:
```
Laberinto cargado exitosamente.
```

2. Avanzar en una dirección:
```
¿A qué dirección quieres avanzar? (W = arriba, S = abajo, A = izquierda, D = derecha):
d
Avanzaste a la sala: Pasillo Angosto
```

3. Recolectar ítems:
```
Ítems disponibles para recoger:
1) Espada (Valor: 10, Peso: 3)
2) Antorcha (Valor: 2, Peso: 1)
Ingrese el número del ítem a recoger (0 para terminar):
```

4. Fin del juego:
```
¡Felicidades! Has llegado a la sala final.
Puntaje final: 47
Ítems recolectados:
1) Espada (Valor: 10, Peso: 3)
2) Pergamino (Valor: 15, Peso: 2)
```