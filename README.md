# R-Tree Implementation: Nearest-X y Sort-Tile-Recursive

Implementación de R-Tree para indexación espacial 2D con dos algoritmos de construcción:

- **Nearest-X**: Ordenamiento por proximidad en eje X
- **Sort-Tile-Recursive (STR)**: Particionamiento 2D

---

## Instalación Rápida

### Requisitos del Sistema

- C++11 o superior
- Make
- Python 3.6+
- Bash (para scripts de automatización)

### 1. Instalar dependencias

**Ubuntu/Debian:**

```bash
sudo apt-get update
sudo apt-get install build-essential python3-pip
pip install matplotlib
```

**macOS:**

```bash
brew install gcc make python3
pip install matplotlib
```

**Windows (WSL 2):**
Usar comandos de Ubuntu/Debian desde la terminal WSL.

### 2. Descargar datos

Descargar los archivos de datos desde [este repositorio](https://github.com/claugaete/tarea1-cc4102-2026-1) (sección Releases) y extraerlos en la carpeta `./data/`

---

## Ejecución

### Compilar

```bash
make
```

### Paso 1: Construir R-Trees

```bash
sh test-creation.sh
```

Genera árboles con ambos algoritmos y guarda resultados en `.rtree`

### Paso 2: Ejecutar consultas

```bash
sh test-query.sh
```

Evalúa rendimiento con búsquedas de rango y genera métricas

#### Bonus: Consultas con puntos específicos

```bash
sh test-bonus.sh
```

Ejecuta consultas con puntos adicionales y guarda resultados en CSV para visualización

### Paso 3: Visualizar resultados

```bash
python plot_creation_times.py
python plot_query_results.py
```

Genera dos gráficos PNG: `creation_times.png` y `query_results.png`

#### Bonus: Visualizar puntos de consulta

```bash
python plot_bonus.py
```

Genera scatterplots de los puntos de consulta bonus para ambos algoritmos.

### Limpiar

```bash
make clean
```

Elimina ejecutables, `.rtree` y gráficos `.png`

---

## Estructura de Archivos

```python
├── nearest-x.cpp           # Algoritmo Nearest-X
├── sort-tile-recursive.cpp # Algoritmo STR
├── query.cpp               # Búsquedas por rango
├── query_bonus.cpp         # Búsquedas con puntos específicos (bonus)
├── plot_creation_times.py  # Gráfico de tiempos
├── plot_query_results.py   # Gráfico de consultas
├── plot_bonus.py           # Scatterplot de puntos bonus
├── test-creation.sh        # Script de construcción
├── test-query.sh           # Script de consultas
├── test-bonus.sh           # Script de consultas bonus
└── data/                   # Datos binarios (descargar)
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
