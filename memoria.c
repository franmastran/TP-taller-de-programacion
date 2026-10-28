#include "memoria.h"
#include "cache.h"

/* Las reservas guardan la identidad, direccion y longitud. El AVL guarda palabras. */
typedef struct Reserva {
    int id;
    int inicio;
    int bytes;
    struct Reserva *sig;
} Reserva;

Nodo *memoria_raiz = NULL;
Reserva *memoria_reservas = NULL;
int memoria_siguiente_id = 0;
int memoria_final = 0;

int memoria_altura(Nodo *n) {
    if (n == NULL) return 0;
    int izq = memoria_altura(n->subIzq);
    int der = memoria_altura(n->subDer);
    return 1 + (izq > der ? izq : der);
}

int memoria_balance(Nodo *n) {
    if (n == NULL) return 0;
    return memoria_altura(n->subIzq) - memoria_altura(n->subDer);
}

Nodo *memoria_rotar_derecha(Nodo *n) {
    Nodo *x = n->subIzq;
    Nodo *aux = x->subDer;
    x->subDer = n;
    n->subIzq = aux;
    return x;
}

Nodo *memoria_rotar_izquierda(Nodo *n) {
    Nodo *x = n->subDer;
    Nodo *aux = x->subIzq;
    x->subIzq = n;
    n->subDer = aux;
    return x;
}

Nodo *memoria_equilibrar(Nodo *n) {
    if (n == NULL) return NULL;
    int b = memoria_balance(n);
    if (b > 1) {
        if (memoria_balance(n->subIzq) < 0)
            n->subIzq = memoria_rotar_izquierda(n->subIzq);
        return memoria_rotar_derecha(n);
    }
    if (b < -1) {
        if (memoria_balance(n->subDer) > 0)
            n->subDer = memoria_rotar_derecha(n->subDer);
        return memoria_rotar_izquierda(n);
    }
    return n;
}

Nodo *memoria_insertar_nodo(Nodo *raiz, int direccion, int valor, int *ok) {
    if (raiz == NULL) {
        Nodo *nuevo = malloc(sizeof(Nodo));
        if (nuevo == NULL) { *ok = 0; return NULL; }
        nuevo->dato = valor;
        nuevo->id = direccion;
        nuevo->subIzq = NULL;
        nuevo->subDer = NULL;
        return nuevo;
    }
    if (direccion < raiz->id)
        raiz->subIzq = memoria_insertar_nodo(raiz->subIzq, direccion, valor, ok);
    else if (direccion > raiz->id)
        raiz->subDer = memoria_insertar_nodo(raiz->subDer, direccion, valor, ok);
    else raiz->dato = valor;
    return memoria_equilibrar(raiz);
}

Nodo *memoria_buscar_nodo(Nodo *raiz, int direccion) {
    while (raiz != NULL) {
        if (direccion == raiz->id) return raiz;
        raiz = direccion < raiz->id ? raiz->subIzq : raiz->subDer;
    }
    return NULL;
}

Nodo *memoria_minimo(Nodo *raiz) {
    while (raiz != NULL && raiz->subIzq != NULL) raiz = raiz->subIzq;
    return raiz;
}

Nodo *memoria_eliminar_nodo(Nodo *raiz, int direccion) {
    if (raiz == NULL) return NULL;
    if (direccion < raiz->id)
        raiz->subIzq = memoria_eliminar_nodo(raiz->subIzq, direccion);
    else if (direccion > raiz->id)
        raiz->subDer = memoria_eliminar_nodo(raiz->subDer, direccion);
    else {
        if (raiz->subIzq == NULL || raiz->subDer == NULL) {
            Nodo *hijo = raiz->subIzq != NULL ? raiz->subIzq : raiz->subDer;
            free(raiz);
            return hijo;
        }
        Nodo *sucesor = memoria_minimo(raiz->subDer);
        raiz->id = sucesor->id;
        raiz->dato = sucesor->dato;
        raiz->subDer = memoria_eliminar_nodo(raiz->subDer, sucesor->id);
    }
    return memoria_equilibrar(raiz);
}

void memoria_liberar_arbol(Nodo *n) {
    if (n == NULL) return;
    memoria_liberar_arbol(n->subIzq);
    memoria_liberar_arbol(n->subDer);
    free(n);
}

