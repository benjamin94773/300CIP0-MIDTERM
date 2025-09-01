#include "PPMimage.h"
#include <algorithm>

PPMImage::PPMImage() : Imagen() {
}

PPMImage::~PPMImage() {
    // El destructor de la clase base se encarga de liberar la memoria
}

bool PPMImage::cargarImagen(const char* filename) {
    FILE* file = fopen(filename, "r");
    std::cout << "DEBUG: intentando abrir [" << filename << "]" << std::endl;
    if (file == nullptr) {
        std::cerr << "Error: No se pudo abrir el archivo " << filename << std::endl;
        return false;
    }
    
    // Leer cabecera
    if (!leerCabecera(file)) {
        std::cerr << "Error: No se pudo leer la cabecera del archivo PPM" << std::endl;
        fclose(file);
        return false;
    }
    
    // Verificar que sea formato PPM
    if (strcmp(magic, "P3") != 0) {
        std::cerr << "Error: El archivo no es formato PPM (P3)" << std::endl;
        fclose(file);
        return false;
    }
    
    // Calcular número de píxeles (3 valores por píxel: R, G, B)
    pixel_count = width * height * 3;
    
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

bool PPMImage::guardarImagen(const char* filename) {
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

RGB PPMImage::getPixelRGB(int x, int y) const {
    if (!validarCoordenadas(x, y) || pixels == nullptr) {
        return RGB(0, 0, 0);
    }
    
    int base_index = (y * width + x) * 3;
    return RGB(pixels[base_index], pixels[base_index + 1], pixels[base_index + 2]);
}

void PPMImage::setPixelRGB(int x, int y, const RGB& color) {
    setPixelRGB(x, y, color.r, color.g, color.b);
}

void PPMImage::setPixelRGB(int x, int y, int r, int g, int b) {
    if (!validarCoordenadas(x, y) || pixels == nullptr) {
        return;
    }
    
    // Asegurar que los valores estén dentro del rango válido
    r = std::max(0, std::min(max_color, r));
    g = std::max(0, std::min(max_color, g));
    b = std::max(0, std::min(max_color, b));
    
    int base_index = (y * width + x) * 3;
    pixels[base_index] = r;
    pixels[base_index + 1] = g;
    pixels[base_index + 2] = b;
}

int PPMImage::getRed(int x, int y) const {
    if (!validarCoordenadas(x, y) || pixels == nullptr) {
        return 0;
    }
    return pixels[getColorIndex(x, y, 0)];
}

int PPMImage::getGreen(int x, int y) const {
    if (!validarCoordenadas(x, y) || pixels == nullptr) {
        return 0;
    }
    return pixels[getColorIndex(x, y, 1)];
}

int PPMImage::getBlue(int x, int y) const {
    if (!validarCoordenadas(x, y) || pixels == nullptr) {
        return 0;
    }
    return pixels[getColorIndex(x, y, 2)];
}

PPMImage* PPMImage::crearImagenVacia() const {
    PPMImage* nueva = new PPMImage();
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

int PPMImage::getColorIndex(int x, int y, int component) const {
    return (y * width + x) * 3 + component;
}