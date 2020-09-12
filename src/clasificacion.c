//Directivas de Preprocesamiento
#include "../incl/clasificacion.h"

extern JpegData jpegDataFiltrada;
extern int cantidadCeros;
extern int ordenHebras;
extern int umbralNeg;
extern int cantHebrasConsumidoras;
extern char *nearlyBlack;
//Entradas:     - Una imagen del tipo JpegData que ha sido binarizada.
//              - Un entero que representa al umbral de clasificación.
//Funcionamiento: se cuentan todos los píxeles que son negros en la
//                imagen de entrada, para luego obtener su porcentaje
//                de aparición. Si el procentaje de píxeles negros es
//                mayor o igual al umbral, se dice que la imagen es 
//                nearly black.
//Salidas:      - Un puntero a char (string) que indica si la imagen es
//                nearly black o no.
void analisisDePropiedad(int *filasACambiar, int largoArreglo){
    
    int posicion, numFila, ancho, alto;
    ancho = jpegDataFiltrada.width;
    alto = jpegDataFiltrada.height; 
    ordenHebras++;

    for (int i = 0; i < largoArreglo; i++)
    {
        posicion = numFila * ancho;
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

