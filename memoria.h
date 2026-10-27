#ifndef MEMORIA_H
#define MEMORIA_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifndef TAM_PALABRA
#define TAM_PALABRA 4               /* bytes por palabra */
#endif

typedef uint32_t Direccion;         /* direccion en bytes */

typedef struct {
    Direccion direccion;            /* siempre alineada a TAM_PALABRA */
    uint8_t   datos[TAM_PALABRA];
} Palabra;

/* Memoria principal: un min-heap binario de palabras ordenado por direccion.
   Guardado en un arreglo: los hijos de i estan en 2i+1 y 2i+2,
   el padre en (i-1)/2. La raiz es la palabra de menor direccion. */
typedef struct {
    Palabra *palabras;
    size_t   cantidad;
    size_t   capacidad;
} Memoria;

void      memoria_iniciar(Memoria *m);
void      memoria_destruir(Memoria *m);

Direccion alinear(Direccion dir);

/* Lee la palabra que contiene dir. Si nunca se escribio, devuelve ceros. */
void      memoria_leer_palabra(const Memoria *m, Direccion dir, uint8_t salida[TAM_PALABRA]);

/* Escribe la palabra que contiene dir. false si no hay memoria disponible. */
bool      memoria_escribir_palabra(Memoria *m, Direccion dir, const uint8_t entrada[TAM_PALABRA]);

/* Libera la palabra que contiene dir (deja un hueco). false si no existia. */
bool      memoria_liberar_palabra(Memoria *m, Direccion dir);

/* Elimina los huecos: las palabras quedan en 0, TAM_PALABRA, 2*TAM_PALABRA, ...
   conservando su orden. Las direcciones cambian. */
void      memoria_compactar(Memoria *m);

void      memoria_imprimir(const Memoria *m);

#endif
