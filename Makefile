########################## MAIN ###########################
CC = clang
CFLAGS = -Wall -Wextra -pedantic -std=c11 -g -I include/ -DDEBUG
###########################################################

####################### DIRECTORIES #######################
DIR_BIN   = bin
DIR_OBJ   = build
DIR_SRC   = src
DIR_TESTS = tests

DIR_COMP 		  = src/compiler
DIR_COMP_CORE 	  = src/compiler/core
DIR_COMP_FRONTEND = src/compiler/frontend
DIR_COMP_MIDDLE   = src/compiler/middle
DIR_COMP_BACKEND  = src/compiler/backend

DIR_CLI     = src/cli
DIR_RUNTIME = src/runtime
###########################################################

####################### EXECUTABLE ########################
EXEC_MAIN 		   = $(DIR_BIN)/crum
EXEC_TEST_LEXER    = $(DIR_TESTS)/output/lexing
EXEC_TEST_PARSER   = $(DIR_TESTS)/output/parsing
EXEC_TEST_SEMANTIC = $(DIR_TESTS)/output/analisis
###########################################################

###################### SOURCE FILES #######################
SRC_MAIN =  $(DIR_SRC)/main.c 							\
            $(DIR_COMP_CORE)/arena_alloc.c 				\
            $(DIR_COMP_CORE)/string_pool.c 				\
            $(DIR_COMP_CORE)/diagnostic.c 				\
            $(DIR_COMP_CORE)/hash_table.c 				\
            $(DIR_COMP_FRONTEND)/tokenizer.c 			\
            $(DIR_COMP_FRONTEND)/lexer.c 				\
            $(DIR_COMP_FRONTEND)/ast.c 					\
            $(DIR_COMP_FRONTEND)/parser.c 				\
            $(DIR_COMP_FRONTEND)/types.c 				\
            $(DIR_COMP_FRONTEND)/symbol.c 				\
            $(DIR_COMP_FRONTEND)/semantic.c 			\

SRC_TEST_LEXER = 	$(DIR_TESTS)/lexing.c 		    	\
					$(DIR_COMP_CORE)/arena_alloc.c 		\
					$(DIR_COMP_CORE)/string_pool.c 		\
					$(DIR_COMP_CORE)/diagnostic.c 		\
					$(DIR_COMP_CORE)/hash_table.c 		\
                    $(DIR_COMP_FRONTEND)/lexer.c 		\
                    $(DIR_COMP_FRONTEND)/tokenizer.c
				
SRC_TEST_PARSER = 	$(DIR_TESTS)/parsing.c				\
					$(DIR_COMP_CORE)/arena_alloc.c 		\
					$(DIR_COMP_CORE)/string_pool.c 		\
					$(DIR_COMP_CORE)/diagnostic.c 		\
					$(DIR_COMP_CORE)/hash_table.c 		\
                    $(DIR_COMP_FRONTEND)/tokenizer.c 	\
                    $(DIR_COMP_FRONTEND)/lexer.c 		\
                    $(DIR_COMP_FRONTEND)/ast.c			\
                    $(DIR_COMP_FRONTEND)/parser.c
				
SRC_TEST_SEMANTIC = $(DIR_TESTS)/analisis.c		 		\
					$(DIR_COMP_CORE)/arena_alloc.c 		\
					$(DIR_COMP_CORE)/string_pool.c 		\
					$(DIR_COMP_CORE)/diagnostic.c 		\
					$(DIR_COMP_CORE)/hash_table.c 		\
                    $(DIR_COMP_FRONTEND)/tokenizer.c 	\
                    $(DIR_COMP_FRONTEND)/lexer.c 		\
                    $(DIR_COMP_FRONTEND)/ast.c 			\
                    $(DIR_COMP_FRONTEND)/parser.c 		\
                    $(DIR_COMP_FRONTEND)/types.c 		\
                    $(DIR_COMP_FRONTEND)/symbol.c 		\
                    $(DIR_COMP_FRONTEND)/semantic.c
###########################################################

################### COMPILE TO OBJECTS ####################
OBJS_MAIN	 	   = $(SRC_MAIN:%.c=$(DIR_OBJ)/%.o)
OBJS_TEST_LEXER    = $(SRC_TEST_LEXER:%.c=$(DIR_OBJ)/%.o)
OBJS_TEST_PARSER   = $(SRC_TEST_PARSER:%.c=$(DIR_OBJ)/%.o)
OBJS_TEST_SEMANTIC = $(SRC_TEST_SEMANTIC:%.c=$(DIR_OBJ)/%.o)
###########################################################

####################### MAKE FLAGS ########################
.PHONY: all build test clean

all: build

build: $(EXEC_MAIN) $(EXEC_TEST_LEXER) $(EXEC_TEST_PARSER) $(EXEC_TEST_SEMANTIC)

test: build
	$(TEST_LEXER) 	 | cat
	$(TEST_PARSER)	 | cat
	$(TEST_SEMANTIC) | cat

run: build
	$(MAIN)

install:
	sudo mkdir -p /usr/local/bin/
	sudo cp $(MAIN) /usr/local/bin/

clean:
	rm -rf $(DIR_BIN) $(DIR_OBJ)
###########################################################

################## COMPILE TO EXECUTABLE ##################
$(DIR_BIN):
	mkdir -p $(DIR_BIN)

$(DIR_OBJ):
	mkdir -p $(DIR_OBJ)

$(DIR_OBJ)/%.o: %.c | $(DIR_OBJ)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(EXEC_MAIN): $(OBJS_MAIN) | $(DIR_BIN)
	$(CC) $(CFLAGS) $^ -o $@

$(EXEC_TEST_LEXER): $(OBJS_TEST_LEXER) | $(DIR_BIN)
	$(CC) $(CFLAGS) $^ -o $@

$(EXEC_TEST_PARSER): $(OBJS_TEST_PARSER) | $(DIR_BIN)
	$(CC) $(CFLAGS) $^ -o $@

$(EXEC_TEST_SEMANTIC): $(OBJS_TEST_SEMANTIC) | $(DIR_BIN)
	$(CC) $(CFLAGS) $^ -o $@
###########################################################