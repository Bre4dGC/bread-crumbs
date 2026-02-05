########################## MAIN ###########################
CC = gcc
CFLAGS = 	-Wall -Wextra -Wpedantic -Wformat=2 		\
		 	-Wno-unused-parameter -Wshadow				\
			-Wwrite-strings -Wstrict-prototypes			\
			-Wold-style-definition -Wredundant-decls	\
			-Wnested-externs -Wmissing-include-dirs 	\
		 	-std=c11 -g -Iinclude/ -DDEBUG -O3
###########################################################

####################### DIRECTORIES #######################
DIR_BIN = bin
DIR_OBJ = obj
DIR_SRC = src

DIR_TESTS 		   = test
DIR_TESTS_OTHER    = test/unit

DIR_TESTS_OUTPUT    = $(DIR_BIN)/tests

DIR_COMP 		  = src/compiler
DIR_COMP_CORE 	  = $(wildcard src/core/*.c)
DIR_COMP_FRONTEND = $(wildcard src/compiler/frontend/*.c)
DIR_COMP_FRONTEND_PARSER = $(wildcard src/compiler/frontend/parser/*.c)
DIR_COMP_FRONTEND_LEXER  = $(wildcard src/compiler/frontend/lexer/*.c)
DIR_COMP_FRONTEND_SEMANTIC = $(wildcard src/compiler/frontend/semantic/*.c)
DIR_COMP_MIDDLE   = $(wildcard src/compiler/middle/*.c)
DIR_COMP_BACKEND  = $(wildcard src/compiler/backend/*.c)
DIR_RUNTIME 	  = $(wildcard src/compiler/runtime/*.c)

DIR_CLI     = $(wildcard src/cli/*.c)
DIR_CLI_COMMANDS = $(wildcard src/cli/commands/*.c)
###########################################################

####################### EXECUTABLE ########################
# Main executable
EXEC_MAIN = $(DIR_BIN)/crum

# Test runners
TEST_LEXER       = $(EXEC_TEST_LEXER)
TEST_PARSER      = $(EXEC_TEST_PARSER)
TEST_SEMANTIC    = $(EXEC_TEST_SEMANTIC)
TEST_ARENA       = $(EXEC_TEST_ARENA)
TEST_STRINGPOOL  = $(EXEC_TEST_STRINGPOOL)
TEST_HASHTABLE   = $(EXEC_TEST_HASHTABLE)
TEST_DIAGNOSTIC  = $(EXEC_TEST_DIAGNOSTIC)

# Compiler tests
EXEC_TEST_LEXER    = $(DIR_TESTS_OUTPUT)/lexing
EXEC_TEST_PARSER   = $(DIR_TESTS_OUTPUT)/parsing
EXEC_TEST_SEMANTIC = $(DIR_TESTS_OUTPUT)/analisis
EXEC_TEST_CODEGEN  = $(DIR_TESTS_OUTPUT)/codegen

# Other tests
EXEC_TEST_ARENA   	 = $(DIR_TESTS_OUTPUT)/arena
EXEC_TEST_STRINGPOOL = $(DIR_TESTS_OUTPUT)/stringpool
EXEC_TEST_HASHTABLE  = $(DIR_TESTS_OUTPUT)/hashmap
EXEC_TEST_DIAGNOSTIC = $(DIR_TESTS_OUTPUT)/diagnostic
###########################################################

###################### SOURCE FILES #######################
SRC_MAIN = $(DIR_SRC)/main.c \
	$(DIR_CLI) \
	$(DIR_CLI_COMMANDS) \
	$(DIR_COMP_CORE) \
	$(DIR_COMP_FRONTEND) \
	$(DIR_COMP_FRONTEND_PARSER) \
	$(DIR_COMP_FRONTEND_LEXER) \
	$(DIR_COMP_FRONTEND_SEMANTIC) \
	$(DIR_COMP_MIDDLE) \
	$(DIR_COMP_BACKEND) \
	$(DIR_RUNTIME)

SRC_TEST_LEXER = \
	$(DIR_TESTS_OTHER)/lexing.c \
	$(DIR_COMP_CORE) \
	$(DIR_SRC)/core/common/filesystem.c \
	$(DIR_SRC)/core/common/benchmark.c \
	$(DIR_SRC)/compiler/frontend/lexer.c \
	$(DIR_SRC)/compiler/frontend/lexer/tokens.c

SRC_TEST_PARSER = \
	$(DIR_TESTS_OTHER)/parsing.c \
	$(DIR_COMP_CORE) \
	$(DIR_SRC)/core/common/filesystem.c \
	$(DIR_SRC)/core/common/benchmark.c \
	$(DIR_SRC)/compiler/frontend/ast.c \
	$(DIR_SRC)/compiler/frontend/lexer.c \
	$(DIR_SRC)/compiler/frontend/lexer/tokens.c \
	$(DIR_SRC)/compiler/frontend/parser.c \
	$(DIR_SRC)/compiler/frontend/parser/decl.c \
	$(DIR_SRC)/compiler/frontend/parser/expr.c \
	$(DIR_SRC)/compiler/frontend/parser/stmt.c

SRC_TEST_SEMANTIC = \
	$(DIR_TESTS_OTHER)/analisis.c \
	$(DIR_COMP_CORE) \
	$(DIR_SRC)/core/common/filesystem.c \
	$(DIR_SRC)/core/common/benchmark.c \
	$(DIR_SRC)/compiler/frontend/ast.c \
	$(DIR_SRC)/compiler/frontend/lexer.c \
	$(DIR_SRC)/compiler/frontend/lexer/tokens.c \
	$(DIR_SRC)/compiler/frontend/parser.c \
	$(DIR_SRC)/compiler/frontend/parser/decl.c \
	$(DIR_SRC)/compiler/frontend/parser/expr.c \
	$(DIR_SRC)/compiler/frontend/parser/stmt.c \
	$(DIR_SRC)/compiler/frontend/semantic.c \
	$(DIR_SRC)/compiler/frontend/semantic/symbol.c \
	$(DIR_SRC)/compiler/frontend/semantic/types.c

SRC_TEST_ARENA = \
	$(DIR_TESTS_OTHER)/arena.c \
	$(DIR_COMP_CORE) \
	$(DIR_SRC)/core/common/benchmark.c

SRC_TEST_STRINGPOOL = \
	$(DIR_TESTS_OTHER)/strings.c \
	$(DIR_COMP_CORE) \
	$(DIR_SRC)/core/common/benchmark.c

SRC_TEST_HASHTABLE = \
	$(DIR_TESTS_OTHER)/hashmap.c \
	$(DIR_COMP_CORE) \
	$(DIR_SRC)/core/common/benchmark.c

SRC_TEST_DIAGNOSTIC = \
	$(DIR_TESTS_OTHER)/diagnostic.c \
	$(DIR_COMP_CORE) \
	$(DIR_SRC)/core/common/benchmark.c
###########################################################

################### COMPILE TO OBJECTS ####################
OBJS_MAIN = 		 $(patsubst %.c, $(DIR_OBJ)/%.o, $(SRC_MAIN))

OBJS_TEST_LEXER = 	 $(patsubst %.c, $(DIR_OBJ)/%.o, $(SRC_TEST_LEXER))
OBJS_TEST_PARSER = 	 $(patsubst %.c, $(DIR_OBJ)/%.o, $(SRC_TEST_PARSER))
OBJS_TEST_SEMANTIC = $(patsubst %.c, $(DIR_OBJ)/%.o, $(SRC_TEST_SEMANTIC))

OBJS_TEST_ARENA	 	 = $(patsubst %.c, $(DIR_OBJ)/%.o, $(SRC_TEST_ARENA))
OBJS_TEST_STRINGPOOL = $(patsubst %.c, $(DIR_OBJ)/%.o, $(SRC_TEST_STRINGPOOL))
OBJS_TEST_HASHTABLE	 = $(patsubst %.c, $(DIR_OBJ)/%.o, $(SRC_TEST_HASHTABLE))
OBJS_TEST_DIAGNOSTIC = $(patsubst %.c, $(DIR_OBJ)/%.o, $(SRC_TEST_DIAGNOSTIC))
###########################################################

####################### MAKE FLAGS ########################
.PHONY: all build test clean

all: build

build: 	$(EXEC_MAIN) 			\
		$(EXEC_TEST_LEXER) 		\
		$(EXEC_TEST_PARSER) 	\
		$(EXEC_TEST_SEMANTIC) 	\
	 	$(EXEC_TEST_ARENA) 		\
		$(EXEC_TEST_STRINGPOOL) \
		$(EXEC_TEST_HASHTABLE) 	\
		$(EXEC_TEST_DIAGNOSTIC)

test: build
	$(TEST_LEXER) 	   | cat
	$(TEST_PARSER)	   | cat
	$(TEST_SEMANTIC)   | cat
	$(TEST_ARENA) 	   | cat
	$(TEST_STRINGPOOL) | cat
	$(TEST_HASHTABLE)  | cat
	$(TEST_DIAGNOSTIC) | cat

run: build
	$(MAIN)

install:
	sudo mkdir -p /usr/local/bin/
	sudo cp $(MAIN) /usr/local/bin/

clean:
	rm -rf $(DIR_BIN) $(DIR_OBJ) $(DIR_TESTS_OUTPUT)
###########################################################

################## COMPILE TO EXECUTABLE ##################
$(DIR_BIN):
	mkdir -p $(DIR_BIN)

$(DIR_TESTS_OUTPUT): | $(DIR_BIN)
	mkdir -p $(DIR_TESTS_OUTPUT)

$(DIR_OBJ):
	mkdir -p $(DIR_OBJ)

$(DIR_OBJ)/%.o: %.c | $(DIR_OBJ)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(EXEC_MAIN): $(OBJS_MAIN) | $(DIR_BIN)
	$(CC) $(CFLAGS) $^ -o $@

$(EXEC_TEST_LEXER): $(OBJS_TEST_LEXER) | $(DIR_TESTS_OUTPUT)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $^ -o $@

$(EXEC_TEST_PARSER): $(OBJS_TEST_PARSER) | $(DIR_TESTS_OUTPUT)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $^ -o $@

$(EXEC_TEST_SEMANTIC): $(OBJS_TEST_SEMANTIC) | $(DIR_TESTS_OUTPUT)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $^ -o $@

$(EXEC_TEST_ARENA): $(OBJS_TEST_ARENA) | $(DIR_TESTS_OUTPUT)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $^ -o $@

$(EXEC_TEST_STRINGPOOL): $(OBJS_TEST_STRINGPOOL) | $(DIR_TESTS_OUTPUT)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $^ -o $@

$(EXEC_TEST_HASHTABLE): $(OBJS_TEST_HASHTABLE) | $(DIR_TESTS_OUTPUT)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $^ -o $@

$(EXEC_TEST_DIAGNOSTIC): $(OBJS_TEST_DIAGNOSTIC) | $(DIR_TESTS_OUTPUT)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $^ -o $@
###########################################################