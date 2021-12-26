#!/bin/bash

set -xe

/home/kounoike/llvm/clang+llvm-13.0.0-x86_64-linux-gnu-ubuntu-20.04/bin/clang++ -o yokai -Ofast -mllvm -polly -march=native -ffast-math yokai.cpp
