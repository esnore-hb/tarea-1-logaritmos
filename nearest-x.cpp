#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

/*
===============================================================================
ESTRUCTURAS DE DATOS
===============================================================================
*/

/**
 * @struct MBR (Minimum Bounding Rectangle)
 * @brief Representa un rectángulo delimitador que contiene puntos espaciales.
 * 
 * Esta estructura define un rectángulo axial-alineado en el espacio 2D mediante
 * sus coordenadas mínimas y máximas en ambos ejes (x e y).
 * 
 * @member x1  - Coordenada X mínima (esquina inferior-izquierda)
 * @member x2  - Coordenada X máxima (esquina superior-derecha)
 * @member y1  - Coordenada Y mínima (esquina inferior-izquierda)
 * @member y2  - Coordenada Y máxima (esquina superior-derecha)
 */
struct MBR {
  float x1;  // Coordenada X mínima del rectángulo
  float x2;  // Coordenada X máxima del rectángulo
  float y1;  // Coordenada Y mínima del rectángulo
  float y2;  // Coordenada Y máxima del rectángulo
};

/**
 * @struct RTreeEntry
 * @brief Entrada en un nodo del R-Tree que mapea un MBR a la posición de un nodo.
 * 
 * Representa un par clave-valor donde la clave es el MBR (rectángulo delimitador)
 * y el valor es la posición (índice) del nodo hijo en la estructura del árbol.
 * Para nodos hoja, position = -1 indica que es un punto de datos.
 * 
 * @member key      - Rectángulo delimitador (MBR) que actúa como clave de búsqueda
 * @member position - Posición del nodo hijo (-1 si es una hoja/punto de datos)
 */
struct RTreeEntry {
  MBR key;     // Rectángulo delimitador de esta entrada
  int position; // Índice del nodo hijo (-1 para hojas)
};

/**
 * @struct RTree
 * @brief Nodo del R-Tree que almacena múltiples entradas (MBR + posición).
 * 
 * Cada nodo puede contener hasta B (204) entradas. El campo 'k' indica cuántas
 * entradas contiene actualmente. El padding se usa para alineación de memoria
 * y eficiencia de lectura desde disco (tamaño de bloque).
 * 
 * @member k       - Cantidad actual de entradas en este nodo (0 <= k <= 204)
 * @member childs  - Array de hasta 204 entradas (RTreeEntry)
 * @member padding - Relleno de 12 bytes para alineación de bloque de disco
 */
struct RTree {
  int k;                // Cantidad de entradas válidas en este nodo
  RTreeEntry childs[204]; // Array de entradas del R-Tree (B = 204)
  char padding[12];     // Relleno para completar el tamaño del bloque de disco
};

/*
===============================================================================
VARIABLES GLOBALES
===============================================================================
*/

/** @brief Factor de bloqueo del R-Tree (máximo de entradas por nodo) */
const int B = 204;

/** @brief Manejador del archivo binario para lectura/escritura */
std::fstream file;

/** @brief Vector que almacena en RAM todo el R-Tree construido con nearest-X */
std::vector<RTree> nearest_x_r_tree;

/*
===============================================================================
FUNCIONES AUXILIARES
===============================================================================
*/

/**
 * @function calculateMBR
 * @brief Calcula el rectángulo delimitador (MBR) que engloba todos los hijos
 *        de un nodo del R-Tree.
 * 
 * Itera sobre todas las entradas del nodo y calcula las coordenadas mínimas
 * y máximas que forman el MBR contenedor. Maneja el caso especial de nodos vacíos.
 * 
 * @param  rTree Referencia constante al nodo del cual se calculará el MBR
 * @return MBR   Rectángulo delimitador que contiene todos los hijos del nodo
 *               Si el nodo está vacío (k <= 0), retorna un MBR con todas las
 *               coordenadas en 0.0f
 */
MBR calculateMBR(const RTree &rTree) {
  // Caso especial: nodo vacío
  if (rTree.k <= 0) {
    return {0.0f, 0.0f, 0.0f, 0.0f};
  }

  // Inicializar resultado con la primera entrada
  MBR result;
  result.x1 = rTree.childs[0].key.x1;
  result.x2 = rTree.childs[0].key.x2;
  result.y1 = rTree.childs[0].key.y1;
  result.y2 = rTree.childs[0].key.y2;

  // Iterar sobre el resto de entradas para expandir el MBR
  for (int i = 1; i < rTree.k; ++i) {
    const RTreeEntry &entry = rTree.childs[i];

    // Actualizar límites mínimos (incluir coordenadas menores)
    result.x1 = std::min(result.x1, entry.key.x1);
    result.y1 = std::min(result.y1, entry.key.y1);

    // Actualizar límites máximos (incluir coordenadas mayores)
    result.x2 = std::max(result.x2, entry.key.x2);
    result.y2 = std::max(result.y2, entry.key.y2);
  }

  return result;
}

/**
 * @function nearestX
 * @brief Construye un R-Tree usando el algoritmo Nearest-X (ordenamiento por
 *        proximidad en el eje X con particionamiento recursivo).
 * 
 * Algoritmo:
 * 1. Ordena las entradas por el centro X de sus MBRs
 * 2. Particiona en grupos de tamaño máximo B
 * 3. Si todos los grupos caben en un nodo raíz, termina
 * 4. Si no, construye recursivamente el nivel superior
 * 
 * @param  entries Vector de RTreeEntry a procesar (puede ser copia)
 * @return int     Índice del nodo raíz en el vector nearest_x_r_tree
 *                 (típicamente 0 para el primer nivel completo)
 * 
 * @note   Modifica el vector global nearest_x_r_tree agregando nodos
 *         @note El vector entries se ordena internamente, afectando su orden
 */
int nearestX(std::vector<RTreeEntry> entries) {
  // Paso 1: Ordenar todas las entradas por el centro de su coordenada X
  std::sort(entries.begin(), entries.end(),
            [](const RTreeEntry &a, const RTreeEntry &b) {
              float center_a = (a.key.x1 + a.key.x2) / 2.0f;
              float center_b = (b.key.x1 + b.key.x2) / 2.0f;
              return center_a < center_b;
            });

  // Paso 2: Dividir en grupos de tamaño B (máximo de entradas por nodo)
  int num_nodes = (entries.size() + B - 1) / B; // Redondear hacia arriba
  std::vector<int> node_positions; // Guardar posiciones de nodos creados

  // Crear nodos de primer nivel
  for (int i = 0; i < num_nodes; ++i) {
    RTree node;
    node.k = 0;

    // Llenar este nodo con hasta B entradas
    int start_idx = i * B;
    int end_idx = std::min(start_idx + B, (int)entries.size());

    for (int j = start_idx; j < end_idx; ++j) {
      node.childs[node.k] = entries[j];
      node.k++;
    }

    // Guardar nodo en el árbol global
    node_positions.push_back(nearest_x_r_tree.size());
    nearest_x_r_tree.push_back(node);
  }

  // Paso 3: Verificar si los nodos creados caben en un solo nodo raíz
  if (num_nodes <= B) {
    // Base de la recursión: crear la raíz
    RTree root;
    root.k = 0;

    // Agregar referencias a todos los nodos como hijos de la raíz
    for (int pos : node_positions) {
      RTreeEntry entry;
      entry.key = calculateMBR(nearest_x_r_tree[pos]);
      entry.position = pos;
      root.childs[root.k] = entry;
      root.k++;
    }

    // Guardar raíz en posición 0
    nearest_x_r_tree[0] = root;
    return 0;
  }

  // Paso 4: Caso recursivo - los nodos no caben en una raíz
  // Crear entradas para el siguiente nivel
  std::vector<RTreeEntry> parent_entries;
  for (int pos : node_positions) {
    RTreeEntry entry;
    entry.key = calculateMBR(nearest_x_r_tree[pos]);
    entry.position = pos;
    parent_entries.push_back(entry);
  }

  // Recursivamente construir el nivel superior
  return nearestX(parent_entries);
}

/*
===============================================================================
FUNCIÓN PRINCIPAL
===============================================================================
*/

/**
 * @function main
 * @brief Función principal que construye un R-Tree usando el algoritmo Nearest-X.
 * 
 * Descripción:
 *     Coordina el proceso completo de construcción de un R-Tree usando el
 *     algoritmo Nearest-X. Lee un archivo binario de puntos, los convierte
 *     en entradas del R-Tree, construye la estructura utilizando ordenamiento
 *     por proximidad en X, y escribe el árbol resultante a disco.
 * 
 * Proceso:
 *     1. Valida argumentos de línea de comandos (ruta archivo y N puntos)
 *     2. Lee N puntos en formato (x, y) del archivo binario
 *     3. Convierte puntos a entradas del R-Tree (puntos degenerados)
 *     4. Construye el R-Tree usando el algoritmo nearestX con profiling
 *     5. Escribe el árbol completo a un archivo binario .rtree
 *     6. Reporta estadísticas: cantidad de nodos, altura, tiempo de ejecución
 * 
 * Parámetros de entrada:
 *     argc (int): Cantidad de argumentos de línea de comandos (debe ser 3)
 *     argv (char**): Array de argumentos:
 *         argv[0] = nombre del programa
 *         argv[1] = ruta del archivo binario de puntos (x, y en float)
 *         argv[2] = cantidad de puntos a leer (N)
 * 
 * Salida:
 *     Imprime en stdout:
 *         - Cantidad de puntos leídos
 *         - Cantidad total de nodos creados
 *         - Índice de la raíz
 *         - Tiempo de construcción en microsegundos (DURATION_MICROSECONDS)
 *         - Nombre del archivo de salida generado
 * 
 * Retorna (Salida):
 *     int: 0 si se ejecutó exitosamente, 1 si hay errores en parámetros o I/O
 */
