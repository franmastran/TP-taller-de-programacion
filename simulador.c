#include "simulador.h"
#include "cache.h"
#include "memoria.h"
#include <stdio.h>
#include <stdlib.h>

/* La cache existente mantiene sus estadisticas en estas variables. */
extern int heap_hits;
extern int heap_misses;
extern int heap_tam_bloque;

int simulador_iniciar(int capacidad_cache, int tam_bloque) {
    if (tam_bloque <= 0 || tam_bloque % MEMORIA_TAM_PALABRA != 0) return -1;
    if (cache_iniciar(capacidad_cache, tam_bloque) != 0) return -1;
    memoria_iniciar();
    return 0;
}

void simulador_destruir(void) {
    memoria_destruir();
    cache_destruir();
}

int simulador_reservar(int bytes) {
    return memoria_reservar(bytes);
}

int simulador_liberar(int id) {
    return memoria_liberar(id);
}

int simulador_direccion(int id) {
    return memoria_direccion(id);
}

/* Arma un bloque completo; las posiciones no reservadas se rellenan con cero. */
int simulador_cargar_bloque(int direccion) {
    char *bloque = malloc(heap_tam_bloque);
    if (bloque == NULL) return -1;
    for (int i = 0; i < heap_tam_bloque; i++) {
        if (memoria_leer(direccion + i, &bloque[i], 1) != 0)
            bloque[i] = 0;
    }
    int resultado = cache_insertar(direccion, bloque);
    free(bloque);
    return resultado;
}

/* Se comprueba el rango completo antes de producir efectos parciales. */
int simulador_validar(int direccion, int n) {
    if (heap_tam_bloque <= 0 || direccion < 0 || n <= 0) return -1;
    char *aux = malloc(n);
    if (aux == NULL) return -1;
    int resultado = memoria_leer(direccion, aux, n);
    free(aux);
    return resultado;
}

int simulador_leer(int direccion, char *destino, int n) {
    if (destino == NULL || simulador_validar(direccion, n) != 0) return -1;
    int procesados = 0;
    while (procesados < n) {
        int actual = direccion + procesados;
        int base = actual - actual % heap_tam_bloque;
        int offset = actual - base;
        int cantidad = heap_tam_bloque - offset;
        if (cantidad > n - procesados) cantidad = n - procesados;
        int resultado = cache_leer(base, offset, destino + procesados, cantidad);
        if (resultado < 0) return -1;
        if (resultado == 0) {
            if (simulador_cargar_bloque(base) != 0) return -1;
            /* Copia sin registrar un segundo acceso: el acceso ya fue un miss. */
            extern Cache *heap;
            int indice = cache_buscar(base);
            if (indice < 0) return -1;
            for (int i = 0; i < cantidad; i++)
                destino[procesados + i] = heap[indice].dato[offset + i];
        }
        procesados += cantidad;
    }
    return 0;
}

int simulador_escribir(int direccion, char *origen, int n) {
    if (origen == NULL || simulador_validar(direccion, n) != 0) return -1;
    int procesados = 0;
    while (procesados < n) {
        int actual = direccion + procesados;
        int base = actual - actual % heap_tam_bloque;
        int offset = actual - base;
        int cantidad = heap_tam_bloque - offset;
        if (cantidad > n - procesados) cantidad = n - procesados;
        int resultado = cache_escribir(base, offset, (char *)origen + procesados, cantidad);
        if (resultado < 0) return -1;
        /* Write-through: memoria principal siempre se actualiza. */
        if (memoria_escribir_simulador(actual, origen + procesados, cantidad) != 0) return -1;
        if (resultado == 0 && simulador_cargar_bloque(base) != 0) return -1;
        procesados += cantidad;
    }
    return 0;
}

void simulador_compactar(void) {
    memoria_compactar(); /* memoria_compactar ya vacia la cache. */
}

void simulador_reiniciar_prueba(void) {
    cache_vaciar();
    cache_reiniciar_estadisticas();
}

void simulador_estadisticas(void) {
    int total = heap_hits + heap_misses;
    double tasa = total == 0 ? 0.0 : 100.0 * heap_hits / total;
    printf("Hits: %d | Misses: %d | Efectividad: %.2f%%\n", heap_hits, heap_misses, tasa);
}

void simulador_imprimir(void) {
    memoria_imprimir();
    printf("Cache LFU:\n");
    cache_imprimir();
    simulador_estadisticas();
}
