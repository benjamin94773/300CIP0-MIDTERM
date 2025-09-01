#include "PGMimage.h"
#include <algorithm>

PGMImage::PGMImage() : Imagen() {
}

PGMImage::~PGMImage() {
    // El destructor de la clase base se encarga de liberar la memoria
}

bool PGMImage::cargarImagen(const char* filename) {
    FILE* file = fopen(filename, "r");
    std::cout << "DEBUG: intentando abrir [" << filename << "]" << std::endl;
    if (file == nullptr) {
        std::cerr << "Error: No se pudo abrir el archivo " << filename << std::endl;
        return false;
    }
    
    // Leer cabecera
    if (!leerCabecera(file)) {
        std::cerr << "Error: No se pudo leer la cabecera del archivo PGM" << std::endl;
        fclose(file);
        return false;
    }
    
    // Verificar que sea formato PGM
    if (strcmp(magic, "P2") != 0) {
        std::cerr << "Error: El archivo no es formato PGM (P2)" << std::endl;
        fclose(file);
        return false;
    }
    
    // Calcular número de píxeles
    pixel_count = width * height;
    
    // Reservar memoria para los píxeles
    pixels = (int*)malloc(pixel_count * sizeof(int));
    if (pixels == nullptr) {
        std::cerr << "Error: No se pudo reservar memoria para los píxeles" << std::endl;
        fclose(file);
        return false;
    }
    
    // Leer píxeles
    for (int i = 0; i < pixel_count; i++) {
        int value;
        if (fscanf(file, "%d", &value) != 1) {
            std::cerr << "Error: No se pudieron leer los píxeles" << std::endl;
            free(pixels);
            pixels = nullptr;
            fclose(file);
            return false;
        }
        pixels[i] = value;
    }
    
    fclose(file);
    return true;
}

bool PGMImage::guardarImagen(const char* filename) {
    if (pixels == nullptr || pixel_count == 0) {
        std::cerr << "Error: No hay imagen cargada para guardar" << std::endl;
        return false;
    }
    
    FILE* output = fopen(filename, "w");
    if (output == nullptr) {
        std::cerr << "Error: No se pudo crear el archivo " << filename << std::endl;
        return false;
    }
    
    // Escribir cabecera
    fprintf(output, "%s\n%d %d\n%d\n", magic, width, height, max_color);
    
    // Escribir píxeles
    for (int i = 0; i < pixel_count; i++) {
        fprintf(output, "%d\n", pixels[i]);
    }
    
    fclose(output);
    return true;
}

int PGMImage::getPixelValue(int x, int y) const {
    if (!validarCoordenadas(x, y) || pixels == nullptr) {
        return 0;
    }
    return pixels[getPixelIndex(x, y)];
}

void PGMImage::setPixelValue(int x, int y, int value) {
    if (!validarCoordenadas(x, y) || pixels == nullptr) {
        return;
    }
    
    // Asegurar que el valor esté dentro del rango válido
    value = std::max(0, std::min(max_color, value));
    pixels[getPixelIndex(x, y)] = value;
}

PGMImage* PGMImage::crearImagenVacia() const {
    PGMImage* nueva = new PGMImage();
    strcpy(nueva->magic, magic);
    nueva->width = width;
    nueva->height = height;
    nueva->max_color = max_color;
    nueva->pixel_count = pixel_count;
    
    nueva->pixels = (int*)malloc(pixel_count * sizeof(int));
    if (nueva->pixels == nullptr) {
        delete nueva;
        return nullptr;
    }
    
    // Inicializar con ceros
    for (int i = 0; i < pixel_count; i++) {
        nueva->pixels[i] = 0;
    }
    
    return nueva;
}