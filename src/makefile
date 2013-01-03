# PGREP Makefile
# JSI - 2007.07.27

# compiler options
CC=g++
CFLAGS=-c -m64 -Wall -O3

SOURCES=BlastInfo.cpp PrimerPair.cpp PrimingSite.cpp TargetCache.cpp WeightMatrix.cpp PrimerSearch.cpp pgrep.cpp 
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=pgrep

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	$(CC) -m64 $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@
	
clean:
	rm -rf *o pgrep

