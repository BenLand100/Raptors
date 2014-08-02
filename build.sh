#!/bin/bash

g++ -I src -O3 -std=c++11 -fopenmp -march=native -mtune=native -o raptors src/*.cc
