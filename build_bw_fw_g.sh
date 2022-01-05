#!/bin/bash

# g++ -std=c++20 -o yokai_bw -Ofast -march=armv8.2-a+fp16+rcpc+dotprod+crypto -mtune=neoverse-n1 yokai_bw.cpp -pthread
# # clang++ -Wall -std=c++20 -o yokai_bw -O0 -g yokai_bw.cpp -pthread

# g++ -std=c++20 -o yokai_fw -Ofast -march=armv8.2-a+fp16+rcpc+dotprod+crypto -mtune=neoverse-n1 yokai_fw.cpp -pthread
# # clang++ -Wall -std=c++20 -o yokai_fw -O0 -g yokai_fw.cpp -pthread

set -e

g++ -std=c++2a -o yokai_fw_bw -Ofast -march=armv8.2-a+fp16+rcpc+dotprod+crypto -mtune=neoverse-n1 yokai_fw_bw.cpp -Iinclude -pthread -lbz2
# g++ -std=c++2a -o yokai_fw_bw -O0 -g yokai_fw_bw.cpp -Iinclude -pthread -lbz2
