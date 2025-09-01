#include <iostream>
#include <cstring>
#include <string>
#include <omp.h>
#include "PGMimage.h"
#include "PPMimage.h"
#include "filter.h"
#include "timer.h"

void mostrarUso(const char* programa) {
    std::cout << "Uso: " << programa << " <imagen_entrada>" << std::endl;
    std::cout << "Ejemplo:" << std::endl;
    std::cout << "  " << programa << " sulfur.pgm" << std::endl;
    std::cout << "  " << programa << " imagen.ppm" << std::endl;
    std::cout << std::endl;
    std::cout << "Nota: Se generarán 3 archivos de salida con los filtros aplicados:" << std::endl;
    std::cout << "  - imagen_blur.ext" << std::endl;
    std::cout << "  - imagen_laplace.ext" << std::endl;
    std::cout << "  - imagen_sharpening.ext" << std::endl;
}

std::string construirNombreSalida(const char* entrada, const char* filtro) {
    std::string nombre(entrada);
    
    // Encontrar la posición del último punto para separar nombre y extensión
    size_t punto_pos = nombre.find_last_of('.');
    
    if (punto_pos != std::string::npos) {
        // Insertar el filtro antes de la extensión
        return nombre.substr(0, punto_pos) + "_" + filtro + nombre.substr(punto_pos);
    } else {
        // Si no hay extensión, simplemente agregar el filtro
        return nombre + "_" + filtro;
    }
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
    if (argc < 2) {
        std::cout << "Error: Falta el archivo de entrada" << std::endl;
        mostrarUso(argv[0]);
        return 1;
    }
    
    const char* archivo_entrada = argv[1];
    
    // Configurar OpenMP para usar 3 hilos
    omp_set_num_threads(3);
    
    Timer timer_total;
    Timer timer_carga;
    
    std::cout << "=== Filtrador de Imágenes con OpenMP ===" << std::endl;
    std::cout << "Archivo de entrada: " << archivo_entrada << std::endl;
    std::cout << "Número de hilos configurados: " << omp_get_max_threads() << std::endl;
    
    timer_total.start();
    
    // Determinar el formato del archivo y procesar
    if (esFormatoPPM(archivo_entrada)) {
        std::cout << "Formato detectado: PPM (P3)" << std::endl;
        
        // Cargar imagen PPM original
        PPMImage imagen_original;
        
        std::cout << "Cargando imagen PPM..." << std::endl;
        timer_carga.start();
        
        if (!imagen_original.cargarImagen(archivo_entrada)) {
            std::cerr << "Error: No se pudo cargar la imagen PPM" << std::endl;
            return 1;
        }
        
        timer_carga.stop();
        std::cout << "Imagen cargada correctamente" << std::endl;
        std::cout << "Dimensiones: " << imagen_original.getWidth() << "x" << imagen_original.getHeight() << std::endl;
        timer_carga.printElapsed("Tiempo de carga");
        
        // Arrays para almacenar resultados y nombres de archivos
        PPMImage* resultados[3] = {nullptr, nullptr, nullptr};
        std::string nombres_salida[3];
        const char* nombres_filtros[3] = {"blur", "laplace", "sharpening"};
        FilterType tipos_filtros[3] = {BLUR, LAPLACE, SHARPENING};
        
        // Construir nombres de archivos de salida
        for (int i = 0; i < 3; i++) {
            nombres_salida[i] = construirNombreSalida(archivo_entrada, nombres_filtros[i]);
        }
        
        // Aplicar filtros en paralelo
        std::cout << "Aplicando filtros en paralelo..." << std::endl;
        Timer timer_filtros;
        timer_filtros.start();
        
        #pragma omp parallel for
        for (int i = 0; i < 3; i++) {
            int thread_id = omp_get_thread_num();
            std::cout << "Hilo " << thread_id << " aplicando filtro " << nombres_filtros[i] << std::endl;
            
            resultados[i] = Filter::aplicarFiltro(&imagen_original, tipos_filtros[i]);
            
            if (resultados[i] != nullptr) {
                std::cout << "Hilo " << thread_id << " completó filtro " << nombres_filtros[i] << std::endl;
            } else {
                std::cerr << "Error en hilo " << thread_id << " aplicando filtro " << nombres_filtros[i] << std::endl;
            }
        }
        
        timer_filtros.stop();
        timer_filtros.printElapsed("Tiempo de aplicación de filtros");
        
        // Guardar imágenes resultantes
        std::cout << "Guardando imágenes filtradas..." << std::endl;
        Timer timer_guardado;
        timer_guardado.start();
        
        bool todas_guardadas = true;
        for (int i = 0; i < 3; i++) {
            if (resultados[i] != nullptr) {
                if (resultados[i]->guardarImagen(nombres_salida[i].c_str())) {
                    std::cout << "Guardada: " << nombres_salida[i] << std::endl;
                } else {
                    std::cerr << "Error guardando: " << nombres_salida[i] << std::endl;
                    todas_guardadas = false;
                }
                delete resultados[i];
            } else {
                todas_guardadas = false;
            }
        }
        
        timer_guardado.stop();
        timer_guardado.printElapsed("Tiempo de guardado");
        
        if (!todas_guardadas) {
            std::cerr << "Error: No se pudieron guardar todas las imágenes" << std::endl;
            return 1;
        }
        
    } else if (esFormatoPGM(archivo_entrada)) {
        std::cout << "Formato detectado: PGM (P2)" << std::endl;
        
        // Cargar imagen PGM original
        PGMImage imagen_original;
        
        std::cout << "Cargando imagen PGM..." << std::endl;
        timer_carga.start();
        
        if (!imagen_original.cargarImagen(archivo_entrada)) {
            std::cerr << "Error: No se pudo cargar la imagen PGM" << std::endl;
            return 1;
        }
        
        timer_carga.stop();
        std::cout << "Imagen cargada correctamente" << std::endl;
        std::cout << "Dimensiones: " << imagen_original.getWidth() << "x" << imagen_original.getHeight() << std::endl;
        timer_carga.printElapsed("Tiempo de carga");
        
        // Arrays para almacenar resultados y nombres de archivos
        PGMImage* resultados[3] = {nullptr, nullptr, nullptr};
        std::string nombres_salida[3];
        const char* nombres_filtros[3] = {"blur", "laplace", "sharpening"};
        FilterType tipos_filtros[3] = {BLUR, LAPLACE, SHARPENING};
        
        // Construir nombres de archivos de salida
        for (int i = 0; i < 3; i++) {
            nombres_salida[i] = construirNombreSalida(archivo_entrada, nombres_filtros[i]);
        }
        
        // Aplicar filtros en paralelo
        std::cout << "Aplicando filtros en paralelo..." << std::endl;
        Timer timer_filtros;
        timer_filtros.start();
        
        #pragma omp parallel for
        for (int i = 0; i < 3; i++) {
            int thread_id = omp_get_thread_num();
            std::cout << "Hilo " << thread_id << " aplicando filtro " << nombres_filtros[i] << std::endl;
            
            resultados[i] = Filter::aplicarFiltro(&imagen_original, tipos_filtros[i]);
            
            if (resultados[i] != nullptr) {
                std::cout << "Hilo " << thread_id << " completó filtro " << nombres_filtros[i] << std::endl;
            } else {
                std::cerr << "Error en hilo " << thread_id << " aplicando filtro " << nombres_filtros[i] << std::endl;
            }
        }
        
        timer_filtros.stop();
        timer_filtros.printElapsed("Tiempo de aplicación de filtros");
        
        // Guardar imágenes resultantes
        std::cout << "Guardando imágenes filtradas..." << std::endl;
        Timer timer_guardado;
        timer_guardado.start();
        
        bool todas_guardadas = true;
        for (int i = 0; i < 3; i++) {
            if (resultados[i] != nullptr) {
                if (resultados[i]->guardarImagen(nombres_salida[i].c_str())) {
                    std::cout << "Guardada: " << nombres_salida[i] << std::endl;
                } else {
                    std::cerr << "Error guardando: " << nombres_salida[i] << std::endl;
                    todas_guardadas = false;
                }
                delete resultados[i];
            } else {
                todas_guardadas = false;
            }
        }
        
        timer_guardado.stop();
        timer_guardado.printElapsed("Tiempo de guardado");
        
        if (!todas_guardadas) {
            std::cerr << "Error: No se pudieron guardar todas las imágenes" << std::endl;
            return 1;
        }
        
    } else {
        std::cerr << "Error: Formato de archivo no soportado o archivo corrupto" << std::endl;
        std::cerr << "Solo se soportan archivos PGM (P2) y PPM (P3)" << std::endl;
        return 1;
    }
    
    timer_total.stop();
    timer_total.printElapsed("Tiempo total de ejecución");
    
    std::cout << "Procesamiento paralelo completado exitosamente" << std::endl;
    std::cout << "Se generaron 3 imágenes con los filtros aplicados" << std::endl;
    
    return 0;
}