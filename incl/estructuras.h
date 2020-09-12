//Directivas de Preprocesamiento
#include <stdint.h>
#include <pthread.h>
#ifndef ESTRUCTURAS_H
#define ESTRUCTURAS_H


// Estructura de imagen JPEG
typedef struct {
    uint8_t *data;   // arreglo de píxeles
    uint32_t width;
    uint32_t height;
    uint32_t ch;     // canal del color:  3 para RGB
                    //                    1 para escala de grises
} JpegData;

typedef struct {
int *buf;
int tamano;
int cantHebrasConsumidoras;
int head, tail;
int full, empty;
int produciendo, consumiendo;
pthread_mutex_t mutex;
pthread_cond_t notFull, notEmpty;
} buffer_t;

#endif