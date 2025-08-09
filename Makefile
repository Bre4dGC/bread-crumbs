FLAGS = -Wall -Wextra -I include/
SRC_DIR = src
OBJ_DIR = build
BIN_DIR = bin
TARGET = $(BIN_DIR)/BreadCrumbs

SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))

.PHONY: build run clean

build:
	mkdir -p $(BIN_DIR)
	cc $(SRCS) $(FLAGS) -o $(TARGET)

run:

clean:
	rm -rf $(BUILD_PATH) $(BIN_PATH)
