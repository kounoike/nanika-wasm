#!/bin/bash

. /opt/AMD/aocc-compiler-3.2.0/setenv_AOCC.sh

clang++ -o yokai_dfs_threads -std=c++20 -Ofast -march=znver3 -mtune=znver3 yokai_dfs_threads.cpp -lpthread
# clang++ -o yokai_dfs_threads -std=c++20 -O0 -g -march=znver3 -mtune=znver3 yokai_dfs_threads.cpp -pthread
# ~/llvm/clang+llvm-13.0.0-x86_64-linux-gnu-ubuntu-20.04/bin/clang++ -Wall -o yokai_dfs_threads -std=c++20 -O0 -g yokai_dfs_threads.cpp -pthread -stdlib=libc++
