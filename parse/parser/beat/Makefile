# dir
INC_DIR = ../../../lib/include
BEAT_DIR = $(shell pwd)

# src
BEAT_SRC = $(wildcard $(BEAT_DIR)/*.cpp)

# obj
BEAT_OBJ = $(patsubst %.cpp, %.o, $(BEAT_SRC))

# option
OPT ?= -std=c++17 -O3

# path
INC_PATH ?= -I$(INC_DIR)

# compile
CC = g++
WARNINGS = -Wall -Wextra
CXXFLAGS  = $(WARNINGS) $(OPT)

all: $(BEAT_OBJ)

$(BEAT_OBJ):%.o : %.cpp
	$(CC) $(CXXFLAGS) -c $< $(INC_PATH) 

.PHONY: clean

clean:
	rm -f *.o *.a
