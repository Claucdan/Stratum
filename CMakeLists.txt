cmake_minimum_required(VERSION 3.20)

project(Stratum VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(stratum INTERFACE)
add_library(stratum::stratum ALIAS stratum )

target_compile_features(stratum INTERFACE cxx_std_20)
target_include_directories(stratum INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)
