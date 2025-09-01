#ifndef FILTER_H
#define FILTER_H

#include "imagen.h"
#include "PGMimage.h"
#include "PPMimage.h"

enum FilterType {
    BLUR,
    LAPLACE,
    SHARPENING
};

class Filter {
public:
    // Aplicar filtro a imagen PGM
    static PGMImage* aplicarFiltro(const PGMImage* imagen, FilterType tipo);
    
    // Aplicar filtro a imagen PPM
    static PPMImage* aplicarFiltro(const PPMImage* imagen, FilterType tipo);
    
    // Conversión de string a FilterType
    static FilterType stringToFilterType(const char* filterName);
    static const char* filterTypeToString(FilterType tipo);

    // Obtener kernel según el tipo de filtro
    static const float (*getKernel(FilterType tipo))[3];

private:
    // Kernels de filtros
    static const float blur_kernel[3][3];
    static const float laplace_kernel[3][3];
    static const float sharpening_kernel[3][3];
    
    // Aplicar convolución con kernel 3x3
    static int aplicarConvolucion(const int* pixels, int width, int height, 
                                 int x, int y, const float kernel[3][3], int max_color);
    
    // Aplicar convolución para un canal de color específico
    static int aplicarConvolucionColor(const int* pixels, int width, int height,
                                      int x, int y, int canal, const float kernel[3][3], int max_color);
    
    
};

#endif