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
pthread_barrier_t rendezvous;
int ordenHebras2;
sem_t semaforo;

//Funcion Main
int main (int argc, char **argv)
{
	//Inicialización de Variables
	int cantImagenes = 0;
	int flagResultados = 0;
	char *nombreArchivoMasc = NULL;
	int tamanoBuffer;
	int c;

	//Variables getopt
	opterr = 0;

	//el siguiente ciclo se utiliza para recibir los parametros de entrada usando getopt
	while ((c = getopt (argc, argv, "c:u:m:n:b:h:f")) != -1)
		switch (c)
			{
			case 'c':
				sscanf(optarg, "%d", &cantImagenes);
				break;
			case 'u':
				sscanf(optarg, "%d", &umbralBin);
				break;
			case 'n':
				sscanf(optarg, "%d", &umbralNeg);
				break;
			case 'm':
				nombreArchivoMasc = optarg;
				break;
			case 'b':
				sscanf(optarg, "%d", &tamanoBuffer);
				break;
			case 'h':
				sscanf(optarg, "%d", &cantHebrasConsumidoras);
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

	if(flagResultados){
		printf("|          image          |       nearly black       |\n");
		printf("|-------------------------|--------------------------|\n");
	}

	//Obtener mascara para hacer el filtrado
	mascara = leerMascara(nombreArchivoMasc);
	
    // Para cada imagen:
	for (int i = 1; i <= cantImagenes; i++)
	{
		pthread_t productora;                             //se crean los id de las hebras
    	pthread_t consumidoras[cantHebrasConsumidoras];
		buffer_t *buffer;                                 //Se crea el buffer
		buffer_init(&buffer, tamanoBuffer);  
		sem_init(&semaforo,0,0);             //Se inicializa el buffer
		pthread_mutex_init(&buffer->mutex, NULL);         //Se inicializa el mutex
		pthread_cond_init(&buffer->notFull, NULL);    
		pthread_cond_init(&buffer->notEmpty, NULL);
    	pthread_barrier_init(&rendezvous, NULL,cantHebrasConsumidoras);  //Se inicializa la barrera
		printf("Se creo el id de las hebras y se inicializo el buffer\n");

		numImagen = i;   //se setea el numero de la imagen actual a procesar (Variable Global)
		ordenHebras = 0; //variable utilizada para saber cual es la ultima hebra que ejecuta ciertas funciones
		cantidadCeros = 0; //Setear variable en 0
		ordenHebras2 = cantHebrasConsumidoras;
		//Comienza la ejecucion de la hebra productora
		pthread_create(&productora, NULL, leerImagenes, (void *)buffer);
		printf("se comenzo a ejecutar la productora\n");
		sem_wait(&semaforo);
		//Consumir Imagen
		for(int i = 0 ; i<cantHebrasConsumidoras; i++)
		{
			//Comienza la ejecucion de las hebras productoras
			pthread_create(&consumidoras[i], NULL, pipeline, (void *)buffer);
			printf("se comienza a ejecutar la consumidora nro: %d\n",i+1);
		}
		pthread_join(productora, NULL);
		for (int i = 0; i < cantHebrasConsumidoras; i++)
		{
			pthread_join(consumidoras[i], NULL);
		}
		
		pthread_barrier_destroy(&rendezvous);
    	printf("se destruye la barrera\n");
		
		printf("Si logra llegar hasta aqui estamos salvados\n");

		//6. Escribir imagen
		char fileout[30];
		sprintf(fileout,"./out_%i.jpg",i);
		escribirImagenes(jpegDataFiltrada, "escalagrises",fileout);

		//7. liberar memoria
		liberarJpeg(&jpegData);
		liberarJpeg(&jpegDataBN);
		liberarJpeg(&jpegDataFiltrada);
		free(buffer);
		

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
	free(mascara);
	return 0;
}