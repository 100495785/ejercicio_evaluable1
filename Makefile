# Variables del compilador
CC = gcc

# Banderas de compilación:
# -Wall -Wextra: muestran todos los warnings (buena práctica)
# -g: añade información para depurar (Valgrind/GDB)
# -pthread: necesario para compilar hilos y mutex
CFLAGS = -Wall -Wextra -g -pthread

# -fPIC: Position Independent Code (obligatorio para bibliotecas dinámicas .so)
PICFLAGS = -fPIC

# -shared: indica que queremos generar una biblioteca dinámica (.so)
LDFLAGS_SO = -shared

# -Wl,-rpath,'$$ORIGIN': hace que el ejecutable busque la .so en su propio directorio
RPATH = -Wl,-rpath,'$$ORIGIN'


# Regla principal que se ejecuta al escribir simplemente 'make'
all: libclaves.so cliente_local


# 1. Compilar el objeto con código independiente de posición (necesario para .so)
claves.o: claves.c claves.h
	$(CC) $(CFLAGS) $(PICFLAGS) -c claves.c -o claves.o


# 2. Crear la biblioteca dinámica libclaves.so (Parte A)
libclaves.so: claves.o
	$(CC) $(LDFLAGS_SO) -o libclaves.so claves.o -lpthread


# 3. Regla para compilar el cliente local y enlazarlo con libclaves.so
# -L. indica que busque bibliotecas en el directorio actual
# -lclaves enlaza con libclaves.so
# $(RPATH) incrusta la ruta del propio ejecutable para encontrar la .so al ejecutar
cliente_local: app-cliente.c libclaves.so claves.h
	$(CC) $(CFLAGS) -o cliente_local app-cliente.c -L. -lclaves $(RPATH)


# Regla para limpiar los archivos generados y dejar el directorio limpio
clean:
	rm -f *.o *.so cliente_local