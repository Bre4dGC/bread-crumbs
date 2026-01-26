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

DIR_TESTS = test
DIR_TESTS_OUTPUT   = test/output
DIR_TESTS_COMPILER = test/compiler
DIR_TESTS_OTHER    = test/other

DIR_COMP 		  = src/compiler
DIR_COMP_CORE 	  = $(wildcard src/compiler/core/*.c)
DIR_COMP_FRONTEND = $(wildcard src/compiler/frontend/*.c)
DIR_COMP_MIDDLE   = $(wildcard src/compiler/middle/*.c)
DIR_COMP_BACKEND  = $(wildcard src/compiler/backend/*.c)

DIR_CLI     = $(wildcard src/cli/*.c)
DIR_RUNTIME = $(wildcard src/runtime/*.c)
###########################################################

####################### EXECUTABLE ########################
EXEC_MAIN = $(DIR_BIN)/crum

EXEC_TEST_LEXER    = $(DIR_TESTS_OUTPUT)/lexing
EXEC_TEST_PARSER   = $(DIR_TESTS_OUTPUT)/parsing
EXEC_TEST_SEMANTIC = $(DIR_TESTS_OUTPUT)/analisis
EXEC_TEST_CODEGEN  = $(DIR_TESTS_OUTPUT)/codegen

EXEC_TEST_ARENA   	 = $(DIR_TESTS_OUTPUT)/arena
EXEC_TEST_STRINGPOOL = $(DIR_TESTS_OUTPUT)/stringpool
EXEC_TEST_HASHTABLE  = $(DIR_TESTS_OUTPUT)/hashtable
EXEC_TEST_DIAGNOSTIC = $(DIR_TESTS_OUTPUT)/diagnostic
###########################################################

###################### SOURCE FILES #######################
SRC_MAIN =  $(DIR_SRC)/main.c 							\
            $(DIR_COMP_CORE)   			 				\
            $(DIR_COMP_FRONTEND)			 			\
			$(DIR_COMP_MIDDLE)							\
			$(DIR_COMP_BACKEND)							\
			$(DIR_CLI)		 							\
			$(DIR_RUNTIME)

SRC_TEST_LEXER = 	$(DIR_TESTS)/compiler/lexing.c 		\
					$(DIR_COMP_CORE)			 		\
					src/compiler/frontend/tokenizer.c 	\
					src/compiler/frontend/lexer.c
				
SRC_TEST_PARSER = 	$(DIR_TESTS)/compiler/parsing.c		\
					$(DIR_COMP_CORE)					\
                    src/compiler/frontend/tokenizer.c 	\
                    src/compiler/frontend/lexer.c 		\
                    src/compiler/frontend/ast.c			\
                    src/compiler/frontend/parser.c
				
SRC_TEST_SEMANTIC = $(DIR_TESTS)/compiler/analisis.c	\
					$(DIR_COMP_CORE)					\
					${DIR_COMP_FRONTEND}

SRC_TEST_ARENA 		= $(DIR_TESTS)/other/arena.c 		$(DIR_COMP_CORE)
SRC_TEST_STRINGPOOL = $(DIR_TESTS)/other/stringpool.c 	$(DIR_COMP_CORE)
SRC_TEST_HASHTABLE 	= $(DIR_TESTS)/other/hashtable.c 	$(DIR_COMP_CORE)
SRC_TEST_DIAGNOSTIC = $(DIR_TESTS)/other/diagnostic.c 	$(DIR_COMP_CORE)
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

$(DIR_OBJ):
	mkdir -p $(DIR_OBJ)

$(DIR_TESTS_OUTPUT):
	mkdir -p $(DIR_TESTS_OUTPUT)

$(DIR_OBJ)/%.o: %.c | $(DIR_OBJ)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(EXEC_MAIN): $(OBJS_MAIN) | $(DIR_BIN)
	$(CC) $(CFLAGS) $^ -o $@

$(EXEC_TEST_LEXER): $(OBJS_TEST_LEXER) | $(DIR_TEST_OUTPUT)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $^ -o $@

$(EXEC_TEST_PARSER): $(OBJS_TEST_PARSER) | $(DIR_TEST_OUTPUT)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $^ -o $@

$(EXEC_TEST_SEMANTIC): $(OBJS_TEST_SEMANTIC) | $(DIR_TEST_OUTPUT)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $^ -o $@

$(EXEC_TEST_ARENA): $(OBJS_TEST_ARENA) | $(DIR_TEST_OUTPUT)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $^ -o $@

$(EXEC_TEST_STRINGPOOL): $(OBJS_TEST_STRINGPOOL) | $(DIR_TEST_OUTPUT)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $^ -o $@

$(EXEC_TEST_HASHTABLE): $(OBJS_TEST_HASHTABLE) | $(DIR_TEST_OUTPUT)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $^ -o $@

$(EXEC_TEST_DIAGNOSTIC): $(OBJS_TEST_DIAGNOSTIC) | $(DIR_TEST_OUTPUT)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $^ -o $@
###########################################################