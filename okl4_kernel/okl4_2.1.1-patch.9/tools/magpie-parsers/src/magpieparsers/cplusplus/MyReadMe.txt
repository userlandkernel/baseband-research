MyReadMe.txt

Notes for C++ grammar file to generate ANTLR parser (in C++)

1. Past
2. Present
3. Future

1. Past

This C++ grammar file was originally written and published in 1994 by,

Authors: Sumana Srinivasan, NeXT Inc.;            sumana_srinivasan@next.com
         Terence Parr, Parr Research Corporation; parrt@parr-research.com
         Russell Quong, Purdue University;        quong@ecn.purdue.edu

as VERSION 1.2 for use with PCCTS (The original C version of ANTLR).

In 1997-1999 it was adapted for use in a project to analyse data flow
in C programs by Lasitha Leelasena, Sue Black (blackse@lsbu.ac.uk) and 
David Wigg (wiggjd@bcs.org.uk). The generated parser was in C++ and 
all of our included statement code was in C.

In 2000, in view of the fact that ANTLR had then been re-written in Java 
and any further development of PCCTS had been suspended, it was decided
that we should convert our version of the C grammar for PCCTS into 
use with ANTLR. As all our included application code was in C it was 
decided to use the option to produce the generated parser in C++ to 
avoid the need to rewrite this application code as well.

During 2001-2002 we were fortunate enough to have the services of a 
visiting tutor, Jiangu Zuo, from Jianghan University,Wuhan,China who
carried out most of this work. However, this conversion was quite a 
lot more difficult than we had hoped and took us about a year to 
complete. We have tried to make a record of problems encountered and 
to give some solutions and this can be found at 
http://antlr.org/fieldguide/cppantlr/index.html.

The most difficult problem concerned the lack of 'hoisting' in ANTLR 
which we were only able to overcome in the time available by copying 
the generated hoisting code from the PCCTS version into our new 
grammar file, hence some of the mysterious C++ statements at the 
beginning of a number of productions. I think Zuo also had some 
problems in using predicates.

In August 2002 I reported that this grammar file would be published
'soon' when remaining problems had been cleared up and the grammar was
fit to be published. In the event, for a variety of reasons, this was
not achieved.

So, in view of the number of requests being made for access to this 
grammar I agreed in February 2003 to it being published on the 
www.antlr.org website for general use under the usual terms, in the 
hope that interested users would let me know how it could be improved.
Unfortunately, though it could handle C code and some C++ it was 
unable to handle namespaces and a lot of templates so left a lot to be 
desired.

In September 2003 I supplied a much improved version which I called 
V2.0. This version was picked up by some users. A few problems were 
raised which have since been solved.

Since then I have been concentrating on tidying up what had become a
rather confusing system and trying to produce a cleaner, tidier and 
easier to understand system and also one easier to use in your 
application. No doubt I have not entirely succeeded yet, but I hope
it is better than it was.

I have introduced the idea of subclassing a users application code.
I hope this clear separation of code will enforce a clear separation
of code between the parser and the application and will enable users
to take CPP_parser updates much more easily.

If you feel the need to change the parser in any way I would be 
grateful if you could let me know.

2. Present, July 2004.

I am using MSVC 6.0 under Windows ME and NT.

I created a static source library for the antlr code (2.7.3) with
some modifications as discussed below.

I have called this latest version Version 3.0 published July 2004

Please note that it continues to be used to parse pre-compiled *.i 
files (with or without embedded #line directives (obtained by using 
the /P command in compilation when using MSVC) ).

I include a small demonstration program, quadratic.i, which you 
could use to test the set up of your system.

Although I cannot say it has been thoroughly tested it appears to
parse support.i, CPPLexer.i and CPPParser.i, all of which contain
a considerable quantity of included files containing a great deal
of complex code.

It should be noted that this version still handles scoping in a
relatively simplistic manner but this does not appear to be a 
problem. To do this properly would entail a lot of work to update 
the antlr supporting code in dictionary.cpp etc.

Briefly, all template parameter names are held in level 0, and
all type names in level 1. All variable names are held in lower
levels but continue to be deleted when they go out of scope.

Each run should end with the following two statements,

