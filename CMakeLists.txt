cmake_minimum_required(VERSION 3.10)

project(bread-crumbs LANGUAGES C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(${CMAKE_SOURCE_DIR}/include)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -pedantic -std=c11 -g -DDEBUG")

set(CORE_SRC
    src/core/arena.c
    src/core/diagnostic.c
    src/core/hashmap.c
    src/core/strings.c
)

set(FRONTEND_SRC
    src/compiler/frontend/lexer/tokens.c
    src/compiler/frontend/lexer.c
    src/compiler/frontend/ast.c
    src/compiler/frontend/parser.c
    src/compiler/frontend/semantic/types.c
    src/compiler/frontend/semantic/symbol.c
    src/compiler/frontend/semantic.c
)

set(BACKEND_SRC
    src/compiler/backend/native/compiler.c
    src/compiler/backend/native/asm.c
)

set(MIDDLE_SRC
    src/compiler/middle/ir.c
    src/compiler/middle/builder.c
    src/compiler/middle/optimizer.c
)

set(RUNTIME_SRC
    src/compiler/runtime/ffi.c
    src/compiler/runtime/memory.c
    src/compiler/runtime/gc.c
    src/compiler/runtime/profiler.c
)

add_library(core_lib STATIC ${CORE_SRC})
add_library(frontend_lib STATIC ${FRONTEND_SRC})
add_library(runtime_lib STATIC ${RUNTIME_SRC})

add_library(compiler_lib STATIC ${MIDDLE_SRC} ${BACKEND_SRC})
target_link_libraries(compiler_lib PRIVATE core_lib frontend_lib runtime_lib)

add_executable(crum src/main.c)
target_link_libraries(crum PRIVATE compiler_lib runtime_lib)

add_executable(lexing test/unit/lexing.c)
target_link_libraries(lexing PRIVATE core_lib frontend_lib runtime_lib)

add_executable(parsing test/unit/parsing.c)
target_link_libraries(parsing PRIVATE core_lib frontend_lib runtime_lib)

add_executable(analysis test/unit/analisis.c)
target_link_libraries(analysis PRIVATE core_lib frontend_lib runtime_lib)

install(TARGETS crum DESTINATION /usr/local/bin)