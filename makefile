# Compiler
CC := g++

# Compiler flags
CFLAGS := -g -Wall -Wextra -std=c++11

# Directories
SRC_DIR := src
BUILD_DIR := build
BIN_DIR := bin
INCLUDE_DIR := include

# Source files
SRCS := $(wildcard $(SRC_DIR)/*/*.cpp)

# Object files
OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SRCS))

# Binary targets
SERVER_TARGET := $(BIN_DIR)/server
CLIENT_TARGET := $(BIN_DIR)/client

# Server target
server: $(SERVER_TARGET)

# Client target
client: $(CLIENT_TARGET)

# Build rules
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@ -I$(INCLUDE_DIR)

# Linking for server
$(SERVER_TARGET): $(BUILD_DIR)/server/main.o $(BUILD_DIR)/network/network.o $(BUILD_DIR)/platform/platform.o $(BUILD_DIR)/server/server.o
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@

# Linking for client
$(CLIENT_TARGET): $(BUILD_DIR)/client/main.o $(BUILD_DIR)/network/network.o $(BUILD_DIR)/platform/platform.o $(BUILD_DIR)/client/client.o
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@

# Clean
clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

