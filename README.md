# Spotifind
Este es un programa diseñado con el fin de gestionar una base de datos de canciones. Permite **cargar y buscar** canciones de un archivo CSV de manera eficiente, usando estructuras de datos como Tablas Hash y Listas Enlazadas. El programa tiene un **menú interactivo** que facilita la búsqueda por género, artista y tempo (ritmo de la canción).

## Funcionalidades

### El programa incluye las siguientes funcionalidades:


**1. Cargar canciones desde un archivo CSV**

Carga todas las canciones de un archivo CSV en memoria. Cada canción tiene información como género, artista, tempo y otros atributos relevantes.

**2. Buscar canciones por género**

Permite al usuario ingresar un género (por ejemplo, "acoustic", "samba", "soul", "anime") y listar todas las canciones que coincidan con ese género.

**3. Buscar canciones por artista**

El usuario puede ingresar el nombre de un artista y se mostrarán todas las canciones asociadas a ese artista.

**4. Buscar canciones por tempo**

El usuario puede filtrar canciones por su tempo (ritmo), eligiendo entre:
  1) **Lentas:** Tempo menor a 80 BPM.
  2) **Moderadas:** Tempo entre 80 y 120 BPM.
  3) **Rápidas:** Tempo mayor a 120 BPM.

## Requisitos

### Software Necesario
- **gcc:** Para la compilación del código.

- **MSYS2** o cualquier terminal compatible con compilación de C en Windows.

## Archivos del Proyecto

Asegúrate de tener todos los siguientes archivos en la misma carpeta antes de ejecutar el programa:

- `tarea2.c:` El archivo principal del programa.

- `tdas/extra.c`, `tdas/extra.h`: Funciones auxiliares.

- `tdas/hashmap.c`, `tdas/hashmap.h`: Implementación de la tabla hash.

- `tdas/list.c`, `tdas/list.h`: Implementación de la lista enlazada.

- `tdas/map.c`, `tdas/map.h`: Implementación del mapa.

- `data/song_dataset_.csv`: El archivo CSV con las canciones.

## Instrucciones de Ejecución
### Paso 1: Compilación

Para compilar el programa, abre tu terminal en la carpeta del proyecto y ejecuta el siguiente comando:
```
gcc tarea2.c tdas\*.c -o spotifind
````
Este comando compila todos los archivos C y genera el ejecutable spotifind.exe.

### Paso 2: Ejecución

Una vez compilado, ejecuta el programa con:
````bash
./spotifind.exe
````

## Menú de Opciones

Al ejecutar el programa, el sistema te presentará un menú con las siguientes opciones:
```
========================================
    Spotifind - Buscador de canciones
========================================
1) Cargar canciones
2) Buscar por género
3) Buscar por artista
4) Buscar por tempo
5) Salir
Ingrese una opción: 
```


## Detalles Técnicos
### Estructura de las canciones

El programa esta almacenando los siguentes campos del CSV:

- Título
- Album
- Genero
- Tempo 
- ID de la cancion
- Ritmo de la cancion (en BPM)

El programa lee este CSV y organiza las canciones en una estructura de mapa hash para facilitar la búsqueda eficiente.

## Notas sobre el código
- Se utiliza un HashMap para almacenar las canciones, permitiendo búsquedas rápidas por género, artista y tempo.

- Las canciones se cargan en memoria y se pueden filtrar rápidamente sin necesidad de leer el archivo repetidamente.

- Se implementa un sistema de búsqueda por diferentes criterios (género, artista, tempo) para hacer la experiencia más fluida.

## Ejemplo de Uso

1. Cargar canciones desde un archivo CSV:
```
Se cargaron X canciones.
```
2. Buscar por género:
```
Ingrese el género: acoustic

Título: Comedy
Artista: Gen Hoshino
Album: Comedy
Género: acoustic
Tempo: 87.92 BPM
(...)
```
3. Buscar por artista:
```
Ingrese el nombre del artista: Metallica

Título: Nothing Else Matters
Artista: Metallica
Album: Metallica
Género: hard-rock
Tempo: 142.35 BPM
(...)
```
4. Buscar por tempo:
````
Selecciona la categoría de tempo:
1) Lentas (menos de 80 BPM)
2) Moderadas (80 - 120 BPM)
3) Rápidas (más de 120 BPM)

1 <--- Opcion lentas

Título: The Departure
Artista: Max Richter;Lang Lang
Album: Voyager - Essential Max Richter
Género: ambient
Tempo: 0.00 BPM
(...)

--- Mostradas 200 canciones Lentas ---
1) Mostrar más
2) Volver al menú
````
En el menú de Genero y Tempo, luego de mostrar 20 canciones, se abre un **menú para evitar sobre carga de informacion**. Esto se puede cambiar en `MAX_SONGS_TEMPO` en las primeras lineas del código.

## Errores conocidos:
- Todos los **menús interactivos** esperan números  como respuesta, si se ingresa texto en estos campos es esperable que ocurran errores.

- No hay un seguro sobre cargar mas de una vez el archivo. Si se cargan **dos o mas veces** es esperable que hayan multiples problemas
