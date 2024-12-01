# Guía de Instalación y Uso

Este documento describe los pasos necesarios para clonar, configurar, compilar y ejecutar este proyecto, incluyendo el análisis de cobertura de código.

## Requisitos Previos
Asegúrate de tener instalados los siguientes programas y herramientas:
```
Git
Conan
CMake
Lcov (para medir la cobertura de código)
Python (para manejar virtual environments, si se requiere)
```
Para instalar lcov en sistemas basados en Debian/Ubuntu:
```
sudo apt-get install lcov
```
## Pasos para la Instalación
### 1. Clonar el Repositorio
```
git clone https://github.com/ICOMP-UNC/so-i-24-chp2-SofiViale.git
cd so-i-24-chp2-SofiViale
```
### 2. Agregar y Actualizar Submódulos
```
git submodule sync
git submodule update --init --recursive
```
## Compilación y Construcción

### 1. Activar Virtual Environment (Opcional)
Si deseas usar un entorno virtual, ejecuta:
```
source venv/bin/activate
```
### 2. Detectar el Perfil de Conan
```
conan profile detect --force
```
### 3. Crear y Configurar Carpeta de Construcción
```
rm -rf build
mkdir build
conan install . --output-folder=build --build=missing
```
### 4. Generar y Construir el Proyecto
```
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=./build/Release/generators/conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release
cmake --build .
make
```
### 5. Ejecutar Pruebas
```
ctest
```
## Cobertura de Código

### 1. Generar Reporte de Cobertura
Después de ejecutar las pruebas:
```
lcov --capture --directory . --output-file coverage.info
```
### 2. Filtrar Archivos Irrelevantes
```
lcov --remove coverage.info '/usr/' 'test/' --output-file coverage_filtered.info
```
## Notas Adicionales
Si encuentras errores relacionados con las dependencias, asegúrate de que conan esté correctamente configurado y las librerías necesarias estén disponibles.
Los reportes de cobertura generados con lcov pueden visualizarse con herramientas compatibles, como navegadores web.
