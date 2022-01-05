#!/bin/bash

# ARCH=znver2

# clang++ -std=c++20 -o yokai_bw -Ofast -march=${ARCH} -mtune=${ARCH} yokai_bw.cpp -pthread
# # clang++ -Wall -std=c++20 -o yokai_bw -O0 -g yokai_bw.cpp -pthread

# clang++ -std=c++20 -o yokai_fw -Ofast -march=${ARCH} -mtune=${ARCH} yokai_fw.cpp -pthread
# # clang++ -Wall -std=c++20 -o yokai_fw -O0 -g yokai_fw.cpp -pthread


ARCH=haswell

clang++-12 -std=c++20 -o yokai_fw_bw -Ofast yokai_fw_bw.cpp -pthread
# clang++ -std=c++20 -o yokai_fw_bw -O0 -g yokai_fw_bw.cpp -pthread
