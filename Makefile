CXX = g++
FLAGS = -Wall -Wextra -Wpedantic -O3 -o

.PHONY: all nearest str query clean

all:
	make nearest
	make str
	make query
	make bonus

nearest:
	$(CXX) nearest-x.cpp $(FLAGS) nearest-x.bin

str:
	$(CXX) sort-tile-recursive.cpp $(FLAGS) sort-tile-recursive.bin

query:
	$(CXX) query.cpp $(FLAGS) query.bin

bonus:
	$(CXX) query_bonus.cpp $(FLAGS) query-bonus.bin

clean:
	@echo "--- Limpiando archivos generados ---"
	rm -f *.bin *.rtree *.png