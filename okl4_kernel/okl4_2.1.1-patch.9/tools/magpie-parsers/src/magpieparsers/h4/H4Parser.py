### $ANTLR 2.7.5 (20050403): "h4.g" -> "H4Parser.py"$
### import antlr and other modules ..
import sys
import antlr

version = sys.version.split()[0]
if version < '2.2.1':
    False = 0
if version < '2.3':
    True = not False
### header action >>> 

### header action <<< 
### preamble action>>>

### preamble action <<<

### import antlr.Token 
from antlr import Token
### >>>The Known Token Types <<<
SKIP                = antlr.SKIP
INVALID_TYPE        = antlr.INVALID_TYPE
EOF_TYPE            = antlr.EOF_TYPE
EOF                 = antlr.EOF
NULL_TREE_LOOKAHEAD = antlr.NULL_TREE_LOOKAHEAD
MIN_USER_TYPE       = antlr.MIN_USER_TYPE
LITERAL_module = 4
IDENT = 5
LCURLY = 6
RCURLY = 7
DEFINED_AS = 8
SEMI = 9
COLON = 10
PLUS = 11
MINUS = 12
STAR = 13
DIV = 14
MOD = 15
TILDE = 16
LBRACKET = 17
RBRACKET = 18
HASH = 19
PERIOD = 20
INT = 21
ATOM = 22
QUOTEDSTRING = 23
COMMA = 24
LETTER = 25
NONZERODIGIT = 26
DIGIT = 27
WS = 28
Comment = 29
EndOfLine = 30

