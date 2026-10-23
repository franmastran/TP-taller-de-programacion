#ifndef MEMORIA_H
#define MEMORIA_H

typedef struct Nodo {
    int dato;
    struct Nodo *subIzq;
    struct Nodo *subDer;
} Nodo;

/* ---------- Operaciones ---------- */
 
void memoria_iniciar();
void memoria_destruir();              /* libera los nodos del arbol */
 
/* Reserva 'bytes' al final de lo ocupado. Devuelve el id (>= 0) o -1 si no hay lugar. */
int  memoria_reservar();
 
/* Borra el dato del arbol (deja un hueco). Devuelve 0 si ok, -1 si el id no existe. */
int  memoria_liberar();
 
/* Direccion actual en bytes del dato con ese id (cambia al compactar). -1 si no existe. */
int  memoria_direccion();
 
/* Lee/escribe n bytes desde una direccion en bytes. */
void memoria_leer();
void memoria_escribir();
 
/* Recorre el arbol en inorder, mueve cada dato al primer lugar libre del
 * principio y actualiza su 'inicio'. */
void memoria_compactar();
 
void memoria_imprimir();        /* opcional, para depurar */
 

#endif
