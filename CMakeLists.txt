# CMakeLists.txt
# >> cmake -B ./build -S .

cmake_minimum_required(VERSION 3.26)

# state project
project(mi_memory_pool VERSION 0.1 LANGUAGES CXX)

# print message
message(STATUS "Cmake building...")

# entry subdirectory and cmake
add_subdirectory(mi_memory_pool)

# create executable file
add_executable(benchmark benchmark.cpp)

# link libraies
target_link_libraries(benchmark PRIVATE lib_mi_memory_pool)
