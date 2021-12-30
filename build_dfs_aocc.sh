#!/bin/bash

. /opt/AMD/aocc-compiler-3.2.0/setenv_AOCC.sh

clang++ -o yokai_dfs -std=c++20 -Ofast -march=znver3 -mtune=znver3 yokai_dfs.cpp -lpthread
