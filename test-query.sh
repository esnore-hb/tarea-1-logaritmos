OUTPUT_DIR="./query"
QUERY_BIN="./query.bin"

if [ ! -d "$OUTPUT_DIR" ]; then
    mkdir -p "$OUTPUT_DIR"
    echo "Diretorio de salida creado: $OUTPUT_DIR"
else
    echo "El directorio de salida $OUTPUT_DIR ya existe."
fi

if [[ ! -f "$QUERY_BIN" ]]; then
  echo "ERROR: Uno o ambos ejecutables no se encontraron. Asegúrate de que $QUERY_BIN existan."
  exit 1
fi

RTREE_FILE=./nearest-x-europa.rtree
if [[ ! -f "$RTREE_FILE" ]]; then
  echo "No se encontro el archivo $RTREE_FILE. Asegúrate de crearlo con <sh test-creation.sh>."
  exit 1
fi

RTREE_FILE=./nearest-x-random.rtree
if [[ ! -f "$RTREE_FILE" ]]; then
  echo "No se encontro el archivo $RTREE_FILE. Asegúrate de crearlo con <sh test-creation.sh>."
  exit 1
fi

RTREE_FILE=./sort-tile-recursive-europa.rtree
if [[ ! -f "$RTREE_FILE" ]]; then
  echo "No se encontro el archivo $RTREE_FILE. Asegúrate de crearlo con <sh test-creation.sh>."
  exit 1
fi

RTREE_FILE=./sort-tile-recursive-random.rtree
if [[ ! -f "$RTREE_FILE" ]]; then
  echo "No se encontro el archivo $RTREE_FILE. Asegúrate de crearlo con <sh test-creation.sh>."
  exit 1
fi
./query.bin ./nearest-x-europa.rtree > ./query/nearest-x-europa.query
./query.bin ./nearest-x-random.rtree > ./query/nearest-x-random.query
./query.bin ./sort-tile-recursive-europa.rtree > ./query/sort-tile-recursive-europa.query
./query.bin ./sort-tile-recursive-random.rtree > ./query/sort-tile-recursive-random.query