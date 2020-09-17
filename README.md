# Pthreads_in_C___Imagen_Processor

## Como Compilar ⚙️

Para compilar el programa se debe utilizar el siguiente comando dentro de la carpeta raiz:

```
make
```

## Como Ejecutar 🚀

El nombre del programa creado es "Ejecutable" por lo que para ejecutarlo se debe usar ese nombre. Además, para ejecutar el programa el programa se deben utilizar las siguientes flags:

* -u [Entero]  ->  Umbral para binarizar la imagen
* -n [Entero]  ->  Umbral para la clasificación de negrura de la imagen
* -c [Entero]  ->  Cantidad de imagenes a analizar
* -m [String]  ->  Nombre del archivo que contiene la mascara a utilizar
* -b [Entero]  ->  Tamaño del buffer
* -h [Entero]  ->  Cantidad de Hebras
* -f (opcional)->  Mostrar resultados por consola (Esta flag es opcional)

A continuación se muestran ejemplos de como ejecutar el programa:

```
./ejecutable -u 80 -n 99 -m mascara -c 7 -b 10 -h 10
```

```
./ejecutable -c 4 -u 50 -n 70 -m mascara -b 10 -h 10 -f
```