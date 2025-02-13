CC = g++
CFLAGS = -Wall -Wextra -std=c++20 -fPIC -I$(SRC_DIR)
SRC_DIR = src
TEST_DIR = test
LIBRARY = libblockchain.so
EXECUTABLE = main

SRC = $(shell find $(SRC_DIR) -type f -name '*.cpp')
TEST_SRC = $(shell find $(TEST_DIR) -type f -name '*.cpp')
OBJ = $(SRC:.cpp=.o)
TEST_OBJ = $(TEST_SRC:.cpp=.o)

.PHONY: all clean

all: $(LIBRARY) $(TEST_OBJ)
	$(CC) $(CFLAGS) -o $(EXECUTABLE) $(TEST_OBJ) -L. -lblockchain

$(LIBRARY): $(filter-out $(TEST_OBJ), $(OBJ))
	$(CC) -shared -o $@ $^

$(SRC_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CC) $(CFLAGS) -c $< -o $@

$(TEST_DIR)/%.o: $(TEST_DIR)/%.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TEST_OBJ) $(EXECUTABLE) $(LIBRARY)

re: clean all
