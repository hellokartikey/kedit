CXX = g++
FLAGS = -Wall
LINK = -lncurses -lfmt

SRC = main.cc
HEADERS = kedit.h
TARGET = kedit

all: $(SRC) $(HEADERS)
	$(CXX) $(FLAGS) $(LINK) $(SRC) -o $(TARGET)
