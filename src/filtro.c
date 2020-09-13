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
        p=0;
        for (int i = posicion+1; i < (posicion+ancho-1); i++)  //Calcula la convolucion
        {
            calcularFiltro(i, ancho);   
            /*
            nuevoPixel = 0;
            if(p=posicion-ancho-1 >= 0 && i-1 >= 0 && numFila-1 >= 0) //posicion superior izquierda
                nuevoPixel = nuevoPixel + ((jpegDataBN.data[p]*mascara[0][0]));
            
            if(p=posicion-ancho >= 0 && numFila-1 >= 0)  //posicion superior central
               nuevoPixel = nuevoPixel + ((jpegDataBN.data[p]*mascara[0][1])); 
            
            if(p=posicion-ancho+1 >= 0 && i+1 < ancho && numFila-1 >= 0)  //posicion superior derecha
               nuevoPixel = nuevoPixel + ((jpegDataBN.data[p]*mascara[0][2]));

            if(p=posicion-1 >= 0 && i-1 >= 0)  //posicion izquierda
               nuevoPixel = nuevoPixel + ((jpegDataBN.data[p]*mascara[1][0]));
            
            if(p=posicion+1 < largoImagen && i+1 < ancho)  //posicion derecha
               nuevoPixel = nuevoPixel + ((jpegDataBN.data[p]*mascara[1][2]));
            
            if(p=posicion+ancho-1 < largoImagen && i-1 >= 0 && numFila+1 < alto)  //posicion inferior izquierda
               nuevoPixel = nuevoPixel + ((jpegDataBN.data[p]*mascara[2][0]));

            if(p=posicion+ancho < largoImagen && numFila+1 < alto)  //posicion inferior central
               nuevoPixel = nuevoPixel + ((jpegDataBN.data[p]*mascara[2][1]));

            if(p=posicion+ancho+1 < largoImagen && i+1 < ancho && numFila+1 < alto)  //posicion inferior derecha
               nuevoPixel = nuevoPixel + ((jpegDataBN.data[p]*mascara[2][2]));
            
            nuevoPixel = nuevoPixel + ((jpegDataBN.data[posicion]*mascara[1][1])); //posicion central
            */
            //jpegDataFiltrada.data[posicion] = nuevoPixel;
        }
        
    }
    
}

void  calcularFiltro(int loc, int w){
    int n1 = jpegDataBN.data[loc - w -1];
    n1 = n1* mascara[0][0];
    int n2 = jpegDataBN.data[loc - w] ; 
    n2= n2 * mascara[0][1];
    int n3 = jpegDataBN.data[loc - w + 1] ; 
    n3 = n3 * mascara[0][2];
    int n4 = jpegDataBN.data[loc - 1] ; 
    n4 = n4 * mascara[1][0];
    int n5 = jpegDataBN.data[loc]; 
    n5 = n5 * mascara[1][1];
    int n6 = jpegDataBN.data[loc + 1] ;
    n6 = n6 * mascara[1][2]; 
    int n7 = jpegDataBN.data[loc + w -1] ;
    n7 = n7 * mascara[2][0]; 
    int n8 = jpegDataBN.data[loc + w] ; 
    n8 = n8 * mascara[2][1];
    int n9 = jpegDataBN.data[loc + w +1] ;
    n9 = n9* mascara[2][2];
    int resultado = n1 + n2 + n3 + n4 + n5 + n6 +n7 +n8 +n9; 
    jpegDataFiltrada.data[loc] = resultado;
 
}
