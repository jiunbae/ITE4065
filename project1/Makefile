# Compiler and Compile options.
CC = g++
CXXFLAGS = -g -Wall -std=c++11

# Macros specifying path for compile.
SRCS := src/main.cc
OBJS := $(SRCS:.cpp=.o)
INC = include/
LIB = lib/


# Pre-Processor.
CPPFLAGS += -I$(INC)

# Compile command.
TARGET = run

$(TARGET): $(OBJS)
	$(CC) $(CXXFLAGS) $(CPPFLAGS) -o $(TARGET) $(OBJS) -L$(LIB)

# Delete binary & object files.
clean:
	rm $(TARGET) $(OBJS)
