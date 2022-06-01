# dir
INC_DIR = ../../lib/include
PARSER_DIR = $(shell pwd)
DARPA_DIR = $(PARSER_DIR)/darpa
BEAT_DIR = $(PARSER_DIR)/beat

# src
PARSER_SRC = $(wildcard $(PARSER_DIR)/*.cpp)
DARPA_SRC = $(wildcard $(DARPA_DIR)/*.cpp)
BEAT_SRC = $(wildcard $(BEAT_DIR)/*.cpp)

# obj
PARSER_OBJ = $(patsubst %.cpp, %.o, $(PARSER_SRC))
BEAT_OBJ = $(patsubst %.cpp, %.o, $(BEAT_SRC))
DARPA_OBJ = $(patsubst %.cpp, %.o, $(DARPA_SRC))
PARSERBEAT_OBJ = $(PARSER_OBJ) $(BEAT_OBJ)
PARSERDARPA_OBJ = $(PARSER_OBJ) $(DARPA_OBJ)

# option
export OPT ?= -std=c++17 -O3

# path
export INC_PATH ?= -I$(INC_DIR)

# compile
CC = g++
WARNINGS = -Wall -Wextra
CXXFLAGS  = $(WARNINGS) $(OPT)

all: $(PARSERBEAT_OBJ) $(PARSERDARPA_OBJ)

$(PARSER_OBJ):%.o : %.cpp
	$(CC) $(CXXFLAGS) -c $< $(INC_PATH) 

$(BEAT_OBJ):%.o : %.cpp
	(cd $(BEAT_DIR) && $(MAKE) all)

$(DARPA_OBJ):%.o : %.cpp
	(cd $(DARPA_DIR) && $(MAKE) all)

.PHONY: clean

clean:
	rm -f *.o *.a
	(cd $(DARPA_DIR) && $(MAKE) clean)
	(cd $(BEAT_DIR) && $(MAKE) clean)