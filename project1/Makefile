# Compiler and Compile options.
CC = g++
CXXFLAGS = -g -Wall -std=c++11 -O3

# Macros specifying path for compile.
SRCS := src/main.cc
OBJS := $(SRCS:.cc=.o)
INC = include/
LIB = lib/*.cc

# Pre-Processor.
CPPFLAGS += -I$(INC)

TARGET = run

$(TARGET): $(OBJS)
	$(CC) $(CXXFLAGS) $(CPPFLAGS) -o $(TARGET) $(OBJS) -L$(LIB)

# Delete binary & object files.
clean:
	rm $(TARGET) $(OBJS)
