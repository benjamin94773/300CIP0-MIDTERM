#include <iostream>
#include <cstring>
#include <cstdlib>
#include "PGMimage.h"
#include "PPMimage.h"
#include "filter.h"
#include "timer.h"

void mostrarUso(const char* programa) {
    std::cout << "Uso: " << programa << " <entrada> <salida> --f <filtro>" << std::endl;
    std::cout << "Ejemplo:" << std::endl;
    std::cout << "  " << programa << " fruit.ppm fruit_blur.ppm --f blur" << std::endl;
    std::cout << "  " << programa << " lena.pgm lena_sharp.pgm --f sharpening" << std::endl;
    std::cout << std::endl;
    std::cout << "Filtros disponibles:" << std::endl;
    std::cout << "  - blur      : Filtro de suavizado" << std::endl;
    std::cout << "  - laplace   : Filtro de Laplace (detección de bordes)" << std::endl;
    std::cout << "  - sharpening: Filtro de realce" << std::endl;
    std::cout << std::endl;
    std::cout << "Formatos soportados:" << std::endl;
    std::cout << "  - PGM (P2): Imágenes en escala de grises" << std::endl;
    std::cout << "  - PPM (P3): Imágenes a color" << std::endl;
}

bool esFormatoPPM(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (file == nullptr) {
        return false;
    }
    
    char magic[3];
    if (fscanf(file, "%2s", magic) != 1) {
        fclose(file);
        return false;
    }
    
    fclose(file);
    return (strcmp(magic, "P3") == 0);
}

bool esFormatoPGM(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (file == nullptr) {
        return false;
    }
    
    char magic[3];
    if (fscanf(file, "%2s", magic) != 1) {
        fclose(file);
        return false;
    }
    
    fclose(file);
    return (strcmp(magic, "P2") == 0);
}

int main(int argc, char* argv[]) {
    if (argc < 5) {
        std::cout << "Error: Argumentos insuficientes" << std::endl;
        mostrarUso(argv[0]);
        return 1;
    }
    
    const char* archivo_entrada = argv[1];
    const char* archivo_salida = argv[2];
    const char* flag_filtro = argv[3];
    const char* nombre_filtro = argv[4];
    
    // Verificar flag de filtro
    if (strcmp(flag_filtro, "--f") != 0) {
        std::cout << "Error: Flag de filtro incorrecto. Use --f" << std::endl;
        mostrarUso(argv[0]);
        return 1;
    }
    
    // Obtener tipo de filtro
    FilterType filtro = Filter::stringToFilterType(nombre_filtro);
    
    Timer timer_total, timer_carga, timer_filtro, timer_guardado;
    
    std::cout << "=== Filterer Secuencial ===" << std::endl;
    std::cout << "Archivo de entrada: " << archivo_entrada << std::endl;
    std::cout << "Archivo de salida: " << archivo_salida << std::endl;
    std::cout << "Filtro: " << Filter::filterTypeToString(filtro) << std::endl;
    std::cout << std::endl;
    
    timer_total.start();
    
    // Determinar el formato del archivo y procesar
    if (esFormatoPPM(archivo_entrada)) {
        std::cout << "Formato detectado: PPM (P3)" << std::endl;
        
        PPMImage* imagen_original = new PPMImage();
        
        // Cargar imagen
        std::cout << "Cargando imagen..." << std::endl;
        timer_carga.start();
        
        if (!imagen_original->cargarImagen(archivo_entrada)) {
            std::cerr << "Error: No se pudo cargar la imagen PPM" << std::endl;
            delete imagen_original;
            return 1;
        }
        
        timer_carga.stop();
        std::cout << "Dimensiones: " << imagen_original->getWidth() << "x" << imagen_original->getHeight() << std::endl;
        std::cout << "Píxeles totales: " << imagen_original->getWidth() * imagen_original->getHeight() << std::endl;
        timer_carga.printElapsed("Tiempo de carga");
        
        // Aplicar filtro
        std::cout << "Aplicando filtro " << Filter::filterTypeToString(filtro) << "..." << std::endl;
        timer_filtro.start();
        
        PPMImage* imagen_filtrada = Filter::aplicarFiltro(imagen_original, filtro);
        
        timer_filtro.stop();
        
        if (imagen_filtrada == nullptr) {
            std::cerr << "Error: No se pudo aplicar el filtro" << std::endl;
            delete imagen_original;
            return 1;
        }
        
        timer_filtro.printElapsed("Tiempo de filtrado");
        
        // Guardar imagen
        std::cout << "Guardando imagen filtrada..." << std::endl;
        timer_guardado.start();
        
        if (!imagen_filtrada->guardarImagen(archivo_salida)) {
            std::cerr << "Error: No se pudo guardar la imagen filtrada" << std::endl;
            delete imagen_original;
            delete imagen_filtrada;
            return 1;
        }
        
        timer_guardado.stop();
        timer_guardado.printElapsed("Tiempo de guardado");
        
        delete imagen_original;
        delete imagen_filtrada;
        
    } else if (esFormatoPGM(archivo_entrada)) {
        std::cout << "Formato detectado: PGM (P2)" << std::endl;
        
        PGMImage* imagen_original = new PGMImage();
        
        // Cargar imagen
        std::cout << "Cargando imagen..." << std::endl;
        timer_carga.start();
        
        if (!imagen_original->cargarImagen(archivo_entrada)) {
            std::cerr << "Error: No se pudo cargar la imagen PGM" << std::endl;
            delete imagen_original;
            return 1;
        }
        
        timer_carga.stop();
        std::cout << "Dimensiones: " << imagen_original->getWidth() << "x" << imagen_original->getHeight() << std::endl;
        std::cout << "Píxeles totales: " << imagen_original->getWidth() * imagen_original->getHeight() << std::endl;
        timer_carga.printElapsed("Tiempo de carga");
        
        // Aplicar filtro
        std::cout << "Aplicando filtro " << Filter::filterTypeToString(filtro) << "..." << std::endl;
        timer_filtro.start();
        
        PGMImage* imagen_filtrada = Filter::aplicarFiltro(imagen_original, filtro);
        
        timer_filtro.stop();
        
        if (imagen_filtrada == nullptr) {
            std::cerr << "Error: No se pudo aplicar el filtro" << std::endl;
            delete imagen_original;
            return 1;
        }
        
        timer_filtro.printElapsed("Tiempo de filtrado");
        
        // Guardar imagen
        std::cout << "Guardando imagen filtrada..." << std::endl;
        timer_guardado.start();
        
        if (!imagen_filtrada->guardarImagen(archivo_salida)) {
            std::cerr << "Error: No se pudo guardar la imagen filtrada" << std::endl;
            delete imagen_original;
            delete imagen_filtrada;
            return 1;
        }
        
        timer_guardado.stop();
        timer_guardado.printElapsed("Tiempo de guardado");
        
        delete imagen_original;
        delete imagen_filtrada;
        
    } else {
        std::cerr << "Error: Formato de archivo no soportado o archivo corrupto" << std::endl;
        std::cerr << "Solo se soportan archivos PGM (P2) y PPM (P3)" << std::endl;
        return 1;
    }
    
    timer_total.stop();
    
    std::cout << std::endl << "=== Resumen de Tiempos ===" << std::endl;
    timer_carga.printElapsed("Carga");
    timer_filtro.printElapsed("Filtrado");
    timer_guardado.printElapsed("Guardado");
    timer_total.printElapsed("Total");
    
    std::cout << "Procesamiento completado exitosamente" << std::endl;
    return 0;
}