cmake_minimum_required(VERSION 3.23)
project(fourier_motzkin)

set(CMAKE_CXX_STANDARD 20)

find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)

bison_target(
    fol_parser
    fol_parser.ypp
    ${CMAKE_CURRENT_BINARY_DIR}/fol_parser.tab.cpp
)

flex_target(
    fol_lexer
    fol_lexer.l
    ${CMAKE_CURRENT_BINARY_DIR}/lex.yy.cpp
)

add_executable(fourier_motzkin
    main.cpp
    fourier_motzkin.hpp
    fraction.cpp
    fraction.hpp
    fol_ast.cpp
    fol_ast.hpp
    fol_driver.cpp
    fol_driver.hpp
    ${BISON_fol_parser_OUTPUTS}
    ${FLEX_fol_lexer_OUTPUTS}
)

target_include_directories(
    fourier_motzkin PUBLIC
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)
