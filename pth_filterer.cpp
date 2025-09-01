#include <iostream>
#include <cstring>
#include <cstdlib>
#include <pthread.h>
#include "PGMimage.h"
#include "PPMimage.h"
#include "filter.h"
#include "timer.h"

#define NUM_THREADS 4

enum RegionType {
    TOP_LEFT = 0,
    TOP_RIGHT = 1,
    BOTTOM_LEFT = 2,
    BOTTOM_RIGHT = 3
};

struct ThreadData {
    const int* pixels_entrada;
    int* pixels_salida;
    int width;
    int height;
    int max_color;
    int start_x, start_y;
    int end_x, end_y;
    FilterType filtro;
    bool es_color; // true para PPM, false para PGM
    RegionType region;
    int thread_id;
    Timer* timer;
};

class FilterPthread {
public:
    static const float blur_kernel[3][3];
    static const float laplace_kernel[3][3];
    static const float sharpening_kernel[3][3];
    
    static const float (*getKernel(FilterType tipo))[3];
    static int aplicarConvolucion(const int* pixels, int width, int height, 
                                 int x, int y, const float kernel[3][3], int max_color);
    static int aplicarConvolucionColor(const int* pixels, int width, int height,
                                      int x, int y, int canal, const float kernel[3][3], int max_color);
};

const float FilterPthread::blur_kernel[3][3] = {
    {1.0f/9, 1.0f/9, 1.0f/9},
    {1.0f/9, 1.0f/9, 1.0f/9},
    {1.0f/9, 1.0f/9, 1.0f/9}
};

const float FilterPthread::laplace_kernel[3][3] = {
    { 0.0f, -1.0f,  0.0f},
    {-1.0f,  4.0f, -1.0f},
    { 0.0f, -1.0f,  0.0f}
};

const float FilterPthread::sharpening_kernel[3][3] = {
    { 0.0f, -1.0f,  0.0f},
    {-1.0f,  5.0f, -1.0f},
    { 0.0f, -1.0f,  0.0f}
};

const float (*FilterPthread::getKernel(FilterType tipo))[3] {
    switch (tipo) {
        case BLUR: return blur_kernel;
        case LAPLACE: return laplace_kernel;
        case SHARPENING: return sharpening_kernel;
        default: return blur_kernel;
    }
}

int FilterPthread::aplicarConvolucion(const int* pixels, int width, int height, 
                                     int x, int y, const float kernel[3][3], int max_color) {
    float sum = 0.0f;
    float weight_sum = 0.0f;
    
    for (int ky = -1; ky <= 1; ky++) {
        for (int kx = -1; kx <= 1; kx++) {
            int nx = x + kx;
            int ny = y + ky;
            
            if (nx >= 0 && nx < width && ny >= 0 && ny < height) {
                int idx = ny * width + nx;
                float kernel_val = kernel[ky + 1][kx + 1];
                sum += pixels[idx] * kernel_val;
                weight_sum += (kernel_val > 0) ? kernel_val : 0;
            }
        }
    }
    
    int result;
    if (weight_sum > 0) {
        result = static_cast<int>(sum / weight_sum);
    } else {
        result = static_cast<int>(sum);
    }
    
    return std::max(0, std::min(max_color, result));
}

int FilterPthread::aplicarConvolucionColor(const int* pixels, int width, int height,
                                          int x, int y, int canal, const float kernel[3][3], int max_color) {
    float sum = 0.0f;
    float weight_sum = 0.0f;
    
    for (int ky = -1; ky <= 1; ky++) {
        for (int kx = -1; kx <= 1; kx++) {
            int nx = x + kx;
            int ny = y + ky;
            
            if (nx >= 0 && nx < width && ny >= 0 && ny < height) {
                int idx = (ny * width + nx) * 3 + canal;
                float kernel_val = kernel[ky + 1][kx + 1];
                sum += pixels[idx] * kernel_val;
                weight_sum += (kernel_val > 0) ? kernel_val : 0;
            }
        }
    }
    
    int result;
    if (weight_sum > 0) {
        result = static_cast<int>(sum / weight_sum);
    } else {
        result = static_cast<int>(sum);
    }
    
    return std::max(0, std::min(max_color, result));
}

