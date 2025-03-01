CC = g++ -g
ifeq ($(shell uname), Darwin)
CFLAGS = -std=c++20 -Wall -Wextra -fPIC -I$(SRC_DIR) -I/opt/homebrew/Cellar/openssl@3/3.4.1/include -I/opt/homebrew/Cellar/secp256k1/0.6.0/include -I/usr/local/include/mqtt
LDFLAGS = -L/opt/homebrew/Cellar/openssl@3/3.4.1/lib -L/opt/homebrew/Cellar/secp256k1/0.6.0/lib -lssl -lcrypto -lsecp256k1 -lpaho-mqttpp3 -lpaho-mqtt3a -Wl,-rpath,/usr/local/lib
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
METRICS_DIR = metrics
LIBRARY = libblockchain.so
EXECUTABLE = main
UNIT_TEST_EXEC = run_unit_tests
METRICS_EXEC = run_metrics

SRC = $(shell find $(SRC_DIR) -type f -name '*.cpp')
TEST_SRC = $(shell find $(TEST_DIR) -type f -name '*.cpp')
UNIT_TEST_SRC = $(shell find $(UNIT_TEST_DIR) -type f -name '*.cpp')
METRICS_SRC = $(shell find $(METRICS_DIR) -type f -name '*.cpp')

OBJ = $(SRC:.cpp=.o)
TEST_OBJ = $(TEST_SRC:.cpp=.o)
UNIT_TEST_OBJ = $(UNIT_TEST_SRC:.cpp=.o)
METRICS_OBJ = $(METRICS_SRC:.cpp=.o)

.PHONY: all clean unit_tests metrics

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

$(METRICS_DIR)/%.o: $(METRICS_DIR)/%.cpp
	$(CC) $(CFLAGS) -I./metrics -c $< -o $@

unit_tests: LDFLAGS += -lgtest -lgtest_main -lpthread $(GTEST)
unit_tests: $(LIBRARY) $(UNIT_TEST_OBJ)
	$(CC) $(CFLAGS) -o $(UNIT_TEST_EXEC) $(UNIT_TEST_OBJ) $(LDFLAGS) -L. -lblockchain

metrics: $(LIBRARY) $(METRICS_OBJ)
	$(CC) $(CFLAGS) -o $(METRICS_EXEC) $(METRICS_OBJ) $(LDFLAGS) -L. -lblockchain

clean:
	rm -f $(OBJ) $(TEST_OBJ) $(UNIT_TEST_OBJ) $(METRICS_OBJ) $(EXECUTABLE) $(LIBRARY) $(UNIT_TEST_EXEC) $(METRICS_EXEC)

re: clean all
