# Compiler
CC := g++

# Compiler flags
CFLAGS := -Wall -Wextra -std=c++11

# Directories
SRC_DIR := src
BUILD_DIR := build
BIN_DIR := bin
INCLUDE_DIR := include

# Source files
SRCS := $(wildcard $(SRC_DIR)/*.cpp)

# Object files
OBJS := $(SRCS:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)

# Binary targets
SERVER_TARGET := $(BIN_DIR)/server
CLIENT_TARGET := $(BIN_DIR)/client

# Server target
server: $(SERVER_TARGET)

# Client target
client: $(CLIENT_TARGET)

# All target 
all: server client

# Build rules
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

# Linking for server
$(SERVER_TARGET): $(BUILD_DIR)/server.o $(BUILD_DIR)/network.o $(BUILD_DIR)/platform.o
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $^ -o $@

# Linking for client
$(CLIENT_TARGET): $(BUILD_DIR)/client.o $(BUILD_DIR)/network.o $(BUILD_DIR)/platform.o
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $^ -o $@
# Clean
clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

