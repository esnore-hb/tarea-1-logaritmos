CXX = g++
FLAGS = -Wall -Wextra -Wpedantic -O3 -o

.PHONY: all nearest str query clean plot-query

all:
	make nearest
	make str
	make query

plot-query:
	python3 plot_query_results.py

nearest:
	$(CXX) nearest-x.cpp $(FLAGS) nearest-x.bin

str:
	$(CXX) sort-tile-recursive.cpp $(FLAGS) sort-tile-recursive.bin

query:
	$(CXX) query.cpp $(FLAGS) query.bin

clean:
	@echo "--- Limpiando archivos generados ---"
	rm -f *.bin *.rtree *.png