int main(int argc, char *argv[]) {
  /*
  ===========================================================================
  VALIDACIÓN DE PARÁMETROS
  ===========================================================================
  */
  if (argc != 3) {
    std::cout << "Uso incorrecto: " << argv[0] << " <ruta_archivo> <N>" << '\n';
    std::cout << "Ejemplo: " << argv[0] << " ./data/europa.bin 16777216"
              << std::endl;
    return 1;
  }

  // Obtener nombre del archivo y cantidad de puntos
  std::string filename = argv[1];
  unsigned long long N;

  // Validar que N sea positivo
  N = std::stoll(argv[2]);
  if (N <= 0) {
    std::cout << "El numero debe ser positivo";
    return 1;
  }

  /*
  ===========================================================================
  LECTURA DEL ARCHIVO BINARIO
  ===========================================================================
  */
  // Abrir archivo en modo lectura binaria
  file.open(filename, std::fstream::in | std::ios::binary);
  if (!file.is_open()) {
    std::cout << "No se pudo abrir el archivo de ruta" << std::endl;
    return 1;
  }

  // Vector para almacenar puntos leídos
  std::vector<std::pair<float, float>> raw_points;
  float xy[2];  // Buffer temporal para leer un punto (x, y)

  // Leer N puntos del archivo
  for (unsigned long long i = 0; i < N; ++i) {
    file.read(reinterpret_cast<char *>(xy), 2 * sizeof(float));
    if (!file.good()) {
      std::cout
          << "\nAdvertencia: Se alcanzó el fin del archivo antes de completar "
          << N << " lecturas. Se leyeron solo " << raw_points.size()
          << " puntos." << '\n';
      break;
    }
    raw_points.push_back(std::make_pair(xy[0], xy[1]));
  }
  file.close();

  /*
  ===========================================================================
  PREPARACIÓN DE DATOS INICIALES
  ===========================================================================
  */
  // Ordenar los puntos para mejorar localidad espacial
  std::sort(raw_points.begin(), raw_points.end());

  // Convertir puntos a entradas del R-Tree
  std::vector<RTreeEntry> points;
  for (unsigned long long i = 0; i < raw_points.size(); i++) {
    std::pair<float, float> p = raw_points[i];
    RTreeEntry entry;
    // Para puntos: x1=x2 y y1=y2 (es un rectángulo degenerado)
    entry.key.x1 = p.first;
    entry.key.x2 = p.first;
    entry.key.y1 = p.second;
    entry.key.y2 = p.second;
    entry.position = -1;  // -1 indica que es un punto (hoja)
    points.push_back(entry);
  }

  /*
  ===========================================================================
  CONSTRUCCIÓN DEL R-TREE CON NEAREST-X
  ===========================================================================
  */
  std::cout << "Iniciando construcción del R-tree con Nearest-X..." << '\n';
  std::cout << "Cantidad de puntos: " << points.size() << '\n';

  // Inicializar el vector del árbol con un nodo raíz vacío
  RTree raiz = {};
  nearest_x_r_tree.push_back(raiz);

  // Medir tiempo de ejecución
  auto start_time = std::chrono::high_resolution_clock::now();
  // Ejecutar el algoritmo de construcción
  int root_index = nearestX(points);
  // Marcar fin de la ejecución
  auto end_time = std::chrono::high_resolution_clock::now();

  /*
  ===========================================================================
  INFORMACIÓN SOBRE EL ÁRBOL CONSTRUIDO
  ===========================================================================
  */
  std::cout << "R-tree construido exitosamente!" << '\n';
  std::cout << "Cantidad de nodos en el árbol: " << nearest_x_r_tree.size()
            << '\n';
  std::cout << "Raíz en posición: " << root_index << '\n';
  std::cout << "Cantidad de hijos de la raíz: "
            << nearest_x_r_tree[root_index].k << '\n';

  // Calcular y mostrar duración de ejecución en microsegundos
  auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
      end_time - start_time);
  std::cout << "---EXECUTION_METRIC---\n";
  std::cout << "DURATION_MICROSECONDS=" << duration.count() << "\n";

  /*
  =============================================================================
  Escribiendo a disco el R-Tree creado
  =============================================================================
  */

  // nombre del archivo parseado de su ruta y extension
  std::string base = argv[1];
  size_t last_slash = base.find_last_of('/');
  if (last_slash != std::string::npos)
    base = base.substr(last_slash + 1);
  size_t last_dot = base.find_last_of('.');
  if (last_dot != std::string::npos && last_dot != 0)
    base = base.substr(0, last_dot);
  std::string output_filename = "nearest-x-" + base + ".rtree";

  std::ofstream output_file(output_filename,
                            std::ios::out | std::ios::binary);
  if (output_file.is_open()) {
    for (const auto &rtree : nearest_x_r_tree) {
      output_file.write(reinterpret_cast<const char *>(&rtree), sizeof(RTree));
    }
    output_file.close();
    std::cout << "Guardado: El R-tree ha sido escrito exitosamente en "
                 + output_filename
              << '\n';
  } else {
    std::cout
        << "Error: No se pudo abrir el archivo para escritura."
        << '\n';
  }

  std::cout << std::endl;
  return 0;
}
