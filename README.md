# R-Tree Implementation: Nearest-X y Sort-Tile-Recursive

## Descripción del Proyecto

Implementación de R-Tree (Spatial Index Tree) con dos algoritmos:

- **Nearest-X**: Ordenamiento por proximidad en eje X
- **Sort-Tile-Recursive (STR)**: Particionamiento 2D para minimizar superposición

El proyecto indexa millones de puntos geoespaciales (coordenadas 2D) en disco para búsquedas por rango eficientes.

---

## Requisitos

- C++11 o superior
- Make
- Sistema operativo: Linux, macOS o Windows (WSL 2)

Esto debido a que para poder automatizar las tareas de rendimiento de los
compilados y redirigir su resultados a los archivos respectivos, se usan
scrips escritos en `bash`.

### Instalar requisitos

**Ubuntu/Debian:**

```bash
sudo apt-get update
sudo apt-get install build-essential
```

**macOS:**

```bash
brew install gcc make
```

**Windows (WSL 2):**
Instala Ubuntu 20.04+ desde Microsoft Store, luego ejecuta comandos de Ubuntu/Debian.

---

## Uso Rápido

### 1. Compilar todo

```bash
make
```

Genera tres ejecutables: `nearest-x`, `sort-tile-recursive`, `query`

### 2. Ejecutar construcción de R-Trees

```bash
sh test-creation.sh
```

Construye R-Trees con ambos algoritmos usando diferentes tamaños de
dataset. Los arboles resultantes se guardan con la extensión `.rtree` en la
raíz del directorio. Son archivos binarios, así que no se pueden abrir
directamente.

### 3. Ejecutar consultas

```bash
sh test-query.sh
```

Evalua el rendimiento de las estructuras con 100 cuadrados con 5 tipos
distintos de largo generados en una ubicacion al azar.

### 4. Visualización de los resultados

```bash
python plot_creation_times.py
python plot_query_results.py
```

Estos dos archivos son para generar imágenes con los resultados obtenidos
de los test ejecutados. Cada uno de ellos genera una imagen que se guarda
en el directorio raíz del proyecto.

Las librerias necesarias para Python se pueden
encontrar en los `import` colocados en los mismos. No se adjunta un
`requirements.txt` pues el ambiente para visualizarlos no es estable entre
Windows y Linux. Se le delega la creación del ambiente al usuario.

### 4. Limpiar archivos compilados y binarios

```bash
make clean
```

Limpiar todos los archivos que terminen en `.bin`, `.rtree` y `.png` dentro de la raíz del directorio.

---

## Estructura del Proyecto

```python
./
├── nearest-x.cpp               # Algoritmo Nearest-X
├── sort-tile-recursive.cpp     # Algoritmo Sort-Tile-Recursive
├── query.cpp                   # Programa de consulta
├── test-creation.sh            # Script de construcción
├── test-query.sh               # Script de consulta
├── Makefile                    # Compilación
├── data/                       # Datos de prueba
│   ├── europa_sample.txt
│   ├── europa_bonus_sample.txt
│   └── random_sample.txt
└── creation/                   # R-Trees compilados (output)
```

---

## Ejecución Manual

```bash
# Construir R-Tree con Nearest-X
./nearest-x ./data/europa.bin 1048576

# Construir R-Tree con Sort-Tile-Recursive
./sort-tile-recursive ./data/europa.bin 1048576

# Ejecutar consulta
./query ./nearest-x-europa.rtree
```

---

## Documentación del Código

Cada estructura y función incluye:

- Descripción de propósito
- Parámetros de entrada
- Valores de salida
- Notas importantes

Ver comentarios en el código fuente para detalles técnicos.
