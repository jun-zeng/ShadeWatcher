# dir
INC_DIR = ../../lib/include
UTIL_DIR = $(shell pwd)

# src (Todo: polish graphviz)
UTIL_SRC = $(filter-out $(UTIL_DIR)/graphviz.cpp, $(wildcard $(UTIL_DIR)/*.cpp))

# obj
UTIL_OBJ = $(patsubst %.cpp, %.o, $(UTIL_SRC))

# option
OPT ?= -std=c++17 -O3

# path
INC_PATH ?= -I$(INC_DIR)

# compile
CC = g++
WARNINGS = -Wall -Wextra
CXXFLAGS  = $(WARNINGS) $(OPT)

all: $(UTIL_OBJ)

$(UTIL_OBJ):%.o : %.cpp
	$(CC) $(CXXFLAGS) -c $< $(INC_PATH) 

.PHONY: clean

clean:
	rm -f *.o *.a
