#!/usr/bin/env python3
"""
Script para graficar los resultados de consultas en función de s.
Muestra:
1. La cantidad promedio de lecturas a disco en función de s
2. La cantidad promedio de puntos encontrados en función de s
"""

import os
import re
import sys
import matplotlib
matplotlib.use('TkAgg')  # Usar backend no interactivo
import matplotlib.pyplot as plt
from pathlib import Path

def extract_query_data(filepath):
    """
    Extrae los datos de consulta del archivo.
    Retorna listas de (s, lecturas_promedio, puntos_promedio, lecturas_std, puntos_std).
    """
    data = []
    try:
        with open(filepath, 'r') as f:
            lines = f.readlines()
            current_s = None
            current_lecturas = None
            current_lecturas_std = None
            current_puntos = None
            current_puntos_std = None
            
            for line in lines:
                line = line.strip()
                if line.startswith('s:'):
                    current_s = float(line.split(':')[1])
                elif line.startswith('result_lecturas_promedio:'):
                    current_lecturas = float(line.split(':')[1])
                elif line.startswith('result_lecturas_std:'):
                    current_lecturas_std = float(line.split(':')[1])
                elif line.startswith('result_puntos_promedio:'):
                    current_puntos = float(line.split(':')[1])
                elif line.startswith('result_puntos_std:'):
                    current_puntos_std = float(line.split(':')[1])
                    if current_s is not None and current_lecturas is not None:
                        data.append((current_s, current_lecturas, current_puntos, 
                                   current_lecturas_std, current_puntos_std))
    except Exception as e:
        print(f"Error leyendo {filepath}: {e}")
    return data

def parse_query_filename(filename):
    """
    Extrae el tipo y algoritmo del nombre del archivo.
    Formato: {algoritmo}-{dataset}.query
    Ejemplos: nearest-x-random.query, sort-tile-recursive-europa.query
    """
    match = re.match(r'([a-z\-]+)-(random|europa)\.query', filename)
    if match:
        algoritmo = match.group(1)
        dataset = match.group(2)
        return algoritmo, dataset
    return None, None

def main():
    # Usar ruta relativa desde el directorio actual
    query_dir = Path('query')
    
    # Diccionarios para almacenar datos
    data = {
        'Nearest-X + Random': [],
        'Nearest-X + Europa': [],
        'STR + Random': [],
        'STR + Europa': []
    }
    
    # Mapeo de (algoritmo, dataset) a etiqueta
    type_mapping = {
        ('nearest-x', 'random'): 'Nearest-X + Random',
        ('nearest-x', 'europa'): 'Nearest-X + Europa',
        ('sort-tile-recursive', 'random'): 'STR + Random',
        ('sort-tile-recursive', 'europa'): 'STR + Europa'
    }
    
    # Procesar archivos
    if not query_dir.exists():
        print(f"Error: No se encontró la carpeta {query_dir}")
        return
    
    for filename in sorted(query_dir.iterdir()):
        if filename.is_file() and filename.suffix == '.query':
            algoritmo, dataset = parse_query_filename(filename.name)
            if algoritmo and dataset:
                label = type_mapping.get((algoritmo, dataset))
                if label:
                    query_data = extract_query_data(filename)
                    if query_data:
                        data[label] = query_data
                        print(f"✓ {filename.name}: {len(query_data)} puntos de datos")
    
    # Preparar datos para graficar
    colors = {
        'Nearest-X + Random': '#1f77b4',
        'Nearest-X + Europa': '#2ca02c',
        'STR + Random': '#ff7f0e',
        'STR + Europa': '#d62728'
    }
    
    markers = {
        'Nearest-X + Random': 'o',
        'Nearest-X + Europa': '^',
        'STR + Random': 's',
        'STR + Europa': 'D'
    }
    
    # Crear figura con 2 subplots
    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(16, 6))
    
    # Gráfico 1: Lecturas a disco en función de s
    for label, query_data in data.items():
        if query_data:
            s_values = [item[0] for item in query_data]
            lecturas = [item[1] for item in query_data]
            lecturas_std = [item[3] for item in query_data]
            
            ax1.errorbar(s_values, lecturas, 
                        yerr=lecturas_std,
                        marker=markers[label],
                        color=colors[label],
                        label=label,
                        linewidth=2,
                        markersize=8,
                        capsize=5,
                        capthick=2,
                        alpha=0.8)
    
    ax1.set_xlabel('s (parámetro de consulta)', fontsize=12)
    ax1.set_ylabel('Cantidad promedio de lecturas a disco', fontsize=12)
    ax1.set_title('Lecturas a disco en función de s', fontsize=14, fontweight='bold')
    ax1.legend(fontsize=10)
    ax1.grid(True, alpha=0.3)
    ax1.set_yscale('log')
    
    # Gráfico 2: Puntos encontrados en función de s
    for label, query_data in data.items():
        if query_data:
            s_values = [item[0] for item in query_data]
            puntos = [item[2] for item in query_data]
            puntos_std = [item[4] for item in query_data]
            
            ax2.errorbar(s_values, puntos, 
                        yerr=puntos_std,
                        marker=markers[label],
                        color=colors[label],
                        label=label,
                        linewidth=2,
                        markersize=8,
                        capsize=5,
                        capthick=2,
                        alpha=0.8)
    
    ax2.set_xlabel('s (parámetro de consulta)', fontsize=12)
    ax2.set_ylabel('Cantidad promedio de puntos encontrados', fontsize=12)
    ax2.set_title('Puntos encontrados en función de s', fontsize=14, fontweight='bold')
    ax2.legend(fontsize=10)
    ax2.grid(True, alpha=0.3)
    ax2.set_yscale('log')
    
    plt.tight_layout()
    plt.savefig('query_results.png', dpi=300, bbox_inches='tight')
    print("\n✓ Gráfico guardado en: query_results.png")
    plt.show()

if __name__ == '__main__':
    main()
