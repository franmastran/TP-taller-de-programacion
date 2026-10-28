#include "cache.h"

Cache *heap = NULL;
int heap_cantidad = 0;
int heap_capacidad = 0;
int heap_tam_bloque = 0;
int heap_tiempo = 0;
int heap_hits = 0;
int heap_misses = 0;

int cache_iniciar(int capacidad, int tam_bloque) {
    if (capacidad <= 0 || tam_bloque <= 0 || heap != NULL) return -1;
    heap = calloc(capacidad, sizeof(Cache));
    if (heap == NULL) return -1;
    heap_capacidad = capacidad;
    heap_tam_bloque = tam_bloque;
    heap_cantidad = 0;
    heap_tiempo = 0;
    heap_hits = 0;
    heap_misses = 0;
    return 0;
}   
void cache_intercambiar(int i, int j) {
    Cache aux = heap[i];
    heap[i] = heap[j];
    heap[j] = aux;
}

int cache_rango_valido(int direccion, int offset, const char *datos, int n) {
    return heap != NULL && datos != NULL && heap_tam_bloque > 0 && direccion >= 0 &&
           direccion % heap_tam_bloque == 0 && offset >= 0 &&
           n >= 0 && offset <= heap_tam_bloque &&
           n <= heap_tam_bloque - offset;
}
void cache_subir(int i) {
    while (i > 0) {
        int padre = (i - 1) / 2;
        if (!cache_menor(i, padre)) 
            break;
        cache_intercambiar(i, padre);
        i = padre;
    }
}

void cache_bajar(int i) {
    while (i < heap_cantidad) {
        int menor = i;
        int izq = 2 * i + 1;
        int der = 2 * i + 2;
        if (izq < heap_cantidad && cache_menor(izq, menor)) 
            menor = izq;
        if (der < heap_cantidad && cache_menor(der, menor)) 
            menor = der;
        if (menor == i) 
            break;
        cache_intercambiar(i, menor);
        i = menor;
    }
}

int cache_buscar(int direccion) {
    for (int i = 0; i < heap_cantidad; i++) {
        if (heap[i].direccion == direccion) return i;
    }
    return -1;
}


int cache_menor(int i, int j) {
    if (heap[i].frecuencia != heap[j].frecuencia)
        return heap[i].frecuencia < heap[j].frecuencia;
    return heap[i].ultimo < heap[j].ultimo;
}




void cache_vaciar() {
    for (int i = 0; i < heap_cantidad; i++) {
        free(heap[i].dato);
        heap[i].dato = NULL;
    }
    heap_cantidad = 0;
    heap_tiempo = 0;
}

void cache_destruir() {
    cache_vaciar();
    free(heap);
    heap = NULL;
    heap_capacidad = 0;
    heap_tam_bloque = 0;
    heap_hits = 0;
    heap_misses = 0;
}


void cache_imprimir() {
    for (int i = 0; i < heap_cantidad; i++) {
        printf("heap[%d]: direccion=%d, frecuencia=%d, ultimo=%d\n",
               i, heap[i].direccion, heap[i].frecuencia, heap[i].ultimo);
    }
}

void cache_marcar_usado(int i) { //un hit aumenta frecuencia y baja el bloque
    heap[i].frecuencia++;
    heap[i].ultimo = ++heap_tiempo;
    cache_bajar(i);
}

int cache_leer(int direccion, int offset, char *destino, int n) {
    if (!cache_rango_valido(direccion, offset, destino, n)) return -1;
    int i = cache_buscar(direccion);
    if (i == -1) {
        heap_misses++;
        return 0;
    }
    for (int j = 0; j < n; j++) {
        destino[j] = heap[i].dato[offset + j];
    }
    heap_hits++;
    cache_marcar_usado(i);
    return 1;
}

int cache_escribir(int direccion, int offset, char *origen, int n) {
    if (!cache_rango_valido(direccion, offset, origen, n)) return -1;
    int i = cache_buscar(direccion);
    if (i == -1) {
        heap_misses++;
        return 0;
    }
    for (int j = 0; j < n; j++) {
        heap[i].dato[offset + j] = origen[j];
    }
    heap_hits++;
    cache_marcar_usado(i);
    /* Write-through: el llamador tambien actualiza memoria principal. */
    return 1;
}

int cache_insertar(int direccion, char *bloque) {
    if (!cache_rango_valido(direccion, 0, bloque, heap_tam_bloque)) return -1;
    int i = cache_buscar(direccion);
    if (i != -1) {
        for (int j = 0; j < heap_tam_bloque; j++) {
            heap[i].dato[j] = bloque[j];
        }
        cache_marcar_usado(i);
        return 0;
    }

    char *copia = malloc(heap_tam_bloque);
    if (copia == NULL) return -1;
    for (int j = 0; j < heap_tam_bloque; j++) {
        copia[j] = bloque[j];
    }

    if (heap_cantidad < heap_capacidad) {
        i = heap_cantidad++;
        heap[i].dato = copia;
        heap[i].direccion = direccion;
        heap[i].frecuencia = 1;
        heap[i].ultimo = ++heap_tiempo;
        cache_subir(i);
    } else {
        /* La raiz es el bloque con menor frecuencia. */
        free(heap[0].dato);
        heap[0].dato = copia;
        heap[0].direccion = direccion;
        heap[0].frecuencia = 1;
        heap[0].ultimo = ++heap_tiempo;
        cache_bajar(0);
    }
    return 0;
}

void cache_reiniciar_estadisticas(void) {
    heap_hits = 0;
    heap_misses = 0;
}