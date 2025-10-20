CC = clang
CFLAGS = -Wall -Wextra -pedantic -std=c11 -g -I include/ -DDEBUG
SRC_DIR = src
BIN_DIR = bin

MAIN = $(BIN_DIR)/crumbs
ERRTEST = $(BIN_DIR)/errtest
LEXTEST = $(BIN_DIR)/lextest
PARSTEST = $(BIN_DIR)/parstest
SEMTEST = $(BIN_DIR)/semtest

SRCS_MAIN = $(SRC_DIR)/main.c \
				$(SRC_DIR)/compiler/lexer.c \
				$(SRC_DIR)/compiler/lexer/tokenizer.c \
				$(SRC_DIR)/compiler/parser.c \
				$(SRC_DIR)/compiler/parser/ast.c \
				$(SRC_DIR)/compiler/errors.c \
				$(SRC_DIR)/compiler/vm.c

SRCS_ERRTEST = tests/test_errors.c \
				$(SRC_DIR)/compiler/errors.c

SRCS_LEXTEST = tests/test_lexer.c \
				$(SRC_DIR)/compiler/lexer.c \
				$(SRC_DIR)/compiler/lexer/tokenizer.c \
				$(SRC_DIR)/compiler/errors.c
				
SRCS_PARSTEST = tests/test_parser.c \
				$(SRC_DIR)/compiler/parser.c \
				$(SRC_DIR)/compiler/lexer.c \
				$(SRC_DIR)/compiler/lexer/tokenizer.c \
				$(SRC_DIR)/compiler/parser/ast.c \
				$(SRC_DIR)/compiler/errors.c
				
SRCS_SEMTEST = tests/test_semantic.c \
				$(SRC_DIR)/compiler/semantic.c \
				$(SRC_DIR)/compiler/semantic/types.c \
				$(SRC_DIR)/compiler/semantic/symbol.c \
				$(SRC_DIR)/compiler/lexer.c \
				$(SRC_DIR)/compiler/lexer/tokenizer.c \
				$(SRC_DIR)/compiler/parser.c \
				$(SRC_DIR)/compiler/parser/ast.c \
				$(SRC_DIR)/compiler/errors.c

.PHONY: all build test clean

all: build

build: $(MAIN) $(ERRTEST) $(LEXTEST) $(PARSTEST) $(SEMTEST)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(MAIN): | $(BIN_DIR)
	$(CC) $(CFLAGS) $(SRCS_MAIN) -o $@

$(ERRTEST): | $(BIN_DIR)
	$(CC) $(CFLAGS) $(SRCS_ERRTEST) -o $@

$(LEXTEST): | $(BIN_DIR)
	$(CC) $(CFLAGS) $(SRCS_LEXTEST) -o $@

$(PARSTEST): | $(BIN_DIR)
	$(CC) $(CFLAGS) $(SRCS_PARSTEST) -o $@

$(SEMTEST): | $(BIN_DIR)
	$(CC) $(CFLAGS) $(SRCS_SEMTEST) -o $@

test: build
	$(ERRTEST) | cat
	$(LEXTEST) | cat
	$(PARSTEST)| cat
	$(SEMTEST) | cat

run: build
	./$(MAIN)

install:
	sudo mkdir -p /usr/local/bin/
	sudo cp $(MAIN) /usr/local/bin/

clean:
	rm -rf $(BIN_DIR)
