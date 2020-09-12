//Directivas de Preprocesamiento
#include <stdio.h>
#include <jpeglib.h>
#include <stdint.h>
#include <stdlib.h>
#include <inttypes.h>
#include "../incl/filtro.h"
#include "../incl/lecturaImagenes.h"

extern JpegData jpegDataBN;
extern JpegData jpegDataFiltrada;
extern int **mascara;

//Entradas:     - string (puntero a char) que representa el nombre del archivo de la mascara
//Funcionamiento: se encarga de abrir el archivo, leer la mascara contenida en el y almacenar
//                la mascara en una matriz de enteros.
//Salidas:      - Matriz de enteros que representa la mascara leída.

int **leerMascara(char *nombreMascara){
    FILE *archivo = fopen(nombreMascara,"r");
    if(archivo == NULL){
        printf("Error: no se ha detectado un archivo de texto \n"); 
    }

    int **mascara = crearPunteroMascara();
    int i = 0;
    int j = 0;
    int num1,num2,num3;
    for(i = 0;i < 3; i++){
        fscanf(archivo,"%d %d %d", &num1,&num2,&num3);
        mascara[i][0] = num1;
        mascara[i][1] = num2;
        mascara[i][2] = num3;
    }
    return mascara;
}

//Entradas:     
//Funcionamiento: asigna un espacio de memoria a la matriz de enteros que representa a la mascara.
//Salidas:      - Matriz de enteros y el espacio de memoria pertinente.
int **crearPunteroMascara(){
    int i= 0;
    int **mascara = (int**)malloc(3*sizeof(int*));
    for(i = 0; i < 3; i++){
        mascara[i] = (int*)malloc(3*sizeof(int));
    }
    return mascara;
}

void AplicarFiltro(int *filasHebra, int largoFilasHebras)
{
    int posicion, nuevoPixel, numFila, p;
    int ancho = jpegDataBN.width;
    int alto = jpegDataBN.height;
    int largoImagen = ancho * alto;
    
    for (int j = 0; j < largoFilasHebras; j++)
    {
        numFila = filasHebra[j];
        posicion = numFila * ancho;
        for (int i = posicion; i < posicion+ancho; i++)  //Calcula la convolucion
        {
            if(p=posicion-ancho-1 >= 0 && i-1 >= 0 && numFila-1 >= 0) //posicion superior izquierda
                nuevoPixel = nuevoPixel + (jpegDataBN.data[p]*mascara[0][0]);
            
            if(p=posicion-ancho >= 0 && numFila-1 >= 0)  //posicion superior central
               nuevoPixel = nuevoPixel + (jpegDataBN.data[p]*mascara[0][1]); 
            
            if(p=posicion-ancho+1 >= 0 && i+1 < ancho && numFila-1 >= 0)  //posicion superior derecha
               nuevoPixel = nuevoPixel + (jpegDataBN.data[p]*mascara[0][2]);

            if(p=posicion-1 >= 0 && i-1 >= 0)  //posicion izquierda
               nuevoPixel = nuevoPixel + (jpegDataBN.data[p]*mascara[1][0]);
            
            if(p=posicion+1 < largoImagen && i+1 < ancho)  //posicion derecha
               nuevoPixel = nuevoPixel + (jpegDataBN.data[p]*mascara[1][2]);
            
            if(p=posicion+ancho-1 < largoImagen && i-1 >= 0 && numFila+1 < alto)  //posicion inferior izquierda
               nuevoPixel = nuevoPixel + (jpegDataBN.data[p]*mascara[2][0]);

            if(p=posicion+ancho < largoImagen && numFila+1 < alto)  //posicion inferior central
               nuevoPixel = nuevoPixel + (jpegDataBN.data[p]*mascara[2][1]);

            if(p=posicion+ancho+1 < largoImagen && i+1 < ancho && numFila+1 < alto)  //posicion inferior derecha
               nuevoPixel = nuevoPixel + (jpegDataBN.data[p]*mascara[2][2]);
            
            nuevoPixel = nuevoPixel + (jpegDataBN.data[posicion]*mascara[1][1]); //posicion central

            jpegDataFiltrada.data[posicion] = nuevoPixel;
        }
        
    }
    
}