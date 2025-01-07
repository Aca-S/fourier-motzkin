cmake_minimum_required(VERSION 3.23)
project(fourier_motzkin)

set(CMAKE_CXX_STANDARD 20)

add_executable(fourier_motzkin
    main.cpp
    fourier_motzkin.hpp
    fraction.cpp
    fraction.hpp
)
