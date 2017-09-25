CC = g++
CXXFLAGS = -g -Wall -std=c++0x -O3 -pthread

SRC := prime_main.cpp
OBJ := $(SRC:.cpp=.o)

TARGET = run
$(TARGET): $(OBJ)
	$(CC) $(CXXFLAGS) -o $(TARGET) $(OBJ)

clean:
	rm $(TARGET) $(OBJ)
