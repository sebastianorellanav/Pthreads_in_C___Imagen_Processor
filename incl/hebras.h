//Directivas de Preprocesamiento
#include "estructuras.h"
#ifndef HEBRAS_H
#define HEBRAS_H

//Cabeceras de Funciones
void put_in_buffer(buffer_t **buffer, int numFila);
int take_from_buffer(buffer_t **buffer);
void *pipeline(void *arg);
void buffer_init(buffer_t **buffer, int tamano);

#endif