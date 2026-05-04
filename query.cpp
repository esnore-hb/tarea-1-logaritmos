#include <cmath>
#include <fstream>
#include <iostream>
#include <ostream>
#include <random>
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
 * y el valor es la posición (índice) del nodo hijo en el archivo de disco.
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

/** @brief Manejador del archivo binario del R-Tree para lectura */
std::fstream file;

/** @brief Contador de lecturas a disco realizadas durante una búsqueda */
unsigned long long disk_reads = 0;

/*
===============================================================================
FUNCIONES AUXILIARES
===============================================================================
*/

/**
 * @function mbrs_intersect
 * @brief Verifica si dos rectángulos delimitadores (MBRs) se intersectan.
 * 
 * Dos MBRs se intersectan si sus proyecciones en ambos ejes (X e Y) se
 * solapan. Usa la prueba de separación de ejes (axis-aligned test).
 * 
 * @param  mbr1 Primer rectángulo a verificar
 * @param  mbr2 Segundo rectángulo a verificar
 * @return bool true si los MBRs se intersectan, false en caso contrario
 */
bool mbrs_intersect(const MBR &mbr1, const MBR &mbr2) {
  // Dos rectángulos se intersectan si no están separados en ningún eje
  return !(mbr1.x2 < mbr2.x1 || mbr1.x1 > mbr2.x2 ||
           mbr1.y2 < mbr2.y1 || mbr1.y1 > mbr2.y2);
}

/**
 * @function readNode
 * @brief Lee un nodo del R-Tree desde el archivo binario en disco.
 * 
 * Busca la posición del nodo (posición * sizeof(RTree)) en el archivo
 * y lo lee en memoria. Incrementa el contador de lecturas a disco.
 * 
 * @param  file     Puntero al archivo abierto en modo binario
 * @param  position Índice del nodo a leer (0 = raíz)
 * @return RTree    Nodo leído del disco
 * 
 * @note   Modifica la variable global disk_reads
 * @note   El archivo debe estar abierto en modo lectura binaria
 */
RTree readNode(std::fstream *file, int position) {
  RTree node;
  // Calcular posición en bytes y buscar en el archivo
  file->seekg(position * sizeof(RTree), std::ios::beg);
  // Leer el nodo completo
  file->read(reinterpret_cast<char *>(&node), sizeof(RTree));
  // Incrementar contador de accesos a disco
  disk_reads++;
  return node;
}

/**
 * @function searchRectangle
 * @brief Búsqueda recursiva de puntos dentro de un rectángulo especificado.
 * 
 * Algoritmo:
 * 1. Lee el nodo actual del disco
 * 2. Para cada entrada en el nodo:
 *    - Verifica si su MBR intersecta con el rectángulo de búsqueda
 *    - Si es hoja (point), agrega a resultados
 *    - Si es nodo interno, continúa la búsqueda recursivamente
 * 
 * @param  file        Puntero al archivo del R-Tree abierto en lectura
 * @param  node_pos    Índice del nodo actual a procesar
 * @param  search_rect Rectángulo que define la región de búsqueda
 * @param  results     Puntero al vector donde se agregan los puntos encontrados
 * @return void        Los resultados se almacenan en el vector results
 * 
 * @note   Modifica la variable global disk_reads en cada llamada a readNode
 * @note   search_rect define una ventana rectangular en el espacio 2D
 * @note   Un punto corresponde a position == -1 en la entrada
 */
void searchRectangle(std::fstream *file, int node_pos, const MBR &search_rect,
                    std::vector<std::pair<float, float>> *results) {
  // Leer el nodo actual desde el disco
  RTree node = readNode(file, node_pos);

  // Procesar cada entrada del nodo
  for (int i = 0; i < node.k; ++i) {
    RTreeEntry &entry = node.childs[i];

    // Verificar si el MBR de esta entrada intersecta con el rectángulo de búsqueda
    if (mbrs_intersect(entry.key, search_rect)) {
      if (entry.position == -1) {
        // Caso: es una hoja (punto de datos)
        // Agregar el punto a los resultados
        results->push_back({entry.key.x1, entry.key.y1});
      } else {
        // Caso: es un nodo interno
        // Continuar búsqueda recursivamente en el nodo hijo
        searchRectangle(file, entry.position, search_rect, results);
      }
    }
  }
}

