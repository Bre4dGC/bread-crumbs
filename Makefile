CC = clang
CFLAGS = -Wall -Wextra -pedantic -std=c11 -g -I include/ -DDEBUG
SRC_DIR = src
BIN_DIR = bin
OBJ_DIR = build

MAIN =     		$(BIN_DIR)/crumbs
TEST_ERRORS =  	$(BIN_DIR)/test_errors
TEST_LEXER =  	$(BIN_DIR)/test_lexer
TEST_PARSER = 	$(BIN_DIR)/test_parser
TEST_SEMANTIC = $(BIN_DIR)/test_semantic

SRCS_MAIN = $(SRC_DIR)/main.c 									\
            $(SRC_DIR)/compiler/frontend/tokenizer.c 			\
            $(SRC_DIR)/compiler/frontend/lexer.c 				\
            $(SRC_DIR)/compiler/frontend/ast.c 					\
            $(SRC_DIR)/compiler/frontend/parser.c 				\
            $(SRC_DIR)/compiler/frontend/types.c 				\
            $(SRC_DIR)/compiler/frontend/symbol.c 				\
            $(SRC_DIR)/compiler/frontend/semantic.c 			\
            $(SRC_DIR)/compiler/diagnostic/errors.c 			\
            $(SRC_DIR)/compiler/vm.c

SRCS_TEST_ERRORS = tests/test_errors.c 							\
                    $(SRC_DIR)/compiler/diagnostic/errors.c

SRCS_TEST_LEXER = tests/test_lexer.c 			    			\
                    $(SRC_DIR)/compiler/frontend/lexer.c 		\
                    $(SRC_DIR)/compiler/frontend/tokenizer.c 	\
                    $(SRC_DIR)/compiler/diagnostic/errors.c
				
SRCS_TEST_PARSER = tests/test_parser.c							\
                    $(SRC_DIR)/compiler/frontend/parser.c 		\
                    $(SRC_DIR)/compiler/frontend/lexer.c 		\
                    $(SRC_DIR)/compiler/frontend/tokenizer.c 	\
                    $(SRC_DIR)/compiler/frontend/ast.c 			\
                    $(SRC_DIR)/compiler/diagnostic/errors.c
				
SRCS_TEST_SEMANTIC = tests/test_semantic.c 						\
                    $(SRC_DIR)/compiler/frontend/tokenizer.c 	\
                    $(SRC_DIR)/compiler/frontend/lexer.c 		\
                    $(SRC_DIR)/compiler/frontend/ast.c 			\
                    $(SRC_DIR)/compiler/frontend/parser.c 		\
                    $(SRC_DIR)/compiler/frontend/types.c 		\
                    $(SRC_DIR)/compiler/frontend/symbol.c 		\
                    $(SRC_DIR)/compiler/frontend/semantic.c 	\
                    $(SRC_DIR)/compiler/diagnostic/errors.c

OBJS_MAIN	 	   = $(SRCS_MAIN:%.c=$(OBJ_DIR)/%.o)
OBJS_TEST_ERRORS   = $(SRCS_TEST_ERRORS:%.c=$(OBJ_DIR)/%.o)
OBJS_TEST_LEXER    = $(SRCS_TEST_LEXER:%.c=$(OBJ_DIR)/%.o)
OBJS_TEST_PARSER   = $(SRCS_TEST_PARSER:%.c=$(OBJ_DIR)/%.o)
OBJS_TEST_SEMANTIC = $(SRCS_TEST_SEMANTIC:%.c=$(OBJ_DIR)/%.o)

.PHONY: all build test clean

all: build

build: $(MAIN) $(TEST_ERRORS) $(TEST_LEXER) $(TEST_PARSER) $(TEST_SEMANTIC)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(OBJ_DIR)/%.o: %.c | $(OBJ_DIR)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(MAIN): $(OBJS_MAIN) | $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@

$(TEST_ERRORS): $(OBJS_TEST_ERRORS) | $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@

$(TEST_LEXER): $(OBJS_TEST_LEXER) | $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@

$(TEST_PARSER): $(OBJS_TEST_PARSER) | $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@

$(TEST_SEMANTIC): $(OBJS_TEST_SEMANTIC) | $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@

test: build
	$(TEST_ERRORS)   | cat
	$(TEST_LEXER) 	 | cat
	$(TEST_PARSER)	 | cat
	$(TEST_SEMANTIC) | cat

run: build
	$(MAIN)

install:
	sudo mkdir -p /usr/local/bin/
	sudo cp $(MAIN) /usr/local/bin/

clean:
	rm -rf $(BIN_DIR) $(OBJ_DIR)