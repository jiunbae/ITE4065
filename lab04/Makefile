g++ prime_main.cpp -std=c++0x -lpthread

CC = g++
CXXFLAGS = -g -Wall -std=c++0x -O3 -lpthread

SRC := prime_main.cpp
OBJ := $(SRC:.cpp=.o)

TARGET = run
$(TARGET): $(OBJ)
	$(CC) $(CXXFLAGS) -o $(TARGET) $(OBJ)

clean:
	rm $(TARGET) $(OBJ)