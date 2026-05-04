./nearest-x.bin ./data/europa_bonus.bin 16777216 > ./creation/europa-bonus-near.n
./sort-tile-recursive.bin ./data/europa_bonus.bin 16777216 > ./creation/europa-bonus-str.n

./query-bonus.bin ./nearest-x-europa_bonus.rtree > ./query/nearest-x-europa-bonus.csv
./query-bonus.bin ./sort-tile-recursive-europa_bonus.rtree > ./query/sort-tile-recursive-europa-bonus.csv
