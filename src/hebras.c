#include <stdio.h>
#include <stdlib.h>
#include "../incl/conversion.h"
#include "../incl/binarizacion.h"
#include "../incl/hebras.h"
#include <pthread_barrier.h>

extern JpegData jpegData;
extern int numImagen;
extern int cantHebrasConsumidoras;
extern int ordenHebras;

void buffer_init(buffer_t **buffer, int tamano)
{
	*buffer = (buffer_t *)malloc(sizeof(buffer_t));
	(*buffer)->buf = (int *)malloc(sizeof(int)*tamano);
	(*buffer)->tamano = tamano;
	for (int i = 0; i < tamano; i++)
	{
		(*buffer)->buf[i] = -1;
	}
	(*buffer)->empty = 1;
	
	//aqui van las otras cosas que no sabemos como inicializarlas
	//...
}

void put_in_buffer(buffer_t **buffer, int numFila)
{
    printf("PRODUCTORA: entra a put_in_buffer y el numFila es %d\n",numFila);
    int vacios = 0;
    for (int i = 0; i < (*buffer)->tamano; i++)
    {
        if((*buffer)->buf[i] == -1){  //posicion vacia entonces se puede agregar
            (*buffer)->buf[i] = numFila; //putin buffer
            i = (*buffer)->tamano;
            (*buffer)->empty = 0;
        }
    }
    for (int i = 0; i < (*buffer)->tamano; i++)
    {
        if((*buffer)->buf[i] == -1){  //posicion vacia entonces se puede agregar
           vacios++;
        }
    }
    if(vacios == 0){
        (*buffer)->full = 1; //buffer esta lleno
        (*buffer)->empty = 0; //vacio = false
        printf("PRODUCTORA: se llena el buffer\n");
    }
    
}

int take_from_buffer(buffer_t **buffer)
{
    printf("CONSUMIDORA: entra a take_from_buffer\n");
    int retorno = -1;
    int lleno = 0;
    for (int i = 0; i < (*buffer)->tamano; i++)
    {
        if((*buffer)->buf[i] != -1){
            retorno = (*buffer)->buf[i];
            (*buffer)->buf[i] = -1;
            i = (*buffer)->tamano;
            (*buffer)->full = 0;  //lleno es falso
        }
    }

    for (int i = 0; i < (*buffer)->tamano; i++)
    {
        if((*buffer)->buf[i] != -1){
            lleno++;
        }
    }

    if(lleno == 0){
        (*buffer)->empty = 1; //esta vacio
        (*buffer)->full = 0;  //lleno es falso
    }
    printf("CONSUMIDORA: sale de take_from_buffer y el retorno es %d\n",retorno);
    return retorno;
}

void *pipeline(void *arg)
{
    pthread_barrier_t rendezvous;
    pthread_barrier_init(&rendezvous, NULL,cantHebrasConsumidoras);

    printf("CONSUMIDORA X: entra una hebra X a consumir\n");
    ordenHebras++;
    int numFila, i=0, ultimaHebra=0;
    buffer_t *buffer;
    buffer = (buffer_t *) arg;
    int filasARecoger = jpegData.height/cantHebrasConsumidoras;
    if(ordenHebras == cantHebrasConsumidoras){
        if(jpegData.height%cantHebrasConsumidoras != 0){
            filasARecoger++;
        }
    }
    int *filasHebra = (int *)malloc(sizeof(int)*filasARecoger);
    int largoFilasHebras = filasARecoger; //largo del arreglo filasHebra
    while( filasARecoger > 0) {
        printf("CONSUMIDORA X: entra al while de filasARecoger\n");
        pthread_mutex_lock (&buffer->mutex);
        while (buffer->empty) {
            printf("CONSUMIDORA X: deberia entrar al buffer->empty\n");
            pthread_cond_wait (&buffer->notEmpty, &buffer->mutex);
        }
        numFila = take_from_buffer(&buffer);
        filasHebra[i] = numFila;
        pthread_cond_signal(&buffer->notFull);
        pthread_mutex_unlock(&buffer->mutex);
        filasARecoger--;
        printf("CONSUMIDORA X: Se esta consumiendo del buffer\n");

    }
    printf("CONSUMIDORA X: ya termine de consumir me voy chao\n");

    //barrier para que todas las hebras esperen a que las demas terminen de consumir
    pthread_barrier_wait(&rendezvous);

    jpegDataBN.width = jpegData.width;
    jpegDataBN.height = jpegData.height;
    jpegDataBN.ch = 1;            //canal = 1. Representa una imagen en escala de grises.
    alloc_jpeg(&jpegDataBN);
    //conversion
    convertirAEscalaGrises(filasHebra, largoFilasHebras);
    //esperar a todas las hebras
    pthread_barrier_wait(&rendezvous);

    //filtro
    //esperar a todas las hebras
    pthread_barrier_wait(&rendezvous);

    //binarizacion
    binarizarImagen(filasHebra, largoFilasHebras)
    //esperar a todas las hebras
    pthread_barrier_wait(&rendezvous);

    //clasificacion

    //volver a la hebra main
}