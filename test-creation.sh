#!/bin/bash

# ============================================================
# CONFIGURACIÓN
# ============================================================

# Lista de todos los valores N (de 32K a 16M)
# El script automáticamente maneja estos valores.
N_VALUES=(
    32768
    65536
    131072
    262144
    524288
    1048576
    2097152
    4194304
    8388608
    16777216
)

# Rutas de los ejecutables y los datos
NEAR_BIN="./nearest-x.bin"
SORT_BIN="./sort-tile-recursive.bin"
EUROPA_DATA="./data/europa.bin"
RANDOM_DATA="./data/random.bin"
OUTPUT_DIR="./creation"

# ============================================================
# VALIDACIONES Y PREPARACIÓN
# ============================================================

echo "--- Iniciando la ejecución de trabajos ---"

# 1. Verificar si los ejecutables existen
if [[ ! -f "$NEAR_BIN" || ! -f "$SORT_BIN" ]]; then
    echo "ERROR: Uno o ambos ejecutables no se encontraron. Asegúrate de que $NEAR_BIN y $SORT_BIN existan."
    exit 1
fi

# 2. Crear el directorio de salida si no existe
if [ ! -d "$OUTPUT_DIR" ]; then
    mkdir -p "$OUTPUT_DIR"
    echo "Diretorio de salida creado: $OUTPUT_DIR"
else
    echo "El directorio de salida $OUTPUT_DIR ya existe."
fi

# 3. Verificar datos de entrada
if [[ ! -f "$EUROPA_DATA" || ! -f "$RANDOM_DATA" ]]; then
    echo "ERROR: No se encontraron los archivos de datos requeridos ($EUROPA_DATA o $RANDOM_DATA)."
    exit 1
fi

# ============================================================
# SECCIÓN 1: nearest-x.bin
# ============================================================

echo -e "\n[START] Ejecutando nearest-x.bin para todos los tamaños N..."

# Ejecución para europa.bin
echo "  -> Procesando europa.bin..."
for N in "${N_VALUES[@]}"; do
    OUTPUT_FILE="$OUTPUT_DIR/europa-near.n${N}"
    echo "    Ejecutando: $NEAR_BIN $EUROPA_DATA $N > $OUTPUT_FILE"
    "$NEAR_BIN" "$EUROPA_DATA" "$N" > "$OUTPUT_FILE"
done

# Ejecución para random.bin
echo "  -> Procesando random.bin..."
for N in "${N_VALUES[@]}"; do
    OUTPUT_FILE="$OUTPUT_DIR/random-near.n${N}"
    echo "    Ejecutando: $NEAR_BIN $RANDOM_DATA $N > $OUTPUT_FILE"
    "$NEAR_BIN" "$RANDOM_DATA" "$N" > "$OUTPUT_FILE"
done

echo -e "\n[SUCCESS] nearest-x.bin completado exitosamente."

# ============================================================
# SECCIÓN 2: sort-tile-recursive.bin
# ============================================================

echo -e "\n[START] Ejecutando sort-tile-recursive.bin para todos los tamaños N..."
 
# Ejecución para europa.bin
echo "  -> Procesando europa.bin..."
for N in "${N_VALUES[@]}"; do
    OUTPUT_FILE="$OUTPUT_DIR/europa-str.n${N}"
    echo "    Ejecutando: $SORT_BIN $EUROPA_DATA $N > $OUTPUT_FILE"
    "$SORT_BIN" "$EUROPA_DATA" "$N" > "$OUTPUT_FILE"
done
 
# Ejecución para random.bin
echo "  -> Procesando random.bin..."
for N in "${N_VALUES[@]}"; do
    OUTPUT_FILE="$OUTPUT_DIR/random-str.n${N}"
    echo "    Ejecutando: $SORT_BIN $RANDOM_DATA $N > $OUTPUT_FILE"
    "$SORT_BIN" "$RANDOM_DATA" "$N" > "$OUTPUT_FILE"
done
echo -e "\n[SUCCESS] sort-tile-recursive.bin completado exitosamente."
