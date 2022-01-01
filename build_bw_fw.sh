#!/bin/bash

clang++ -std=c++20 -o yokai_bw -Ofast -march=znver3 -mtune=znver3 yokai_bw.cpp
# clang++ -Wall -std=c++20 -o yokai_bw -O0 -g yokai_bw.cpp

clang++ -std=c++20 -o yokai_fw -Ofast -march=znver3 -mtune=znver3 yokai_fw.cpp
# clang++ -Wall -std=c++20 -o yokai_fw -O0 -g yokai_fw.cpp
