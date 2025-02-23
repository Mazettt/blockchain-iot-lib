CC = g++ -g
ifeq ($(shell uname), Darwin)
CFLAGS = -std=c++20 -Wall -Wextra -fPIC -I$(SRC_DIR) -I/opt/homebrew/Cellar/openssl@3/3.4.1/include
LDFLAGS = -L/opt/homebrew/Cellar/openssl@3/3.4.1/lib -lssl -lcrypto -lsecp256k1
GTEST = /usr/lib/libgtest.a
# TODO: gtest path may be incorrect on macos
else
CFLAGS = -std=c++20 -Wall -Wextra -fPIC -I$(SRC_DIR)
LDFLAGS = -lssl -lcrypto -lsecp256k1
GTEST = /usr/lib/libgtest.a
endif

SRC_DIR = src
TEST_DIR = test
UNIT_TEST_DIR = unit_tests
LIBRARY = libblockchain.so
EXECUTABLE = main
UNIT_TEST_EXEC = run_unit_tests

SRC = $(shell find $(SRC_DIR) -type f -name '*.cpp')
TEST_SRC = $(shell find $(TEST_DIR) -type f -name '*.cpp')
UNIT_TEST_SRC = $(shell find $(UNIT_TEST_DIR) -type f -name '*.cpp')

OBJ = $(SRC:.cpp=.o)
TEST_OBJ = $(TEST_SRC:.cpp=.o)
UNIT_TEST_OBJ = $(UNIT_TEST_SRC:.cpp=.o)

.PHONY: all clean unit_tests

all: $(LIBRARY) $(TEST_OBJ)
	$(CC) $(CFLAGS) -o $(EXECUTABLE) $(TEST_OBJ) $(LDFLAGS) -L. -lblockchain

$(LIBRARY): $(filter-out $(TEST_OBJ), $(OBJ))
	$(CC) -shared -o $@ $^ $(LDFLAGS)

$(SRC_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CC) $(CFLAGS) -c $< -o $@

$(TEST_DIR)/%.o: $(TEST_DIR)/%.cpp
	$(CC) $(CFLAGS) -c $< -o $@

$(UNIT_TEST_DIR)/%.o: $(UNIT_TEST_DIR)/%.cpp
	$(CC) $(CFLAGS) -I./unit_tests -c $< -o $@

unit_tests: LDFLAGS += -lgtest -lgtest_main -lpthread $(GTEST)
unit_tests: $(LIBRARY) $(UNIT_TEST_OBJ)
	$(CC) $(CFLAGS) -o $(UNIT_TEST_EXEC) $(UNIT_TEST_OBJ) $(LDFLAGS) -L. -lblockchain

clean:
	rm -f $(OBJ) $(TEST_OBJ) $(UNIT_TEST_OBJ) $(EXECUTABLE) $(LIBRARY) $(UNIT_TEST_EXEC)

re: clean all
