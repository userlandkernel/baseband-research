PARSERDIR=../../src/magpieparsers/cplusplus/
CC=g++
CFLAGS=-c -Wall -g -I$(PARSERDIR) -I/usr/local/include
LDFLAGS=-L/usr/lib
LIBS=-lantlr
BUILDDIR=build
VPATH=src/:$(PARSERDIR)
SOURCES=CPPLexer.cpp CPPParser.cpp Dictionary.cpp LineObject.cpp Support.cpp ASTNode.cpp main.cpp
OBJECTS=$(patsubst %.cpp,$(BUILDDIR)/%.o,$(notdir $(SOURCES)))
EXECUTABLE=$(BUILDDIR)/get_typedefs

all: $(SOURCES) $(EXECUTABLE)

clean:
	rm -f $(OBJECTS) $(EXECUTABLE)
	
builddir:
	@if [[ ! -d $(BUILDDIR) ]]; then mkdir $(BUILDDIR); fi


$(EXECUTABLE): builddir $(OBJECTS) 
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS) $(LIBS) 

$(BUILDDIR)/%.o: $(notdir %.cpp)
	$(CC) $(CFLAGS) $< -o $@
