CXX = g++
FLAGS =
LINK = -lncurses -lfmt

SRC = main.cc
HEADERS = kedit.h
TARGET = kedit

all: $(SRC) $(HEADERS)
	$(CXX) $(FLAGS) $(LINK) $(SRC) -o $(TARGET)

install: all
	install -s ./kedit ${HOME}/.local/bin
