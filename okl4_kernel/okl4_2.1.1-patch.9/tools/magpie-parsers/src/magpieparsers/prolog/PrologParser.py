### $ANTLR 2.7.6 (20060903): "prolog.g" -> "PrologParser.py"$
### import antlr and other modules ..
import sys
import antlr

version = sys.version.split()[0]
if version < '2.2.1':
    False = 0
if version < '2.3':
    True = not False
### header action >>> 
from magpieparsers.parser_common import Node

def expr_node(parent, oper, child):
   container = Node(None, "expression", leaf = oper, children = [parent, child])
   return container
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
DEFINEDAS = 4
FULLSTOP = 5
SEMICOLON = 6
COMMA = 7
NOT = 8
LESSTHANOREQ = 9
GREATERTHANOREQ = 10
GREATERTHAN = 11
LESSTHAN = 12
EQUAL = 13
LBRACKET = 14
RBRACKET = 15
ATOM = 16
VARIABLE = 17
NUMBER = 18
LCURLY = 19
RCURLY = 20
LSQUARE = 21
RSQUARE = 22
BAR = 23
WS = 24
COMMENT = 25

class Parser(antlr.LLkParser):
    ### user action >>>
    ### user action <<<
    
    def __init__(self, *args, **kwargs):
        antlr.LLkParser.__init__(self, *args, **kwargs)
        self.tokenNames = _tokenNames
        ### __init__ header action >>> 
        self.terms = []
        ### __init__ header action <<< 
        
    def topterm(self):    
        t = None
        
        try:      ## for error handling
            pass
            t=self.term()
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_0)
        
        return t
    
    def term(self):    
        t = None
        
        try:      ## for error handling
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [VARIABLE]:
                pass
                v=self.variable()
                t = v
            elif la1 and la1 in [LSQUARE]:
                pass
                l=self.prologlist()
                t = l
            elif la1 and la1 in [NUMBER]:
                pass
                n=self.number()
                t = n
            elif la1 and la1 in [LCURLY]:
                pass
                d=self.dictionary()
                t = d
            else:
                if (self.LA(1)==ATOM) and (_tokenSet_1.member(self.LA(2))):
                    pass
                    a=self.atom()
                    t = a
                elif (self.LA(1)==ATOM) and (self.LA(2)==LBRACKET):
                    pass
                    s=self.structure()
                    t = s
                else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_1)
        
        return t
    
    def clauses(self):    
        l = None
        
        l = Node(None, "clauses")
        try:      ## for error handling
            pass
            c=self.clause()
            l.add_child(c)
            while True:
                if (self.LA(1)==DEFINEDAS or self.LA(1)==FULLSTOP or self.LA(1)==ATOM):
                    pass
                    c=self.clause()
                    l.add_child(c)
                else:
                    break
                
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_0)
        
        return l
    
    def clause(self):    
        c = None
        
        c = Node(None, "clause")
        try:      ## for error handling
            pass
            if (self.LA(1)==ATOM) and (self.LA(2)==LBRACKET):
                pass
                s=self.structure()
                c.add_child(s)
            elif (self.LA(1)==ATOM) and (self.LA(2)==DEFINEDAS or self.LA(2)==FULLSTOP):
                pass
                a=self.atom()
                c.add_child(a)
            elif (self.LA(1)==DEFINEDAS or self.LA(1)==FULLSTOP):
                pass
            else:
                raise antlr.NoViableAltException(self.LT(1), self.getFilename())
            
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [DEFINEDAS]:
                pass
                self.match(DEFINEDAS)
                e=self.expression()
                c.add_child(e)
            elif la1 and la1 in [FULLSTOP]:
                pass
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
            self.match(FULLSTOP)
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_2)
        
        return c
    
    def structure(self):    
        s = None
        
        f = None
        try:      ## for error handling
            pass
            s = Node(None, "structure")
            f = self.LT(1)
            self.match(ATOM)
            s.leaf = f.getText()
            self.match(LBRACKET)
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [ATOM,VARIABLE,NUMBER,LCURLY,LSQUARE]:
                pass
                t=self.termlist()
                s.children.extend(t)
            elif la1 and la1 in [RBRACKET]:
                pass
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
            self.match(RBRACKET)
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_3)
        
        return s
    
    def atom(self):    
        t = None
        
        a = None
        try:      ## for error handling
            pass
            a = self.LT(1)
            self.match(ATOM)
            t = Node(None, "atom", leaf = a.getText())
            if t.leaf.startswith("'"): t.leaf = t.leaf[1:-1]
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_3)
        
        return t
    
    def expression(self):    
        e = None
        
        try:      ## for error handling
            pass
            o=self.or_expr()
            e = o
            while True:
                if (self.LA(1)==SEMICOLON):
                    pass
                    self.match(SEMICOLON)
                    op = ";"
                    o2=self.or_expr()
                    e = expr_node(e, op, o2)
                else:
                    break
                
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_4)
        
        return e
    
    def or_expr(self):    
        e = None
        
        try:      ## for error handling
            pass
            o=self.not_expr()
            e = o
            while True:
                if (self.LA(1)==COMMA):
                    pass
                    self.match(COMMA)
                    o2=self.not_expr()
                    e = expr_node(e, ",", o2)
                else:
                    break
                
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_5)
        
        return e
    
    def not_expr(self):    
        e = None
        
        try:      ## for error handling
            pass
            hitnot = False
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [NOT]:
                pass
                self.match(NOT)
                hitnot = True
            elif la1 and la1 in [LBRACKET,ATOM,VARIABLE,NUMBER,LCURLY,LSQUARE]:
                pass
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
            e=self.relation_expr()
            if hitnot: e = Node(None, "expression", leaf = "not", children = [e])
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_6)
        
        return e
    
    def relation_expr(self):    
        e = None
        
        try:      ## for error handling
            pass
            e=self.basic_expr()
            while True:
                if ((self.LA(1) >= LESSTHANOREQ and self.LA(1) <= EQUAL)):
                    pass
                    o=self.relation_op()
                    e2=self.basic_expr()
                    e = Node(None, "expression", leaf = o, children=[e, e2])
                else:
                    break
                
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_6)
        
        return e
    
    def basic_expr(self):    
        e = None
        
        try:      ## for error handling
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [ATOM,VARIABLE,NUMBER,LCURLY,LSQUARE]:
                pass
                t=self.term()
                e = t
            elif la1 and la1 in [LBRACKET]:
                pass
                self.match(LBRACKET)
                o=self.expression()
                e = o
                self.match(RBRACKET)
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_7)
        
        return e
    
    def relation_op(self):    
        o = None
        
        try:      ## for error handling
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [LESSTHANOREQ]:
                pass
                self.match(LESSTHANOREQ)
                o = "lessthanorequals"
            elif la1 and la1 in [GREATERTHANOREQ]:
                pass
                self.match(GREATERTHANOREQ)
                o = "greaterthanorequals"
            elif la1 and la1 in [GREATERTHAN]:
                pass
                self.match(GREATERTHAN)
                o = "greaterthan"
            elif la1 and la1 in [LESSTHAN]:
                pass
                self.match(LESSTHAN)
                o = "lessthan"
            elif la1 and la1 in [EQUAL]:
                pass
                self.match(EQUAL)
                o = "unify"
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_8)
        
        return o
    
    def variable(self):    
        t = None
        
        v = None
        try:      ## for error handling
            pass
            v = self.LT(1)
            self.match(VARIABLE)
            t = Node(None, "variable", leaf = v.getText())
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_1)
        
        return t
    
    def prologlist(self):    
        l = None
        
        try:      ## for error handling
            if (self.LA(1)==LSQUARE) and (self.LA(2)==RSQUARE):
                pass
                self.match(LSQUARE)
                self.match(RSQUARE)
                l = Node(None, "structure", leaf = '.')
            elif (self.LA(1)==LSQUARE) and (_tokenSet_9.member(self.LA(2))):
                pass
                self.match(LSQUARE)
                c=self.list_contents()
                self.match(RSQUARE)
                l = c
            else:
                raise antlr.NoViableAltException(self.LT(1), self.getFilename())
            
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_1)
        
        return l
    
    def number(self):    
        n = None
        
        nt = None
        try:      ## for error handling
            pass
            nt = self.LT(1)
            self.match(NUMBER)
            n = Node(None, "number", leaf = nt.getText())
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_1)
        
        return n
    
    def dictionary(self):    
        d = None
        
        try:      ## for error handling
            pass
            self.match(LCURLY)
            self.match(RCURLY)
            d = Node(None, "dictionary"); d._internal_data = {}
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_1)
        
        return d
    
    def list_contents(self):    
        l = None
        
        def add_listterm(t, parent):
           child = Node(None, "structure", leaf = '.', children = [t])
           parent.add_child(child)
           return child
        try:      ## for error handling
            pass
            t=self.term()
            l = Node(None, "structure", leaf = '.', children = [t]); subnode = l
            while True:
                if (self.LA(1)==COMMA):
                    pass
                    self.match(COMMA)
                    t=self.term()
                    subnode = add_listterm(t, subnode)
                else:
                    break
                
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [BAR]:
                pass
                self.match(BAR)
                la1 = self.LA(1)
                if False:
                    pass
                elif la1 and la1 in [VARIABLE]:
                    pass
                    t2=self.variable()
                elif la1 and la1 in [LSQUARE]:
                    pass
                    t2=self.prologlist()
                else:
                        raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                    
                subnode.add_child(t2)
            elif la1 and la1 in [RSQUARE]:
                pass
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
            if len(subnode.children) == 1:
               subnode.add_child(Node(None, "structure", leaf = '.'))
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_10)
        
        return l
    
    def termlist(self):    
        tl = None
        
        try:      ## for error handling
            pass
            t=self.term()
            tl = [t]
            while True:
                if (self.LA(1)==COMMA):
                    pass
                    self.match(COMMA)
                    t=self.term()
                    tl.append(t)
                else:
                    break
                
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_11)
        
        return tl
    

