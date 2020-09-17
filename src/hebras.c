#include <stdio.h>
#include <stdlib.h>
#include "../incl/conversion.h"
#include "../incl/binarizacion.h"
#include "../incl/hebras.h"
#include "../incl/filtro.h"
#include "../incl/clasificacion.h"
#include <pthread.h>
#include <semaphore.h>

//********************************************************************************************************
//Variables Globales
extern JpegData jpegData;
extern JpegData jpegDataBN;
extern int numImagen;
extern int cantHebrasConsumidoras;
extern int ordenHebras;
extern pthread_barrier_t rendezvous;
extern sem_t semaforo2;
extern int ordenHebras2;

//ENTRADA:        - Doble puntero de tipo buffer_t, int que contiene el tamaño del buffer
//FUNCIONAMIENTO: - Reserva memoria para guardar las filas en el buffer e inicializa las variables
//                  internas del buffer 
//SALIDA:         - Void
void buffer_init(buffer_t **buffer, int tamano)
{
	*buffer = (buffer_t *)malloc(sizeof(buffer_t));      //Reservar memoria para la estructura
	(*buffer)->buf = (int *)malloc(sizeof(int)*tamano);  //Reservar memoria para el buffer
	(*buffer)->tamano = tamano;                          //Setear tamaño del buffer
	
    for (int i = 0; i < tamano; i++)  //Para cada elemento del buffer   
	{
		(*buffer)->buf[i] = -1;       //Setearlo en -1 -> Espacio vacio
	}

    //Setear otras variables del buffer
	(*buffer)->empty = 1;
    (*buffer)->full = 0;
    (*buffer)->produciendo = 1;
    (*buffer)->consumiendo = 0;

}

//ENTRADA:        - Doble puntero de tipo buffer_t, int que contiene el numero de fila
//FUNCIONAMIENTO: - Coloca una fila en el buffer
//SALIDA:         - Void
void put_in_buffer(buffer_t **buffer, int numFila)
{
    int vacios = 0;   //Variable que cuenta espacios vacios
    for (int i = 0; i < (*buffer)->tamano; i++)
    {
        if((*buffer)->buf[i] == -1){  //posicion vacia entonces se puede agregar
            (*buffer)->buf[i] = numFila; //agregar fila al buffer
            i = (*buffer)->tamano;       //Salir del ciclo
            (*buffer)->empty = 0;        //Buffer ya no esta vacio
        }
    }

    for (int i = 0; i < (*buffer)->tamano; i++)
    {
        if((*buffer)->buf[i] == -1){  //posicion vacia entonces se puede agregar
           vacios++;                  //Contar posiciones vacias
        }
    }

    if(vacios == 0){          //Si no hay posiciones vacias
        (*buffer)->full = 1;  //buffer lleno = verdadero
        (*buffer)->empty = 0; //buffer vacio = falso
    }
    else{
        (*buffer)->full = 0;  //buffer lleno = falso
    }
    
}

//ENTRADA:        - Doble puntero de tipo buffer_t
//FUNCIONAMIENTO: - Se obtiene la primera fila que se encuentre disponible en el buffer
//SALIDA:         - Int que contiene una fila obtenida del buffer
int take_from_buffer(buffer_t **buffer)
{
    int retorno = -1; //retorno en caso de no encontrar ninguna fila
    int lleno = 0;    //Variable que cuenta posiciones ocupadas
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
        (*buffer)->empty = 0; //vacio = falso
    }
    return retorno;
}

