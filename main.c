#include "simulador.h"
#include <stdio.h>

void prueba_temporal(int direccion) {
    char dato;
    printf("\n--- Alta localidad temporal ---\n");
    for (int i = 0; i < 12; i++) {
        if (simulador_leer(direccion, &dato, 1) != 0) printf("Error de lectura\n");
    }
    simulador_estadisticas();
}

void prueba_espacial(int direccion) {
    char dato;
    printf("\n--- Alta localidad espacial ---\n");
    for (int i = 0; i < 32; i++) {
        if (simulador_leer(direccion + i, &dato, 1) != 0) printf("Error de lectura\n");
    }
    simulador_estadisticas();
}

void prueba_baja_localidad(int direccion) {
    char dato;
    printf("\n--- Baja localidad ---\n");
    /* Accesos a bloques distintos, con mas bloques que capacidad de cache. */
    for (int i = 0; i < 16; i++) {
        if (simulador_leer(direccion + i * 8, &dato, 1) != 0) printf("Error de lectura\n");
    }
    simulador_estadisticas();
}

int main(void) {
    if (simulador_iniciar(4, 8) != 0) {
        printf("No se pudo iniciar el simulador\n");
        return 1;
    }
    int id = simulador_reservar(128);
    if (id < 0) {
        printf("No se pudo reservar memoria\n");
        simulador_destruir();
        return 1;
    }
    int direccion = simulador_direccion(id);
    char datos[128];
    for (int i = 0; i < 128; i++) datos[i] = (char)(i + 1);
    if (simulador_escribir(direccion, datos, 128) != 0) {
        printf("Error al escribir\n");
        simulador_destruir();
        return 1;
    }
    /* Cada escenario comienza con cache vacia y estadisticas en cero. */
    simulador_reiniciar_prueba();
    prueba_temporal(direccion);
    simulador_reiniciar_prueba();
    prueba_espacial(direccion);
    simulador_reiniciar_prueba();
    prueba_baja_localidad(direccion);
    printf("\n--- Estado final ---\n");
    simulador_imprimir();
    simulador_destruir();
    return 0;
}