_tokenNames = [
    "<0>", 
    "EOF", 
    "<2>", 
    "NULL_TREE_LOOKAHEAD", 
    "DEFINEDAS", 
    "FULLSTOP", 
    "SEMICOLON", 
    "COMMA", 
    "NOT", 
    "LESSTHANOREQ", 
    "GREATERTHANOREQ", 
    "GREATERTHAN", 
    "LESSTHAN", 
    "EQUAL", 
    "LBRACKET", 
    "RBRACKET", 
    "ATOM", 
    "VARIABLE", 
    "NUMBER", 
    "LCURLY", 
    "RCURLY", 
    "LSQUARE", 
    "RSQUARE", 
    "BAR", 
    "WS", 
    "COMMENT"
]
    

### generate bit set
def mk_tokenSet_0(): 
    ### var1
    data = [ 2L, 0L]
    return data
_tokenSet_0 = antlr.BitSet(mk_tokenSet_0())

### generate bit set
def mk_tokenSet_1(): 
    ### var1
    data = [ 12631778L, 0L]
    return data
_tokenSet_1 = antlr.BitSet(mk_tokenSet_1())

### generate bit set
def mk_tokenSet_2(): 
    ### var1
    data = [ 65586L, 0L]
    return data
_tokenSet_2 = antlr.BitSet(mk_tokenSet_2())