class Parser(antlr.LLkParser):
    ### user action >>>
    ### user action <<<
    
    def __init__(self, *args, **kwargs):
        antlr.LLkParser.__init__(self, *args, **kwargs)
        self.tokenNames = _tokenNames
        self.buildTokenTypeASTClassMap()
        self.astFactory = antlr.ASTFactory(self.getTokenTypeToASTClassMap())
        self.astFactory.setASTNodeClass()
        
    def declarations(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        declarations_AST = None
        try:      ## for error handling
            pass
            while True:
                if (self.LA(1)==LITERAL_module or self.LA(1)==IDENT):
                    pass
                    self.declaration()
                    self.addASTChild(currentAST, self.returnAST)
                else:
                    break
                
            declarations_AST = currentAST.root
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_0)
        
        self.returnAST = declarations_AST
    
    def declaration(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        declaration_AST = None
        try:      ## for error handling
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [IDENT]:
                pass
                self.func_decl()
                self.addASTChild(currentAST, self.returnAST)
                declaration_AST = currentAST.root
            elif la1 and la1 in [LITERAL_module]:
                pass
                self.module_decl()
                self.addASTChild(currentAST, self.returnAST)
                declaration_AST = currentAST.root
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_1)
        
        self.returnAST = declaration_AST
    
    def func_decl(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        func_decl_AST = None
        try:      ## for error handling
            pass
            self.func_name()
            self.addASTChild(currentAST, self.returnAST)
            self.match(DEFINED_AS)
            self.expression()
            self.addASTChild(currentAST, self.returnAST)
            self.match(SEMI)
            func_decl_AST = currentAST.root
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_1)
        
        self.returnAST = func_decl_AST
    
    def module_decl(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        module_decl_AST = None
        try:      ## for error handling
            pass
            self.match(LITERAL_module)
            tmp4_AST = None
            tmp4_AST = self.astFactory.create(self.LT(1))
            self.addASTChild(currentAST, tmp4_AST)
            self.match(IDENT)
            self.match(LCURLY)
            self.declarations()
            self.addASTChild(currentAST, self.returnAST)
            self.match(RCURLY)
            module_decl_AST = currentAST.root
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_1)
        
        self.returnAST = module_decl_AST
    
    def func_name(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        func_name_AST = None
        try:      ## for error handling
            if (self.LA(1)==IDENT) and (_tokenSet_2.member(self.LA(2))):
                pass
                tmp7_AST = None
                tmp7_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp7_AST)
                self.match(IDENT)
                func_name_AST = currentAST.root
            elif (self.LA(1)==IDENT) and (self.LA(2)==COLON):
                pass
                _cnt10= 0
                while True:
                    if (self.LA(1)==IDENT) and (self.LA(2)==COLON):
                        pass
                        tmp8_AST = None
                        tmp8_AST = self.astFactory.create(self.LT(1))
                        self.addASTChild(currentAST, tmp8_AST)
                        self.match(IDENT)
                        tmp9_AST = None
                        tmp9_AST = self.astFactory.create(self.LT(1))
                        self.addASTChild(currentAST, tmp9_AST)
                        self.match(COLON)
                        la1 = self.LA(1)
                        if False:
                            pass
                        elif la1 and la1 in [INT,ATOM,QUOTEDSTRING]:
                            pass
                            self.literal()
                            self.addASTChild(currentAST, self.returnAST)
                        elif la1 and la1 in [HASH]:
                            pass
                            self.constructed_type()
                            self.addASTChild(currentAST, self.returnAST)
                        elif la1 and la1 in [IDENT]:
                            pass
                            tmp10_AST = None
                            tmp10_AST = self.astFactory.create(self.LT(1))
                            self.addASTChild(currentAST, tmp10_AST)
                            self.match(IDENT)
                        else:
                                raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                            
                    else:
                        break
                    
                    _cnt10 += 1
                if _cnt10 < 1:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                la1 = self.LA(1)
                if False:
                    pass
                elif la1 and la1 in [IDENT]:
                    pass
                    tmp11_AST = None
                    tmp11_AST = self.astFactory.create(self.LT(1))
                    self.addASTChild(currentAST, tmp11_AST)
                    self.match(IDENT)
                elif la1 and la1 in [DEFINED_AS,SEMI,PLUS,MINUS,STAR,DIV,MOD,RBRACKET,HASH]:
                    pass
                else:
                        raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                    
                func_name_AST = currentAST.root
            else:
                raise antlr.NoViableAltException(self.LT(1), self.getFilename())
            
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_2)
        
        self.returnAST = func_name_AST
    
    def expression(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        expression_AST = None
        try:      ## for error handling
            pass
            self.add_expr()
            self.addASTChild(currentAST, self.returnAST)
            expression_AST = currentAST.root
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_3)
        
        self.returnAST = expression_AST
    
    def literal(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        literal_AST = None
        try:      ## for error handling
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [INT]:
                pass
                self.integer_literal()
                self.addASTChild(currentAST, self.returnAST)
                literal_AST = currentAST.root
            elif la1 and la1 in [ATOM]:
                pass
                self.atom_literal()
                self.addASTChild(currentAST, self.returnAST)
                literal_AST = currentAST.root
            elif la1 and la1 in [QUOTEDSTRING]:
                pass
                self.string_literal()
                self.addASTChild(currentAST, self.returnAST)
                literal_AST = currentAST.root
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_4)
        
        self.returnAST = literal_AST
    
    def constructed_type(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        constructed_type_AST = None
        try:      ## for error handling
            pass
            self.node_constructor()
            self.addASTChild(currentAST, self.returnAST)
            constructed_type_AST = currentAST.root
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_4)
        
        self.returnAST = constructed_type_AST
    
    def add_expr(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        add_expr_AST = None
        try:      ## for error handling
            pass
            self.mult_expr()
            self.addASTChild(currentAST, self.returnAST)
            while True:
                if (self.LA(1)==PLUS or self.LA(1)==MINUS):
                    pass
                    la1 = self.LA(1)
                    if False:
                        pass
                    elif la1 and la1 in [PLUS]:
                        pass
                        tmp12_AST = None
                        tmp12_AST = self.astFactory.create(self.LT(1))
                        self.addASTChild(currentAST, tmp12_AST)
                        self.match(PLUS)
                    elif la1 and la1 in [MINUS]:
                        pass
                        tmp13_AST = None
                        tmp13_AST = self.astFactory.create(self.LT(1))
                        self.addASTChild(currentAST, tmp13_AST)
                        self.match(MINUS)
                    else:
                            raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                        
                    self.mult_expr()
                    self.addASTChild(currentAST, self.returnAST)
                else:
                    break
                
            add_expr_AST = currentAST.root
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_3)
        
        self.returnAST = add_expr_AST
    
    def mult_expr(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        mult_expr_AST = None
        try:      ## for error handling
            pass
            self.unary_expr()
            self.addASTChild(currentAST, self.returnAST)
            while True:
                if ((self.LA(1) >= STAR and self.LA(1) <= MOD)):
                    pass
                    la1 = self.LA(1)
                    if False:
                        pass
                    elif la1 and la1 in [STAR]:
                        pass
                        tmp14_AST = None
                        tmp14_AST = self.astFactory.create(self.LT(1))
                        self.addASTChild(currentAST, tmp14_AST)
                        self.match(STAR)
                    elif la1 and la1 in [DIV]:
                        pass
                        tmp15_AST = None
                        tmp15_AST = self.astFactory.create(self.LT(1))
                        self.addASTChild(currentAST, tmp15_AST)
                        self.match(DIV)
                    elif la1 and la1 in [MOD]:
                        pass
                        tmp16_AST = None
                        tmp16_AST = self.astFactory.create(self.LT(1))
                        self.addASTChild(currentAST, tmp16_AST)
                        self.match(MOD)
                    else:
                            raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                        
                    self.unary_expr()
                    self.addASTChild(currentAST, self.returnAST)
                else:
                    break
                
            mult_expr_AST = currentAST.root
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_5)
        
        self.returnAST = mult_expr_AST
    
    def unary_expr(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        unary_expr_AST = None
        try:      ## for error handling
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [PLUS,MINUS,TILDE]:
                pass
                la1 = self.LA(1)
                if False:
                    pass
                elif la1 and la1 in [MINUS]:
                    pass
                    tmp17_AST = None
                    tmp17_AST = self.astFactory.create(self.LT(1))
                    self.addASTChild(currentAST, tmp17_AST)
                    self.match(MINUS)
                elif la1 and la1 in [PLUS]:
                    pass
                    tmp18_AST = None
                    tmp18_AST = self.astFactory.create(self.LT(1))
                    self.addASTChild(currentAST, tmp18_AST)
                    self.match(PLUS)
                elif la1 and la1 in [TILDE]:
                    pass
                    tmp19_AST = None
                    tmp19_AST = self.astFactory.create(self.LT(1))
                    self.addASTChild(currentAST, tmp19_AST)
                    self.match(TILDE)
                else:
                        raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                    
                self.primary_expr()
                self.addASTChild(currentAST, self.returnAST)
                unary_expr_AST = currentAST.root
            elif la1 and la1 in [IDENT,LBRACKET,HASH,INT,ATOM,QUOTEDSTRING]:
                pass
                self.primary_expr()
                self.addASTChild(currentAST, self.returnAST)
                unary_expr_AST = currentAST.root
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_6)
        
        self.returnAST = unary_expr_AST
    
    def primary_expr(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        primary_expr_AST = None
        try:      ## for error handling
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [INT,ATOM,QUOTEDSTRING]:
                pass
                self.literal()
                self.addASTChild(currentAST, self.returnAST)
                primary_expr_AST = currentAST.root
            elif la1 and la1 in [HASH]:
                pass
                self.constructed_type()
                self.addASTChild(currentAST, self.returnAST)
                primary_expr_AST = currentAST.root
            elif la1 and la1 in [IDENT]:
                pass
                self.scoped_name()
                self.addASTChild(currentAST, self.returnAST)
                primary_expr_AST = currentAST.root
            elif la1 and la1 in [LBRACKET]:
                pass
                tmp20_AST = None
                tmp20_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp20_AST)
                self.match(LBRACKET)
                self.expression()
                self.addASTChild(currentAST, self.returnAST)
                tmp21_AST = None
                tmp21_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp21_AST)
                self.match(RBRACKET)
                primary_expr_AST = currentAST.root
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_6)
        
        self.returnAST = primary_expr_AST
    
    def scoped_name(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        scoped_name_AST = None
        try:      ## for error handling
            pass
            if (self.LA(1)==IDENT) and (self.LA(2)==PERIOD):
                pass
                self.scope()
                self.addASTChild(currentAST, self.returnAST)
            elif (self.LA(1)==IDENT) and (_tokenSet_7.member(self.LA(2))):
                pass
            else:
                raise antlr.NoViableAltException(self.LT(1), self.getFilename())
            
            self.func_name()
            self.addASTChild(currentAST, self.returnAST)
            scoped_name_AST = currentAST.root
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_6)
        
        self.returnAST = scoped_name_AST
    
    def node_constructor(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        node_constructor_AST = None
        try:      ## for error handling
            pass
            self.match(HASH)
            self.expression()
            self.addASTChild(currentAST, self.returnAST)
            self.match(HASH)
            node_constructor_AST = currentAST.root
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_4)
        
        self.returnAST = node_constructor_AST
    
    def scope(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        scope_AST = None
        try:      ## for error handling
            pass
            tmp24_AST = None
            tmp24_AST = self.astFactory.create(self.LT(1))
            self.addASTChild(currentAST, tmp24_AST)
            self.match(IDENT)
            tmp25_AST = None
            tmp25_AST = self.astFactory.create(self.LT(1))
            self.addASTChild(currentAST, tmp25_AST)
            self.match(PERIOD)
            scope_AST = currentAST.root
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_8)
        
        self.returnAST = scope_AST
    
    def integer_literal(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        integer_literal_AST = None
        try:      ## for error handling
            pass
            tmp26_AST = None
            tmp26_AST = self.astFactory.create(self.LT(1))
            self.addASTChild(currentAST, tmp26_AST)
            self.match(INT)
            integer_literal_AST = currentAST.root
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_4)
        
        self.returnAST = integer_literal_AST
    
    def atom_literal(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        atom_literal_AST = None
        try:      ## for error handling
            pass
            tmp27_AST = None
            tmp27_AST = self.astFactory.create(self.LT(1))
            self.addASTChild(currentAST, tmp27_AST)
            self.match(ATOM)
            atom_literal_AST = currentAST.root
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_4)
        
        self.returnAST = atom_literal_AST
    
    def string_literal(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        string_literal_AST = None
        try:      ## for error handling
            pass
            tmp28_AST = None
            tmp28_AST = self.astFactory.create(self.LT(1))
            self.addASTChild(currentAST, tmp28_AST)
            self.match(QUOTEDSTRING)
            string_literal_AST = currentAST.root
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_4)
        
        self.returnAST = string_literal_AST
    
    
    def buildTokenTypeASTClassMap(self):
        self.tokenTypeToASTClassMap = None

