#include "filter.h"
#include <algorithm>
#include <cstring>

// Definición de kernels
const float Filter::blur_kernel[3][3] = {
    {1.0f/9, 1.0f/9, 1.0f/9},
    {1.0f/9, 1.0f/9, 1.0f/9},
    {1.0f/9, 1.0f/9, 1.0f/9}
};

const float Filter::laplace_kernel[3][3] = {
    { 0.0f, -1.0f,  0.0f},
    {-1.0f,  4.0f, -1.0f},
    { 0.0f, -1.0f,  0.0f}
};

const float Filter::sharpening_kernel[3][3] = {
    { 0.0f, -1.0f,  0.0f},
    {-1.0f,  5.0f, -1.0f},
    { 0.0f, -1.0f,  0.0f}
};

PGMImage* Filter::aplicarFiltro(const PGMImage* imagen, FilterType tipo) {
    if (imagen == nullptr || imagen->getPixels() == nullptr) {
        return nullptr;
    }
    
    PGMImage* resultado = imagen->crearImagenVacia();
    if (resultado == nullptr) {
        return nullptr;
    }
    
    const float (*kernel)[3] = getKernel(tipo);
    int width = imagen->getWidth();
    int height = imagen->getHeight();
    int max_color = imagen->getMaxColor();
    const int* pixels = imagen->getPixels();
    
    // Aplicar filtro píxel por píxel
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int valor_filtrado = aplicarConvolucion(pixels, width, height, x, y, kernel, max_color);
            resultado->setPixelValue(x, y, valor_filtrado);
        }
    }
    
    return resultado;
}

PPMImage* Filter::aplicarFiltro(const PPMImage* imagen, FilterType tipo) {
    if (imagen == nullptr || imagen->getPixels() == nullptr) {
        return nullptr;
    }
    
    PPMImage* resultado = imagen->crearImagenVacia();
    if (resultado == nullptr) {
        return nullptr;
    }
    
    const float (*kernel)[3] = getKernel(tipo);
    int width = imagen->getWidth();
    int height = imagen->getHeight();
    int max_color = imagen->getMaxColor();
    const int* pixels = imagen->getPixels();
    
    // Aplicar filtro píxel por píxel para cada canal (R, G, B)
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int r = aplicarConvolucionColor(pixels, width, height, x, y, 0, kernel, max_color);
            int g = aplicarConvolucionColor(pixels, width, height, x, y, 1, kernel, max_color);
            int b = aplicarConvolucionColor(pixels, width, height, x, y, 2, kernel, max_color);
            resultado->setPixelRGB(x, y, r, g, b);
        }
    }
    
    return resultado;
}

FilterType Filter::stringToFilterType(const char* filterName) {
    if (strcmp(filterName, "blur") == 0) {
        return BLUR;
    } else if (strcmp(filterName, "laplace") == 0) {
        return LAPLACE;
    } else if (strcmp(filterName, "sharpening") == 0 || strcmp(filterName, "sharpen") == 0) {
        return SHARPENING;
    }
    return BLUR; // Por defecto
}

const char* Filter::filterTypeToString(FilterType tipo) {
    switch (tipo) {
        case BLUR: return "blur";
        case LAPLACE: return "laplace";
        case SHARPENING: return "sharpening";
        default: return "unknown";
    }
}

int Filter::aplicarConvolucion(const int* pixels, int width, int height, 
                              int x, int y, const float kernel[3][3], int max_color) {
    float sum = 0.0f;
    float weight_sum = 0.0f;
    
    // Aplicar kernel 3x3
    for (int ky = -1; ky <= 1; ky++) {
        for (int kx = -1; kx <= 1; kx++) {
            int nx = x + kx;
            int ny = y + ky;
            
            if (nx >= 0 && nx < width && ny >= 0 && ny < height) {
                int idx = ny * width + nx;
                float kernel_val = kernel[ky + 1][kx + 1];
                sum += pixels[idx] * kernel_val;
                weight_sum += (kernel_val > 0) ? kernel_val : 0; // Solo para normalización con kernels positivos
            }
        }
    }
    
    // Normalizar solo para blur (que tiene todos valores positivos)
    int result;
    if (weight_sum > 0) {
        result = static_cast<int>(sum / weight_sum);
    } else {
        result = static_cast<int>(sum);
    }
    
    // Asegurar valores dentro del rango válido
    return std::max(0, std::min(max_color, result));
}

int Filter::aplicarConvolucionColor(const int* pixels, int width, int height,
                                   int x, int y, int canal, const float kernel[3][3], int max_color) {
    float sum = 0.0f;
    float weight_sum = 0.0f;
    
    // Aplicar kernel 3x3
    for (int ky = -1; ky <= 1; ky++) {
        for (int kx = -1; kx <= 1; kx++) {
            int nx = x + kx;
            int ny = y + ky;
            
            if (nx >= 0 && nx < width && ny >= 0 && ny < height) {
                int idx = (ny * width + nx) * 3 + canal; // índice para el canal específico
                float kernel_val = kernel[ky + 1][kx + 1];
                sum += pixels[idx] * kernel_val;
                weight_sum += (kernel_val > 0) ? kernel_val : 0;
            }
        }
    }
    
    // Normalizar solo para blur
    int result;
    if (weight_sum > 0) {
        result = static_cast<int>(sum / weight_sum);
    } else {
        result = static_cast<int>(sum);
    }
    
    // Asegurar valores dentro del rango válido
    return std::max(0, std::min(max_color, result));
}

const float (*Filter::getKernel(FilterType tipo))[3] {
    switch (tipo) {
        case BLUR: return blur_kernel;
        case LAPLACE: return laplace_kernel;
        case SHARPENING: return sharpening_kernel;
        default: return blur_kernel;
    }
}