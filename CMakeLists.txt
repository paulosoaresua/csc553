cmake_minimum_required(VERSION 3.10)
project(csc553)

message(${CMAKE_C_COMPILER})

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -DDEBUG")

include_directories(${PROJECT_SOURCE_DIR})
include_directories(${CMAKE_BINARY_DIR})

set(CFILES
        error.c
        main.c
        print.c
        symbol-table.c
        syntax-tree.c
        util.c
        instruction.c
        code_generation.c
        code_translation.c
        code_optimization.c
        control_flow.c
        block.c
        set.c
        reaching_definitions_analysis.c
        liveness_analysis.c
        graph.c
        stack.c
        max_heap.c)

find_package(BISON 3.7.1)
find_package(FLEX 2.5.4)

BISON_TARGET(parser parser.y ${CMAKE_BINARY_DIR}/y.tab.c)
FLEX_TARGET(scanner scanner.l ${CMAKE_BINARY_DIR}/lex.yy.c)
ADD_FLEX_BISON_DEPENDENCY(scanner parser)

add_executable(compile
        ${CFILES}
        ${BISON_parser_OUTPUTS}
        ${FLEX_scanner_OUTPUTS}
        )
