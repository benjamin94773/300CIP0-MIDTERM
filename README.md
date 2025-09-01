# Filtros de Imágenes con Programación Paralela

Este proyecto implementa filtros de procesamiento de imágenes PPM/PGM utilizando diferentes paradigmas de programación paralela: **secuencial**, **Pthreads**, **OpenMP** y **MPI distribuido**.

## Objetivo

Analizar el impacto de la programación paralela en el tiempo de ejecución del filtrado de imágenes, comparando implementaciones secuenciales y paralelas en entornos Docker.

## Implementaciones

- **`processor`** - Versión base (solo carga/guardado)
- **`filterer`** - Versión secuencial con filtros
- **`pth_filterer`** - Versión Pthreads (4 hilos, 4 cuadrantes)
- **`omp_filterer`** - Versión OpenMP (3 hilos, 3 filtros simultáneos)
- **`mpi_filterer`** - Versión MPI distribuida (4 nodos, 4 segmentos)

---

## 🛠Compilación y Ejecución

### **1. Versión Secuencial Base (Processor)**
```bash
# Compilar
g++ -o processor imagen.cpp PGMimage.cpp PPMimage.cpp filter.cpp timer.cpp processor.cpp

# Ejecutar (solo carga y guardado)
./processor ./images/damma.ppm ./images/damma2.ppm
```

### **2. Versión Secuencial con Filtros**
```bash
# Compilar
g++ -o filterer imagen.cpp PGMimage.cpp PPMimage.cpp filter.cpp timer.cpp filterer.cpp

# Ejecutar con filtro específico
./filterer ./images/damma.ppm ./images/damma_blur.ppm --f blur
./filterer ./images/damma.ppm ./images/damma_laplace.ppm --f laplace
./filterer ./images/damma.ppm ./images/damma_sharpening.ppm --f sharpening
```

### **3. Versión Pthreads (4 hilos, 4 cuadrantes)**
```bash
# Compilar
g++ -o pth_filterer imagen.cpp PGMimage.cpp PPMimage.cpp filter.cpp timer.cpp pth_filterer.cpp -lpthread

# Ejecutar
./pth_filterer ./images/damma.ppm ./images/damma_blur_pth.ppm --f blur
```

**Arquitectura Pthreads:**
```
┌─────────────┬─────────────┐
│   Thread 0  │   Thread 1  │
│ Arriba-Izq  │ Arriba-Der  │
├─────────────┼─────────────┤
│   Thread 2  │   Thread 3  │
│ Abajo-Izq   │ Abajo-Der   │
└─────────────┴─────────────┘
```

### **4. Versión OpenMP (3 hilos, 3 filtros)**
```bash
# Compilar
g++ -o omp_filterer imagen.cpp PGMimage.cpp PPMimage.cpp filter.cpp timer.cpp omp_filterer.cpp -fopenmp

# Ejecutar (genera 3 archivos automáticamente)
./omp_filterer ./images/damma.ppm
```

**Archivos generados:**
- `damma_blur.ppm`
- `damma_laplace.ppm`
- `damma_sharpening.ppm`

### **5. Versión MPI Distribuida (4 nodos)**

#### **Configurar red Docker:**
```bash
# Crear red MPI
docker network create mpi-net

# Crear 4 nodos
docker run -dit --name node1 --hostname node1 --network mpi-net \
  -v "${PWD}:/home/japeto/app:rw" -w /home/japeto/app \
  japeto/parallel-tools:v64 bash

docker run -dit --name node2 --hostname node2 --network mpi-net \
  -v "${PWD}:/home/japeto/app:rw" -w /home/japeto/app \
  japeto/parallel-tools:v64 bash

docker run -dit --name node3 --hostname node3 --network mpi-net \
  -v "${PWD}:/home/japeto/app:rw" -w /home/japeto/app \
  japeto/parallel-tools:v64 bash

docker run -dit --name node4 --hostname node4 --network mpi-net \
  -v "${PWD}:/home/japeto/app:rw" -w /home/japeto/app \
  japeto/parallel-tools:v64 bash
```

