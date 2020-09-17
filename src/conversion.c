//Directivas de Preprocesamiento
#include <stdio.h>
#include <jpeglib.h>
#include <stdint.h>
#include <unistd.h>
#include "../incl/conversion.h"
#include "../incl/lecturaImagenes.h"

extern JpegData jpegData;
extern JpegData jpegDataBN;

//Entradas:       - Puntero de tipo int, int que representa el largo del arreglo
//Funcionamiento: - La hebra que ejecute esta funcion realiza el calculo para obtener pixeles grises unicamente 
//                 en las filas que le corresponde
//Salidas:        - Void
void convertirAEscalaGrises(int *filasACambiar, int largoArreglo){
    int ancho = jpegData.width;
    int ch = jpegData.ch;
    int posicion, fila, R, G, B, p;
    uint8_t Y;    
    sleep(1); //Sincronizacion de hebras
    //Para cada fila que haya consumido la hebra
    for (int i = 0; i < largoArreglo; i++)
    {
        fila = filasACambiar[i];
        posicion = fila*ancho*ch;
        p = fila*ancho;
        //Para cada posicion de la fila
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