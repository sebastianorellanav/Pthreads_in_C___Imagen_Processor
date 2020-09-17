//Directivas de Preprocesamiento
#include "../incl/binarizacion.h"
#include <stdint.h>
#include <stdio.h>

extern JpegData jpegDataFiltrada;
extern int umbralBin;

//Entradas:     - Puntero de tipo int, int que representa el largo del arreglo      
//Funcionamiento: se compara el valor de cada pixel de la imagen de entrada con
//                el umbral ingresado. Si el pixel es mayor al umbral, entonces
//                cambiará su valor a 255 (blanco) y en caso contrario, el valor 
//                del pixel  será 0 (negro). Lo anterior se realiza en las filas 
//                a cada hebra
//Salidas:      - Void
void binarizarImagen(int *filasACambiar, int largoArreglo){

    int ancho = jpegDataFiltrada.width;
    int posicion, fila;

    //Para cada fila que tenga que modificar la hebra
    for (int i = 0; i < largoArreglo; i++)  
    {
        fila = filasACambiar[i];
        posicion = fila*ancho;
        //Para cada posicion de la fila
        for (int j = posicion; j < posicion+ancho; j++)
        {
            if(jpegDataFiltrada.data[j] > umbralBin){
                jpegDataFiltrada.data[j] = 255;
            }
            else{
                jpegDataFiltrada.data[j] = 0;
            }
        }
    }
}