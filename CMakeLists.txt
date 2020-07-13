cmake_minimum_required(VERSION 3.16)
project(getLogFile C)

find_package(MPI REQUIRED)
include_directories(${MPI_INCLUDE_PATH})
set(CMAKE_C_STANDARD 99)
SET(CMAKE_C_COMPILER /home/zhongqi/App/Openmpi/bin/mpicc)
SET(CMAKE_CXX_COMPILER /home/zhongqi/App/Openmpi/bin/mpic++)
add_executable(getLogFile main.c MPI_mod.c MPI_mod.h)