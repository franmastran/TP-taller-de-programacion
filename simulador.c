#include "simulador.h"
#include "cache.h"
#include "memoria.h"
#include <stdio.h>
#include <stdlib.h>


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
/* Lee un bloque completo de memoria principal; rellena huecos con cero. */
int simulador_cargar_bloque(int direccion, char *bloque) {
    if (bloque == NULL) return -1;
    for (int i = 0; i < cache_tamano_bloque(); i++) {
        if (memoria_leer(direccion + i, &bloque[i], 1) != 0)
            bloque[i] = 0;
    }
    return 0;
}

/* Se comprueba el rango completo antes de producir efectos parciales. */
int simulador_validar(int direccion, int n) {
    if (cache_tamano_bloque() <= 0 || direccion < 0 || n <= 0) return -1;
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
        int base = actual - actual % cache_tamano_bloque();
        int offset = actual - base;
        int cantidad = cache_tamano_bloque() - offset;
        if (cantidad > n - procesados) cantidad = n - procesados;
        int resultado = cache_leer(base, offset, destino + procesados, cantidad);
        if (resultado < 0) return -1;
        if (resultado == 0) {
            int tam_bloque = cache_tamano_bloque();
            char *bloque = malloc(tam_bloque);
            if (bloque == NULL) return -1;
            if (simulador_cargar_bloque(base, bloque) != 0 ||
                cache_insertar(base, bloque) != 0) {
                free(bloque);
                return -1;
            }
            /* Ya tenemos el bloque: no volvemos a buscarlo en la cache. */
            for (int i = 0; i < cantidad; i++) {
                destino[procesados + i] = bloque[offset + i];
            }
            free(bloque);
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
        int base = actual - actual % cache_tamano_bloque();
        int offset = actual - base;
        int cantidad = cache_tamano_bloque() - offset;
        if (cantidad > n - procesados) cantidad = n - procesados;
        int resultado = cache_escribir(base, offset, origen + procesados, cantidad);
        if (resultado < 0) return -1;
        /* Write-through: memoria principal siempre se actualiza. */
        if (memoria_escribir_simulador(actual, origen + procesados, cantidad) != 0) return -1;
        if (resultado == 0) {
            char *bloque = malloc(cache_tamano_bloque());
            if (bloque == NULL) return -1;
            if (simulador_cargar_bloque(base, bloque) != 0 ||
                cache_insertar(base, bloque) != 0) {
                free(bloque);
                return -1;
            }
            free(bloque);
        }
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
    int hits = cache_hits();
    int misses = cache_misses();
    int total = hits + misses;
    double tasa = total == 0 ? 0.0 : 100.0 * hits / total;
    printf("Hits: %d | Misses: %d | Efectividad: %.2f%%\n", hits, misses, tasa);
}

void simulador_imprimir(void) {
    memoria_imprimir();
    printf("Cache LFU:\n");
    cache_imprimir();
    simulador_estadisticas();
}
