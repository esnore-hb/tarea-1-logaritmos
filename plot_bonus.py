
#!/usr/bin/env python3
"""
Script para graficar los puntos de consulta del bonus.
Visualiza:
1. Scatterplot de puntos consultados con Nearest-X
2. Scatterplot de puntos consultados con Sort-Tile-Recursive
"""

import matplotlib.pyplot as plt
import pandas as pd
import os

# Rutas de los archivos CSV
ruta_archivo1 = "./query/nearest-x-europa-bonus.csv"
ruta_archivo2 = "./query/sort-tile-recursive-europa-bonus.csv"

def plot_query_results(archivo_csv, titulo, nombre_salida):
    """
    Crea un scatterplot de puntos de consulta desde un archivo CSV.
    
    Descripción:
        Lee un archivo CSV con coordenadas (x, y) de puntos de consulta
        y genera un scatterplot visualizando la distribución espacial de
        estos puntos en el espacio 2D.
    
    Parámetros de entrada:
        archivo_csv (str): Ruta del archivo CSV con coordenadas (x, y)
        titulo (str): Título del gráfico a mostrar
        nombre_salida (str): Nombre base del archivo PNG de salida
    
    Retorna (Salida):
        None. Genera un archivo PNG en ./plots/{nombre_salida}.png
              e imprime mensajes de estado de la ejecución.
    """
    try:
        # Leer el archivo CSV saltando la primera fila (header)
        df = pd.read_csv(archivo_csv, header=None, skiprows=1, 
                        low_memory=False)
        
        # Asignar nombres a las columnas
        df.columns = ['x', 'y']
        
        # Convertir explícitamente a float
        df['x'] = pd.to_numeric(df['x'], errors='coerce')
        df['y'] = pd.to_numeric(df['y'], errors='coerce')
        
        # Eliminar filas donde la conversión falló
        df = df.dropna()
        
        if len(df) == 0:
            print(f"✗ No hay datos válidos en {archivo_csv}")
            return
        
        # Crear figura
        plt.figure(figsize=(10, 8))
        
        # Crear scatterplot
        plt.scatter(df['x'], df['y'], alpha=0.6, s=20, edgecolors='black', linewidth=0.5)
        
        # Configurar labels y título
        plt.xlabel('X', fontsize=12)
        plt.ylabel('Y', fontsize=12)
        plt.title(titulo, fontsize=14, fontweight='bold')
        plt.grid(True, alpha=0.3)
        
        # Guardar figura
        nombre_archivo_salida = f"./plots/{nombre_salida}.png"
        os.makedirs("./plots", exist_ok=True)
        plt.savefig(nombre_archivo_salida, dpi=300, bbox_inches='tight')
        print(f"✓ Gráfico guardado: {nombre_archivo_salida}")
        print(f"  Puntos graficados: {len(df)}")
        
        # Mostrar en pantalla
        plt.show()
        
    except FileNotFoundError:
        print(f"✗ Archivo no encontrado: {archivo_csv}")
    except Exception as e:
        print(f"✗ Error procesando {archivo_csv}: {e}")
        import traceback
        traceback.print_exc()

if __name__ == "__main__":
    # Crear visualización para nearest-x-europa-bonus
    if os.path.exists(ruta_archivo1):
        plot_query_results(ruta_archivo1, 
                          "Nearest-X Europa Bonus Query Points", 
                          "nearest-x-europa-bonus")
    
    # Crear visualización para sort-tile-recursive-europa-bonus
    if os.path.exists(ruta_archivo2):
        plot_query_results(ruta_archivo2, 
                          "Sort-Tile-Recursive Europa Bonus Query Points", 
                          "sort-tile-recursive-europa-bonus")