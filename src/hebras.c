#include <stdio.h>
#include <stdlib.h>
#include "../incl/conversion.h"
#include "../incl/binarizacion.h"
#include "../incl/hebras.h"
#include "../incl/filtro.h"
#include "../incl/clasificacion.h"
#include <pthread.h>
#include <semaphore.h>

extern JpegData jpegData;
extern JpegData jpegDataBN;
extern int numImagen;
extern int cantHebrasConsumidoras;
extern int ordenHebras;
extern pthread_barrier_t rendezvous;
extern sem_t semaforo2;
extern int ordenHebras2;



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
    (*buffer)->full = 0;
    (*buffer)->produciendo = 1;
    (*buffer)->consumiendo = 0;
	
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
    else{
        (*buffer)->full = 0;
    }
    
}

int take_from_buffer(buffer_t **buffer)
{
    printf("CONSUMIDORA: entra a take_from_buffer\n");
    int retorno = -1;
    int lleno = 0;
    for (int i = 0; i < (*buffer)->tamano; i++)
    {
        if((*buffer)->buf[i] != -1){       //si hay algo en esa posicion
            retorno = (*buffer)->buf[i];   //guardar el numero
            (*buffer)->buf[i] = -1;        //la posicion se coloca como vacia
            i = (*buffer)->tamano;         //se cambia el i para salir del for
            (*buffer)->full = 0;           //lleno es falso
        }
    }

    for (int i = 0; i < (*buffer)->tamano; i++) //para cada posicion del buffer
    {
        if((*buffer)->buf[i] != -1){      //si hay algo en esa posicion
            lleno++;                      //Se aumenta el contador
        }
    }

    if(lleno == 0){ //Si el buffer esta vacio
        (*buffer)->empty = 1; //vacio = true
        (*buffer)->full = 0;  //lleno = falso
    }
    else{
        (*buffer)->empty = 0;
    }
    printf("CONSUMIDORA: sale de take_from_buffer y el retorno es %d\n",retorno);
    return retorno;
}

void *pipeline(void *arg)
{
    printf("CONSUMIDORA X: entra una hebra X a consumir\n");
    int numFila, i=0;
    buffer_t *buffer;
    buffer = (buffer_t *) arg;   //Se castea el Buffer

    int filasARecoger = jpegData.height/cantHebrasConsumidoras;  //Se obtiene la cantidad de filas a recoger por cada hebra
    
    pthread_mutex_lock (&buffer->mutex);
    //Si es la ultima hebra y si m es decimal la ultima hebra debe ejecutar una fila mas
    ordenHebras++;   //Se aumenta el contador para saber cuantas hebras han ejecutado este codigo
    printf("CONSUMIDORA: La hebra nro: %d Entroal pipeline\n", ordenHebras);
    if(ordenHebras == cantHebrasConsumidoras){
        if(jpegData.height%cantHebrasConsumidoras != 0){
            filasARecoger = filasARecoger + (jpegData.height%cantHebrasConsumidoras);
        }
    }
    pthread_mutex_unlock(&buffer->mutex);
    //printf("CONSUMIDORA: las filas que tengo que recoger son: %d\n\n\n", filasARecoger);

    //*****************************************************************************************************
    //Algoritmo del Conusmidor
    //*****************************************************************************************************
    int *filasHebra = (int *)malloc(sizeof(int)*filasARecoger);
    int largoFilasHebras = filasARecoger; //largo del arreglo filasHebra
    for (int i = 0; i < filasARecoger; i++)
    {
        printf("CONSUMIDORA X: entra al while de filasARecoger\n");
        pthread_mutex_lock (&buffer->mutex);
        if(buffer->empty){
            buffer->consumiendo = 0;
            buffer->produciendo = 1;
        }
        while(buffer->empty || buffer->produciendo) {
            printf("empty: %d ------ produciendo: %d\n",buffer->empty, buffer->produciendo);
            printf("ordenHebras2 ante de restar: %d\n",ordenHebras2);
            ordenHebras2--;
            printf("ordenHebras2 dsps de restar: %d\n",ordenHebras2);
            if(ordenHebras2 == 0) //es la ultima hebra
            {
                printf("CONSUMIDORA: soy la ultima y le mando la señal a la productora para que de desbloquee y produzca\n");
                pthread_cond_signal(&buffer->notFull);
            }
            printf("CONSUMIDORA X: Me bloqueo\n");
            pthread_cond_wait (&buffer->notEmpty, &buffer->mutex);
            printf("CONSUMIDORA X: me acaban de desbloquear\n");
            ordenHebras2++;
            printf("ordenHebras2 dsps de desbloquear: %d\n",ordenHebras2);
        }       
        numFila = take_from_buffer(&buffer);
        printf("Obtengo el: %d\n\n\n\n",numFila);
        filasHebra[i] = numFila;
        if(i == (filasARecoger-1))
            ordenHebras2--;
        pthread_mutex_unlock(&buffer->mutex);
        printf("CONSUMIDORA X: Se esta consumiendo del buffer\n");

    }
    //*****************************************************************************************************
    //*****************************************************************************************************
    printf("CONSUMIDORA X: ya termine de consumir me voy chao\n");
    pthread_barrier_wait(&rendezvous);
    //barrier para que todas las hebras esperen a que las demas terminen de consumir
    ordenHebras = 0;   //Para saber cual es la ultima hebra que ejecuta ciertos codigos
    pthread_barrier_wait(&rendezvous);
    pthread_cond_signal(&buffer->notFull);
    
    printf("Aqui ya pasan el primer barrier\n");
    //conversion
    convertirAEscalaGrises(filasHebra, largoFilasHebras);
    printf("Ya se convirtio a blanco y negro\n");
    //esperar a todas las hebras
    pthread_barrier_wait(&rendezvous);
    printf("Aqui ya pasan el segundo barrier\n");

    //filtro
    AplicarFiltro(filasHebra, largoFilasHebras);
    printf("Aqui ya aplico el filtro\n");
    //esperar a todas las hebras
    pthread_barrier_wait(&rendezvous);
    printf("Aqui ya pasan el tercer barrier\n");

    //binarizacion
    binarizarImagen(filasHebra, largoFilasHebras);
    printf("Aqui ya se binarizo la imagen\n");
    //esperar a todas las hebras
    pthread_barrier_wait(&rendezvous);
    printf("Aqui ya pasan el cuarto barrier\n");

    //clasificacion -> Seccion Critica
    printf("Aqui estan antes de la seccion critica\n");
    pthread_mutex_lock (&buffer->mutex);
    printf("CONSUMIDORA X: entra a la seccion critica\n");
    analisisDePropiedad(filasHebra, largoFilasHebras);
    printf("CONSUMIDORA X: clasifica la imagen\n");
    pthread_mutex_unlock(&buffer->mutex);
    printf("CONSUMIDORA X: sale de la seccion critica\n");

    free(filasHebra);
    printf("se libera el malloc y vuelve al main\n");
    //volver a la hebra main

}