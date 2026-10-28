#ifndef SIMULADOR_H
#define SIMULADOR_H

/* La cache almacena bloques de tam_bloque bytes, multiplo del tamano de palabra. */
int simulador_iniciar(int capacidad_cache, int tam_bloque);
void simulador_destruir(void);
int simulador_reservar(int bytes);
int simulador_liberar(int id);
int simulador_direccion(int id);
int simulador_leer(int direccion, char *destino, int n);
int simulador_escribir(int direccion, char *origen, int n);
void simulador_compactar(void);
void simulador_reiniciar_prueba(void);
void simulador_estadisticas(void);
void simulador_imprimir(void);

#endif
