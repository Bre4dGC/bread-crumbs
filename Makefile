CC = cc
CFLAGS = -Wall -Wextra -std=c11 -I include/
SRC_DIR = src
BIN_DIR = bin

MAIN = $(BIN_DIR)/BreadCrumbs
ERRTEST = $(BIN_DIR)/errtest
LEXTEST = $(BIN_DIR)/lextest
PARSTEST = $(BIN_DIR)/parstest

SRCS_MAIN = $(SRC_DIR)/main.c $(SRC_DIR)/lexer.c $(SRC_DIR)/tokenizer.c $(SRC_DIR)/parser.c $(SRC_DIR)/ast.c $(SRC_DIR)/errors.c $(SRC_DIR)/vm.c
SRCS_ERRTEST = tests/test_errors.c $(SRC_DIR)/errors.c
SRCS_LEXTEST = tests/test_lexer.c $(SRC_DIR)/lexer.c $(SRC_DIR)/tokenizer.c $(SRC_DIR)/errors.c
SRCS_PARSTEST = tests/test_parser.c $(SRC_DIR)/parser.c $(SRC_DIR)/lexer.c $(SRC_DIR)/tokenizer.c $(SRC_DIR)/ast.c $(SRC_DIR)/errors.c

.PHONY: all build test clean

all: build

build: $(MAIN) $(ERRTEST) $(LEXTEST) $(PARSTEST)

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

test: build
	$(ERRTEST) | cat
	$(LEXTEST) | cat
	$(PARSTEST) | cat

clean:
	rm -rf $(BIN_DIR)
