classfiles = LineObject.class PreprocessorInfoChannel.class StdCParser.class StdCLexer.class GnuCParser.class GnuCLexer.class GnuCTreeParser.class GnuCEmitter.class
javafiles = CSymbolTable.java TNode.java TNodeFactory.java CToken.java LineObject.java PreprocessorInfoChannel.java StdCParser.java StdCLexer.java GnuCParser.java GnuCLexer.java GnuCTreeParser.java GnuCEmitter.java


all : $(javafiles) $(classfiles)
	


clean : 
	


StdCParser.java StdCLexer.java : StdCParser.g
	java antlr.Tool StdCParser.g


GnuCParser.java GnuCLexer.java : GnuCParser.g StdCParser.g
	java antlr.Tool -glib "StdCParser.g" GnuCParser.g


GnuCTreeParser.java : GnuCTreeParser.g
	java antlr.Tool GnuCTreeParser.g


GnuCEmitter.java : GnuCEmitter.g GnuCTreeParser.g
	java antlr.Tool -glib "GnuCTreeParser.g" GnuCEmitter.g



.SUFFIXES: .java .class

.java.class : 
	javac $<



