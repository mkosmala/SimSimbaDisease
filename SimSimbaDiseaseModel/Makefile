CC=icpc
CFLAGS=-I.
SOURCES = Coalition.cpp Engine.cpp LionGroup.cpp Lion.cpp Parser.cpp Population.cpp Pride.cpp Statistics.cpp stdafx.cpp Territory.cpp SimSimbaDisease.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=SimSimbaDisease

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) -g -c -o $@ $<

clean:
	rm -f *.o *~ core 

