# Compiler and Compile options.
CXX = g++
CXXFLAGS = -g -Wall -std=c++1z -O3 -pthread
# Macros specifying path for compile.
SRCS := src/main.cc
OBJS := $(SRCS:.cc=.o)

MARIABASE = mariadb/server/include 
INNOBASE = mariadb/server/storage/innobase/include/
LIB = lib/
INC = include/

# Pre-Processor.
CPPFLAGS += -I$(INNOBASE) -I$(MARIABASE) -I$(INC)
TARGET = run
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o $(TARGET) $(OBJS) -L$(LIB)
all: $(TARGET)
# Delete binary & object files.
clean:
	rm $(TARGET) $(OBJS)
