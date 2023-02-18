#!/bin/bash

# echo "running Valgrind"
# clang++ -g -O3 -Wall -Wextra -pedantic -Werror -std=c++20 -pthread -fPIE -pie order.cpp safemap_test.cpp -o a.out
# valgrind ./a.out
# echo ""

echo "running TSAN"
clang++ -g -O3 -Wall -Wextra -pedantic -Werror -std=c++20 -pthread -fsanitize=thread -fPIE -pie order.cpp safemap_test.cpp -o a.tsan
./a.tsan
echo ""

echo "running ASAN"
clang++ -g -O3 -Wall -Wextra -pedantic -Werror -std=c++20 -pthread -fsanitize=address -fPIE -pie order.cpp safemap_test.cpp -o a.asan
./a.asan
echo ""

# rm a.out 
rm a.tsan 
rm a.asan