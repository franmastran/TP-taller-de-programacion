#include "memoria.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CAPACIDAD_INICIAL 16
#define NO_ENCONTRADO ((size_t)-1)

/* ---------------- heap: operaciones basicas ---------------- */

size_t padre(size_t i)     { return (i - 1) / 2; }
size_t hijo_izq(size_t i)  { return 2 * i + 1; }
size_t hijo_der(size_t i)  { return 2 * i + 2; }

void intercambiar(Palabra *a, Palabra *b)
{
    Palabra aux = *a;
    *a = *b;
    *b = aux;
}


void subir(Palabra *p, size_t i)
{
    while (i > 0 && p[i].direccion < p[padre(i)].direccion) {
        intercambiar(&p[i], &p[padre(i)]);
        i = padre(i);
    }
}


void bajar(Palabra *p, size_t cantidad, size_t i)
{
    for (;;) {
        size_t menor = i;
        size_t izq = hijo_izq(i);
        size_t der = hijo_der(i);

        if (izq < cantidad && p[izq].direccion < p[menor].direccion) menor = izq;
        if (der < cantidad && p[der].direccion < p[menor].direccion) menor = der;
        if (menor == i)
            return;
        intercambiar(&p[i], &p[menor]);
        i = menor;
    }
}

size_t buscar_desde(const Memoria *m, size_t i, Direccion dir)
{
    if (i >= m->cantidad || m->palabras[i].direccion > dir)
        return NO_ENCONTRADO;
    if (m->palabras[i].direccion == dir)
        return i;

    size_t encontrado = buscar_desde(m, hijo_izq(i), dir);
    return encontrado != NO_ENCONTRADO ? encontrado : buscar_desde(m, hijo_der(i), dir);
}

size_t buscar(const Memoria *m, Direccion dir)
{
    return buscar_desde(m, 0, dir);
}

bool asegurar_capacidad(Memoria *m)
{
    if (m->cantidad < m->capacidad)
        return true;
    size_t nueva_capacidad = m->capacidad == 0 ? CAPACIDAD_INICIAL : 2 * m->capacidad;
    Palabra *agrandado = realloc(m->palabras, nueva_capacidad * sizeof *agrandado);
    if (agrandado == NULL)
        return false;
    m->palabras = agrandado;
    m->capacidad = nueva_capacidad;
    return true;
}

/* ---------------- memoria ---------------- */

void memoria_iniciar(Memoria *m)
{
    m->palabras = NULL;
    m->cantidad = 0;
    m->capacidad = 0;
}

void memoria_destruir(Memoria *m)
{
    free(m->palabras);
    memoria_iniciar(m);
}

Direccion alinear(Direccion dir)
{
    return dir - dir % TAM_PALABRA;
}

void memoria_leer_palabra(const Memoria *m, Direccion dir, uint8_t salida[TAM_PALABRA])
{
    size_t i = buscar(m, alinear(dir));
    if (i == NO_ENCONTRADO)
        memset(salida, 0, TAM_PALABRA);
    else
        memcpy(salida, m->palabras[i].datos, TAM_PALABRA);
}

bool memoria_escribir_palabra(Memoria *m, Direccion dir, const uint8_t entrada[TAM_PALABRA])
{
    dir = alinear(dir);

    size_t i = buscar(m, dir);
    if (i != NO_ENCONTRADO) {                  /* ya existe: se sobrescribe */
        memcpy(m->palabras[i].datos, entrada, TAM_PALABRA);
        return true;
    }

    if (!asegurar_capacidad(m))                /* no existe: insercion en el heap */
        return false;
    m->palabras[m->cantidad].direccion = dir;
    memcpy(m->palabras[m->cantidad].datos, entrada, TAM_PALABRA);
    subir(m->palabras, m->cantidad);
    m->cantidad++;
    return true;
}

bool memoria_liberar_palabra(Memoria *m, Direccion dir)
{
    size_t i = buscar(m, alinear(dir));
    if (i == NO_ENCONTRADO)
        return false;

    /* Se reemplaza por la ultima y se reacomoda hacia arriba o hacia abajo. */
    m->cantidad--;
    if (i != m->cantidad) {
        m->palabras[i] = m->palabras[m->cantidad];
        subir(m->palabras, i);
        bajar(m->palabras, m->cantidad, i);
    }
    return true;
}

void memoria_compactar(Memoria *m)
{
    /* 1) Heapsort en el lugar: se saca el minimo y se pone al final.
          El arreglo queda ordenado de mayor a menor direccion. */
    for (size_t fin = m->cantidad; fin > 1; fin--) {
        intercambiar(&m->palabras[0], &m->palabras[fin - 1]);
        bajar(m->palabras, fin - 1, 0);
    }

    /* 2) Se invierte para tener orden creciente. */
    for (size_t i = 0, j = m->cantidad; i + 1 < j; i++, j--)
        intercambiar(&m->palabras[i], &m->palabras[j - 1]);

    /* 3) Se reasignan direcciones contiguas. Un arreglo ordenado de menor
          a mayor ya cumple la propiedad de min-heap: no hay que reacomodar. */
    for (size_t i = 0; i < m->cantidad; i++)
        m->palabras[i].direccion = (Direccion)(i * TAM_PALABRA);
}

void memoria_imprimir(const Memoria *m)
{
    printf("Memoria: %zu palabras de %d bytes\n", m->cantidad, TAM_PALABRA);
    for (size_t i = 0; i < m->cantidad; i++) {
        printf("  [%zu] dir %6u:", i, m->palabras[i].direccion);
        for (int b = 0; b < TAM_PALABRA; b++)
            printf(" %02x", m->palabras[i].datos[b]);
        printf("\n");
    }
}
