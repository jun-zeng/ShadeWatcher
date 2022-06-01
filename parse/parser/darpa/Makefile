# dir
INC_DIR = ../../../lib/include
DAR_DIR = $(shell pwd)

# src
DAR_SRC = $(wildcard $(DAR_DIR)/*.cpp)

# obj
DAR_OBJ = $(patsubst %.cpp, %.o, $(DAR_SRC))

# option
OPT ?= -std=c++17 -O3

# path
INC_PATH ?= -I$(INC_DIR)

# compile
CC = g++
WARNINGS = -Wall -Wextra
CXXFLAGS  = $(WARNINGS) $(OPT)

all: $(DAR_OBJ)

$(DAR_OBJ):%.o : %.cpp
	$(CC) $(CXXFLAGS) -c $< $(INC_PATH) 

.PHONY: clean

clean:
	rm -f *.o *.a
