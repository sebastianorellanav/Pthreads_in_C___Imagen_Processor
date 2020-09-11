//Directivas de Preprocesamiento
#include "../incl/binarizacion.h"
#include <stdint.h>
#include <stdio.h>

extern JpegData jpegDataBN;
extern int umbralBin;

//Entradas:     - JpegData que representa la imagen filtrada (Filtro de realce).
//              - uint8_t que representa el umbral de binarización de la imagen.       
//Funcionamiento: se compara el valor de cada pixel de la imagen de entrada con
//                el umbral ingresado. Si el pixel es mayor al umbral, entonces
//                cambiará su valor a 255 (blanco) y en caso contrario, el valor 
//                del pixel  será 0 (negro). 
//Salidas:      - Una imagen del tipo JpegData binarizada (con píxeles brancos y negros).

void binarizarImagen(int *filasACambiar, int largoArreglo){

    int ancho = jpegDataBN.width;
    int posicion, fila;

    for (int i = 0; i < largoArreglo; i++)
    {
        fila = filasACambiar[i];
        posicion = fila*ancho;
        for (int j = posicion; j < posicion+ancho; j++)
        {
            if(jpegDataBN.data[j] > umbralBin){
                jpegDataBN.data[j] = 255;
            }
            else{
                jpegDataBN.data[j] = 0;
            }
        }
    }
}