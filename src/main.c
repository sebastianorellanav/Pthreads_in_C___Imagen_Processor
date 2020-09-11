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
#include "../incl/lecturaImagenes.h"
#include "../incl/escrituraImagenes.h"
#include "../incl/binarizacion.h"
#include "../incl/clasificacion.h"
#include "../incl/conversion.h"
#include "../incl/filtro.h"
#include "../incl/hebras.h"


JpegData jpegData;
JpegData jpegDataBN;
int numImagen;
int cantHebrasConsumidoras;
int ordenHebras;
int umbralBin;

//Funcion Main
int main (int argc, char **argv)
{
	//Inicialización de Variables
	int cantImagenes = 0;
	//int umbralBin = 0;
	int umbralNeg = 0;
	int flagResultados = 0;
	char *nombreArchivoMasc = NULL;
	int tamanoBuffer;
	int cantidadHebras;
	int index;
	int c;

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
				sscanf(optarg, "%d", &cantidadHebras);
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

	

	int **mascara = leerMascara(nombreArchivoMasc);

	uint8_t num = 10;
	num = num*(-1);
	int entero = (int)num;


    // Para cada imagen
	for (int i = 1; i <= cantImagenes; i++)
	{
		//se crean los id de las hebras
		pthread_t productora;
    	pthread_t consumidoras[cantidadHebras];
		buffer_t *buffer;
		buffer_init(&buffer, tamanoBuffer);
		pthread_mutex_init(&buffer->mutex, NULL);
		pthread_cond_init(&buffer->notFull, NULL);
		pthread_cond_init(&buffer->notEmpty, NULL);
		printf("Se creo el id de las hebras y se inicializo el buffer\n");
		//Obtener el nombre de archivo y nombre de imagen
		
		char imagename[30];
		sprintf(imagename, "imagen_%i",i);

		//1. Leer la imagen
		//comienza la ejecucion de la hebra productora
		numImagen = i; //variable global
		cantHebrasConsumidoras = cantidadHebras;  //variable global
		ordenHebras = 0;
		pthread_create(&productora, NULL, leerImagenes, (void *)buffer);
		printf("se comenzo a ejecutar la productora\n");
		
		//Consumir Imagen
		for(int i = 0 ; i<cantidadHebras; i++)
		{
			pthread_create(&consumidoras[i], NULL, pipeline, (void *)buffer);
			printf("se comienza a ejecutar la consumidora nro: %d\n",i+1);
		}
		pthread_join(productora, NULL);
		printf("Si logra llegar hasta aqui estamos salvados\n");

		//2. Convertir a escala de grises
		jpegData = convertirAEscalaGrises(jpegData); 
		
		//3. aplicar filtro laplaciano
		jpegData = aplicarFiltroLaplaciano(jpegData,mascara); 
		
		//4. binarizar imagen
		jpegData = binarizarImagen(jpegData, umbralBin);
		
		//5. Clasificar imagen
		char *nearlyBlack = analisisDePropiedad(jpegData, umbralNeg);

		//6. Escribir imagen
		char fileout[30];
		sprintf(fileout,"./out_%i.jpg",i);
		escribirImagenes(jpegData, "escalagrises",fileout);

		//7. liberar memoria
		liberarJpeg(&jpegData);

		if(flagResultados){
			if(nearlyBlack[0] == 'n'){
				printf("|          %s       |             %s           |\n", imagename, nearlyBlack);
			}
			else{
				printf("|          %s       |             %s          |\n", imagename, nearlyBlack);
			}
		}
	}

   
    
	return 0;
}