Reserva *memoria_buscar_reserva(int id) {
    Reserva *r = memoria_reservas;
    while (r != NULL && r->id != id) r = r->sig;
    return r;
}

int memoria_alinear(int bytes) {
    int t = MEMORIA_TAM_PALABRA;
    return bytes + (t - bytes % t) % t;
}

/* Se admite una operacion dentro de una unica reserva, sin cruzar huecos. */
int memoria_rango_valido(int direccion, int n) {
    if (direccion < 0 || n < 0) return 0;
    Reserva *r = memoria_reservas;
    while (r != NULL) {
        if (direccion >= r->inicio && direccion - r->inicio <= r->bytes &&
            n <= r->bytes - (direccion - r->inicio)) return 1;
        r = r->sig;
    }
    return 0;
}

/* Obtiene un byte de una palabra, sin depender del orden de bytes de la CPU. */
unsigned int memoria_byte(Nodo *n, int offset) {
    return ((unsigned int)n->dato >> (8 * offset)) & 255u;
}

void memoria_poner_byte(Nodo *n, int offset, unsigned int valor) {
    unsigned int desplazamiento = (unsigned int)(8 * offset);
    unsigned int mascara = 255u << desplazamiento;
    unsigned int palabra = (unsigned int)n->dato;
    n->dato = (int)((palabra & ~mascara) | ((valor & 255u) << desplazamiento));
}

void memoria_destruir(void) {
    memoria_liberar_arbol(memoria_raiz);
    memoria_raiz = NULL;
    while (memoria_reservas != NULL) {
        Reserva *siguiente = memoria_reservas->sig;
        free(memoria_reservas);
        memoria_reservas = siguiente;
    }
    memoria_siguiente_id = 0;
    memoria_final = 0;
    cache_vaciar();
}

void memoria_iniciar(void) {
    memoria_destruir();
}
int memoria_reservar(int bytes) {
    if (MEMORIA_TAM_PALABRA < 1 || MEMORIA_TAM_PALABRA > (int)sizeof(int) ||
        bytes <= 0) return -1;
    int ocupacion = memoria_alinear(bytes);
    /* No hay capacidad fija: solo evitamos desbordar las direcciones int. */
    if ((long long)memoria_final + ocupacion > 2147483647LL) return -1;
    Reserva *r = malloc(sizeof(Reserva));
    if (r == NULL) return -1;
    int comienzo = memoria_final;
    int ok = 1;
    for (int d = comienzo; d < comienzo + ocupacion; d += MEMORIA_TAM_PALABRA) {
        memoria_raiz = memoria_insertar_nodo(memoria_raiz, d, 0, &ok);
        if (!ok) {
            for (int x = comienzo; x < d; x += MEMORIA_TAM_PALABRA)
                memoria_raiz = memoria_eliminar_nodo(memoria_raiz, x);
            free(r);
            return -1;
        }
    }
    r->id = memoria_siguiente_id++;
    r->inicio = comienzo;
    r->bytes = bytes;
    r->sig = NULL;
    if (memoria_reservas == NULL) memoria_reservas = r;
    else {
        Reserva *p = memoria_reservas;
        while (p->sig != NULL) p = p->sig;
        p->sig = r;
    }
    memoria_final += ocupacion;
    return r->id;
}

int memoria_liberar(int id) {
    Reserva **p = &memoria_reservas;
    while (*p != NULL && (*p)->id != id) p = &(*p)->sig;
    if (*p == NULL) return -1;
    Reserva *r = *p;
    int ocupacion = memoria_alinear(r->bytes);
    for (int d = r->inicio; d < r->inicio + ocupacion; d += MEMORIA_TAM_PALABRA)
        memoria_raiz = memoria_eliminar_nodo(memoria_raiz, d);
    *p = r->sig;
    free(r);
    cache_vaciar();
    return 0;
}

int memoria_direccion(int id) {
    Reserva *r = memoria_buscar_reserva(id);
    return r == NULL ? -1 : r->inicio;
}

