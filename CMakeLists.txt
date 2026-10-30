cmake_minimum_required(VERSION 3.10)

project(bread-crumbs LANGUAGES C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(${CMAKE_SOURCE_DIR}/include)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -pedantic -std=c11 -g -DDEBUG")

find_program(LLVM_CONFIG llvm-config)
if(LLVM_CONFIG)
    execute_process(COMMAND ${LLVM_CONFIG} --includedir
                    OUTPUT_VARIABLE LLVM_INCLUDE_DIR
                    OUTPUT_STRIP_TRAILING_WHITESPACE)
    execute_process(COMMAND ${LLVM_CONFIG} --libs core support mcjit native
                    OUTPUT_VARIABLE LLVM_LIBS
                    OUTPUT_STRIP_TRAILING_WHITESPACE)
    execute_process(COMMAND ${LLVM_CONFIG} --ldflags
                    OUTPUT_VARIABLE LLVM_LDFLAGS
                    OUTPUT_STRIP_TRAILING_WHITESPACE)
    if(LLVM_INCLUDE_DIR)
        include_directories(${LLVM_INCLUDE_DIR})
    endif()
else()
    message(STATUS "llvm-config not found; LLVM backend will not be linked")
endif()

set(CORE_SRC
    src/core/ds/arena.c
    src/core/ds/hashmap.c
    src/core/ds/strings.c
    src/core/lang/filesystem.c
    src/core/lang/source.c
    src/core/lang/diagnostic.c
)

set(FRONTEND_SRC
    src/compiler/frontend/lexer/tokens.c
    src/compiler/frontend/lexer.c
    src/compiler/frontend/ast.c
    src/compiler/frontend/parser.c
    src/compiler/frontend/parser/decl.c
    src/compiler/frontend/parser/expr.c
    src/compiler/frontend/parser/stmt.c
    src/compiler/frontend/semantic/types.c
    src/compiler/frontend/semantic/symbol.c
    src/compiler/frontend/semantic.c
)

set(BACKEND_SRC
    src/compiler/backend/jit.c
    src/compiler/backend/native.c
    src/compiler/backend/codegen.c
)

set(MIDDLE_SRC
    src/compiler/middle/ir.c
)

set(COMPILER_SRC
    src/compiler/context.c
)

set(CLI_SRC
    src/cli/args.c
    src/cli/core.c
    src/cli/register.c
    src/cli/commands/commands.c
    src/cli/commands/run.c
    src/cli/commands/build.c
    src/cli/commands/check.c
    src/cli/commands/repl.c
    src/cli/commands/doc.c
    src/cli/commands/new.c
    src/cli/commands/init.c
    src/cli/commands/debug.c
    src/cli/commands/test.c
    src/cli/commands/fmt.c
    src/cli/commands/install.c
    src/cli/commands/add.c
    src/cli/commands/remove.c
)

set(RUNTIME_SRC
    src/runtime/io.c
)

add_library(core_lib STATIC ${CORE_SRC})
add_library(frontend_lib STATIC ${FRONTEND_SRC})

add_library(breadcrumbs_rt STATIC ${RUNTIME_SRC})
target_include_directories(breadcrumbs_rt PUBLIC ${CMAKE_SOURCE_DIR}/include)

add_library(compiler_lib STATIC ${MIDDLE_SRC} ${BACKEND_SRC} ${COMPILER_SRC})
target_link_libraries(compiler_lib PUBLIC core_lib frontend_lib)

add_executable(crum src/main.c ${CLI_SRC})
target_link_libraries(crum PRIVATE compiler_lib ${LLVM_LIBS})
target_compile_definitions(crum PRIVATE BRT_LIB_DIR="${CMAKE_BINARY_DIR}")
if(LLVM_LDFLAGS)
    target_link_options(crum PRIVATE ${LLVM_LDFLAGS})
endif()

add_executable(lexing test/integration/lexing.c)
target_link_libraries(lexing PRIVATE compiler_lib ${LLVM_LIBS})

add_executable(parsing test/integration/parsing.c)
target_link_libraries(parsing PRIVATE compiler_lib ${LLVM_LIBS})

add_executable(analysis test/integration/analisis.c)
target_link_libraries(analysis PRIVATE compiler_lib ${LLVM_LIBS})

install(TARGETS crum DESTINATION /usr/local/bin)
install(TARGETS breadcrumbs_rt ARCHIVE DESTINATION /usr/local/lib)