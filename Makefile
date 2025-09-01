# Makefile para el proyecto de procesamiento de imágenes PPM/PGM

# Compilador y flags
CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++11 -O2
LDFLAGS = 

# Directorios
SRCDIR = .
OBJDIR = obj
BINDIR = bin

# Crear directorios si no existen
$(shell mkdir -p $(OBJDIR) $(BINDIR))

# Archivos fuente
SOURCES = imagen.cpp PGMimage.cpp PPMimage.cpp filter.cpp timer.cpp
OBJECTS = $(addprefix $(OBJDIR)/, $(SOURCES:.cpp=.o))

# Ejecutables
TARGETS = $(BINDIR)/processor

# Regla principal
all: $(TARGETS)

# Compilar processor
$(BINDIR)/processor: $(OBJECTS) $(OBJDIR)/processor.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

# Compilar archivos objeto
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# Limpiar archivos compilados
clean:
	rm -rf $(OBJDIR) $(BINDIR)

# Limpiar solo objetos
clean-obj:
	rm -rf $(OBJDIR)

# Reconstruir todo
rebuild: clean all

# Mostrar ayuda
help:
	@echo "Targets disponibles:"
	@echo "  all       - Compilar todos los ejecutables"
	@echo "  processor - Compilar solo el procesador de imágenes"
	@echo "  clean     - Limpiar todos los archivos compilados"
	@echo "  clean-obj - Limpiar solo archivos objeto"
	@echo "  rebuild   - Limpiar y reconstruir todo"
	@echo "  help      - Mostrar esta ayuda"

# Instalar (opcional)
install: all
	@echo "Para instalar, copie los ejecutables de $(BINDIR)/ al directorio deseado"

# Reglas que no crean archivos
.PHONY: all clean clean-obj rebuild help install

# Dependencias específicas
$(OBJDIR)/imagen.o: imagen.h
$(OBJDIR)/PGMimage.o: PGMimage.h imagen.h
$(OBJDIR)/PPMimage.o: PPMimage.h imagen.h
$(OBJDIR)/filter.o: filter.h imagen.h PGMimage.h PPMimage.h
$(OBJDIR)/timer.o: timer.h
$(OBJDIR)/processor.o: PGMimage.h PPMimage.h timer.h