### $ANTLR 2.7.6 (20060903): "prolog.g" -> "PrologLexer.py"$
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
### preamble action >>> 

### preamble action <<< 
### >>>The Literals<<<
literals = {}


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

class Lexer(antlr.CharScanner) :
    ### user action >>>
    ### user action <<<
    def __init__(self, *argv, **kwargs) :
        antlr.CharScanner.__init__(self, *argv, **kwargs)
        self.caseSensitiveLiterals = True
        self.setCaseSensitive(True)
        self.literals = literals
    
    def nextToken(self):
        while True:
            try: ### try again ..
                while True:
                    _token = None
                    _ttype = INVALID_TYPE
                    self.resetText()
                    try: ## for char stream error handling
                        try: ##for lexical error handling
                            la1 = self.LA(1)
                            if False:
                                pass
                            elif la1 and la1 in u'\t\n ':
                                pass
                                self.mWS(True)
                                theRetToken = self._returnToken
                            elif la1 and la1 in u'%':
                                pass
                                self.mCOMMENT(True)
                                theRetToken = self._returnToken
                            elif la1 and la1 in u';':
                                pass
                                self.mSEMICOLON(True)
                                theRetToken = self._returnToken
                            elif la1 and la1 in u'|':
                                pass
                                self.mBAR(True)
                                theRetToken = self._returnToken
                            elif la1 and la1 in u'[':
                                pass
                                self.mLSQUARE(True)
                                theRetToken = self._returnToken
                            elif la1 and la1 in u']':
                                pass
                                self.mRSQUARE(True)
                                theRetToken = self._returnToken
                            elif la1 and la1 in u'(':
                                pass
                                self.mLBRACKET(True)
                                theRetToken = self._returnToken
                            elif la1 and la1 in u')':
                                pass
                                self.mRBRACKET(True)
                                theRetToken = self._returnToken
                            elif la1 and la1 in u'{':
                                pass
                                self.mLCURLY(True)
                                theRetToken = self._returnToken
                            elif la1 and la1 in u'}':
                                pass
                                self.mRCURLY(True)
                                theRetToken = self._returnToken
                            elif la1 and la1 in u',':
                                pass
                                self.mCOMMA(True)
                                theRetToken = self._returnToken
                            elif la1 and la1 in u'.':
                                pass
                                self.mFULLSTOP(True)
                                theRetToken = self._returnToken
                            elif la1 and la1 in u':':
                                pass
                                self.mDEFINEDAS(True)
                                theRetToken = self._returnToken
                            elif la1 and la1 in u'<':
                                pass
                                self.mLESSTHAN(True)
                                theRetToken = self._returnToken
                            elif la1 and la1 in u'>':
                                pass
                                self.mGREATERTHAN(True)
                                theRetToken = self._returnToken
                            elif la1 and la1 in u'\\':
                                pass
                                self.mNOT(True)
                                theRetToken = self._returnToken
                            elif la1 and la1 in u'0123456789':
                                pass
                                self.mNUMBER(True)
                                theRetToken = self._returnToken
                            elif la1 and la1 in u'ABCDEFGHIJKLMNOPQRSTUVWXYZ_':
                                pass
                                self.mVARIABLE(True)
                                theRetToken = self._returnToken
                            elif la1 and la1 in u'\'abcdefghijklmnopqrstuvwxyz':
                                pass
                                self.mATOM(True)
                                theRetToken = self._returnToken
                            else:
                                if (self.LA(1)==u'=') and (self.LA(2)==u'<'):
                                    pass
                                    self.mLESSTHANOREQ(True)
                                    theRetToken = self._returnToken
                                elif (self.LA(1)==u'=') and (self.LA(2)==u'>'):
                                    pass
                                    self.mGREATERTHANOREQ(True)
                                    theRetToken = self._returnToken
                                elif (self.LA(1)==u'=') and (True):
                                    pass
                                    self.mEQUAL(True)
                                    theRetToken = self._returnToken
                                else:
                                    self.default(self.LA(1))
                                
                            if not self._returnToken:
                                raise antlr.TryAgain ### found SKIP token
                            ### option { testLiterals=true } 
                            self.testForLiteral(self._returnToken)
                            ### return token to caller
                            return self._returnToken
                        ### handle lexical errors ....
                        except antlr.RecognitionException, e:
                            raise antlr.TokenStreamRecognitionException(e)
                    ### handle char stream errors ...
                    except antlr.CharStreamException,cse:
                        if isinstance(cse, antlr.CharStreamIOException):
                            raise antlr.TokenStreamIOException(cse.io)
                        else:
                            raise antlr.TokenStreamException(str(cse))
            except antlr.TryAgain:
                pass
        
    def mWS(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = WS
        _saveIndex = 0
        pass
        la1 = self.LA(1)
        if False:
            pass
        elif la1 and la1 in u' ':
            pass
            self.match(' ')
        elif la1 and la1 in u'\t':
            pass
            self.match('\t')
        elif la1 and la1 in u'\n':
            pass
            self.match('\n')
            self.newline();
        else:
                self.raise_NoViableAlt(self.LA(1))
            
        _ttype = SKIP
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mCOMMENT(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = COMMENT
        _saveIndex = 0
        pass
        self.match('%')
        while True:
            if (_tokenSet_0.member(self.LA(1))):
                pass
                self.match(_tokenSet_0)
            else:
                break
            
        _ttype = SKIP
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mSEMICOLON(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = SEMICOLON
        _saveIndex = 0
        pass
        self.match(';')
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mBAR(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = BAR
        _saveIndex = 0
        pass
        self.match('|')
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mLSQUARE(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = LSQUARE
        _saveIndex = 0
        pass
        self.match('[')
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mRSQUARE(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = RSQUARE
        _saveIndex = 0
        pass
        self.match(']')
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mLBRACKET(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = LBRACKET
        _saveIndex = 0
        pass
        self.match('(')
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mRBRACKET(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = RBRACKET
        _saveIndex = 0
        pass
        self.match(')')
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mLCURLY(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = LCURLY
        _saveIndex = 0
        pass
        self.match('{')
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mRCURLY(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = RCURLY
        _saveIndex = 0
        pass
        self.match('}')
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mCOMMA(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = COMMA
        _saveIndex = 0
        pass
        self.match(',')
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mFULLSTOP(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = FULLSTOP
        _saveIndex = 0
        pass
        self.match('.')
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mDEFINEDAS(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = DEFINEDAS
        _saveIndex = 0
        pass
        self.match(":-")
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mLESSTHANOREQ(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = LESSTHANOREQ
        _saveIndex = 0
        pass
        self.match("=<")
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mGREATERTHANOREQ(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = GREATERTHANOREQ
        _saveIndex = 0
        pass
        self.match("=>")
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mLESSTHAN(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = LESSTHAN
        _saveIndex = 0
        pass
        self.match("<")
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mGREATERTHAN(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = GREATERTHAN
        _saveIndex = 0
        pass
        self.match(">")
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mEQUAL(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = EQUAL
        _saveIndex = 0
        pass
        self.match("=")
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mNOT(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = NOT
        _saveIndex = 0
        pass
        self.match("\\+")
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mNUMBER(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = NUMBER
        _saveIndex = 0
        pass
        _cnt62= 0
        while True:
            if ((self.LA(1) >= u'0' and self.LA(1) <= u'9')):
                pass
                self.matchRange(u'0', u'9')
            else:
                break
            
            _cnt62 += 1
        if _cnt62 < 1:
            self.raise_NoViableAlt(self.LA(1))
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mVARIABLE(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = VARIABLE
        _saveIndex = 0
        la1 = self.LA(1)
        if False:
            pass
        elif la1 and la1 in u'_':
            pass
            pass
            self.match('_')
        elif la1 and la1 in u'ABCDEFGHIJKLMNOPQRSTUVWXYZ':
            pass
            pass
            pass
            self.matchRange(u'A', u'Z')
            while True:
                la1 = self.LA(1)
                if False:
                    pass
                elif la1 and la1 in u'abcdefghijklmnopqrstuvwxyz':
                    pass
                    self.matchRange(u'a', u'z')
                elif la1 and la1 in u'ABCDEFGHIJKLMNOPQRSTUVWXYZ':
                    pass
                    self.matchRange(u'A', u'Z')
                elif la1 and la1 in u'0123456789':
                    pass
                    self.matchRange(u'0', u'9')
                elif la1 and la1 in u'_':
                    pass
                    self.match('_')
                else:
                        break
                    
        else:
                self.raise_NoViableAlt(self.LA(1))
            
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mATOM(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = ATOM
        _saveIndex = 0
        la1 = self.LA(1)
        if False:
            pass
        elif la1 and la1 in u'abcdefghijklmnopqrstuvwxyz':
            pass
            pass
            pass
            self.matchRange(u'a', u'z')
            while True:
                la1 = self.LA(1)
                if False:
                    pass
                elif la1 and la1 in u'abcdefghijklmnopqrstuvwxyz':
                    pass
                    self.matchRange(u'a', u'z')
                elif la1 and la1 in u'ABCDEFGHIJKLMNOPQRSTUVWXYZ':
                    pass
                    self.matchRange(u'A', u'Z')
                elif la1 and la1 in u'0123456789':
                    pass
                    self.matchRange(u'0', u'9')
                elif la1 and la1 in u'_':
                    pass
                    self.match('_')
                else:
                        break
                    
        elif la1 and la1 in u'\'':
            pass
            pass
            self.match("'")
            _cnt76= 0
            while True:
                if (_tokenSet_1.member(self.LA(1))):
                    pass
                    self.matchNot('\'')
                else:
                    break
                
                _cnt76 += 1
            if _cnt76 < 1:
                self.raise_NoViableAlt(self.LA(1))
            self.match("'")
        else:
                self.raise_NoViableAlt(self.LA(1))
            
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    

### generate bit set
def mk_tokenSet_0(): 
    ### var1
    data = [ -1025L, -1L, 0L, 0L]
    return data
_tokenSet_0 = antlr.BitSet(mk_tokenSet_0())

### generate bit set
def mk_tokenSet_1(): 
    ### var1
    data = [ -549755813889L, -1L, 0L, 0L]
    return data
_tokenSet_1 = antlr.BitSet(mk_tokenSet_1())
    
### __main__ header action >>> 
if __name__ == '__main__' :
    import sys
    import antlr
    import PrologLexer
    
    ### create lexer - shall read from stdin
    try:
        for token in PrologLexer.Lexer():
            print token
            
    except antlr.TokenStreamException, e:
        print "error: exception caught while lexing: ", e
### __main__ header action <<< 
