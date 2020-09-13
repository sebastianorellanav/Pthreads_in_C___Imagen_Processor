//Directivas de Preprocesamiento
#include <stdio.h>
#include <jpeglib.h>
#include <stdint.h>
#include "../incl/conversion.h"
#include "../incl/lecturaImagenes.h"

extern JpegData jpegData;
extern JpegData jpegDataBN;

//Entradas:     - Imagen en formato RGB del tipo JpegData.
//Funcionamiento: se crea una copia de la imagen original, para luego recorrer
//                cada píxel y efectuar la ecuación de luminiscencia en cada uno
//                de ellos. Cada resultado de la ecuación se guarda en la variable
//                Y, la cual representará los nuevos pixeles de la imagen en 
//                escala de grises.
//Salidas:      - Imagen en escala de grises del tipo JpegData
void convertirAEscalaGrises(int *filasACambiar, int largoArreglo){
    int ancho = jpegData.width;
    int ch = jpegData.ch;
    int posicion, fila, R, G, B, p;
    uint8_t Y;

    printf("Tengo las filas:\n");
    for (int i = 0; i < largoArreglo; i++)
    {
        printf("%d  ",filasACambiar[i]);
    }
    

    for (int i = 0; i < largoArreglo; i++)
    {
        fila = filasACambiar[i];
        posicion = fila*ancho*ch;
        p = fila*ancho;
        for (int j = posicion; j < posicion+(ancho*ch); j+=3)
        {
            
            R = jpegData.data[j]; 
            G = jpegData.data[j+1];
            B = jpegData.data[j+2];
            Y = R*0.3 + G*0.59 + B*0.11;
            jpegDataBN.data[p] = Y;
            p++;
        }
        
    }
}