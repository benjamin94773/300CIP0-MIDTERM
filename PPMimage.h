#ifndef PPMIMAGE_H
#define PPMIMAGE_H

#include "imagen.h"

struct RGB {
    int r, g, b;
    
    RGB() : r(0), g(0), b(0) {}
    RGB(int red, int green, int blue) : r(red), g(green), b(blue) {}
};

class PPMImage : public Imagen {
public:
    PPMImage();
    virtual ~PPMImage();
    
    // Implementación de métodos virtuales
    virtual bool cargarImagen(const char* filename) override;
    virtual bool guardarImagen(const char* filename) override;
    
    // Métodos específicos para PPM
    RGB getPixelRGB(int x, int y) const;
    void setPixelRGB(int x, int y, const RGB& color);
    void setPixelRGB(int x, int y, int r, int g, int b);
    
    // Obtener componentes individuales
    int getRed(int x, int y) const;
    int getGreen(int x, int y) const;
    int getBlue(int x, int y) const;
    
    // Crear una nueva imagen PPM con las mismas dimensiones
    PPMImage* crearImagenVacia() const;
    
private:
    // Obtener índice para componente específica (r=0, g=1, b=2)
    int getColorIndex(int x, int y, int component) const;
};

#endif