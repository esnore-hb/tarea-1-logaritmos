#!/usr/bin/env python3
"""
Script para graficar el tiempo de creación de R-trees en función de N.
Compara 4 configuraciones:
- Dataset aleatorio con Nearest-X
- Dataset aleatorio con STR
- Dataset de Europa con Nearest-X
- Dataset de Europa con STR
"""

import re
import matplotlib
matplotlib.use('TkAgg')
import matplotlib.pyplot as plt
from pathlib import Path

def extract_creation_time(filepath):
    """
    Extrae el tiempo de creación en microsegundos del archivo de salida.
    Retorna el tiempo en microsegundos o None si no lo encuentra.
    """
    try:
        with open(filepath, 'r') as f:
            content = f.read()
            match = re.search(r'DURATION_MICROSECONDS=(\d+)', content)
            if match:
                microseconds = int(match.group(1))
                return microseconds
    except Exception as e:
        print(f"Error leyendo {filepath}: {e}")
    return None

def parse_filename(filename):
    """
    Extrae el tipo y N del nombre del archivo.
    Formato: {tipo}.n{N}
    Donde tipo puede ser: random-near, random-str, europa-near, europa-str
    """
    match = re.match(r'([a-z\-]+)\.n(\d+)', filename)
    if match:
        tipo = match.group(1)
        n = int(match.group(2))
        return tipo, n
    return None, None

def main():
    creation_dir = Path(r'./creation')
    
    # Diccionarios para almacenar datos por tipo
    data = {
        'Random + Nearest-X': {'N': [], 'time': []},
        'Random + STR': {'N': [], 'time': []},
        'Europa + Nearest-X': {'N': [], 'time': []},
        'Europa + STR': {'N': [], 'time': []}
    }
    
    # Mapeo de tipo a etiqueta
    type_mapping = {
        'random-near': 'Random + Nearest-X',
        'random-str': 'Random + STR',
        'europa-near': 'Europa + Nearest-X',
        'europa-str': 'Europa + STR'
    }
    
    # Procesar archivos
    if not creation_dir.exists():
        print(f"Error: No se encontró la carpeta {creation_dir}")
        return
    
    for filename in sorted(creation_dir.iterdir()):
        if filename.is_file():
            tipo, n = parse_filename(filename.name)
            if tipo and n:
                label = type_mapping.get(tipo)
                if label:
                    creation_time = extract_creation_time(filename)
                    if creation_time is not None:
                        data[label]['N'].append(n)
                        data[label]['time'].append(creation_time)
                        print(f"✓ {filename.name}: N={n}, Tiempo={creation_time:.2f} μs")
    
    # Crear gráfico
    plt.figure(figsize=(12, 7))
    
    colors = {
        'Random + Nearest-X': '#1f77b4',
        'Random + STR': '#ff7f0e',
        'Europa + Nearest-X': '#2ca02c',
        'Europa + STR': '#d62728'
    }
    
    markers = {
        'Random + Nearest-X': 'o',
        'Random + STR': 's',
        'Europa + Nearest-X': '^',
        'Europa + STR': 'D'
    }
    
    for label, color in colors.items():
        if data[label]['N']:  # Si hay datos
            # Ordenar por N
            sorted_pairs = sorted(zip(data[label]['N'], data[label]['time']))
            n_values = [x[0] for x in sorted_pairs]
            time_values = [x[1] for x in sorted_pairs]
            
            plt.plot(n_values, time_values, 
                    label=label, 
                    color=color, 
                    marker=markers[label],
                    linewidth=2,
                    markersize=6)
    
    # Configurar gráfico
    plt.xlabel('N (cantidad de elementos)', fontsize=12, fontweight='bold')
    plt.ylabel('Tiempo de creación (microsegundos)', fontsize=12, fontweight='bold')
    plt.title('Tiempo de Creación de R-trees en función de N', fontsize=14, fontweight='bold')
    plt.xscale('log')
    plt.yscale('log')
    plt.grid(True, alpha=0.3, linestyle='--')
    plt.legend(fontsize=11, loc='best')
    plt.tight_layout()
    
    # Guardar gráfico
    plt.savefig('creation_times.png', dpi=300, bbox_inches='tight')
    print("\n✓ Gráfico guardado en: creation_times.png")
    
    # Mostrar gráfico
    plt.show()

if __name__ == '__main__':
    main()
