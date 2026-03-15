mkdir -p ./bin
clang++ -O3 -std=c++20 -march=native -pthread worms2x2/bmpgen2x2.cpp -o ./bin/bmpgen2x2
clang++ -O3 -std=c++20 -march=native -pthread worms2x2/worms2x2.cpp -o ./bin/worms2x2
clang++ -O3 -std=c++20 -march=native -pthread worms3x3/bmpgen3x3.cpp -o ./bin/bmpgen3x3
clang++ -O3 -std=c++20 -march=native -pthread worms3x3/worms3x3.cpp -o ./bin/worms3x3
