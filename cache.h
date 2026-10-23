#ifndef CACHE_H
#define CACHE_H
 
#include "memoria.h"

typedef struct Heap {
    int *dato;
    int ultimo;
} Heap;


void cache_iniciar();
 
/* Si el bloque esta en cache copia 'n' bytes desde 'offset' dentro del bloque
 * a 'destino', marca el bloque como recien usado y devuelve 1 (hit).
 * Si no esta devuelve 0 (miss) y no toca nada. */
int  cache_leer();
 
/* Igual que cache_leer pero escribe 'origen' en la linea (write-through:
 * quien llama tambien escribe en memoria). 1 = estaba, 0 = no estaba. */
int  cache_escribir();
 
/* Mete un bloque completo (TAM_BLOQUE_BYTES bytes). Si esta llena, descarta
 * la raiz del heap (LRU) y lo reemplaza. */
void cache_insertar();
 
/* Deja la cache vacia (se usa antes de compactar). */
void cache_vaciar();
 
void cache_imprimir();        /* opcional, para depurar */
 
#endif
 
