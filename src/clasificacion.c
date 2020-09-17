//Directivas de Preprocesamiento
#include "../incl/clasificacion.h"
#include <stdio.h>

extern JpegData jpegDataFiltrada;
extern int cantidadCeros;
extern int ordenHebras;
extern int umbralNeg;
extern int cantHebrasConsumidoras;
extern char *nearlyBlack;

//Entradas:     - Puntero de tipo int, int que contiene el largo del arreglo
//Funcionamiento: se cuentan todos los píxeles que son negros en la
//                imagen de entrada, para luego obtener su porcentaje
//                de aparición. Si el procentaje de píxeles negros es
//                mayor o igual al umbral, se dice que la imagen es 
//                nearly black. Lo anterior se realiza para cada fila 
//                correspondiente a la hebra
//Salidas:      - Void
void analisisDePropiedad(int *filasACambiar, int largoArreglo){
    
    int posicion, numFila, ancho, alto;
    ancho = jpegDataFiltrada.width;
    alto = jpegDataFiltrada.height; 
    ordenHebras++;

    //Por cada fila consumida por la hebra
    for (int i = 0; i < largoArreglo; i++)
    {
        numFila = filasACambiar[i];
        posicion = numFila * ancho;
        //Por cada posicion de la fila
        for (int j = posicion; j < posicion+ancho; j++)
        {
            //Si es pixel negro
            if(jpegDataFiltrada.data[j] == 0){
                //se suma 1 al contador
                cantidadCeros++;
            }
        }   
    }
    if(ordenHebras == cantHebrasConsumidoras) //es la ultima hebra
    {
        //clasifica la imagen
        float porcentajeNegrura = (cantidadCeros*100)/(alto*ancho);

        if (porcentajeNegrura >= umbralNeg){
            nearlyBlack = "yes";
        }
        else{
            nearlyBlack = "no";
        }
    }
}

