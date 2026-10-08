########################## MAIN ###########################
CC = clang
CFLAGS = -Wall -Wextra -Wpedantic -std=c11 -g -Iinclude/ -DDEBUG -O3
###########################################################

####################### DIRECTORIES #######################
DIR_BIN = bin
DIR_OBJ = obj
DIR_SRC = src

DIR_TESTS 		   	  = test
DIR_TESTS_UNIT     	  = test/unit
DIR_TESTS_INTEGRATION = test/integration
DIR_TESTS_OUTPUT   	  = test/output

DIR_CORE 		  = $(wildcard src/core/*.c)
DIR_CORE_DS 	  = $(wildcard src/core/ds/*.c)
DIR_CORE_LANG 	  = $(wildcard src/core/lang/*.c)
DIR_CORE_PLATFORM = $(wildcard src/core/platform/*.c)

DIR_COMP 		  		   = $(wildcard src/compiler/*.c)
DIR_COMP_FRONTEND 		   = $(wildcard src/compiler/frontend/*.c)
DIR_COMP_FRONTEND_PARSER   = $(wildcard src/compiler/frontend/parser/*.c)
DIR_COMP_FRONTEND_LEXER    = $(wildcard src/compiler/frontend/lexer/*.c)
DIR_COMP_FRONTEND_SEMANTIC = $(wildcard src/compiler/frontend/semantic/*.c)

DIR_COMP_MIDDLE   = $(wildcard src/compiler/middle/*.c)
DIR_COMP_BACKEND  = $(wildcard src/compiler/backend/*.c)

DIR_COMPILER = $(DIR_COMP) $(DIR_CORE) $(DIR_CORE_DS) $(DIR_CORE_LANG) \
			   $(DIR_CORE_PLATFORM) $(DIR_COMP_FRONTEND) $(DIR_COMP_FRONTEND_PARSER) \
			   $(DIR_COMP_FRONTEND_LEXER) $(DIR_COMP_FRONTEND_SEMANTIC) $(DIR_COMP_MIDDLE) \
			   $(DIR_COMP_BACKEND)

DIR_CLI     	 = $(wildcard src/cli/*.c)
DIR_CLI_COMMANDS = $(wildcard src/cli/commands/*.c)
###########################################################

####################### EXECUTABLE ########################
# Main executable
EXEC_MAIN = $(DIR_BIN)/crum

# Compiler tests
EXEC_TEST_LEXER    = $(DIR_TESTS_OUTPUT)/lexing
EXEC_TEST_PARSER   = $(DIR_TESTS_OUTPUT)/parsing
EXEC_TEST_SEMANTIC = $(DIR_TESTS_OUTPUT)/analisis
EXEC_TEST_CODEGEN  = $(DIR_TESTS_OUTPUT)/codegen

# Other tests
EXEC_TEST_ARENA   = $(DIR_TESTS_OUTPUT)/arena
EXEC_TEST_STRINGS = $(DIR_TESTS_OUTPUT)/strings
EXEC_TEST_HASHMAP = $(DIR_TESTS_OUTPUT)/hashmap
EXEC_TEST_CLI     = $(DIR_TESTS_OUTPUT)/cli
###########################################################

###################### SOURCE FILES #######################
SRC_MAIN = $(DIR_SRC)/main.c \
		   $(DIR_CLI) \
		   $(DIR_CLI_COMMANDS) \
		   $(DIR_COMPILER) \
		   $(DIR_RUNTIME)

SRC_TEST_LEXER    = $(DIR_TESTS_INTEGRATION)/lexing.c   $(DIR_COMPILER)
SRC_TEST_PARSER   = $(DIR_TESTS_INTEGRATION)/parsing.c  $(DIR_COMPILER)
SRC_TEST_SEMANTIC = $(DIR_TESTS_INTEGRATION)/analisis.c $(DIR_COMPILER)
SRC_TEST_CODEGEN  = $(DIR_TESTS_INTEGRATION)/codegen.c  $(DIR_COMPILER)

SRC_TEST_ARENA   = $(DIR_TESTS_UNIT)/arena.c   $(DIR_CORE_DS)
SRC_TEST_STRINGS = $(DIR_TESTS_UNIT)/strings.c $(DIR_CORE_DS)
SRC_TEST_HASHMAP = $(DIR_TESTS_UNIT)/hashmap.c $(DIR_CORE_DS)
SRC_TEST_CLI     = $(DIR_TESTS_INTEGRATION)/cli.c $(DIR_CLI) $(DIR_CLI_COMMANDS) $(DIR_CORE_DS)
###########################################################

################### COMPILE TO OBJECTS ####################
OBJS_MAIN =	$(patsubst %.c, $(DIR_OBJ)/%.o, $(SRC_MAIN))

OBJS_TEST_LEXER    = $(patsubst %.c, $(DIR_OBJ)/%.o, $(SRC_TEST_LEXER))
OBJS_TEST_PARSER   = $(patsubst %.c, $(DIR_OBJ)/%.o, $(SRC_TEST_PARSER))
OBJS_TEST_SEMANTIC = $(patsubst %.c, $(DIR_OBJ)/%.o, $(SRC_TEST_SEMANTIC))

OBJS_TEST_ARENA	  = $(patsubst %.c, $(DIR_OBJ)/%.o, $(SRC_TEST_ARENA))
OBJS_TEST_STRINGS  = $(patsubst %.c, $(DIR_OBJ)/%.o, $(SRC_TEST_STRINGS))
OBJS_TEST_HASHMAP  = $(patsubst %.c, $(DIR_OBJ)/%.o, $(SRC_TEST_HASHMAP))
OBJS_TEST_CLI      = $(patsubst %.c, $(DIR_OBJ)/%.o, $(SRC_TEST_CLI))
OBJS_TEST_CODEGEN  = $(patsubst %.c, $(DIR_OBJ)/%.o, $(SRC_TEST_CODEGEN))
###########################################################

####################### MAKE FLAGS ########################
.PHONY: all build clean

all: build

build: 	$(EXEC_MAIN) 		  \
		$(EXEC_TEST_LEXER) 	  \
		$(EXEC_TEST_PARSER)   \
		$(EXEC_TEST_SEMANTIC) \
		$(EXEC_TEST_CODEGEN)  \
		$(EXEC_TEST_ARENA) 	  \
		$(EXEC_TEST_STRINGS)  \
		$(EXEC_TEST_HASHMAP)  \
		$(EXEC_TEST_CLI)

run: build
	$(EXEC_MAIN)

install:
	sudo mkdir -p /usr/local/bin/
	sudo cp $(EXEC_MAIN) /usr/local/bin/

clean:
	rm -rf $(DIR_BIN) $(DIR_OBJ) $(DIR_TESTS_OUTPUT)
###########################################################

################## COMPILE TO EXECUTABLE ##################
$(DIR_BIN):
	mkdir -p $(DIR_BIN)

$(DIR_TESTS_OUTPUT):
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

$(EXEC_TEST_STRINGS): $(OBJS_TEST_STRINGS) | $(DIR_TESTS_OUTPUT)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $^ -o $@

$(EXEC_TEST_CODEGEN): $(OBJS_TEST_CODEGEN) | $(DIR_TESTS_OUTPUT)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $^ -o $@

$(EXEC_TEST_HASHMAP): $(OBJS_TEST_HASHMAP) | $(DIR_TESTS_OUTPUT)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $^ -o $@

$(EXEC_TEST_CLI): $(OBJS_TEST_CLI) | $(DIR_TESTS_OUTPUT)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $^ -o $@
###########################################################