//ENTRADA:        - Puntero de tipo void que contiene el buffer
//FUNCIONAMIENTO: - Esta es la funcion ejecutada por cada hebra consumidora:
//                - Se realiza la lectura del buffer por cada una de las hebras consumidoras y
//                  cada hebra procesa las filas leidas
//SALIDA:         - Void
void *pipeline(void *arg)
{
    pthread_barrier_wait(&rendezvous);   //Se espera a que todas la hebras consumidoras esten creadas
    int numFila, i=0;                    //NumFila se utiliza para guardar la fila obtenida del buffer
    buffer_t *buffer;                    //Se crea una variable de tipo buffer_t
    buffer = (buffer_t *) arg;           //Se castea el Buffer

    int filasARecoger = jpegData.height/cantHebrasConsumidoras;  //Se obtiene la cantidad de filas a recoger por cada hebra
    
    pthread_mutex_lock (&buffer->mutex); //Se entra a la seccion critica y se bloquea el acceso a las demas hebras
    ordenHebras++;   //Se aumenta el contador para saber cuantas hebras han ejecutado este codigo (Variable Global)
    //Si es la ultima hebra y si m es decimal la ultima hebra debe ejecutar mas filas
    if(ordenHebras == cantHebrasConsumidoras){
        if(jpegData.height%cantHebrasConsumidoras != 0){                              //Si m es decimal
            filasARecoger = filasARecoger + (jpegData.height%cantHebrasConsumidoras); //la ultima hebra debe ejecutar mas filas
        }
    }
    pthread_mutex_unlock(&buffer->mutex);  //Se sale de la seccion critica


    //*****************************************************************************************************
    //Algoritmo del Conusmidor
    //*****************************************************************************************************
    int *filasHebra = (int *)malloc(sizeof(int)*filasARecoger);  //Arreglo para guardar las filas de cada hebra
    int largoFilasHebras = filasARecoger;                        //largo del arreglo filasHebra
    for (int i = 0; i < filasARecoger; i++)                      //Por cada fila a consumir
    {
        pthread_mutex_lock (&buffer->mutex);  //Se entra a seccion critica y se bloquea el acceso a las demas hebras
        if(buffer->empty){                    //Si el buffer esta vacio
            buffer->consumiendo = 0;          //se deja de consumir
            buffer->produciendo = 1;          //Se comienza a producir
        }
        while(buffer->empty || buffer->produciendo) {  //Mientras el buffer este vacio o la productora este produciendo
            ordenHebras2--;                            //Variable Global utilizada para saber cual es la ultima hebra
            if(ordenHebras2 == 0)                      //es la ultima hebra
            {
                pthread_cond_signal(&buffer->notFull); //Se desbloquea la hebra productora
            }
            pthread_cond_wait (&buffer->notEmpty, &buffer->mutex); //Se bloquea la hebra consumidora
            ordenHebras2++;
        }       
        numFila = take_from_buffer(&buffer);    //La conusmidora obtiene una fila del buffer
        filasHebra[i] = numFila;                //Se guarda la fila en el arreglo filasHebras
        if(i == (filasARecoger-1))              //Si la hebra ya consumio todas las filas que le corresponden
            ordenHebras2--;                     //Se le resta 1 a la Variable Global
        pthread_mutex_unlock(&buffer->mutex);   //Se sale de la seccion critica
    }
    //*****************************************************************************************************
    //*****************************************************************************************************

    pthread_barrier_wait(&rendezvous);     //que todas las hebras esperen a que las demas terminen de consumir
    pthread_cond_signal(&buffer->notFull); //Se desbloquea la productora para que termine su ejecucion en caso de que haya sido bloqueada
    ordenHebras = 0;                       //Para saber cual es la ultima hebra que ejecuta ciertos codigos
    
    pthread_barrier_wait(&rendezvous);     //Segunda barrera para sincrinozar las hebras
    pthread_cond_signal(&buffer->notFull); 
    
    //******************************************************************************************************
    //conversion
    convertirAEscalaGrises(filasHebra, largoFilasHebras);
    //esperar a todas las hebras
    pthread_barrier_wait(&rendezvous);

    //******************************************************************************************************
    //filtro
    AplicarFiltro(filasHebra, largoFilasHebras);
    //esperar a todas las hebras
    pthread_barrier_wait(&rendezvous);

    //******************************************************************************************************
    //binarizacion
    binarizarImagen(filasHebra, largoFilasHebras);
    //esperar a todas las hebras
    pthread_barrier_wait(&rendezvous);

    //******************************************************************************************************
    //clasificacion -> Seccion Critica
    pthread_mutex_lock (&buffer->mutex);
    analisisDePropiedad(filasHebra, largoFilasHebras);
    pthread_mutex_unlock(&buffer->mutex);


    free(filasHebra);    //Liberar memoria

    //******************************************************************************************************
    //volver a la hebra main

}