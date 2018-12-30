/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

        Copyright (c) Non, Inc. 1997 -- All Rights Reserved

PROJECT:        C Compiler
MODULE:         Parser
FILE:           stdc.g

AUTHOR:         John D. Mitchell (john@non.net), Jul 12, 1997

REVISION HISTORY:

        Name    Date            Description
        ----    ----            -----------
        JDM     97.07.12        Initial version.
        JTC     97.11.18        Declaration vs declarator & misc. hacking.
        JDM     97.11.20        Fixed:  declaration vs funcDef,
                                        parenthesized expressions,
                                        declarator iteration,
                                        varargs recognition,
                                        empty source file recognition,
                                        and some typos.
		NFD		06.09.04		Ported to Python.
                                        

DESCRIPTION:

        This grammar supports the Standard C language.

        Note clearly that this grammar does *NOT* deal with
        preprocessor functionality (including things like trigraphs)
        Nor does this grammar deal with multi-byte characters nor strings
        containing multi-byte characters [these constructs are "exercises
        for the reader" as it were :-)].

        Please refer to the ISO/ANSI C Language Standard if you believe
        this grammar to be in error.  Please cite chapter and verse in any
        correspondence to the author to back up your claim.

TODO:

        - typedefName is commented out, needs a symbol table to resolve
        ambiguity.

        - trees

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/


header "StdCParser.__init__" {
    // access to symbol table
    self.symbolTable = CSymbolTable()

    // source for names to unnamed scopes
    self.unnamedScopeCounter = 0

    self.traceDepth = 0
}

header "StdCLexer.__init__" {
    self.lineObject = LineObject()
    self.originalSource = ""
    self.preprocessorInfoChannel = PreprocessorInfoChannel()
    self.tokenNumber = 0
    self.countingTokens = True
    self.deferredLineCount = 0
}

options {
	language = "Python";
}

{
from antlr import CommonAST
from csymboltable import CSymbolTable
}

                     
class StdCParser extends Parser;

options {
    k = 2;
    exportVocab = STDC;
    buildAST = true;
    //ASTLabelType = "TNode";

    // Copied following options from java grammar.
    codeGenMakeSwitchThreshold = 2;
    codeGenBitsetTestThreshold = 3;
}


{
    def isTypedefName(self, name):
        foundit = False
        node = self.symbolTable.lookupNameInCurrentScope(name)
        while node:
            if node.getType == LITERAL_typedef:
                foundit = True
                break
            node = node.getNextSibling()

        return foundit

    def getAScopeName(self):
        self.unnamedScopeCounter += 1
        return str(self.unnamedScopeCounter)

    def pushScope(scopeName):
        self.symbolTable.pushScope(scopeName)

    def popScope():
        self.symbolTable.popScope()


    def reportError(ex):
        if isinstance(ex, basestring):
            print >>sys.stderr, "ANTLR Parsing Error from string: " + ex
        else:
            // Assume it's an exception object
            print >>sys.stderr, "ANTLR Parsing Error: "+ex + " token name:" + tokenNames[LA(1)]
            ex.printStackTrace(System.err)

    def reportWarning(s):
        print >>sys.stderr,"ANTLR Parsing Warning from String: " + s

//    def match(self, t):
//        debugging = False
//
//        if debugging:
//            sys.stdout.write(' ' * len(self.traceDepth))
//            if inputState.guessing:
//                guessString = "[inputState.guessing %s]" % (inputState.guessing)
//            else:
//                guessString = ""
//            print "Match("+tokenNames[t]+") with LA(1)="+ tokenNames[LA(1)] + guessString
//
//            if LA(1) != t:
//                if debugging:
//                    sys.stdout.write(' ' * len(self.traceDepth))
//                    print "token mismatch: %s != %s" % (tokenNames[LA(1)], tokenNames[t])
//                raise MismatchedTokenException(tokenNames, LT(1),t , False, getFilename())
//            else:
//                consume()

    def traceIn(name):
        self.traceDepth += 1

        sys.stdout.write(' ' * len(self.traceDepth))

        guessString = "[inputState.guessing %s]" % (inputState.guessing)
        print "> %s; LA(1)==('%s) %s" % (rname, tokenNames[LT(1).getType()], LT(1).getText(), guessString)

    def traceOut(name):
        sys.stdout.write(' ' * len(self.traceDepth))

        guessString = "[inputState.guessing %s]" % (inputState.guessing)
        print "< %s; LA(1)==('%s) %s" % (rname, tokenNames[LT(1).getType()], LT(1).getText(), guessString)

        self.traceDepth -= 1
}


translationUnit
        :       externalList

        |       /* Empty source files are *not* allowed.  */
                {print >>sys.stderr, "Empty source file!"}
        ;


externalList
        :       ( externalDef )+
        ;


externalDef
        :       ( "typedef" | declaration )=> declaration
        |       functionDef
        |       asm_expr
        ;


asm_expr
        :       "asm"^ 
                ("volatile")? LCURLY! expr RCURLY! SEMI!
        ;


declaration
                                        { ds1 = None}
        :       ds:declSpecifiers       { ds1 = self.astFactory.dupList(#ds) }
                (                       
                    initDeclList[ds1]
                )?
                SEMI!
                                        //{
                                        //## = #( #[NDeclaration], ##)
                                        //}
                
        ;


declSpecifiers 
               {specCount = 0}
        :       (               options { // this loop properly aborts when
                                          //  it finds a non-typedefName ID MBZ
                                          warnWhenFollowAmbig = false;
                                        } :
                  s:storageClassSpecifier
                | typeQualifier
                | (typeSpecifier[specCount]) =>
                        specCount = typeSpecifier[specCount]
                )+
        ;

storageClassSpecifier
        :       "auto"                  
        |       "register"              
        |       "typedef"               
        |       functionStorageClassSpecifier
        ;


functionStorageClassSpecifier
        :       "extern"
        |       "static"
        ;


typeQualifier
        :       "const"
        |       "volatile"
        ;

typeSpecifier [specCount] returns [retSpecCount]
                                                        { retSpecCount = specCount + 1}
        :
        (       "void"
        |       "char"
        |       "short"
        |       "int"
        |       "long"
        |       "float"
        |       "double"
        |       "signed"
        |       "unsigned"
        |       structOrUnionSpecifier
        |       enumSpecifier
        |       { specCount == 0 }? typedefName
        )
        ;


typedefName
        :       { self.isTypedefName ( self.LT(1).getText() ) }?
                i:ID                    { ## = #(#[NTypedefName], #i); }
        ;

structOrUnionSpecifier
        :       sou:structOrUnion!
                ( ( ID LCURLY )=> i:ID l:LCURLY
                            {
                            scopeName = #sou.getText() + " " + #i.getText()
                            #l.setText(scopeName)
                            pushScope(scopeName)
                            }
                        structDeclarationList
                                            { popScope()}
                        RCURLY!
                |   l1:LCURLY
                                            {
                                            scopeName = getAScopeName()
                                            #l1.setText(scopeName)
                                            pushScope(scopeName)
                                            }
                    structDeclarationList
                                            { popScope() }
                    RCURLY!
                | ID
                )
                                            { ## = #( #sou, ## ) }
        ;


structOrUnion
        :       "struct"
        |       "union"
        ;


structDeclarationList
        :       ( structDeclaration )+
        ;


structDeclaration
        :       specifierQualifierList structDeclaratorList ( SEMI! )+
        ;


specifierQualifierList
                                { specCount = 0 }
        :       (               options {   // this loop properly aborts when
                                            // it finds a non-typedefName ID MBZ
                                            warnWhenFollowAmbig = false;
                                        } :
                ( typeSpecifier[specCount] )=>
                        specCount = typeSpecifier[specCount]
                | typeQualifier
                )+
        ;


structDeclaratorList
        :       structDeclarator ( COMMA! structDeclarator )*
        ;


structDeclarator
        :
        (       COLON constExpr
        |       d = declarator[False] ( COLON constExpr )?
        )
                                    { ## = #( #[NStructDeclarator], ##) }
        ;


enumSpecifier
        :       "enum"^
                ( ( ID LCURLY )=> i:ID LCURLY enumList[i.getText()] RCURLY!
                | LCURLY enumList["anonymous"] RCURLY!
                | ID
                )
        ;


enumList[enumName]
        :       enumerator[enumName] ( COMMA! enumerator[enumName] )*  
        ;

enumerator[enumName]
        :       i:ID                {
                                    self.symbolTable.add(  i.getText(),
                                                        #(   None,
                                                            #[LITERAL_enum, "enum"],
                                                            #[ ID, enumName]
                                                         )
                                                     )
                                    }
                (ASSIGN constExpr)?
        ;


initDeclList[declarationSpecifiers]
        :       initDecl[declarationSpecifiers] 
                ( COMMA! initDecl[declarationSpecifiers] )*
        ;


initDecl[declarationSpecifiers]
                                        { declName = "" }
        :       declName = d:declarator[False]
                                        {
                                        ds1 = self.astFactory.dupList(declarationSpecifiers)
                                        d1 = self.astFactory.dupList(#d)
                                        self.symtable_add(declName, ds1, d1)
                                        //self.symbolTable.add(declName, #(None, ds1, d1) )
                                        }
                ( ASSIGN initializer
                | COLON expr
                )?
                                        //{
                                        //## = #( #[NInitDecl], ## )
                                        //}

        ;

pointerGroup
        :       ( STAR ( typeQualifier )* )+    { ## = #( #[NPointerGroup], ##) }
        ;



idList
        :       ID ( COMMA! ID )*
        ;


initializer
        :       ( assignExpr
                |       LCURLY initializerList ( COMMA! )? RCURLY!
                )
                        { ## = #( #[NInitializer], ## ) }
        ;


initializerList
        :       initializer ( COMMA! initializer )*
        ;


declarator[isFunctionDefinition] returns [declName]
                                                { declName = "" }
        :
                ( pointerGroup )?               

                ( id:ID                         { declName = id.getText() }
                | LPAREN declName = declarator[False] RPAREN
                )

                ( !  LPAREN
                                                { 
                                                if (isFunctionDefinition):
                                                    pushScope(declName)
                                                else:
                                                    pushScope("!"+declName)
                                                }
                    (                           
                        (declSpecifiers)=> p:parameterTypeList
                                                //{
                                                //## = #( None, ##, #( #[NParameterTypeList], #p ) )
                                                //}

                        | (i:idList)?
                                                //{
                                                //## = #( None, ##, #( #[NParameterTypeList], #i ) )
                                                //}
                    )
                                                { popScope() }    
                  RPAREN                        
                | LBRACKET ( constExpr )? RBRACKET
                )*
                                                //{
                                                //## = #( #[NDeclarator], ## )
                                                //}
        ;
 
parameterTypeList
        :       parameterDeclaration
                (   options {
                            warnWhenFollowAmbig = false;
                        } : 
                  COMMA!
                  parameterDeclaration
                )*
                ( COMMA!
                  VARARGS
                )?
        ;


parameterDeclaration
        :       ds:declSpecifiers
                ( ( declarator[False] )=> declName = d:declarator[False]
                            {
                            d2 = self.astFactory.dupList(#d)
                            ds2 = self.astFactory.dupList(#ds)
                            self.symbolTable.add(declName, #(None, ds2, d2))
                            }
                | nonemptyAbstractDeclarator
                )?
                            { ## = #( #[NParameterDeclaration], ## ) }
        ;

/* JTC:
 * This handles both new and old style functions.
 * see declarator rule to see differences in parameters
 * and here (declaration SEMI)* is the param type decls for the
 * old style.  may want to do some checking to check for illegal
 * combinations (but I assume all parsed code will be legal?)
 */

functionDef
        :       ( (functionDeclSpecifiers)=> ds:functionDeclSpecifiers
                |  //epsilon
                )
                declName = d:declarator[True]
                            {
                            d2 = self.astFactory.dupList(#d)
                            ds2 = self.astFactory.dupList(#ds)
                            self.symbolTable.add(declName, #(None, ds2, d2))
                            pushScope(declName)
                            }
                ( declaration )* (VARARGS)? ( SEMI! )*
                            { popScope() }
                compoundStatement[declName]
                            { ## = #( #[NFunctionDef], ## )}
        ;

functionDeclSpecifiers
        :       (               options {   // this loop properly aborts when
                                            // it finds a non-typedefName ID MBZ
                                            warnWhenFollowAmbig = false;
                                        } :
                  functionStorageClassSpecifier
                | typeQualifier
                | ( typeSpecifier[specCount] )=>
                        specCount = typeSpecifier[specCount]
                )+
        ;

declarationList
        :       (               options {   // this loop properly aborts when
                                            // it finds a non-typedefName ID MBZ
                                            warnWhenFollowAmbig = false;
                                        } :
                declaration
                )+
        ;

declarationPredictor
        :       (options {      //only want to look at declaration if I don't see typedef
                    warnWhenFollowAmbig = false;
                }:
                "typedef"
                | declaration
                )
        ;


compoundStatement[scopeName]
        :       LCURLY!
                            {
                                pushScope(scopeName);
                            }
                ( ( declarationPredictor)=> declarationList )?
                ( statementList )?
                            { popScope() }
                RCURLY!
                            { ## = #( #[NCompoundStatement, scopeName], ##) }
        ;

    
statementList
        :       ( statement )+
        ;
statement
        :       SEMI                    // Empty statements

        |       compoundStatement[self.getAScopeName()]       // Group of statements

        |       expr SEMI!               { ## = #( #[NStatementExpr], ## ) } // Expressions

// Iteration statements:

        |       "while"^ LPAREN! expr RPAREN! statement
        |       "do"^ statement "while"! LPAREN! expr RPAREN! SEMI!
        |!       "for"
                LPAREN ( e1:expr )? SEMI ( e2:expr )? SEMI ( e3:expr )? RPAREN
                s:statement
                                    {
                                    if ( #e1 is None):
                                        #e1 = #[ NEmptyExpression ]
                                    if ( #e2 is None):
                                        #e2 = #[ NEmptyExpression ]
                                    if ( #e3 is None):
                                        #e3 = #[ NEmptyExpression ]
                                    ## = #( #[LITERAL_for, "for"], #e1, #e2, #e3, #s )
                                    }


// Jump statements:

        |       "goto"^ ID SEMI!
        |       "continue" SEMI!
        |       "break" SEMI!
        |       "return"^ ( expr )? SEMI!


// Labeled statements:
        |       ID COLON! (options {warnWhenFollowAmbig=false;}:statement)? { ## = #( #[NLabel], ## ) }
        |       "case"^ constExpr COLON! statement
        |       "default"^ COLON! statement



// Selection statements:

        |       "if"^
                 LPAREN! expr RPAREN! statement  
                ( //standard if-else ambiguity
                        options {
                            warnWhenFollowAmbig = false;
                        } :
                "else" statement )?
        |       "switch"^ LPAREN! expr RPAREN! statement
        ;






expr
        :       assignExpr (options {
                                /* MBZ:
                                    COMMA is ambiguous between comma expressions and
                                    argument lists.  argExprList should get priority,
                                    and it does by being deeper in the expr rule tree
                                    and using (COMMA assignExpr)*
                                */
                                warnWhenFollowAmbig = false;
                            } :
                            c:COMMA^ { #c.setType(NCommaExpr) } assignExpr         
                            )*
        ;


assignExpr
        :       conditionalExpr ( a:assignOperator! assignExpr { ## = #( #a, ## )} )?
        ;

assignOperator
        :       ASSIGN
        |       DIV_ASSIGN
        |       PLUS_ASSIGN
        |       MINUS_ASSIGN
        |       STAR_ASSIGN
        |       MOD_ASSIGN
        |       RSHIFT_ASSIGN
        |       LSHIFT_ASSIGN
        |       BAND_ASSIGN
        |       BOR_ASSIGN
        |       BXOR_ASSIGN
        ;


conditionalExpr
        :       logicalOrExpr
                ( QUESTION^ expr COLON! conditionalExpr )?
        ;


constExpr
        :       conditionalExpr
        ;

logicalOrExpr
        :       logicalAndExpr ( LOR^ logicalAndExpr )*
        ;


logicalAndExpr
        :       inclusiveOrExpr ( LAND^ inclusiveOrExpr )*
        ;

inclusiveOrExpr
        :       exclusiveOrExpr ( BOR^ exclusiveOrExpr )*
        ;


exclusiveOrExpr
        :       bitAndExpr ( BXOR^ bitAndExpr )*
        ;


bitAndExpr
        :       equalityExpr ( BAND^ equalityExpr )*
        ;



equalityExpr
        :       relationalExpr
                ( ( EQUAL^ | NOT_EQUAL^ ) relationalExpr )*
        ;


relationalExpr
        :       shiftExpr
                ( ( LT^ | LTE^ | GT^ | GTE^ ) shiftExpr )*
        ;



shiftExpr
        :       additiveExpr
                ( ( LSHIFT^ | RSHIFT^ ) additiveExpr )*
        ;


additiveExpr
        :       multExpr
                ( ( PLUS^ | MINUS^ ) multExpr )*
        ;


multExpr
        :       castExpr
                ( ( STAR^ | DIV^ | MOD^ ) castExpr )*
        ;


castExpr
        :       ( LPAREN typeName RPAREN )=>
                LPAREN! typeName RPAREN! ( castExpr )
                            { ## = #( #[NCast, "("], ## ) }

        |       unaryExpr
        ;


typeName
        :       specifierQualifierList (nonemptyAbstractDeclarator)?
        ;

nonemptyAbstractDeclarator
        :   (
                pointerGroup
                (   (LPAREN  
                    (   nonemptyAbstractDeclarator
                        | parameterTypeList
                    )?
                    RPAREN)
                | (LBRACKET (expr)? RBRACKET)
                )*

            |   (   (LPAREN  
                    (   nonemptyAbstractDeclarator
                        | parameterTypeList
                    )?
                    RPAREN)
                | (LBRACKET (expr)? RBRACKET)
                )+
            )
                            {   ## = #( #[NNonemptyAbstractDeclarator], ## ) }
                                
        ;

/* JTC:

LR rules:

abstractDeclarator
        :       nonemptyAbstractDeclarator
        |       // null
        ;

nonemptyAbstractDeclarator
        :       LPAREN  nonemptyAbstractDeclarator RPAREN
        |       abstractDeclarator LPAREN RPAREN
        |       abstractDeclarator (LBRACKET (expr)? RBRACKET)
        |       STAR abstractDeclarator
        ;
*/

unaryExpr
        :       postfixExpr
        |       INC^ unaryExpr
        |       DEC^ unaryExpr
        |       u:unaryOperator castExpr { ## = #( #[NUnaryExpr], ## ) }

        |       "sizeof"^
                ( ( LPAREN typeName )=> LPAREN typeName RPAREN
                | unaryExpr
                )
        ;


unaryOperator
        :       BAND
        |       STAR
        |       PLUS
        |       MINUS
        |       BNOT
        |       LNOT
        ;

postfixExpr
        :       primaryExpr
                ( 
                postfixSuffix                   {## = #( #[NPostfixExpr], ## )} 
                )?
        ;
postfixSuffix
        :
                ( PTR ID
                | DOT ID
                | functionCall
                | LBRACKET expr RBRACKET
                | INC
                | DEC
                )+
        ;

functionCall
        :
                LPAREN^ (a:argExprList)? RPAREN
                        {
                        ##.setType( NFunctionCallArgs )
                        }
        ;
    

primaryExpr
        :       ID
        |       charConst
        |       intConst
        |       floatConst
        |       stringConst

// JTC:
// ID should catch the enumerator
// leaving it in gives ambiguous err
//      | enumerator
        |       LPAREN! expr RPAREN!        { ## = #( #[NExpressionGroup, "("], ## ) }
        ;

argExprList
        :       assignExpr ( COMMA! assignExpr )*
        ;



protected
charConst
        :       CharLiteral
        ;


protected
stringConst
        :       (StringLiteral)+                { ## = #(#[NStringSeq], ##) }
        ;


protected
intConst
        :       IntOctalConst
        |       LongOctalConst
        |       UnsignedOctalConst
        |       IntIntConst
        |       LongIntConst
        |       UnsignedIntConst
        |       IntHexConst
        |       LongHexConst
        |       UnsignedHexConst
        ;


protected
floatConst
        :       FloatDoubleConst
        |       DoubleDoubleConst
        |       LongDoubleConst
        ;




    

dummy
        :       NTypedefName
        |       NInitDecl
        |       NDeclarator
        |       NStructDeclarator
        |       NDeclaration
        |       NCast
        |       NPointerGroup
        |       NExpressionGroup
        |       NFunctionCallArgs
        |       NNonemptyAbstractDeclarator
        |       NInitializer
        |       NStatementExpr
        |       NEmptyExpression
        |       NParameterTypeList
        |       NFunctionDef
        |       NCompoundStatement
        |       NParameterDeclaration
        |       NCommaExpr
        |       NUnaryExpr
        |       NLabel
        |       NPostfixExpr
        |       NRangeExpr
        |       NStringSeq
        |       NInitializerElementLabel
        |       NLcurlyInitializer
        |       NAsmAttribute
        |       NGnuAsmExpr
        |       NTypeMissing
        ;



    


{
        from ctoken import CToken
        from lineobject import LineObject
        from preprocessorinfochannel import PreprocessorInfoChannel
        from antlr import *
}

class StdCLexer extends Lexer;

options
        {
        k = 3;
        exportVocab = STDC;
        testLiterals = false;
        }

{
    def setCountingTokens(self, ct):
        self.countingTokens = ct
        if self.countingTokens:
            self.tokenNumber = 0
        else:
            self.tokenNumber = 1

    def setOriginalSource(self, src):
        self.originalSource = src
        self.lineObject.setSource(src)

    def setSource(self, src):
        self.lineObject.setSource(src)


    def getPreprocessorInfoChannel(self):
        return self.preprocessorInfoChannel

    def setPreprocessingDirective(self, pre):
        self.preprocessorInfoChannel.addLineForTokenNumber(pre, self.tokenNumber)

    def makeToken(self, t):
        if t != Token.SKIP and self.countingTokens:
            self.tokenNumber += 1

        tok = antlr.CharScanner.makeToken(self, t)
        tok.setLine(self.lineObject.line)
        //tok.setSource(self.lineObject.source)
        //tok.setTokenNumber(self.tokenNumber)

        self.lineObject.line += self.deferredLineCount
        self.deferredLineCount = 0

        return tok

    def deferredNewline(self):
        self.deferredLineCount += 1

    def newline(self):
        self.lineObject.newline()
}

protected
Vocabulary
        :       '\3'..'\377'
        ;


/* Operators: */

ASSIGN          : '=' ;
COLON           : ':' ;
COMMA           : ',' ;
QUESTION        : '?' ;
SEMI            : ';' ;
PTR             : "->" ;


// DOT & VARARGS are commented out since they are generated as part of
// the Number rule below due to some bizarre lexical ambiguity shme.

// DOT  :       '.' ;
protected
DOT:;

// VARARGS      : "..." ;
protected
VARARGS:;


LPAREN          : '(' ;
RPAREN          : ')' ;
LBRACKET        : '[' ;
RBRACKET        : ']' ;
LCURLY          : '{' ;
RCURLY          : '}' ;

EQUAL           : "==" ;
NOT_EQUAL       : "!=" ;
LTE             : "<=" ;
LT              : "<" ;
GTE             : ">=" ;
GT              : ">" ;

DIV             : '/' ;
DIV_ASSIGN      : "/=" ;
PLUS            : '+' ;
PLUS_ASSIGN     : "+=" ;
INC             : "++" ;
MINUS           : '-' ;
MINUS_ASSIGN    : "-=" ;
DEC             : "--" ;
STAR            : '*' ;
STAR_ASSIGN     : "*=" ;
MOD             : '%' ;
MOD_ASSIGN      : "%=" ;
RSHIFT          : ">>" ;
RSHIFT_ASSIGN   : ">>=" ;
LSHIFT          : "<<" ;
LSHIFT_ASSIGN   : "<<=" ;

LAND            : "&&" ;
LNOT            : '!' ;
LOR             : "||" ;

BAND            : '&' ;
BAND_ASSIGN     : "&=" ;
BNOT            : '~' ;
BOR             : '|' ;
BOR_ASSIGN      : "|=" ;
BXOR            : '^' ;
BXOR_ASSIGN     : "^=" ;


Whitespace
        :       ( ( '\003'..'\010' | '\t' | '\013' | '\f' | '\016'.. '\037' | '\177'..'\377' | ' ' )
                | "\r\n"                { self.newline() }
                | '\r' {self.newline()}
                | '\n' {self.newline()}
                )                       { $skip }
        ;


Comment
        :       "/*"
                ( { LA(2) != '/' }? '*'
                | ( '\r' | '\n' )       { self.deferredNewline()    }
                | ~( '*'| '\r' | '\n' )
                )*
                "*/"                    { $skip}
        ;


CPPComment
        :
                "//" ( ~('\n') )* 
                        {$skip}
        ;

PREPROC_DIRECTIVE
options {
  paraphrase = "a line directive";
}

        :
        '#'
        ( ( "line" || (( ' ' | '\t' | '\014')+ '0'..'9')) => LineDirective      
            | (~'\n')*                                  { self.setPreprocessingDirective(getText()) }
        )
                {$skip}
        ;

protected  Space:
        (' ' | '\t' | '\014')
        ;

protected LineDirective
{ oldCountingTokens = self.countingTokens; self.countingTokens = False;}
:
        {self.lineObject = LineObject(); self.deferredLineCount = 0}
        ("line")?  //this would be for if the directive started "#line", but not there for GNU directives
        (Space)+
        n:Number { self.lineObject.line = int(n.getText()) } 
        (options {greedy = true;} : Space)+
        (       fn:StringLiteral {self.lineObject.source = fn.getText()[1:-1]}  (options {greedy = true;} : Space) *
                | fi:ID {self.lineObject.source = fi.getText() } (options {greedy = true;} : Space)+
        )?
        ('1'            {self.lineObject.enteringFile = True} (options {greedy=true;}:Space)*)?
        ('2'            {self.lineObject.returningToFile = True } (options {greedy=true;}:Space)*)?
        ('3'            {self.lineObject.systemHeader = True } (options {greedy=true;}:Space)* )?
        ('4'            {self.lineObject.treatAsC = True } (options {greedy=true;}:Space)* )?
        ("\r\n" | "\r" | "\n")
                {self.preprocessorInfoChannel.addLineForTokenNumber(LineObject(self.lineObject), self.tokenNumber); self.countingTokens = oldCountingTokens; }
        ;



/* Literals: */

/*
 * Note that we do NOT handle tri-graphs nor multi-byte sequences.
 */


/*
 * Note that we can't have empty character constants (even though we
 * can have empty strings :-).
 */
CharLiteral
        :       '\'' ( Escape | ~( '\'' ) ) '\''
        ;


/*
 * Can't have raw imbedded newlines in string constants.  Strict reading of
 * the standard gives odd dichotomy between newlines & carriage returns.
 * Go figure.
 */
StringLiteral
        :       '"'
                ( Escape
                | ( 
                    '\r'        { self.deferredNewline() }
                  | '\n'        {
                                self.deferredNewline()
                                _ttype = BadStringLiteral;
                                }
                  )
                | ~( '"' | '\r' | '\n' | '\\' )
                )*
                '"'
        ;


protected BadStringLiteral
        :       // Imaginary token.
        ;


/*
 * Handle the various escape sequences.
 *
 * Note carefully that these numeric escape *sequences* are *not* of the
 * same form as the C language numeric *constants*.
 *
 * There is no such thing as a binary numeric escape sequence.
 *
 * Octal escape sequences are either 1, 2, or 3 octal digits exactly.
 *
 * There is no such thing as a decimal escape sequence.
 *
 * Hexadecimal escape sequences are begun with a leading \x and continue
 * until a non-hexadecimal character is found.
 *
 * No real handling of tri-graph sequences, yet.
 */

protected
Escape  
        :       '\\'
                ( options{warnWhenFollowAmbig=false;}:
                  'a'
                | 'b'
                | 'f'
                | 'n'
                | 'r'
                | 't'
                | 'v'
                | '"'
                | '\''
                | '\\'
                | '?'
                | ('0'..'3') ( options{warnWhenFollowAmbig=false;}: Digit ( options{warnWhenFollowAmbig=false;}: Digit )? )?
                | ('4'..'7') ( options{warnWhenFollowAmbig=false;}: Digit )?
                | 'x' ( options{warnWhenFollowAmbig=false;}: Digit | 'a'..'f' | 'A'..'F' )+
                )
        ;


/* Numeric Constants: */

protected
Digit
        :       '0'..'9'
        ;

protected
LongSuffix
        :       'l'
        |       'L'
        ;

protected
UnsignedSuffix
        :       'u'
        |       'U'
        ;

protected
FloatSuffix
        :       'f'
        |       'F'
        ;

protected
Exponent
        :       ( 'e' | 'E' ) ( '+' | '-' )? ( Digit )+
        ;


protected
DoubleDoubleConst:;

protected
FloatDoubleConst:;

protected
LongDoubleConst:;

protected
IntOctalConst:;

protected
LongOctalConst:;

protected
UnsignedOctalConst:;

protected
IntIntConst:;

protected
LongIntConst:;

protected
UnsignedIntConst:;

protected
IntHexConst:;

protected
LongHexConst:;

protected
UnsignedHexConst:;




Number
        :       ( ( Digit )+ ( '.' | 'e' | 'E' ) )=> ( Digit )+
                ( '.' ( Digit )* ( Exponent )?
                | Exponent
                )                       { _ttype = DoubleDoubleConst;   }
                ( FloatSuffix           { _ttype = FloatDoubleConst;    }
                | LongSuffix            { _ttype = LongDoubleConst;     }
                )?

        |       ( "..." )=> "..."       { _ttype = VARARGS;     }

        |       '.'                     { _ttype = DOT; }
                ( ( Digit )+ ( Exponent )?
                                        { _ttype = DoubleDoubleConst;   }
                  ( FloatSuffix         { _ttype = FloatDoubleConst;    }
                  | LongSuffix          { _ttype = LongDoubleConst;     }
                  )?
                )?

        |       '0' ( '0'..'7' )*       { _ttype = IntOctalConst;       }
                ( LongSuffix            { _ttype = LongOctalConst;      }
                | UnsignedSuffix        { _ttype = UnsignedOctalConst;  }
                )?

        |       '1'..'9' ( Digit )*     { _ttype = IntIntConst;         }
                ( LongSuffix            { _ttype = LongIntConst;        }
                | UnsignedSuffix        { _ttype = UnsignedIntConst;    }
                )?

        |       '0' ( 'x' | 'X' ) ( 'a'..'f' | 'A'..'F' | Digit )+
                                        { _ttype = IntHexConst;         }
                ( LongSuffix            { _ttype = LongHexConst;        }
                | UnsignedSuffix        { _ttype = UnsignedHexConst;    }
                )?
        ;


ID
        options 
                {
                testLiterals = true; 
                }
        :       ( 'a'..'z' | 'A'..'Z' | '_' )
                ( 'a'..'z' | 'A'..'Z' | '_' | '0'..'9' )*
        ;


