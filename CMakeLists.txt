cmake_minimum_required(VERSION 3.22)
project(untitled3)

set(CMAKE_CXX_STANDARD 23)

add_executable(untitled3 main.cpp
        DoubleHashingHashTable.h
        Hash.h
        LinearProbingHashTable.h
        PrimeNumbers.h
        BucketHashTable.h
)
