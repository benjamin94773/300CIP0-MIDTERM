#ifndef IMAGEN_H
#define IMAGEN_H

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>

class Imagen {
protected:
    char magic[3];
    int width;
    int height;
    int max_color;
    int* pixels;
    int pixel_count;

public:
    // Constructor y destructor
    Imagen();
    virtual ~Imagen();

    // Métodos virtuales puros para implementar en clases derivadas
    virtual bool cargarImagen(const char* filename) = 0;
    virtual bool guardarImagen(const char* filename) = 0;
    
    // Getters
    int getWidth() const { return width; }
    int getHeight() const { return height; }
    int getMaxColor() const { return max_color; }
    int* getPixels() const { return pixels; }
    int getPixelCount() const { return pixel_count; }
    const char* getMagic() const { return magic; }
    
    // Setters
    void setPixels(int* new_pixels);
    
    // Métodos de utilidad
    bool validarCoordenadas(int x, int y) const;
    int getPixelIndex(int x, int y) const;
    
protected:
    // Métodos auxiliares para lectura
    bool leerCabecera(FILE* file);
    void saltarComentarios(FILE* file);
};

#endif