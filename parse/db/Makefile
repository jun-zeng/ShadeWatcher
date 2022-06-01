# dir
INC_DIR = ../../lib/include
SRC_DIR = ../
DB_DIR = $(shell pwd)

# src
DB_SRC = $(wildcard $(DB_DIR)/*.cpp)

# obj
DB_OBJ = $(patsubst %.cpp, %.o, $(DB_SRC))

# option
OPT ?= -std=c++17 -O3

# path
INC_PATH ?= -I$(INC_DIR) -I$(SRC_DIR)

# compile
CC = g++
WARNINGS = -Wall -Wextra
CXXFLAGS  = $(WARNINGS) $(OPT)

all: $(DB_OBJ)

$(DB_OBJ):%.o : %.cpp
	$(CC) $(CXXFLAGS) -c $< $(INC_PATH) 

.PHONY: clean

clean:
	rm -f *.o *.a
