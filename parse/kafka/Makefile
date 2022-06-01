# dir
INC_DIR = ../../lib/include
SRC_DIR = ../
KAFKA_DIR = $(shell pwd)

# src
KAFKA_SRC = $(wildcard $(KAFKA_DIR)/*.cpp)

# obj
KAFKA_OBJ = $(patsubst %.cpp, %.o, $(KAFKA_SRC))

# option
OPT ?= -std=c++17 -O3

# path
INC_PATH ?= -I$(INC_DIR) -I$(SRC_DIR)

# compile
CC = g++
WARNINGS = -Wall -Wextra
CXXFLAGS  = $(WARNINGS) $(OPT)

all: $(KAFKA_OBJ)

$(KAFKA_OBJ):%.o : %.cpp
	$(CC) $(CXXFLAGS) -c $< $(INC_PATH) 

.PHONY: clean

clean:
	rm -f *.o *.a
