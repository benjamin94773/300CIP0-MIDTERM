#include "imagen.h"

Imagen::Imagen() : width(0), height(0), max_color(0), pixels(nullptr), pixel_count(0) {
    magic[0] = '\0';
}

Imagen::~Imagen() {
    if (pixels != nullptr) {
        free(pixels);
        pixels = nullptr;
    }
}

void Imagen::setPixels(int* new_pixels) {
    if (pixels != nullptr) {
        free(pixels);
    }
    pixels = new_pixels;
}

bool Imagen::validarCoordenadas(int x, int y) const {
    return (x >= 0 && x < width && y >= 0 && y < height);
}

int Imagen::getPixelIndex(int x, int y) const {
    return y * width + x;
}

bool Imagen::leerCabecera(FILE* file) {
    if (file == nullptr) {
        return false;
    }
    
    // Leer número mágico
    if (fscanf(file, "%2s", magic) != 1) {
        std::cout << "DEBUG: magic leído = [" << magic << "]" << std::endl;
        return false;
    }

    
    
    // Saltar comentarios
    saltarComentarios(file);
    
    // Leer dimensiones
    if (fscanf(file, "%d %d", &width, &height) != 2) {
        return false;
    }
    
    // Saltar comentarios adicionales
    saltarComentarios(file);
    
    // Leer valor máximo de color
    if (fscanf(file, "%d", &max_color) != 1) {
        return false;
    }
    
    return true;
}

void Imagen::saltarComentarios(FILE* file) {
    int c;
    while ((c = fgetc(file)) != EOF) {
        if (c == '#') {
            // Saltar hasta el final de la línea
            while ((c = fgetc(file)) != EOF && c != '\n');
        } else if (c != ' ' && c != '\t' && c != '\n' && c != '\r') {
            // Retroceder un carácter si no es espacio en blanco
            ungetc(c, file);
            break;
        }
    }
}