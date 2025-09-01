#include <iostream>
#include <cstring>
#include <cstdlib>
#include <mpi.h>
#include "PGMimage.h"
#include "PPMimage.h"
#include "filter.h"
#include "timer.h"

class FilterMPI {
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

const float FilterMPI::blur_kernel[3][3] = {
    {1.0f/9, 1.0f/9, 1.0f/9},
    {1.0f/9, 1.0f/9, 1.0f/9},
    {1.0f/9, 1.0f/9, 1.0f/9}
};

const float FilterMPI::laplace_kernel[3][3] = {
    { 0.0f, -1.0f,  0.0f},
    {-1.0f,  4.0f, -1.0f},
    { 0.0f, -1.0f,  0.0f}
};

const float FilterMPI::sharpening_kernel[3][3] = {
    { 0.0f, -1.0f,  0.0f},
    {-1.0f,  5.0f, -1.0f},
    { 0.0f, -1.0f,  0.0f}
};

const float (*FilterMPI::getKernel(FilterType tipo))[3] {
    switch (tipo) {
        case BLUR: return blur_kernel;
        case LAPLACE: return laplace_kernel;
        case SHARPENING: return sharpening_kernel;
        default: return blur_kernel;
    }
}

int FilterMPI::aplicarConvolucion(const int* pixels, int width, int height, 
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

int FilterMPI::aplicarConvolucionColor(const int* pixels, int width, int height,
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

void mostrarUso(const char* programa) {
    std::cout << "Uso: mpirun -np <num_procesos> " << programa << " <entrada> <salida> --f <filtro>" << std::endl;
    std::cout << "Ejemplo:" << std::endl;
    std::cout << "  mpirun -np 4 " << programa << " fruit.pgm fruit_blur.pgm --f blur" << std::endl;
    std::cout << "  mpirun -np 2 " << programa << " damma.ppm damma_sharp.ppm --f sharpening" << std::endl;
    std::cout << std::endl;
    std::cout << "Este programa distribuye el procesamiento de filtros entre procesos MPI" << std::endl;
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

// Función para aplicar filtro a una porción de imagen PGM
void procesarPortionPGM(int* result_portion, int width, int height, 
                       int start_row, int end_row, FilterType filtro, 
                       int max_color, int* full_image, int rank) {
    
    const float (*kernel)[3] = FilterMPI::getKernel(filtro);
    
    std::cout << "Proceso " << rank << " procesando filas " << start_row << " a " << end_row - 1 << std::endl;
    
    for (int row = start_row; row < end_row; row++) {
        for (int x = 0; x < width; x++) {
            int valor = FilterMPI::aplicarConvolucion(full_image, width, height, x, row, kernel, max_color);
            int local_idx = (row - start_row) * width + x;
            result_portion[local_idx] = valor;
        }
    }
}

// Función para aplicar filtro a una porción de imagen PPM
void procesarPortionPPM(int* result_portion, int width, int height,
                       int start_row, int end_row, FilterType filtro, 
                       int max_color, int* full_image, int rank) {
    
    const float (*kernel)[3] = FilterMPI::getKernel(filtro);
    
    std::cout << "Proceso " << rank << " procesando filas " << start_row << " a " << end_row - 1 << std::endl;
    
    for (int row = start_row; row < end_row; row++) {
        for (int x = 0; x < width; x++) {
            // Procesar cada canal (R, G, B)
            int r = FilterMPI::aplicarConvolucionColor(full_image, width, height, x, row, 0, kernel, max_color);
            int g = FilterMPI::aplicarConvolucionColor(full_image, width, height, x, row, 1, kernel, max_color);
            int b = FilterMPI::aplicarConvolucionColor(full_image, width, height, x, row, 2, kernel, max_color);
            
            int local_idx = ((row - start_row) * width + x) * 3;
            result_portion[local_idx] = r;
            result_portion[local_idx + 1] = g;
            result_portion[local_idx + 2] = b;
        }
    }
}

int main(int argc, char* argv[]) {
    int rank, size;
    
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    if (argc < 5) {
        if (rank == 0) {
            std::cout << "Error: Argumentos insuficientes" << std::endl;
            mostrarUso(argv[0]);
        }
        MPI_Finalize();
        return 1;
    }
    
    const char* archivo_entrada = argv[1];
    const char* archivo_salida = argv[2];
    const char* flag_filtro = argv[3];
    const char* nombre_filtro = argv[4];
    
    if (strcmp(flag_filtro, "--f") != 0) {
        if (rank == 0) {
            std::cout << "Error: Flag de filtro incorrecto. Use --f" << std::endl;
            mostrarUso(argv[0]);
        }
        MPI_Finalize();
        return 1;
    }
    
    FilterType filtro = Filter::stringToFilterType(nombre_filtro);
    
    Timer timer_total, timer_carga, timer_comunicacion, timer_filtro, timer_guardado;
    
    if (rank == 0) {
        std::cout << "=== Filterer con MPI (" << size << " procesos) ===" << std::endl;
        std::cout << "Archivo de entrada: " << archivo_entrada << std::endl;
        std::cout << "Archivo de salida: " << archivo_salida << std::endl;
        std::cout << "Filtro: " << Filter::filterTypeToString(filtro) << std::endl;
        std::cout << std::endl;
    }
    
    timer_total.start();
    
    // Variables para almacenar información de la imagen
    int width = 0, height = 0, max_color = 0, pixel_count = 0;
    int* full_image = nullptr;
    bool es_ppm = false;
    
    if (rank == 0) {
        // El proceso maestro carga la imagen
        if (esFormatoPPM(archivo_entrada)) {
            es_ppm = true;
            std::cout << "Formato detectado: PPM (P3)" << std::endl;
            
            PPMImage imagen_original;
            timer_carga.start();
            
            if (!imagen_original.cargarImagen(archivo_entrada)) {
                std::cerr << "Error: No se pudo cargar la imagen PPM" << std::endl;
                MPI_Abort(MPI_COMM_WORLD, 1);
            }
            
            timer_carga.stop();
            
            width = imagen_original.getWidth();
            height = imagen_original.getHeight();
            max_color = imagen_original.getMaxColor();
            pixel_count = imagen_original.getPixelCount();
            
            std::cout << "Dimensiones: " << width << "x" << height << std::endl;
            timer_carga.printElapsed("Tiempo de carga");
            
            // Copiar píxeles
            full_image = (int*)malloc(pixel_count * sizeof(int));
            memcpy(full_image, imagen_original.getPixels(), pixel_count * sizeof(int));
            
        } else if (esFormatoPGM(archivo_entrada)) {
            es_ppm = false;
            std::cout << "Formato detectado: PGM (P2)" << std::endl;
            
            PGMImage imagen_original;
            timer_carga.start();
            
            if (!imagen_original.cargarImagen(archivo_entrada)) {
                std::cerr << "Error: No se pudo cargar la imagen PGM" << std::endl;
                MPI_Abort(MPI_COMM_WORLD, 1);
            }
            
            timer_carga.stop();
            
            width = imagen_original.getWidth();
            height = imagen_original.getHeight();
            max_color = imagen_original.getMaxColor();
            pixel_count = imagen_original.getPixelCount();
            
            std::cout << "Dimensiones: " << width << "x" << height << std::endl;
            timer_carga.printElapsed("Tiempo de carga");
            
            // Copiar píxeles
            full_image = (int*)malloc(pixel_count * sizeof(int));
            memcpy(full_image, imagen_original.getPixels(), pixel_count * sizeof(int));
            
        } else {
            std::cerr << "Error: Formato no soportado" << std::endl;
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
    }
    
    // Broadcast de información básica
    timer_comunicacion.start();
    MPI_Bcast(&width, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&height, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&max_color, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&pixel_count, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&es_ppm, 1, MPI_C_BOOL, 0, MPI_COMM_WORLD);
    
    // Todos los procesos necesitan la imagen completa para el filtro (por los bordes)
    if (rank != 0) {
        full_image = (int*)malloc(pixel_count * sizeof(int));
    }
    
    MPI_Bcast(full_image, pixel_count, MPI_INT, 0, MPI_COMM_WORLD);
    timer_comunicacion.stop();
    
    if (rank == 0) {
        timer_comunicacion.printElapsed("Tiempo de comunicación inicial");
        std::cout << std::endl << "Iniciando procesamiento distribuido..." << std::endl;
    }
    
    // Dividir trabajo por filas
    int rows_per_process = height / size;
    int extra_rows = height % size;
    
    int start_row = rank * rows_per_process + std::min(rank, extra_rows);
    int end_row = start_row + rows_per_process + (rank < extra_rows ? 1 : 0);
    int local_rows = end_row - start_row;
    
    // Aplicar filtro especificado
    int local_size = es_ppm ? local_rows * width * 3 : local_rows * width;
    int* local_result = (int*)malloc(local_size * sizeof(int));
    
    timer_filtro.start();
    
    if (es_ppm) {
        procesarPortionPPM(local_result, width, height, start_row, end_row, 
                          filtro, max_color, full_image, rank);
    } else {
        procesarPortionPGM(local_result, width, height, start_row, end_row,
                          filtro, max_color, full_image, rank);
    }
    
    timer_filtro.stop();
    
    if (rank == 0) {
        timer_filtro.printElapsed("Tiempo de filtrado distribuido");
    }
    
    // Recopilar resultados en el proceso maestro
    timer_comunicacion.reset();
    timer_comunicacion.start();
    
    int* final_result = nullptr;
    if (rank == 0) {
        final_result = (int*)malloc(pixel_count * sizeof(int));
    }
    
    // Calcular desplazamientos y tamaños para Gatherv
    int* recvcounts = nullptr;
    int* displs = nullptr;
    
    if (rank == 0) {
        recvcounts = (int*)malloc(size * sizeof(int));
        displs = (int*)malloc(size * sizeof(int));
        
        for (int i = 0; i < size; i++) {
            int i_start_row = i * rows_per_process + std::min(i, extra_rows);
            int i_end_row = i_start_row + rows_per_process + (i < extra_rows ? 1 : 0);
            int i_local_rows = i_end_row - i_start_row;
            
            recvcounts[i] = es_ppm ? i_local_rows * width * 3 : i_local_rows * width;
            displs[i] = es_ppm ? i_start_row * width * 3 : i_start_row * width;
        }
    }
    
    MPI_Gatherv(local_result, local_size, MPI_INT,
                final_result, recvcounts, displs, MPI_INT,
                0, MPI_COMM_WORLD);
    
    timer_comunicacion.stop();
    
    if (rank == 0) {
        timer_comunicacion.printElapsed("Tiempo de comunicación final");
        
        std::cout << std::endl << "Guardando resultado..." << std::endl;
        
        // Guardar resultado
        timer_guardado.start();
        
        bool guardado_exitoso = false;
        
        if (es_ppm) {
            // Cargar imagen original como template
            PPMImage temp_image;
            if (temp_image.cargarImagen(archivo_entrada)) {
                PPMImage* resultado = temp_image.crearImagenVacia();
                if (resultado != nullptr) {
                    // Copiar los datos procesados
                    memcpy(resultado->getPixels(), final_result, pixel_count * sizeof(int));
                    
                    if (resultado->guardarImagen(archivo_salida)) {
                        guardado_exitoso = true;
                    } else {
                        std::cerr << "Error guardando imagen PPM" << std::endl;
                    }
                    
                    delete resultado;
                }
            }
        } else {
            // Cargar imagen original como template
            PGMImage temp_image;
            if (temp_image.cargarImagen(archivo_entrada)) {
                PGMImage* resultado = temp_image.crearImagenVacia();
                if (resultado != nullptr) {
                    // Copiar los datos procesados
                    memcpy(resultado->getPixels(), final_result, pixel_count * sizeof(int));
                    
                    if (resultado->guardarImagen(archivo_salida)) {
                        guardado_exitoso = true;
                    } else {
                        std::cerr << "Error guardando imagen PGM" << std::endl;
                    }
                    
                    delete resultado;
                }
            }
        }
        
        timer_guardado.stop();
        
        if (guardado_exitoso) {
            timer_guardado.printElapsed("Tiempo de guardado");
        }
        
        free(recvcounts);
        free(displs);
    }
    
    timer_total.stop();
    
    if (rank == 0) {
        std::cout << std::endl << "=== Resumen de Tiempos (Proceso Maestro) ===" << std::endl;
        timer_carga.printElapsed("Carga");
        timer_filtro.printElapsed("Filtrado distribuido");
        timer_guardado.printElapsed("Guardado");
        timer_total.printElapsed("Total");
        
        std::cout << "Procesamiento completado exitosamente con " << size << " procesos MPI" << std::endl;
    }
    
    // Limpiar memoria
    free(full_image);
    free(local_result);
    if (rank == 0 && final_result) {
        free(final_result);
    }
    
    MPI_Finalize();
    return 0;
}