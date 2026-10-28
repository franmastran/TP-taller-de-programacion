#ifndef CACHE_H
#define CACHE_H

#include <stdio.h>
#include <stdlib.h>

typedef struct Cache {
    char *dato;
    int direccion;
    int ultimo;
    int frecuencia;
} Cache;

/* Cada elemento del arreglo es un bloque; el arreglo es un min-heap LFU (desempate por antiguedad).
 * Se inicializa una sola cache por vez. */
int cache_iniciar(int capacidad, int tam_bloque);
void cache_destruir(void);

/* 1: hit, 0: miss, -1: error. direccion es el inicio del bloque. */
int cache_leer(int direccion, int offset, char *destino, int n);
int cache_escribir(int direccion, int offset, char *origen, int n);

/* Copia un bloque completo. 0: correcto, -1: error. */
int cache_insertar(int direccion, char *bloque);
void cache_vaciar(void);

/* Operaciones del heap. */
int cache_buscar(int direccion);
int cache_menor(int i, int j);
void cache_marcar_usado(int i);
int cache_rango_valido(int direccion, int offset, char *datos, int n);
void cache_intercambiar(int i, int j);
void cache_subir(int i);
void cache_bajar(int i);

/* Estadisticas. */
int cache_hits(void);
int cache_misses(void);
double cache_tasa_aciertos(void);
void cache_reiniciar_estadisticas(void);
int cache_tamano_bloque(void);
void cache_imprimir(void);

#endif