std::vector<std::vector<MBR>> generate_squares() {
    // Definición de los posibles largos de lado (s)
    const std::vector<float> s_values = {0.0025f, 0.005f, 0.01f, 0.025f, 0.05f};
    
    // Inicialización de la estructura de resultados: 5 listas, cada una con 100 MBRs.
    std::vector<std::vector<MBR>> all_squares;
    
    // Setup de generación de números aleatorios
    std::random_device rd;
    std::mt19937 generator(rd());
    // Usaremos un rango arbitrario grande (ej. [0, 10]) para la posición inicial (x1, y1)
    std::uniform_real_distribution<float> distribution(0.0f, 1.0f);

    // Iterar sobre cada valor de largo de lado
    for (float s : s_values) {
        std::vector<MBR> squares_for_current_s;
        
        // Generar 100 cuadrados para el tamaño 's'
        for (int i = 0; i < 100; ++i) {
            // Generar coordenadas iniciales aleatorias (esquina inferior izquierda)
            float x1 = distribution(generator);
            float y1 = distribution(generator);
            
            // Calcular la segunda esquina (debido a que es un cuadrado, el delta es 's')
            float x2 = x1 + s;
            float y2 = y1 + s;
            
            // Almacenar el MBR
            squares_for_current_s.push_back({x1, x2, y1, y2});
        }
        
        // Añadir la lista de 100 cuadrados al contenedor principal
        all_squares.push_back(squares_for_current_s);
    }

    return all_squares;
}


/*
===============================================================================
FUNCIÓN PRINCIPAL
===============================================================================
*/

/**
 * @function main
 * @brief Función principal que realiza una búsqueda de puntos en el R-Tree
 *        dentro de un rectángulo especificado.
 * 
 * Proceso:
 * 1. Abre el archivo binario del R-Tree "nearest-x.rtree"
 * 2. Define un rectángulo de búsqueda [0.3-0.4] x [0.3-0.4]
 * 3. Realiza búsqueda recursiva desde la raíz
 * 4. Imprime los puntos encontrados en formato CSV
 * 
 * @return int  0 si se ejecutó exitosamente, 1 si no se pudo abrir el archivo
 */
int main(int argc, char *argv[]) {
  /*
  ===========================================================================
  APERTURA DEL ARCHIVO
  ===========================================================================
  */

  if (argc != 2) {
    std::cout << "Uso incorrecto: " << argv[0] << " <ruta_archivo.rtree>" << '\n';
    std::cout << "Ejemplo: " << argv[0] << " ./nearest-x-europa.rtree"
              << std::endl;
    return 1;
  }

  // Obtener nombre del archivo
  std::string filename = argv[1];

  // Abrir el archivo binario del R-Tree
  file.open(filename, std::fstream::in | std::ios::binary);
  if (!file.is_open()) {
    std::cout << "No se pudo abrir el archivo" << std::endl;
    return 1;
  }

	auto test_cases = generate_squares();

	for(auto s: test_cases){

		std::vector<unsigned long long> test_disk_reads;
		std::vector<unsigned long long> test_results;

		for(auto search_rect: s){

			std::vector<std::pair<float, float>> results;
			disk_reads = 0;

			searchRectangle(&file, 0, search_rect, &results);
        

			test_disk_reads.push_back(disk_reads);
			test_results.push_back(results.size());
			// Salida detallada de búsqueda
			/*
			std::cout << "Puntos encontrados en rectángulo [" << search_rect.x1 << ", "
					<< search_rect.x2 << "] x [" << search_rect.y1 << ", " << search_rect.y2
					<< "]:" << std::endl;
			std::cout << "Total de puntos: " << results.size() << std::endl;
			std::cout << "Lecturas a disco: " << disk_reads << std::endl;
			*/

			// Salida en formato CSV (x, y)
			/*
			std::cout << "x, y\n";
			for (const auto &point : results) {
				std::cout << point.first << ", " << point.second << '\n';
			}
			*/
		}

		unsigned long long promedio = 0;
		for (auto x : test_disk_reads){
			promedio += x;
		}
		double result_lecturas_promedio = (double) promedio / (double) test_disk_reads.size();

		// Calcular desviación estándar de lecturas a disco
		double suma_cuadrados = 0;
		for (auto x : test_disk_reads) {
			double diff = (double)x - result_lecturas_promedio;
			suma_cuadrados += diff * diff;
		}
		double result_lecturas_std = std::sqrt(suma_cuadrados / (double)test_disk_reads.size());

		promedio = 0;
		for (auto x : test_results){
			promedio += x;
		}
		double result_puntos_promedio = (double) promedio / (double) test_results.size();

		// Calcular desviación estándar de puntos encontrados
		suma_cuadrados = 0;
		for (auto x : test_results) {
			double diff = (double)x - result_puntos_promedio;
			suma_cuadrados += diff * diff;
		}
		double result_puntos_std = std::sqrt(suma_cuadrados / (double)test_results.size());

		std::cout << "s:" << s[0].x2 - s[0].x1 << '\n';
		std::cout << "result_lecturas_promedio:" << result_lecturas_promedio << '\n';
		std::cout << "result_lecturas_std:" << result_lecturas_std << '\n';
		std::cout << "result_puntos_promedio:" << result_puntos_promedio << '\n';
		std::cout << "result_puntos_std:" << result_puntos_std << '\n';
	}
  /*
  ===========================================================================
  CIERRE DE ARCHIVO
  ===========================================================================
  */
  std::cout << std::endl;
  file.close();
  return 0;
}