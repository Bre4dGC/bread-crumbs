cmake_minimum_required(VERSION 3.10)

project(bread-crumbs LANGUAGES C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(${CMAKE_SOURCE_DIR}/include)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -pedantic -std=c11 -g -DDEBUG")

set(CORE_SRC
    src/compiler/core/arena_alloc.c
    src/compiler/core/diagnostic.c
    src/compiler/core/hash_table.c
    src/compiler/core/string_pool.c
)

set(FRONTEND_SRC
    src/compiler/frontend/tokenizer.c
    src/compiler/frontend/lexer.c
    src/compiler/frontend/parser.c
    src/compiler/frontend/types.c
    src/compiler/frontend/symbol.c
    src/compiler/frontend/semantic.c
)

set(BACKEND_SRC
    src/compiler/backend/codegen.c
)
    
set(MIDDLE_SRC
    src/compiler/middle/ir.c
    src/compiler/middle/optimizer.c
)

set(RUNTIME_SRC
    src/runtime/ffi.c
    src/runtime/memory.c
    src/runtime/profiler.c
    src/runtime/gc.c
)

add_library(core_lib STATIC ${CORE_SRC})
add_library(frontend_lib STATIC ${FRONTEND_SRC})
add_library(runtime_lib STATIC ${RUNTIME_SRC})

add_library(compiler_lib STATIC ${MIDDLE_SRC} ${BACKEND_SRC})
target_link_libraries(compiler_lib PRIVATE core_lib frontend_lib runtime_lib)

add_executable(crum src/main.c)
target_link_libraries(crum PRIVATE compiler_lib runtime_lib)

add_executable(lexing test/compiler/lexing.c)
target_link_libraries(lexing PRIVATE core_lib frontend_lib runtime_lib)

add_executable(parsing test/compiler/parsing.c)
target_link_libraries(parsing PRIVATE core_lib frontend_lib runtime_lib)

add_executable(analysis test/compiler/analisis.c)
target_link_libraries(analysis PRIVATE core_lib frontend_lib runtime_lib)

install(TARGETS crum DESTINATION /usr/local/bin)