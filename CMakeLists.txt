cmake_minimum_required(VERSION 3.10)

project(bread-crumbs LANGUAGES C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(${CMAKE_SOURCE_DIR}/include)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -pedantic -std=c11 -g -DDEBUG")

set(CORE_SRC
    src/core/ds/arena.c
    src/core/ds/hashmap.c
    src/core/ds/strings.c
    src/core/lang/filesystem.c
    src/core/lang/source.c
    src/core/lang/resolver.c
    src/core/lang/diagnostic.c
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
    src/compiler/backend/jit.c
    src/compiler/backend/native.c
)

set(MIDDLE_SRC
    src/compiler/middle/ir.c
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

add_library(core_lib STATIC ${CORE_SRC})
add_library(frontend_lib STATIC ${FRONTEND_SRC})

add_library(compiler_lib STATIC ${MIDDLE_SRC} ${BACKEND_SRC})
target_link_libraries(compiler_lib PRIVATE core_lib frontend_lib)

add_executable(crum src/main.c ${CLI_SRC})
target_link_libraries(crum PRIVATE compiler_lib)

add_executable(lexing test/integration/lexing.c)
target_link_libraries(lexing PRIVATE core_lib frontend_lib)

add_executable(parsing test/integration/parsing.c)
target_link_libraries(parsing PRIVATE core_lib frontend_lib)

add_executable(analysis test/integration/analisis.c)
target_link_libraries(analysis PRIVATE core_lib frontend_lib)

install(TARGETS crum DESTINATION /usr/local/bin)