void* procesarRegion(void* arg) {
    ThreadData* data = static_cast<ThreadData*>(arg);
    
    data->timer->start();
    
    const float (*kernel)[3] = FilterPthread::getKernel(data->filtro);
    
    std::cout << "Thread " << data->thread_id << " procesando región (" 
              << data->start_x << "," << data->start_y << ") a (" 
              << data->end_x << "," << data->end_y << ")" << std::endl;
    
    if (data->es_color) {
        // Procesar imagen PPM
        for (int y = data->start_y; y < data->end_y; y++) {
            for (int x = data->start_x; x < data->end_x; x++) {
                int r = FilterPthread::aplicarConvolucionColor(data->pixels_entrada, data->width, data->height, x, y, 0, kernel, data->max_color);
                int g = FilterPthread::aplicarConvolucionColor(data->pixels_entrada, data->width, data->height, x, y, 1, kernel, data->max_color);
                int b = FilterPthread::aplicarConvolucionColor(data->pixels_entrada, data->width, data->height, x, y, 2, kernel, data->max_color);
                
                int base_idx = (y * data->width + x) * 3;
                data->pixels_salida[base_idx] = r;
                data->pixels_salida[base_idx + 1] = g;
                data->pixels_salida[base_idx + 2] = b;
            }
        }
    } else {
        // Procesar imagen PGM
        for (int y = data->start_y; y < data->end_y; y++) {
            for (int x = data->start_x; x < data->end_x; x++) {
                int valor = FilterPthread::aplicarConvolucion(data->pixels_entrada, data->width, data->height, x, y, kernel, data->max_color);
                int idx = y * data->width + x;
                data->pixels_salida[idx] = valor;
            }
        }
    }
    
    data->timer->stop();
    
    std::cout << "Thread " << data->thread_id << " completado" << std::endl;
    
    pthread_exit(nullptr);
}

void mostrarUso(const char* programa) {
    std::cout << "Uso: " << programa << " <entrada> <salida> --f <filtro>" << std::endl;
    std::cout << "Ejemplo:" << std::endl;
    std::cout << "  " << programa << " fruit.pgm fruit_blur2.pgm --f blur" << std::endl;
    std::cout << "  " << programa << " damma.ppm damma_sharp.ppm --f sharpening" << std::endl;
    std::cout << std::endl;
    std::cout << "Este programa usa 4 threads para procesar 4 regiones de la imagen" << std::endl;
}