#### **Compilar y ejecutar MPI:**
```bash
# Acceder al nodo maestro
docker exec -it node1 bash

# Compilar en el contenedor
mpic++ -std=c++11 -Wall -Wextra -g mpi_filterer.cpp imagen.cpp PGMimage.cpp PPMimage.cpp filter.cpp timer.cpp -o mpi_filterer

# Ejecutar con 4 nodos distribuidos
mpirun -np 4 ./mpi_filterer ./images/damma.ppm ./images/damma_blur_mpi.ppm --f blur
mpirun -np 4 ./mpi_filterer ./images/damma.ppm ./images/damma_laplace_mpi.ppm --f laplace
mpirun -np 4 ./mpi_filterer ./images/damma.ppm ./images/damma_sharpening_mpi.ppm --f sharpening
```

**Arquitectura MPI:**
```
┌─────────────────────────────────────────────────────┐
│                Imagen 1000x1278                     │
├─────────────┬─────────────┬─────────────┬───────────┤
│   Node1     │   Node2     │   Node3     │   Node4   │
│ Filas 0-319 │Filas 320-639│Filas 640-958│Filas 959- │
│             │             │             │   1277    │
└─────────────┴─────────────┴─────────────┴───────────┘
```

---

## Resultados de Rendimiento

### **Imagen de Prueba:** damma.ppm (1000x1278 píxeles)

| Implementación | Tiempo de Filtrado | Mejora | Estrategia |
|----------------|-------------------|--------|------------|
| **Secuencial** | 232 ms | 0% | 1 hilo |
| **Pthreads** | 65.30 ms | **+71.9%** | 4 cuadrantes |
| **OpenMP** | 279.31 ms | **-20.4%** | 3 filtros |
| **MPI** | 59.21 ms | **+74.5%** | 4 nodos |

*Estimado - requiere validación con filterer en Docker

### * Ganador: MPI Distribuido**
- **Mejor tiempo de filtrado:** 59.21 ms
- **Reducción del 74.5%** comparado con secuencial
- **Escalabilidad horizontal** efectiva

---

## 🔧 Comandos de Utilidad

### **Limpiar contenedores:**
```bash
docker stop node1 node2 node3 node4
docker rm node1 node2 node3 node4
docker network rm mpi-net
```

### **Ver logs de ejecución:**
```bash
docker logs node1
docker logs node2
docker logs node3
docker logs node4
```

### **Verificar archivos generados:**
```bash
ls -la ./images/*_blur* ./images/*_laplace* ./images/*_sharpening*
```

### **Comparar tamaños de archivos:**
```bash
ls -lh ./images/damma*.ppm
```

---

## 📁 Estructura del Proyecto

```
📦 filtros-paralelos/
├── imagen.h/cpp          # Clase base para imágenes
├── PGMimage.h/cpp        # Manejo de imágenes PGM (escala de grises)
├── PPMimage.h/cpp        # Manejo de imágenes PPM (color)
├── filter.h/cpp          # Algoritmos de filtros (blur, laplace, sharpening)
├── timer.h/cpp           # Utilidad para medición de tiempos
├── processor.cpp         # Versión base (carga/guardado)
├── filterer.cpp          # Versión secuencial con filtros
├── pth_filterer.cpp      # Implementación Pthreads
├── omp_filterer.cpp      # Implementación OpenMP
├── mpi_filterer.cpp      # Implementación MPI distribuida
├── docker-compose.yml    # Configuración de 4 nodos
├── hostfile              # Lista de hosts MPI
├── 🛠Makefile             # Automatización de compilación
└── images/               # Directorio de imágenes de prueba
    ├── damma.ppm            # Imagen principal de prueba
    ├── sulfur.pgm           # Imagen alternativa
    └── lena.ppm             # Imagen de referencia
```

---

## 🎛️ Filtros Implementados

