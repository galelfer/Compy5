cmake_minimum_required(VERSION 3.16)
project(compy5)

set(CMAKE_CXX_STANDARD 14)

add_executable(compy5 main.cpp hw3_output.hpp hw3_output.cpp hw3_aux.cpp hw3_aux.h parser.h parser.ypp scanner.lex bp.hpp bp.cpp llvm_code.cpp llvm_code.h)