bool esFormatoPPM(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (file == nullptr) return false;
    
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
    if (file == nullptr) return false;
    
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
    
    if (strcmp(flag_filtro, "--f") != 0) {
        std::cout << "Error: Flag de filtro incorrecto. Use --f" << std::endl;
        mostrarUso(argv[0]);
        return 1;
    }
    
    FilterType filtro = Filter::stringToFilterType(nombre_filtro);
    
    Timer timer_total, timer_carga, timer_filtro, timer_guardado;
    Timer timers_threads[NUM_THREADS];
    
    std::cout << "=== Filterer con Pthreads (" << NUM_THREADS << " threads) ===" << std::endl;
    std::cout << "Archivo de entrada: " << archivo_entrada << std::endl;
    std::cout << "Archivo de salida: " << archivo_salida << std::endl;
    std::cout << "Filtro: " << Filter::filterTypeToString(filtro) << std::endl;
    std::cout << std::endl;
    
    timer_total.start();
    
    if (esFormatoPPM(archivo_entrada)) {
        std::cout << "Formato detectado: PPM (P3)" << std::endl;
        
        PPMImage imagen_original;
        timer_carga.start();
        
        if (!imagen_original.cargarImagen(archivo_entrada)) {
            std::cerr << "Error: No se pudo cargar la imagen PPM" << std::endl;
            return 1;
        }
        
        timer_carga.stop();
        
        int width = imagen_original.getWidth();
        int height = imagen_original.getHeight();
        int max_color = imagen_original.getMaxColor();
        
        std::cout << "Dimensiones: " << width << "x" << height << std::endl;
        timer_carga.printElapsed("Tiempo de carga");
        
        // Crear imagen de salida
        PPMImage* imagen_salida = imagen_original.crearImagenVacia();
        
        // Configurar threads
        pthread_t threads[NUM_THREADS];
        ThreadData thread_data[NUM_THREADS];
        
        int mid_x = width / 2;
        int mid_y = height / 2;
        
        // Thread 0: Top-left
        thread_data[0] = {imagen_original.getPixels(), imagen_salida->getPixels(), 
                          width, height, max_color, 0, 0, mid_x, mid_y, 
                          filtro, true, TOP_LEFT, 0, &timers_threads[0]};
        
        // Thread 1: Top-right
        thread_data[1] = {imagen_original.getPixels(), imagen_salida->getPixels(), 
                          width, height, max_color, mid_x, 0, width, mid_y, 
                          filtro, true, TOP_RIGHT, 1, &timers_threads[1]};
        
        // Thread 2: Bottom-left
        thread_data[2] = {imagen_original.getPixels(), imagen_salida->getPixels(), 
                          width, height, max_color, 0, mid_y, mid_x, height, 
                          filtro, true, BOTTOM_LEFT, 2, &timers_threads[2]};
        
        // Thread 3: Bottom-right
        thread_data[3] = {imagen_original.getPixels(), imagen_salida->getPixels(), 
                          width, height, max_color, mid_x, mid_y, width, height, 
                          filtro, true, BOTTOM_RIGHT, 3, &timers_threads[3]};
        
        std::cout << "Iniciando procesamiento paralelo..." << std::endl;
        timer_filtro.start();
        
        // Crear threads
        for (int i = 0; i < NUM_THREADS; i++) {
            if (pthread_create(&threads[i], nullptr, procesarRegion, &thread_data[i]) != 0) {
                std::cerr << "Error creando thread " << i << std::endl;
                delete imagen_salida;
                return 1;
            }
        }
        
        // Esperar threads
        for (int i = 0; i < NUM_THREADS; i++) {
            pthread_join(threads[i], nullptr);
        }
        
        timer_filtro.stop();
        timer_filtro.printElapsed("Tiempo de filtrado paralelo");
        
        // Guardar imagen
        timer_guardado.start();
        if (!imagen_salida->guardarImagen(archivo_salida)) {
            std::cerr << "Error guardando imagen" << std::endl;
            delete imagen_salida;
            return 1;
        }
        timer_guardado.stop();
        
        delete imagen_salida;
        
    } else if (esFormatoPGM(archivo_entrada)) {
        std::cout << "Formato detectado: PGM (P2)" << std::endl;
        
        PGMImage imagen_original;
        timer_carga.start();
        
        if (!imagen_original.cargarImagen(archivo_entrada)) {
            std::cerr << "Error: No se pudo cargar la imagen PGM" << std::endl;
            return 1;
        }
        
        timer_carga.stop();
        
        int width = imagen_original.getWidth();
        int height = imagen_original.getHeight();
        int max_color = imagen_original.getMaxColor();
        
        std::cout << "Dimensiones: " << width << "x" << height << std::endl;
        timer_carga.printElapsed("Tiempo de carga");
        
        // Crear imagen de salida
        PGMImage* imagen_salida = imagen_original.crearImagenVacia();
        
        // Configurar threads
        pthread_t threads[NUM_THREADS];
        ThreadData thread_data[NUM_THREADS];
        
        int mid_x = width / 2;
        int mid_y = height / 2;
        
        // Thread 0: Top-left
        thread_data[0] = {imagen_original.getPixels(), imagen_salida->getPixels(), 
                          width, height, max_color, 0, 0, mid_x, mid_y, 
                          filtro, false, TOP_LEFT, 0, &timers_threads[0]};
        
        // Thread 1: Top-right
        thread_data[1] = {imagen_original.getPixels(), imagen_salida->getPixels(), 
                          width, height, max_color, mid_x, 0, width, mid_y, 
                          filtro, false, TOP_RIGHT, 1, &timers_threads[1]};
        
        // Thread 2: Bottom-left
        thread_data[2] = {imagen_original.getPixels(), imagen_salida->getPixels(), 
                          width, height, max_color, 0, mid_y, mid_x, height, 
                          filtro, false, BOTTOM_LEFT, 2, &timers_threads[2]};
        
        // Thread 3: Bottom-right
        thread_data[3] = {imagen_original.getPixels(), imagen_salida->getPixels(), 
                          width, height, max_color, mid_x, mid_y, width, height, 
                          filtro, false, BOTTOM_RIGHT, 3, &timers_threads[3]};
        
        std::cout << "Iniciando procesamiento paralelo..." << std::endl;
        timer_filtro.start();
        
        // Crear threads
        for (int i = 0; i < NUM_THREADS; i++) {
            if (pthread_create(&threads[i], nullptr, procesarRegion, &thread_data[i]) != 0) {
                std::cerr << "Error creando thread " << i << std::endl;
                delete imagen_salida;
                return 1;
            }
        }
        
        // Esperar threads
        for (int i = 0; i < NUM_THREADS; i++) {
            pthread_join(threads[i], nullptr);
        }
        
        timer_filtro.stop();
        timer_filtro.printElapsed("Tiempo de filtrado paralelo");
        
        // Guardar imagen
        timer_guardado.start();
        if (!imagen_salida->guardarImagen(archivo_salida)) {
            std::cerr << "Error guardando imagen" << std::endl;
            delete imagen_salida;
            return 1;
        }
        timer_guardado.stop();
        
        delete imagen_salida;
        
    } else {
        std::cerr << "Error: Formato no soportado" << std::endl;
        return 1;
    }
    
    timer_total.stop();
    
    std::cout << std::endl << "=== Resumen de Tiempos ===" << std::endl;
    timer_carga.printElapsed("Carga");
    timer_filtro.printElapsed("Filtrado paralelo");
    timer_guardado.printElapsed("Guardado");
    
    std::cout << std::endl << "=== Tiempos por Thread ===" << std::endl;
    for (int i = 0; i < NUM_THREADS; i++) {
        std::cout << "Thread " << i << ": " << timers_threads[i].getElapsedMilliseconds() << " ms" << std::endl;
    }
    
    timer_total.printElapsed("Total");
    
    std::cout << "Procesamiento completado exitosamente" << std::endl;
    return 0;
}