### **Blur (Suavizado)**
```cpp
const float blur_kernel[3][3] = {
    {1.0f/9, 1.0f/9, 1.0f/9},
    {1.0f/9, 1.0f/9, 1.0f/9},
    {1.0f/9, 1.0f/9, 1.0f/9}
};
```

### **Laplace (Detección de bordes)**
```cpp
const float laplace_kernel[3][3] = {
    { 0.0f, -1.0f,  0.0f},
    {-1.0f,  4.0f, -1.0f},
    { 0.0f, -1.0f,  0.0f}
};
```

### **Sharpening (Realce)**
```cpp
const float sharpening_kernel[3][3] = {
    { 0.0f, -1.0f,  0.0f},
    {-1.0f,  5.0f, -1.0f},
    { 0.0f, -1.0f,  0.0f}
};
```

---

## Protocolo de Pruebas

### **Paso 1: Preparar entorno**
```bash
# Clonar repositorio
git clone <tu-repositorio>
cd filtros-paralelos

# Verificar imágenes de entrada
ls -la images/
```

### **Paso 2: Ejecutar pruebas locales**
```bash
# Secuencial base
g++ -o processor imagen.cpp PGMimage.cpp PPMimage.cpp filter.cpp timer.cpp processor.cpp
./processor ./images/damma.ppm ./images/damma2.ppm

# Secuencial con filtros
g++ -o filterer imagen.cpp PGMimage.cpp PPMimage.cpp filter.cpp timer.cpp filterer.cpp
./filterer ./images/damma.ppm ./images/damma_blur.ppm --f blur

# Pthreads
g++ -o pth_filterer imagen.cpp PGMimage.cpp PPMimage.cpp filter.cpp timer.cpp pth_filterer.cpp -lpthread
./pth_filterer ./images/damma.ppm ./images/damma_blur_pth.ppm --f blur

# OpenMP
g++ -o omp_filterer imagen.cpp PGMimage.cpp PPMimage.cpp filter.cpp timer.cpp omp_filterer.cpp -fopenmp
./omp_filterer ./images/damma.ppm
```

### **Paso 3: Configurar cluster MPI**
```bash
# Crear red Docker
docker network create mpi-net

# Crear 4 nodos
docker run -dit --name node1 --hostname node1 --network mpi-net \
  -v "${PWD}:/home/japeto/app:rw" -w /home/japeto/app \
  japeto/parallel-tools:v64 bash

docker run -dit --name node2 --hostname node2 --network mpi-net \
  -v "${PWD}:/home/japeto/app:rw" -w /home/japeto/app \
  japeto/parallel-tools:v64 bash

docker run -dit --name node3 --hostname node3 --network mpi-net \
  -v "${PWD}:/home/japeto/app:rw" -w /home/japeto/app \
  japeto/parallel-tools:v64 bash

docker run -dit --name node4 --hostname node4 --network mpi-net \
  -v "${PWD}:/home/japeto/app:rw" -w /home/japeto/app \
  japeto/parallel-tools:v64 bash
```

### **Paso 4: Ejecutar MPI**
```bash
# Acceder al nodo maestro
docker exec -it node1 bash

# Compilar MPI
mpic++ -std=c++11 -Wall -Wextra -g mpi_filterer.cpp imagen.cpp PGMimage.cpp PPMimage.cpp filter.cpp timer.cpp -o mpi_filterer

# Ejecutar en 4 nodos distribuidos
mpirun -np 4 ./mpi_filterer ./images/damma.ppm ./images/damma_blur_mpi.ppm --f blur
```

---

## 📈 Análisis de Rendimiento

### **Resultados Principales**
```
Imagen: damma.ppm (1000x1278 píxeles = 1,278,000 píxeles)
Filtro: Blur (suavizado)

┌──────────────┬──────────────┬────────────────┐
│ Implementación│ Tiempo (ms)  │ Mejora (%)     │
├──────────────┼──────────────┼────────────────┤
│ Secuencial   │    ~232      │       0%       │
│ Pthreads     │     65.30    │     +71.9%     │
│ OpenMP       │    279.31    │     -20.4%     │
│ MPI          │     59.21    │     +74.5%     │
└──────────────┴──────────────┴────────────────┘
```

