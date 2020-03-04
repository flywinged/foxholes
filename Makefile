# Makefile for Project Stark
# Developed by Aptus Engineering, Inc.
# See LICENSE.md file in project root directory

# Compiler
CXX:= g++

# Executables
MAIN := main

# Compiler flags
FLAGS := -Ofast -std=c++17 -fPIC -Wno-narrowing -g -pthread

LIBS := board.hxx solver.hxx stpl_stl.h

OBJECTS := board.cxx solver.cxx

# Make rules
all: $(MAIN)

$(MAIN): $(OBJECTS) main.cxx
	$(CXX) $^ -o $@ $(LIBS) $(FLAGS)

%.obj: %.cxx
	$(CXX) $(LIBS) $(FLAGS) -c $< -o $@

clean:
	rm -f main