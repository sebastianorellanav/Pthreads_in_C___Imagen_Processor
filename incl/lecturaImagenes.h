//Directivas de Preprocesamiento
#ifndef EXAMPLE_H
#define EXAMPLE_H
#include "estructuras.h"
#include <pthread.h>

//Cabeceras de Funciones
void alloc_jpeg(JpegData *jpegData);
void liberarJpeg(JpegData *jpegData);
void *leerImagenes(void *params);
int leerJpeg(JpegData *jpegData,
              const char *srcfile,
              struct jpeg_error_mgr *jerr, void *arg);
void put_in_buffer(buffer_t **buffer, int numFila);
void *pipeline(void *arg);

#endif