#include <iostream>
#include <cstring>
#include "PGMimage.h"
#include "PPMimage.h"
#include "timer.h"

void mostrarUso(const char* programa) {
    std::cout << "Uso: " << programa << " <imagen_entrada> <imagen_salida>" << std::endl;
    std::cout << "Ejemplo:" << std::endl;
    std::cout << "  " << programa << " lena.ppm lena2.ppm" << std::endl;
    std::cout << "  " << programa << " imagen.pgm imagen2.pgm" << std::endl;
    std::cout << std::endl;
    std::cout << "Formatos soportados:" << std::endl;
    std::cout << "  - PGM (P2): Imágenes en escala de grises" << std::endl;
    std::cout << "  - PPM (P3): Imágenes a color" << std::endl;
}

bool esFormatoPPM(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (file == nullptr) {
        std::cerr << "DEBUG: no se pudo abrir archivo [" << filename << "]" << std::endl;
        return false;
    }
    
    char magic[3];
    if (fscanf(file, "%2s", magic) != 1) {
        std::cerr << "DEBUG: no se pudo leer cabecera de [" << filename << "]" << std::endl;
        fclose(file);
        return false;
    }
    std::cerr << "DEBUG: magic leído en esFormatoPPM = [" << magic << "]" << std::endl;
    fclose(file);
    return (strcmp(magic, "P3") == 0);
}

bool esFormatoPGM(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (file == nullptr) {
        std::cerr << "DEBUG: no se pudo abrir archivo [" << filename << "]" << std::endl;
        return false;
    }
    
    char magic[3];
    if (fscanf(file, "%2s", magic) != 1) {
        std::cerr << "DEBUG: no se pudo leer cabecera de [" << filename << "]" << std::endl;
        fclose(file);
        return false;
    }
    std::cerr << "DEBUG: magic leído en esFormatoPPM = [" << magic << "]" << std::endl;
    fclose(file);
    return (strcmp(magic, "P2") == 0);
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cout << "Error: Faltan argumentos de entrada y salida" << std::endl;
        mostrarUso(argv[0]);
        return 1;
    }
    
    const char* archivo_entrada = argv[1];
    const char* archivo_salida = argv[2];
    
    Timer timer;
    
    std::cout << "=== Procesador de Imágenes PPM/PGM ===" << std::endl;
    std::cout << "Archivo de entrada: " << archivo_entrada << std::endl;
    std::cout << "Archivo de salida: " << archivo_salida << std::endl;
    
    // Determinar el formato del archivo
    if (esFormatoPPM(archivo_entrada)) {
        std::cout << "Formato detectado: PPM (P3)" << std::endl;
        
        PPMImage imagen;
        
        // Cargar imagen
        std::cout << "Cargando imagen PPM..." << std::endl;
        timer.start();
        
        if (!imagen.cargarImagen(archivo_entrada)) {
            std::cerr << "Error: No se pudo cargar la imagen PPM" << std::endl;
            return 1;
        }
        
        timer.stop();
        std::cout << "Imagen cargada correctamente" << std::endl;
        std::cout << "Dimensiones: " << imagen.getWidth() << "x" << imagen.getHeight() << std::endl;
        std::cout << "Valor máximo de color: " << imagen.getMaxColor() << std::endl;
        timer.printElapsed("Tiempo de carga");
        
        // Guardar imagen
        std::cout << "Guardando imagen PPM..." << std::endl;
        timer.reset();
        timer.start();
        
        if (!imagen.guardarImagen(archivo_salida)) {
            std::cerr << "Error: No se pudo guardar la imagen PPM" << std::endl;
            return 1;
        }
        
        timer.stop();
        std::cout << "Imagen guardada correctamente" << std::endl;
        timer.printElapsed("Tiempo de guardado");
        
    } else if (esFormatoPGM(archivo_entrada)) {
        std::cout << "Formato detectado: PGM (P2)" << std::endl;
        
        PGMImage imagen;
        
        // Cargar imagen
        std::cout << "Cargando imagen PGM..." << std::endl;
        timer.start();
        
        if (!imagen.cargarImagen(archivo_entrada)) {
            std::cerr << "Error: No se pudo cargar la imagen PGM" << std::endl;
            return 1;
        }
        
        timer.stop();
        std::cout << "Imagen cargada correctamente" << std::endl;
        std::cout << "Dimensiones: " << imagen.getWidth() << "x" << imagen.getHeight() << std::endl;
        std::cout << "Valor máximo de color: " << imagen.getMaxColor() << std::endl;
        timer.printElapsed("Tiempo de carga");
        
        // Guardar imagen
        std::cout << "Guardando imagen PGM..." << std::endl;
        timer.reset();
        timer.start();
        
        if (!imagen.guardarImagen(archivo_salida)) {
            std::cerr << "Error: No se pudo guardar la imagen PGM" << std::endl;
            return 1;
        }
        
        timer.stop();
        std::cout << "Imagen guardada correctamente" << std::endl;
        timer.printElapsed("Tiempo de guardado");
        
    } else {
        std::cerr << "Error: Formato de archivo no soportado o archivo corrupto" << std::endl;
        std::cerr << "Solo se soportan archivos PGM (P2) y PPM (P3)" << std::endl;
        return 1;
    }
    
    std::cout << "Procesamiento completado exitosamente" << std::endl;
    return 0;
}