
CC := clang
CSTD := -std=c23
WARN := -Wall -Wextra -Wpedantic -Werror
SAN := -fsanitize=address,undefined

INCLUDE_DIR := include
INCLUDES := -I$(INCLUDE_DIR)

DEBUG := -g -Og -fno-omit-frame-pointer

CFLAGS := $(CSTD) $(WARN) $(SAN) $(INCLUDES) $(DEBUG) -MMD -MP
LDFLAGS := $(SAN)

SRC_DIR := src
OBJ_DIR := build

SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRCS))
DEPS := $(OBJS:.o=.d)

BIN := l.kafo.dev

.PHONY: all run format tidy clean
all: $(BIN)

run: $(BIN)
	./$(BIN)

$(BIN): $(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

format:
	clang-format -i --style=file $(SRCS) $(wildcard $(INCLUDE_DIR)/*.h)

tidy:
	bear -- $(MAKE) -B $(BIN)
	clang-tidy -p . --fix --warnings-as-errors='*' $(SRCS)

clean:
	rm -rf $(OBJ_DIR) $(BIN) compile_commands.json

-include $(DEPS)

.DELETE_ON_ERROR:
