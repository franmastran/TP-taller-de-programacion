#ifndef MEMORIA_H
#define MEMORIA_H

#include <stdio.h>
#include <stdlib.h>

#ifndef MEMORIA_TAM_PALABRA
#define MEMORIA_TAM_PALABRA 4
#endif

typedef struct Nodo {
    int dato;             /* bytes de una palabra empaquetados en un int */
    int id;               /* direccion inicial de esa palabra, en bytes */
    struct Nodo *subIzq;
    struct Nodo *subDer;
} Nodo;



void memoria_iniciar(void);
void memoria_destruir(void);
int memoria_reservar(int bytes);        /* ID estable, o -1 */
int memoria_liberar(int id);           /* 0 correcto, -1 error */
int memoria_direccion(int id);         /* direccion actual, o -1 */
int memoria_leer(int direccion, char *destino, int n);  /* 0 correcto, -1 error */
int memoria_escribir(int direccion, char *origen, int n);
void memoria_compactar(void);
void memoria_imprimir(void);
int memoria_escribir_simulador(int direccion, char *origen, int n);
#endif
