//Directivas de Preprocesamiento
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <jpeglib.h>
#include <inttypes.h>
#include "../incl/lecturaImagenes.h"
#include <semaphore.h>

extern JpegData jpegData;
extern JpegData jpegDataBN;
extern JpegData jpegDataFiltrada;
extern int numImagen;
extern int cantHebrasConsumidoras;
extern int ordenHebras;
extern sem_t semaforo1;
extern sem_t semaforo2;


//Entradas:     - Una imagen del tipo JpegData.
//Funcionamiento: crea un espacio de memoria para la imagen
//Salidas:      
void alloc_jpeg(JpegData *jpegData){
    jpegData->data = NULL;
    jpegData->data = (uint8_t*) malloc(sizeof(uint8_t)  *
                                       jpegData->width  *
                                       jpegData->height *
                                       jpegData->ch);
}

//Entradas:     - Imagen del tipo JpegData.
//Funcionamiento: libera la memoria asignada para la imagen
//Salidas:    
void liberarJpeg(JpegData *jpegData){
    if (jpegData->data != NULL) {
        free(jpegData->data);
        jpegData->data = NULL;
    }
}

// read JPEG image
// 1. create JPEG decompression object
// 2. specify source data
// 3. read JPEG header
// 4. set parameters
// 5. start decompression
// 6. scan lines
// 7. finish decompression

//Entradas:     - Imagen del tipo JpegData llamada jpegData
//              - Nombre de archivo de entrada del tipo char llamado nombreArchivo
//              - Puntero a estructura de libreria jpeglib llamado jerr
//Funcionamiento: Se lee una imagen del tipo JPEG, para lo cual se 
//                descomprime la imagen.
//Salidas:      - Entero que indica la correcta lectura del archivo (valor 1).
//              - Imagen del tipo JpegData   
int leerJpeg(JpegData *jpegData,
              const char *nombreArchivo,
              struct jpeg_error_mgr *jerr, void *arg)
{
    printf("entra a int leerJpeg()\n");
    // 1. Creación del objeto de descompresión JPEG
    struct jpeg_decompress_struct cinfo;
    jpeg_create_decompress(&cinfo);
    cinfo.err = jpeg_std_error(jerr);

    FILE *fp = fopen(nombreArchivo, "rb");
    if (fp == NULL) {
        printf("Error: failed to open %s\n", nombreArchivo);
        return 0;
    }
    // 2. Especificando los datos de origen.
    jpeg_stdio_src(&cinfo, fp);

    // 3.
    jpeg_read_header(&cinfo, 1);

    // 4. Inicio de descompresión.
    jpeg_start_decompress(&cinfo);

    jpegData->width  = cinfo.image_width;
    jpegData->height = cinfo.image_height;
    jpegData->ch     = cinfo.num_components;

    alloc_jpeg(jpegData);

    //Se desbloquea la hebra main
    sem_post(&semaforo1);
    // 5. Lectura linea a linea.
    //*****************************************************************************************************
    //Algoritmo del Productor
    //*****************************************************************************************************
    uint8_t *row = jpegData->data;   //Representa una fila de la imagen
    const uint32_t stride = jpegData->width * jpegData->ch;    //el ancho de la imagen
    buffer_t *buffer;                        
    buffer = (buffer_t *) arg;      //Se castea el buffer a buffer_t
    buffer->empty = 0;              //El buffer esta vacio
    int numFila = 0;
    printf("PRODUCTORA: va a comenzar la produccion\n");
    for (int y = 0; y < jpegData->height; y++) {
        printf("se esta produciendo la fila: %d\n",y);
        jpeg_read_scanlines(&cinfo, &row, 1);
        numFila = y;
        row += stride;
        pthread_mutex_lock (&buffer->mutex);
        if(buffer->full){
            buffer->produciendo = 0;
            buffer->consumiendo = 1;
        }
        while (buffer->full || buffer->consumiendo) {
            pthread_cond_broadcast(&buffer->notEmpty);
            pthread_cond_wait (&buffer->notFull, &buffer->mutex);
        }
        put_in_buffer(&buffer, numFila);
        pthread_mutex_unlock(&buffer->mutex);
        printf("PRODUCTORA: estamos en el for de leer la imagen en el ciclo: %d\n", y);
    }
    buffer->produciendo = 0;
    buffer->consumiendo = 1;
    pthread_cond_broadcast(&buffer->notEmpty);
    
    //*****************************************************************************************************
    //*****************************************************************************************************


    // 6. Finalización de la descompresión.
    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    fclose(fp);

    return 1;
}


//Entradas:     - Nombre de archivo de entrada del tipo char llamado nombreEntrada
//Funcionamiento: Se lee una imagen del tipo JPEG. Esta función es llamada  por el main.
//Salidas:       - Un JpegData que representa a la imagen de salida.
void *leerImagenes(void *buffer){
    printf("PRODUCTORA: entra a void *LeerImagenes()\n");
    struct jpeg_error_mgr jerr;
    char filename[30];
    sprintf(filename,"./imagen_%i.jpg", numImagen);     //numImagen variable global
    
    if (!leerJpeg(&jpegData, filename, &jerr, buffer)){    //jpegData es variable global
        liberarJpeg(&jpegData);
        exit(-1);
    }
    //Reservar memoria para la imagen en blanco y negro que se utilizara mas adelante
    jpegDataBN.width = jpegData.width;
	jpegDataBN.height = jpegData.height;
	jpegDataBN.ch = 1;            //canal = 1. Representa una imagen en escala de grises.
	alloc_jpeg(&jpegDataBN);

    //Reservar memoria para la imagen filtrada que se utilizara mas adelante
    jpegDataFiltrada.width = jpegData.width;
	jpegDataFiltrada.height = jpegData.height;
	jpegDataFiltrada.ch = 1;            //canal = 1. Representa una imagen en escala de grises.
	alloc_jpeg(&jpegDataFiltrada);
}