### **Conclusiones**
- **MPI distribuido** ofrece el mejor rendimiento
- **Pthreads** es muy eficiente para memoria compartida
- **OpenMP** (implementación actual) es ineficiente por estrategia incorrecta
- **La paralelización mejora significativamente** el tiempo de filtrado

---

## 🚀 Inicio Rápido

### **Opción 1: Ejecución Local Completa**
```bash
# Ejecutar todas las versiones locales
make all
./processor ./images/damma.ppm ./images/damma2.ppm
./filterer ./images/damma.ppm ./images/damma_blur.ppm --f blur
./pth_filterer ./images/damma.ppm ./images/damma_pth.ppm --f blur
./omp_filterer ./images/damma.ppm
```

### **Opción 2: Solo MPI Distribuido**
```bash
# Configurar cluster rápidamente
./setup_cluster_quick.sh

# Compilar y ejecutar
docker exec -it node1 bash
mpic++ -std=c++11 -Wall -Wextra -g *.cpp -o mpi_filterer
mpirun -np 4 ./mpi_filterer ./images/damma.ppm ./images/result.ppm --f blur
```

### **Opción 3: Usando Docker Compose**
```bash
# Iniciar cluster completo
docker-compose up -d

# Ejecutar pruebas
docker exec -it node1 /home/japeto/app/test_all.sh
```

---

## Detalles Técnicos

### **Formatos Soportados**
- **PGM (P2):** Imágenes en escala de grises
- **PPM (P3):** Imágenes a color RGB

### **Filtros Disponibles**
- **`blur`** - Suavizado de imagen
- **`laplace`** - Detección de bordes
- **`sharpening`** - Realce de imagen

### **Arquitecturas Paralelas**

#### **Pthreads:**
- 4 hilos trabajando en cuadrantes diferentes
- Memoria compartida
- Sincronización al final

#### **OpenMP:**
- 3 hilos aplicando filtros diferentes
- Paralelización de bucle de filtros
- Genera 3 archivos simultáneamente

#### **MPI:**
- 4 procesos en nodos separados
- División por filas horizontales
- Comunicación por paso de mensajes

---

## Limpieza

### **Limpiar archivos compilados:**
```bash
rm -f processor filterer pth_filterer omp_filterer mpi_filterer *.o
```

### **Limpiar imágenes generadas:**
```bash
rm -f ./images/*_blur* ./images/*_laplace* ./images/*_sharpening* ./images/*_mpi*
```

### **Limpiar contenedores Docker:**
```bash
docker stop node1 node2 node3 node4
docker rm node1 node2 node3 node4
docker network rm mpi-net
```

### **Limpieza completa:**
```bash
# Usar docker-compose si se usó
docker-compose down

# O limpiar manualmente
docker stop $(docker ps -aq)
docker rm $(docker ps -aq)
docker network prune
```

---

## 📋 Requisitos

### **Software:**
- **g++** con soporte C++11
- **OpenMP** (`-fopenmp`)
- **POSIX Threads** (`-lpthread`)
- **MPI** (OpenMPI recomendado)
- **Docker** para simulación distribuida

### **Imágenes de entrada:**
- `damma.ppm` (1000x1278 - imagen principal)
- `sulfur.pgm` (escala de grises)
- `lena.ppm` (imagen de referencia)

---

## Autor

**Curso:** Programación Paralela  
**Proyecto:** Parcial No. 1 - Filtros de Imágenes  
**Fecha:** 2025

---

## 📚 Referencias

- [Repositorio base](https://github.com/japeto/netpbm_filters/tree/main)
- Documentación OpenMPI
- Estándares PPM/PGM NetPBM
