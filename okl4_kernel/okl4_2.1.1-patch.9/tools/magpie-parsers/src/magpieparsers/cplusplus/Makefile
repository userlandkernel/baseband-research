CC=g++
CFLAGS=-c -Wall -g
LDFLAGS=-L/usr/lib
LIBS=-lantlr
SOURCES = CPPLexer.cpp  CPPParser.cpp  Dictionary.cpp  LineObject.cpp  Main.cpp  MyCode.cpp Support.cpp ASTNode.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=example

all: $(SOURCES) $(EXECUTABLE)

clean:
	rm -f $(OBJECTS) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS) $(LIBS) 

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@
