//Directivas de Preprocesamiento
#include "estructuras.h"
#ifndef FILTRO_H
#define FILTRO_H

//Cabeceras de funciones
int **leerMascara(char *nombreMascara);
int **crearPunteroMascara();
void AplicarFiltro(int *filasHebra, int largoFilasHebras);
#endif