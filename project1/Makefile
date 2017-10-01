# Compiler and Compile options.
CC = g++
CXXFLAGS = -g -Wall -std=c++11 -pthread -O3

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


# marker build
MARKER = marker 
MARKER_SRC := test/marker/marker.cc
MARKER_OBJ := $(MARKER_SRC:.cc=.o)
$(MARKER): $(MARKER_OBJ)
	$(CC) $(CXXFLAGS) $(CPPFLAGS) -o $(MARKER) $(MARKER_OBJ)

# ref solution build
REF = ref 
REF_SRC := src/main_ref.cc
REF_OBJ := $(REF_SRC:.cc=.o)
$(REF): $(REF_OBJ)
	$(CC) $(CXXFLAGS) $(CPPFLAGS) -o $(REF) $(REF_OBJ)

all: $(TARGET) $(MARKER) $(REF)

# Delete binary & object files.
clean:
	rm $(TARGET) $(OBJS) $(MARKER) $(REF)
