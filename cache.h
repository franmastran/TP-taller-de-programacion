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

int cache_iniciar(int capacidad, int tam_bloque);

int  cache_leer();

void cache_imprimir();

void cache_vaciar();

void cache_destruir();

void cache_intercambiar(int i, int j);

void cache_subir(int i);

void cache_bajar(int i);

int cache_buscar(int direccion);

int cache_menor(int i, int j);

void cache_marcar_usado(int i);

int cache_escribir(int direccion, int offset,char *origen, int n);

int cache_insertar(int direccion, char *bloque);


#endif
 
 
