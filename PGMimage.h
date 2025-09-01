#ifndef PGMIMAGE_H
#define PGMIMAGE_H

#include "imagen.h"

class PGMImage : public Imagen {
public:
    PGMImage();
    virtual ~PGMImage();
    
    // Implementación de métodos virtuales
    virtual bool cargarImagen(const char* filename) override;
    virtual bool guardarImagen(const char* filename) override;
    
    // Métodos específicos para PGM
    int getPixelValue(int x, int y) const;
    void setPixelValue(int x, int y, int value);
    
    // Crear una nueva imagen PGM con las mismas dimensiones
    PGMImage* crearImagenVacia() const;
};

#endif