//**************************************************************************************************************
//Directivas de Preprocesamiento
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <jpeglib.h>
#include <inttypes.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include "../incl/lecturaImagenes.h"
#include "../incl/escrituraImagenes.h"
#include "../incl/binarizacion.h"
#include "../incl/clasificacion.h"
#include "../incl/conversion.h"
#include "../incl/filtro.h"
#include "../incl/hebras.h"


//**************************************************************************************************************
//Variables Globales
JpegData jpegData;          //Imagen Original
JpegData jpegDataBN;        //Imagen en Blanco y Negro
JpegData jpegDataFiltrada;  //Imagen Filtrada
int numImagen;              //numero de Imagen
int cantHebrasConsumidoras; //cantidad de Hebras Consumidoras
int ordenHebras;            //para saber cual es la ultima hebra que ejecuta ciertas funciones
int umbralBin;              //Umbral de binarizacion
int umbralNeg;              //Umbral de negrura
int **mascara;              //Mascara para filtro laplaciano
int cantidadCeros;          //VARIABLE GLOBAL PARA LA CLASIFICACION
char *nearlyBlack;          //variable que almacena el resultado de la clasificacion
pthread_barrier_t rendezvous; //barrera para sincronizar hebras
int ordenHebras2;           //para saber cual es la ultima hebra que ejecuta ciertas funciones
sem_t semaforo;
char *aux;             //Semaforo para sincronizar hebras


//**************************************************************************************************************
//Funcion Main
int main (int argc, char **argv)
{
	//Inicialización de Variables
	int cantImagenes = 0;             //Cantidad de Imagenes
	int flagResultados = 0;           //Flag para mostrar Resultados
	char *nombreArchivoMasc = NULL;   //Nombre del archivo que contiene la mascara para filtrar
	int tamanoBuffer;                 //Tamaño del buffer
	int c;                            //variable para almacenar parametros de entrada

	//Variables getopt
	opterr = 0;

	if(argc == 1){
		printf("No se ha ingresado ningun parametro.\n");
		exit(-1);
	}

	//el siguiente ciclo se utiliza para recibir los parametros de entrada usando getopt
	while ((c = getopt (argc, argv, "c:u:m:n:b:h:f")) != -1)
		switch (c)
			{
			case 'c':
				sscanf(optarg, "%d", &cantImagenes);
				if(cantImagenes <= 0){
					printf("La cantidad de imagenes ingresada no es valida\n");
					exit(-1);
				}
				break;
			case 'u':
				sscanf(optarg, "%d", &umbralBin);
				if(umbralBin < 0){
					printf("El umbral de binarizacion ingresado no es valido\n");
					exit(-1);
				}
				break;
			case 'n':
				sscanf(optarg, "%d", &umbralNeg);
				if(umbralNeg < 0){
					printf("El umbral de negrura ingresado no es valido\n");
					exit(-1);
				}
				break;
			case 'm':
				nombreArchivoMasc = optarg;
				break;
			case 'b':
				sscanf(optarg, "%d", &tamanoBuffer);
				if(tamanoBuffer < 0){
					printf("El tamaño de buffer ingresado no es valido\n");
					exit(-1);
				}
				break;
			case 'h':
				sscanf(optarg, "%d", &cantHebrasConsumidoras);
				if(cantHebrasConsumidoras < 0){
					printf("La cantidad de hebras ingresada no es valida\n");
					exit(-1);
				}
				break;
			case 'f':
				flagResultados = 1;
				break;
			case '?':
				if (optopt == 'c')
				fprintf (stderr, "Option -%c requires an argument.\n", optopt);
				else if (isprint (optopt))
				fprintf (stderr, "Unknown option `-%c'.\n", optopt);
				else
				fprintf (stderr,
						"Unknown option character `\\x%x'.\n",
						optopt);
				return 1;
			default:
				abort ();
			}

	
	if(flagResultados){ //Si flagResultado == 1 -> se muestran los resultados
		printf("|          image          |       nearly black       |\n");
		printf("|-------------------------|--------------------------|\n");
	}

	//Obtener mascara para hacer el filtrado
	mascara = leerMascara(nombreArchivoMasc);
	
	//*******************************************************************************************************
    //Ciclo para procesar multiples iagenes
	for (int i = 1; i <= cantImagenes; i++)
	{
		pthread_t productora;                             //se crea el id de la hebra productora
		buffer_t *buffer;                                 //Se crea el buffer
		buffer_init(&buffer, tamanoBuffer);               //Se inicializa el buffer  
		sem_init(&semaforo,0,0);                          //Se inicializa el semaforo
		pthread_mutex_init(&buffer->mutex, NULL);         //Se inicializa el mutex
		pthread_cond_init(&buffer->notFull, NULL);        //Se inicializa el pthread_cond 1
		pthread_cond_init(&buffer->notEmpty, NULL);       //Se inicializa el pthread_cond 2

		numImagen = i;   //se setea el numero de la imagen actual a procesar (Variable Global)
		ordenHebras = 0; //variable utilizada para saber cual es la ultima hebra que ejecuta ciertas funciones
		cantidadCeros = 0; //Setear variable en 0
		
		//Comienza la ejecucion de la hebra productora
		pthread_create(&productora, NULL, leerImagenes, (void *)buffer);
		sem_wait(&semaforo);    //Se bloquea la hebra main
		
		//Consumir Imagen
		pthread_t consumidoras[cantHebrasConsumidoras];                  //Se crean los id para las hebras consumidoras
		pthread_barrier_init(&rendezvous, NULL,cantHebrasConsumidoras);  //Se inicializa el barrier
		ordenHebras2 = cantHebrasConsumidoras;                           //variable utilizada para saber cual es la ultima hebra que ejecuta ciertas funciones
		
		for(int i = 0 ; i<cantHebrasConsumidoras; i++)
		{
			//Comienza la ejecucion de las hebras productoras
			pthread_create(&consumidoras[i], NULL, pipeline, (void *)buffer);
		}

		pthread_join(productora, NULL);      //Se espera a que la productora termine de ejecutarse
		for (int i = 0; i < cantHebrasConsumidoras; i++)
		{
			pthread_join(consumidoras[i], NULL); //Se espera a que las consumidoras terminen de ejecutarse
		}
		
		pthread_barrier_destroy(&rendezvous);  //Se destruye la barrera

		//6. Escribir imagen
		char fileout[30];
		sprintf(fileout,"./out_%i.jpg",i);
		escribirImagenes(jpegDataFiltrada, "escalagrises",fileout);

		//7. liberar memoria
		liberarJpeg(&jpegData);
		liberarJpeg(&jpegDataBN);
		liberarJpeg(&jpegDataFiltrada);
		free(buffer);
		
		//Se muestran por pantalla los resultados obtenidos
		if(flagResultados){
			//Obtener el nombre de imagen
			char imagename[30];
			sprintf(imagename, "imagen_%i",i);
			if(nearlyBlack[0] == 'n'){
				printf("|          %s       |             %s           |\n", imagename, nearlyBlack);
			}
			else{
				printf("|          %s       |             %s          |\n", imagename, nearlyBlack);
			}
		}
	} 

	//Se libera la memoria de la mascara para el filtro
	free(mascara);
	return 0;  //exit
}