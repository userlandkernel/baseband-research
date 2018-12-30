This directory contains the source for a GNU C source to source translator.  Here's a brief summary of the subdirectories:

/tests contains many little C programs which test various syntax elements of Standard and GNU C.

/grammars contains:
    CSymbolTable.java       Symbol table support
    GnuCEmitter.g           Turns the AST back into source code, subclassed from GnuCTreeParser
    GnuCParser.g            Parser for GNU C, subclassed from StdCParser
    GnuCTreeParser.g        AST Parser for GNU C
    Makefile    
    StdCParser.g            Parser for Standard C
    TNode.java              AST node class with line number and other features, we don't use
                            its double-linked facilities currently.
    TNodeFactory.java       Creates TNodes

/examples contains example code
    Test2.java              Does source to source with no translation
    TestLex.java            Lexes an input stream and writes one token per line on output.
                            Used for testing before and after source with no translation because
                            formatting is different and diff doesn't do a good job with it



How to use:

I recommend adding antlr, ../grammars and ../examples to your classpath.  To build, first build grammars, then examples.  Then you can cd into tests and 'make testall' will run all of the test*.c programs through the Test class in examples.  You will need to tweak the Makefiles for your java setup, but it shouldn't be hard to do.

'java Test2 source.c' will parse, build the tree, walk the tree and write it out to stdout with no translation, just different formatting.  Use for diagnosis of parser problems or missing code.

'java TestLex source.c' will simply lex the input stream and write one token per line.  Do this on both the original source and the source run through TestThrough.  Diff the two files to see what is missing.  I remove any trailing commas in lists, so this will normally show up and is no cause for concern.

When you build you will notice many warnings.  I turned off all the warnings that I could but ANTLR doesn't let you turn them all off, especially ones inside syntactic predicates.  The warnings are legitimate ambiguities and are handled properly by ANTLR's default behavior.  I have looked into every one of them and am satisfied that all are handled properly.  It sure does look ugly though.

GnuCParser is a subclass of StdCParser, every rule in it is in some way different from normal C, or I had to subclass it to turn off a warning from being generated.  Some rules are obvious, but most are documented as to what feature of gcc is being parsed in that rule.


Notes:
	I developed this grammar using noweb.  Take a look at c.html or c.ps to see what it looks like as a "literate program".  I've decided not to release the noweb source simply because it isn't that easy to get noweb working and I think it would not interest most people.  If you want to have it send me a note: jamz@cdsnet.net or mzukowski@bco.com.
	I used noweb because it lets you do "random access" programming.  By that I mean that I was able to organize the source code so that all four .g files were actually in one file-c.nw.  I could organize the rules in any order, and I choose to have all related rules together.  For instance my chunk of declaration rules for the ANSI C parser is follwed immediately by the GCC parser, GCC tree parser and GCC emitter so that if I change a rule in the parser I don't have to go far to change it in the tree parsers.  A smart editor could let you do the same thing, but I didn't want to learn emacs-lisp or something else to get going.
	I also used RCS extensively--I did a checkin at every build.  Programming parsers is difficult.  Doing it without a running log of what you do and the ability to recover to any point in your development cycle is masochistic.  Leave off a ";" or brace and it can take quite a while to track it down from ANTLR's error message.  Example makefile rules:

zRCS/c.nw,v : c.nw
	ci -l $@


zRCS/CSymbolTable.java,v : CSymbolTable.java
	ci -l $@

I used this zRCS directory instead of RCS because make automatically wants to check things out before building instead of checking things in after a successful compile.  My makefile generator would just add zRCS/c.nw,v and friends to the all target.  Try it, you'll like it.


Acknowledgments:
	John Mitchell and Jim Coker started the initial ANSI C parser.  I (Monty Zukowski) finished it up and then added all the GCC stuff.  Monty's always looking for ANTLR work: jamz@cdsnet.net.  Many friendly folks on the antlr-interest mailing list and comp.compilers.tools.pccts helped a lot.  And a big thanks to Terence Parr for not only building the tool but also helping me figure out how to use it right!