_tokenNames = [
    "<0>", 
    "EOF", 
    "<2>", 
    "NULL_TREE_LOOKAHEAD", 
    "\"module\"", 
    "IDENT", 
    "LCURLY", 
    "RCURLY", 
    "DEFINED_AS", 
    "SEMI", 
    "COLON", 
    "PLUS", 
    "MINUS", 
    "STAR", 
    "DIV", 
    "MOD", 
    "TILDE", 
    "LBRACKET", 
    "RBRACKET", 
    "HASH", 
    "PERIOD", 
    "INT", 
    "ATOM", 
    "QUOTEDSTRING", 
    "COMMA", 
    "LETTER", 
    "NONZERODIGIT", 
    "DIGIT", 
    "WS", 
    "Comment", 
    "EndOfLine"
]
    

### generate bit set
def mk_tokenSet_0(): 
    ### var1
    data = [ 128L, 0L]
    return data
_tokenSet_0 = antlr.BitSet(mk_tokenSet_0())

### generate bit set
def mk_tokenSet_1(): 
    ### var1
    data = [ 176L, 0L]
    return data
_tokenSet_1 = antlr.BitSet(mk_tokenSet_1())

### generate bit set
def mk_tokenSet_2(): 
    ### var1
    data = [ 850688L, 0L]
    return data
_tokenSet_2 = antlr.BitSet(mk_tokenSet_2())

### generate bit set
def mk_tokenSet_3(): 
    ### var1
    data = [ 786944L, 0L]
    return data
_tokenSet_3 = antlr.BitSet(mk_tokenSet_3())

### generate bit set
def mk_tokenSet_4(): 
    ### var1
    data = [ 850720L, 0L]
    return data
_tokenSet_4 = antlr.BitSet(mk_tokenSet_4())

### generate bit set
def mk_tokenSet_5(): 
    ### var1
    data = [ 793088L, 0L]
    return data
_tokenSet_5 = antlr.BitSet(mk_tokenSet_5())

### generate bit set
def mk_tokenSet_6(): 
    ### var1
    data = [ 850432L, 0L]
    return data
_tokenSet_6 = antlr.BitSet(mk_tokenSet_6())

### generate bit set
def mk_tokenSet_7(): 
    ### var1
    data = [ 851456L, 0L]
    return data
_tokenSet_7 = antlr.BitSet(mk_tokenSet_7())

### generate bit set
def mk_tokenSet_8(): 
    ### var1
    data = [ 32L, 0L]
    return data
_tokenSet_8 = antlr.BitSet(mk_tokenSet_8())
    