int memoria_leer(int direccion, char *destino, int n) {
    if (destino == NULL || !memoria_rango_valido(direccion, n)) return -1;
    for (int i = 0; i < n; i++) {
        int d = direccion + i;
        int inicio = d - d % MEMORIA_TAM_PALABRA;
        Nodo *nodo = memoria_buscar_nodo(memoria_raiz, inicio);
        if (nodo == NULL) return -1;
        destino[i] = (char)memoria_byte(nodo, d - inicio);
    }
    return 0;
}

int memoria_escribir(int direccion, char *origen, int n) {
    if (origen == NULL || !memoria_rango_valido(direccion, n)) return -1;
    for (int i = 0; i < n; i++) {
        int d = direccion + i;
        int inicio = d - d % MEMORIA_TAM_PALABRA;
        Nodo *nodo = memoria_buscar_nodo(memoria_raiz, inicio);
        if (nodo == NULL) return -1;
        memoria_poner_byte(nodo, d - inicio, (unsigned char)origen[i]);
    }
    /* Escrituras externas pueden invalidar copias residentes. */
    cache_vaciar();
    return 0;
}

void memoria_compactar(void) {
    /* No se usa memoria_escribir: compactar debe conservar datos sin invalidar
       repetidamente la cache. Las reservas se recorren por direccion. */
    int destino = 0;
    Reserva *r = memoria_reservas;
    while (r != NULL) {
        int nuevo_inicio = destino;
        if (r->inicio != nuevo_inicio) {
            for (int i = 0; i < r->bytes; i++) {
                int anterior = r->inicio + i;
                Nodo *viejo = memoria_buscar_nodo(memoria_raiz,
                    anterior - anterior % MEMORIA_TAM_PALABRA);
                unsigned int valor = memoria_byte(viejo, anterior % MEMORIA_TAM_PALABRA);
                int actual = nuevo_inicio + i;
                int palabra = actual - actual % MEMORIA_TAM_PALABRA;
                Nodo *nuevo = memoria_buscar_nodo(memoria_raiz, palabra);
                if (nuevo == NULL) {
                    int ok = 1;
                    memoria_raiz = memoria_insertar_nodo(memoria_raiz, palabra, 0, &ok);
                    /* El destino ocupa un hueco existente o una direccion previa.
                       Si falla la reserva de un nodo, no podemos continuar. */
                    if (!ok) return;
                    nuevo = memoria_buscar_nodo(memoria_raiz, palabra);
                }
                memoria_poner_byte(nuevo, actual % MEMORIA_TAM_PALABRA, valor);
            }
            int ocupado = memoria_alinear(r->bytes);
            for (int d = r->inicio; d < r->inicio + ocupado; d += MEMORIA_TAM_PALABRA) {
                if (d >= nuevo_inicio + ocupado)
                    memoria_raiz = memoria_eliminar_nodo(memoria_raiz, d);
            }
            r->inicio = nuevo_inicio;
        }
        destino += memoria_alinear(r->bytes);
        r = r->sig;
    }
    memoria_final = destino;
    cache_vaciar();
}

void memoria_imprimir_nodos(Nodo *n) {
    if (n == NULL) return;
    memoria_imprimir_nodos(n->subIzq);
    printf("  palabra %d: 0x%08x\n", n->id, (unsigned int)n->dato);
    memoria_imprimir_nodos(n->subDer);
}

void memoria_imprimir(void) {
    printf("Memoria dinamica (palabra=%d bytes):\n", MEMORIA_TAM_PALABRA);
    for (Reserva *r = memoria_reservas; r != NULL; r = r->sig)
        printf("  reserva id=%d inicio=%d bytes=%d\n", r->id, r->inicio, r->bytes);
    memoria_imprimir_nodos(memoria_raiz);
}

int memoria_escribir_simulador(int direccion, char *origen, int n) {
    if (origen == NULL || !memoria_rango_valido(direccion, n))
        return -1;

    for (int i = 0; i < n; i++) {
        int d = direccion + i;
        int inicio = d - d % MEMORIA_TAM_PALABRA;

        Nodo *nodo = memoria_buscar_nodo(memoria_raiz, inicio);

        if (nodo == NULL)
            return -1;

        memoria_poner_byte(nodo, d - inicio, (unsigned char)origen[i]);
    }

    return 0;
}