### generate bit set
def mk_tokenSet_3(): 
    ### var1
    data = [ 12631794L, 0L]
    return data
_tokenSet_3 = antlr.BitSet(mk_tokenSet_3())

### generate bit set
def mk_tokenSet_4(): 
    ### var1
    data = [ 32800L, 0L]
    return data
_tokenSet_4 = antlr.BitSet(mk_tokenSet_4())

### generate bit set
def mk_tokenSet_5(): 
    ### var1
    data = [ 32864L, 0L]
    return data
_tokenSet_5 = antlr.BitSet(mk_tokenSet_5())

### generate bit set
def mk_tokenSet_6(): 
    ### var1
    data = [ 32992L, 0L]
    return data
_tokenSet_6 = antlr.BitSet(mk_tokenSet_6())

### generate bit set
def mk_tokenSet_7(): 
    ### var1
    data = [ 48864L, 0L]
    return data
_tokenSet_7 = antlr.BitSet(mk_tokenSet_7())

### generate bit set
def mk_tokenSet_8(): 
    ### var1
    data = [ 3096576L, 0L]
    return data
_tokenSet_8 = antlr.BitSet(mk_tokenSet_8())

### generate bit set
def mk_tokenSet_9(): 
    ### var1
    data = [ 3080192L, 0L]
    return data
_tokenSet_9 = antlr.BitSet(mk_tokenSet_9())

### generate bit set
def mk_tokenSet_10(): 
    ### var1
    data = [ 4194304L, 0L]
    return data
_tokenSet_10 = antlr.BitSet(mk_tokenSet_10())

### generate bit set
def mk_tokenSet_11(): 
    ### var1
    data = [ 32768L, 0L]
    return data
_tokenSet_11 = antlr.BitSet(mk_tokenSet_11())
    
