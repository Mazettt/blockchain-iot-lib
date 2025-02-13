CC = g++
ifeq ($(shell uname), Darwin)
CFLAGS = -std=c++20 -Wall -Wextra -fPIC -I$(SRC_DIR) -I/opt/homebrew/Cellar/openssl@3/3.4.1/include
LDFLAGS = -L/opt/homebrew/Cellar/openssl@3/3.4.1/lib -lssl -lcrypto
else
CFLAGS = -std=c++20 -Wall -Wextra -fPIC -I$(SRC_DIR)
LDFLAGS = -lssl -lcrypto
endif
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
	$(CC) $(CFLAGS) -o $(EXECUTABLE) $(TEST_OBJ) $(LDFLAGS) -L. -lblockchain

$(LIBRARY): $(filter-out $(TEST_OBJ), $(OBJ))
	$(CC) -shared -o $@ $^ $(LDFLAGS)

$(SRC_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CC) $(CFLAGS) -c $< -o $@

$(TEST_DIR)/%.o: $(TEST_DIR)/%.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TEST_OBJ) $(EXECUTABLE) $(LIBRARY)

re: clean all
