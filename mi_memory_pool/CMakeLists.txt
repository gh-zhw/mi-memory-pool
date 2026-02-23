message("Building mi_memory_pool...")

add_library(lib_mi_memory_pool ./memory_pool.cpp)

target_include_directories(lib_mi_memory_pool PUBLIC .)