Support exitExternalScope scope now 0

Parse ended

showing that the scope level had been returned to zero correctly.

I have included a C++ syntax definition (grammar.txt) which appears
to be up to date. If not, please let me know.

Please address any problems you have with this version to me
preferably with a cut down version of the problem code.

Notes about running this version.

* I am currently using antlr 2.7.3  

* Note that the latest version of antlr for MSVC users may be on Ric 
   Klaren's website at http://wwwhome.cs.utwente.nl/~klaren/antlr/

* The following type of warning produced during compilation of 
   CPPLexer.cpp and CPPParser.cpp can be ignored,

   CPPParser.cpp(163) : warning C4101: 'pe' : unreferenced local variable

* I have introduced a "statementTrace" feature in CPP_parser.g during
   testing which I have found useful. See CPP_parser.g . This can be
   set on (or off) by altering statementTrace in CPPParser.cpp and 
   recompiling and linking only.
   
   With statementTrace set to 1 you get a list of statement types as
   they are detected from external_declaration and member_declaration
   in CPP_parser.g.

   With statement trace set to 2 you also get a record of each variable
   declared showing its name, scope level, and type (See list in 
   CPPSymbol.hpp).

   The trace output will display but you should be able to place trace 
   output in a trace file like this,

   ...\debug\CPP_parser program.i > program.trace

   I have found this feature useful for providing the ability to check
   the output from one run to another after making modifications to
   either the parser or the application code. Just keep your "standard"
   or "correct" version of the trace output in a separate "archive" 
   file and use this to compare with the output of any updated version.
   
   You can do a file compare like this,

   ...\fc /n program_010704.trace program.trace

* I have also implemented a dynamic trace facility by including some
   code in LLkParser.hpp and LLkParser.cpp called antlrTrace() before 
   generating the static antlr library, as shown below. 
   
   The advantage of this facility is that, by always generating with
   antlr tracing initially (using -traceParser etc.), antlr tracing 
   can be switched on or off completely by changing the antlrTrace() 
   statement in init() in CPPParser.cpp appropriately and recompiling
   and linking without also having to regenerate from the grammar file
   each time. It also enables tracing to be implemented on a more 
   selective basis by including antlrTrace(true/false) statements in
   the grammar, though of course in this case it would entail 
   regenerating the parser as well, but it would enable you to reduce
   the amount of trace output to a specific area. 

   LLkParser.hpp 

private:
	// DW 060204 For dynamic tracing
	bool antlrTracing;
public:
	// DW 060204 For dynamic tracing
	virtual void antlrTrace(bool traceFlag);

	LLkParser.cpp

// DW For dynamic tracing
void LLkParser::antlrTrace(bool traceFlag)
	{
	antlrTracing = traceFlag;
	}

void LLkParser::traceIn(const char* rname)
	{
	traceDepth++;
	// DW For dynamic tracing
	if (antlrTracing)
		trace("> ",rname);
	}

void LLkParser::traceOut(const char* rname)
	{
	// DW For dynamic tracing
	if (antlrTracing)	
		trace("< ",rname);
	traceDepth--;
	}

*  I have also introduced MyCode.cpp with MyCode.hpp to demonstrate how 
   your application code can be subclassed in CPPParser. You can, of
   course, delete, include and amend any of these to suit your 
   application.
   
   However, I strongly recommend using this feature with this grammar
   as I think it will make it easier both for me to issue updated 
   versions of the CPP_parser grammar from time to time and for you to
   accept and use them since the code for the parser and your 
   application will be kept strictly apart.

*  I would be grateful if you could let me know if you need to correct
   anything in CPP_parser.g, support.cpp etc.

* End of notes.

3. Future

I would be grateful if any future user of this grammar would advise me
and/or the e-mail group (antlr-interest@yahoogroups.com) direct of any  
improvement they have been able to make to this grammar, for the 
benefit of other users.

In the meantime I will advise the group of any improvements and 
continue to update the master files as arranged with Terence from time
to time.


Thankyou.

David Wigg
Research Fellow
Centre for Systems and Software Engineering
London South Bank University
London, UK.
wiggjd@bcs.org.uk
blackse@lsbu.ac.uk

1 July 2004