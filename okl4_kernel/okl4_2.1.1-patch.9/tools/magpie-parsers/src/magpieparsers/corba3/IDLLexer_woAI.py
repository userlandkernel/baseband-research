### $ANTLR 2.7.5 (20050201): "idl.g" -> "IDLLexer.py"$
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
### preamble action >>> 

### preamble action <<< 
### >>>The Literals<<<
literals = {}
literals[u"switch"] = 58
literals[u"case"] = 59
literals[u"manages"] = 91
literals[u"finder"] = 93
literals[u"interface"] = 7
literals[u"typeprefix"] = 78
literals[u"provides"] = 84
literals[u"exception"] = 67
literals[u"public"] = 24
literals[u"wstring"] = 66
literals[u"default"] = 60
literals[u"sequence"] = 62
literals[u"wchar"] = 51
literals[u"uses"] = 85
literals[u"typeid"] = 77
literals[u"consumes"] = 89
literals[u"void"] = 69
literals[u"inout"] = 71
literals[u"import"] = 76
literals[u"publishes"] = 88
literals[u"boolean"] = 52
literals[u"private"] = 25
literals[u"string"] = 65
literals[u"abstract"] = 5
literals[u"const"] = 28
literals[u"custom"] = 8
literals[u"local"] = 6
literals[u"union"] = 57
literals[u"attribute"] = 80
literals[u"oneway"] = 68
literals[u"Object"] = 55
literals[u"fixed"] = 74
literals[u"short"] = 48
literals[u"typedef"] = 43
literals[u"long"] = 47
literals[u"FALSE"] = 42
literals[u"truncatable"] = 22
literals[u"ValueBase"] = 75
literals[u"in"] = 27
literals[u"readonly"] = 79
literals[u"supports"] = 23
literals[u"TRUE"] = 41
literals[u"enum"] = 61
literals[u"home"] = 90
literals[u"multiple"] = 86
literals[u"octet"] = 53
literals[u"context"] = 73
literals[u"char"] = 50
literals[u"component"] = 83
literals[u"emits"] = 87
literals[u"getraises"] = 81
literals[u"setraises"] = 82
literals[u"float"] = 45
literals[u"factory"] = 26
literals[u"valuetype"] = 9
literals[u"raises"] = 72
literals[u"out"] = 70
literals[u"any"] = 54
literals[u"double"] = 46
literals[u"primarykey"] = 92
literals[u"eventtype"] = 10
literals[u"unsigned"] = 49
literals[u"struct"] = 56
literals[u"module"] = 16
literals[u"native"] = 44


### import antlr.Token 
from antlr import Token
### >>>The Known Token Types <<<
SKIP                = antlr.SKIP
INVALID_TYPE        = antlr.INVALID_TYPE
EOF_TYPE            = antlr.EOF_TYPE
EOF                 = antlr.EOF
NULL_TREE_LOOKAHEAD = antlr.NULL_TREE_LOOKAHEAD
MIN_USER_TYPE       = antlr.MIN_USER_TYPE
SEMI = 4
LITERAL_abstract = 5
LITERAL_local = 6
LITERAL_interface = 7
LITERAL_custom = 8
LITERAL_valuetype = 9
LITERAL_eventtype = 10
LBRACK = 11
COMMA = 12
RBRACK = 13
LPAREN = 14
RPAREN = 15
LITERAL_module = 16
LCURLY = 17
RCURLY = 18
COLON = 19
SCOPEOP = 20
IDENT = 21
LITERAL_truncatable = 22
LITERAL_supports = 23
LITERAL_public = 24
LITERAL_private = 25
LITERAL_factory = 26
LITERAL_in = 27
LITERAL_const = 28
ASSIGN = 29
OR = 30
XOR = 31
AND = 32
LSHIFT = 33
RSHIFT = 34
PLUS = 35
MINUS = 36
STAR = 37
DIV = 38
MOD = 39
TILDE = 40
LITERAL_TRUE = 41
LITERAL_FALSE = 42
LITERAL_typedef = 43
LITERAL_native = 44
LITERAL_float = 45
LITERAL_double = 46
LITERAL_long = 47
LITERAL_short = 48
LITERAL_unsigned = 49
LITERAL_char = 50
LITERAL_wchar = 51
LITERAL_boolean = 52
LITERAL_octet = 53
LITERAL_any = 54
LITERAL_Object = 55
LITERAL_struct = 56
LITERAL_union = 57
LITERAL_switch = 58
LITERAL_case = 59
LITERAL_default = 60
LITERAL_enum = 61
LITERAL_sequence = 62
LT = 63
GT = 64
LITERAL_string = 65
LITERAL_wstring = 66
LITERAL_exception = 67
LITERAL_oneway = 68
LITERAL_void = 69
LITERAL_out = 70
LITERAL_inout = 71
LITERAL_raises = 72
LITERAL_context = 73
LITERAL_fixed = 74
LITERAL_ValueBase = 75
LITERAL_import = 76
LITERAL_typeid = 77
LITERAL_typeprefix = 78
LITERAL_readonly = 79
LITERAL_attribute = 80
LITERAL_getraises = 81
LITERAL_setraises = 82
LITERAL_component = 83
LITERAL_provides = 84
LITERAL_uses = 85
LITERAL_multiple = 86
LITERAL_emits = 87
LITERAL_publishes = 88
LITERAL_consumes = 89
LITERAL_home = 90
LITERAL_manages = 91
LITERAL_primarykey = 92
LITERAL_finder = 93
INT = 94
OCTAL = 95
HEX = 96
STRING_LITERAL = 97
WIDE_STRING_LITERAL = 98
CHAR_LITERAL = 99
WIDE_CHAR_LITERAL = 100
FIXED = 101
FLOAT = 102
QUESTION = 103
DOT = 104
NOT = 105
UNDERLINE = 106
WS = 107
PREPROC_DIRECTIVE = 108
SL_COMMENT = 109
ML_COMMENT = 110
ESC = 111
VOCAB = 112
DIGIT = 113
NONZERODIGIT = 114
OCTDIGIT = 115
HEXDIGIT = 116
ESCAPED_IDENT = 117

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
                            elif la1 and la1 in u';':
                                pass
                                self.mSEMI(True)
                                theRetToken = self._returnToken
                            elif la1 and la1 in u'?':
                                pass
                                self.mQUESTION(True)
                                theRetToken = self._returnToken
                            elif la1 and la1 in u'(':
                                pass
                                self.mLPAREN(True)
                                theRetToken = self._returnToken
                            elif la1 and la1 in u')':
                                pass
                                self.mRPAREN(True)
                                theRetToken = self._returnToken
                            elif la1 and la1 in u'[':
                                pass
                                self.mLBRACK(True)
                                theRetToken = self._returnToken
                            elif la1 and la1 in u']':
                                pass
                                self.mRBRACK(True)
                                theRetToken = self._returnToken
                            elif la1 and la1 in u'{':
                                pass
                                self.mLCURLY(True)
                                theRetToken = self._returnToken
                            elif la1 and la1 in u'}':
                                pass
                                self.mRCURLY(True)
                                theRetToken = self._returnToken
                            elif la1 and la1 in u'|':
                                pass
                                self.mOR(True)
                                theRetToken = self._returnToken
                            elif la1 and la1 in u'^':
                                pass
                                self.mXOR(True)
                                theRetToken = self._returnToken
                            elif la1 and la1 in u'&':
                                pass
                                self.mAND(True)
                                theRetToken = self._returnToken
                            elif la1 and la1 in u',':
                                pass
                                self.mCOMMA(True)
                                theRetToken = self._returnToken
                            elif la1 and la1 in u'=':
                                pass
                                self.mASSIGN(True)
                                theRetToken = self._returnToken
                            elif la1 and la1 in u'!':
                                pass
                                self.mNOT(True)
                                theRetToken = self._returnToken
                            elif la1 and la1 in u'+':
                                pass
                                self.mPLUS(True)
                                theRetToken = self._returnToken
                            elif la1 and la1 in u'-':
                                pass
                                self.mMINUS(True)
                                theRetToken = self._returnToken
                            elif la1 and la1 in u'~':
                                pass
                                self.mTILDE(True)
                                theRetToken = self._returnToken
                            elif la1 and la1 in u'*':
                                pass
                                self.mSTAR(True)
                                theRetToken = self._returnToken
                            elif la1 and la1 in u'%':
                                pass
                                self.mMOD(True)
                                theRetToken = self._returnToken
                            elif la1 and la1 in u'\t\n\r ':
                                pass
                                self.mWS(True)
                                theRetToken = self._returnToken
                            elif la1 and la1 in u'#':
                                pass
                                self.mPREPROC_DIRECTIVE(True)
                                theRetToken = self._returnToken
                            elif la1 and la1 in u'\'':
                                pass
                                self.mCHAR_LITERAL(True)
                                theRetToken = self._returnToken
                            elif la1 and la1 in u'"':
                                pass
                                self.mSTRING_LITERAL(True)
                                theRetToken = self._returnToken
                            elif la1 and la1 in u'123456789':
                                pass
                                self.mINT(True)
                                theRetToken = self._returnToken
                            else:
                                if (self.LA(1)==u'<') and (self.LA(2)==u'<'):
                                    pass
                                    self.mLSHIFT(True)
                                    theRetToken = self._returnToken
                                elif (self.LA(1)==u'>') and (self.LA(2)==u'>'):
                                    pass
                                    self.mRSHIFT(True)
                                    theRetToken = self._returnToken
                                elif (self.LA(1)==u':') and (self.LA(2)==u':'):
                                    pass
                                    self.mSCOPEOP(True)
                                    theRetToken = self._returnToken
                                elif (self.LA(1)==u'/') and (self.LA(2)==u'/'):
                                    pass
                                    self.mSL_COMMENT(True)
                                    theRetToken = self._returnToken
                                elif (self.LA(1)==u'/') and (self.LA(2)==u'*'):
                                    pass
                                    self.mML_COMMENT(True)
                                    theRetToken = self._returnToken
                                elif (self.LA(1)==u'L') and (self.LA(2)==u'\''):
                                    pass
                                    self.mWIDE_CHAR_LITERAL(True)
                                    theRetToken = self._returnToken
                                elif (self.LA(1)==u'L') and (self.LA(2)==u'"'):
                                    pass
                                    self.mWIDE_STRING_LITERAL(True)
                                    theRetToken = self._returnToken
                                elif (self.LA(1)==u'0') and (self.LA(2)==u'X' or self.LA(2)==u'x'):
                                    pass
                                    self.mHEX(True)
                                    theRetToken = self._returnToken
                                elif (self.LA(1)==u'.') and ((self.LA(2) >= u'0' and self.LA(2) <= u'9')):
                                    pass
                                    self.mFLOAT(True)
                                    theRetToken = self._returnToken
                                elif (self.LA(1)==u'_') and (_tokenSet_0.member(self.LA(2))):
                                    pass
                                    self.mESCAPED_IDENT(True)
                                    theRetToken = self._returnToken
                                elif (self.LA(1)==u':') and (True):
                                    pass
                                    self.mCOLON(True)
                                    theRetToken = self._returnToken
                                elif (self.LA(1)==u'.') and (True):
                                    pass
                                    self.mDOT(True)
                                    theRetToken = self._returnToken
                                elif (self.LA(1)==u'<') and (True):
                                    pass
                                    self.mLT(True)
                                    theRetToken = self._returnToken
                                elif (self.LA(1)==u'>') and (True):
                                    pass
                                    self.mGT(True)
                                    theRetToken = self._returnToken
                                elif (self.LA(1)==u'/') and (True):
                                    pass
                                    self.mDIV(True)
                                    theRetToken = self._returnToken
                                elif (self.LA(1)==u'_') and (True):
                                    pass
                                    self.mUNDERLINE(True)
                                    theRetToken = self._returnToken
                                elif (self.LA(1)==u'0') and (True):
                                    pass
                                    self.mOCTAL(True)
                                    theRetToken = self._returnToken
                                elif (_tokenSet_0.member(self.LA(1))) and (True):
                                    pass
                                    self.mIDENT(True)
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
        
    def mSEMI(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = SEMI
        _saveIndex = 0
        pass
        self.match(';')
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mQUESTION(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = QUESTION
        _saveIndex = 0
        pass
        self.match('?')
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mLPAREN(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = LPAREN
        _saveIndex = 0
        pass
        self.match('(')
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mRPAREN(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = RPAREN
        _saveIndex = 0
        pass
        self.match(')')
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mLBRACK(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = LBRACK
        _saveIndex = 0
        pass
        self.match('[')
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mRBRACK(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = RBRACK
        _saveIndex = 0
        pass
        self.match(']')
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
    
    def mOR(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = OR
        _saveIndex = 0
        pass
        self.match('|')
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mXOR(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = XOR
        _saveIndex = 0
        pass
        self.match('^')
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mAND(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = AND
        _saveIndex = 0
        pass
        self.match('&')
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mCOLON(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = COLON
        _saveIndex = 0
        pass
        self.match(':')
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
    
    def mDOT(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = DOT
        _saveIndex = 0
        pass
        self.match('.')
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mASSIGN(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = ASSIGN
        _saveIndex = 0
        pass
        self.match('=')
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mNOT(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = NOT
        _saveIndex = 0
        pass
        self.match('!')
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mLT(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = LT
        _saveIndex = 0
        pass
        self.match('<')
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mLSHIFT(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = LSHIFT
        _saveIndex = 0
        pass
        self.match("<<")
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mGT(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = GT
        _saveIndex = 0
        pass
        self.match('>')
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mRSHIFT(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = RSHIFT
        _saveIndex = 0
        pass
        self.match(">>")
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mDIV(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = DIV
        _saveIndex = 0
        pass
        self.match('/')
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mPLUS(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = PLUS
        _saveIndex = 0
        pass
        self.match('+')
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mMINUS(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = MINUS
        _saveIndex = 0
        pass
        self.match('-')
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mTILDE(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = TILDE
        _saveIndex = 0
        pass
        self.match('~')
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mSTAR(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = STAR
        _saveIndex = 0
        pass
        self.match('*')
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mMOD(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = MOD
        _saveIndex = 0
        pass
        self.match('%')
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mSCOPEOP(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = SCOPEOP
        _saveIndex = 0
        pass
        self.match("::")
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mUNDERLINE(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = UNDERLINE
        _saveIndex = 0
        pass
        self.match("_")
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
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
        elif la1 and la1 in u'\r':
            pass
            self.match('\r')
        else:
                self.raise_NoViableAlt(self.LA(1))
            
        _ttype = Token.SKIP;
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mPREPROC_DIRECTIVE(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = PREPROC_DIRECTIVE
        _saveIndex = 0
        pass
        _saveIndex = self.text.length()
        self.match('#')
        self.text.setLength(_saveIndex)
        while True:
            if (_tokenSet_1.member(self.LA(1))):
                pass
                self.matchNot('\n')
            else:
                break
            
        _saveIndex = self.text.length()
        self.match('\n')
        self.text.setLength(_saveIndex)
        _ttype = Token.SKIP; self.newline();
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mSL_COMMENT(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = SL_COMMENT
        _saveIndex = 0
        pass
        _saveIndex = self.text.length()
        self.match("//")
        self.text.setLength(_saveIndex)
        while True:
            if (_tokenSet_1.member(self.LA(1))):
                pass
                self.matchNot('\n')
            else:
                break
            
        self.match('\n')
        _ttype = Token.SKIP; self.newline();
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mML_COMMENT(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = ML_COMMENT
        _saveIndex = 0
        pass
        _saveIndex = self.text.length()
        self.match("/*")
        self.text.setLength(_saveIndex)
        while True:
            if (self.LA(1)==u'*') and (_tokenSet_2.member(self.LA(2))):
                pass
                _cnt353= 0
                while True:
                    if (self.LA(1)==u'*'):
                        pass
                        self.match('*')
                    else:
                        break
                    
                    _cnt353 += 1
                if _cnt353 < 1:
                    self.raise_NoViableAlt(self.LA(1))
                if (self.LA(1)==u'\n'):
                    pass
                    self.match('\n')
                    self.newline();
                elif (_tokenSet_3.member(self.LA(1))):
                    pass
                    self.match(_tokenSet_3)
                else:
                    self.raise_NoViableAlt(self.LA(1))
                
            elif (self.LA(1)==u'\n'):
                pass
                self.match('\n')
                self.newline();
            elif (_tokenSet_4.member(self.LA(1))):
                pass
                self.match(_tokenSet_4)
            else:
                break
            
        _saveIndex = self.text.length()
        self.match("*/")
        self.text.setLength(_saveIndex)
        _ttype = Token.SKIP;
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mCHAR_LITERAL(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = CHAR_LITERAL
        _saveIndex = 0
        pass
        _saveIndex = self.text.length()
        self.match('\'')
        self.text.setLength(_saveIndex)
        if (self.LA(1)==u'\\'):
            pass
            self.mESC(False)
        elif (_tokenSet_5.member(self.LA(1))):
            pass
            self.matchNot('\'')
        else:
            self.raise_NoViableAlt(self.LA(1))
        
        _saveIndex = self.text.length()
        self.match('\'')
        self.text.setLength(_saveIndex)
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mESC(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = ESC
        _saveIndex = 0
        pass
        _saveIndex = self.text.length()
        self.match('\\')
        self.text.setLength(_saveIndex)
        la1 = self.LA(1)
        if False:
            pass
        elif la1 and la1 in u'n':
            pass
            self.match('n')
            self.text.setLength(_begin) ; self.text.append("\n");
        elif la1 and la1 in u't':
            pass
            self.match('t')
            self.text.setLength(_begin) ; self.text.append("\t");
        elif la1 and la1 in u'v':
            pass
            self.match('v')
            self.text.setLength(_begin) ; self.text.append("\013");
        elif la1 and la1 in u'b':
            pass
            self.match('b')
            self.text.setLength(_begin) ; self.text.append("\b");
        elif la1 and la1 in u'r':
            pass
            self.match('r')
            self.text.setLength(_begin) ; self.text.append("\r");
        elif la1 and la1 in u'f':
            pass
            self.match('f')
            self.text.setLength(_begin) ; self.text.append("\r");
        elif la1 and la1 in u'a':
            pass
            self.match('a')
            self.text.setLength(_begin) ; self.text.append("\007");
        elif la1 and la1 in u'\\':
            pass
            self.match('\\')
            self.text.setLength(_begin) ; self.text.append("\\");
        elif la1 and la1 in u'?':
            pass
            self.match('?')
            self.text.setLength(_begin) ; self.text.append("?");
        elif la1 and la1 in u'\'':
            pass
            self.match('\'')
            self.text.setLength(_begin) ; self.text.append("'");
        elif la1 and la1 in u'"':
            pass
            self.match('"')
            self.text.setLength(_begin) ; self.text.append("\"");
        elif la1 and la1 in u'01234567':
            pass
            self.mOCTDIGIT(False)
            if ((self.LA(1) >= u'0' and self.LA(1) <= u'7')) and ((self.LA(2) >= u'\u0000' and self.LA(2) <= u'\ufffe')) and (True) and (True):
                pass
                self.mOCTDIGIT(False)
                if ((self.LA(1) >= u'0' and self.LA(1) <= u'7')) and ((self.LA(2) >= u'\u0000' and self.LA(2) <= u'\ufffe')) and (True) and (True):
                    pass
                    self.mOCTDIGIT(False)
                elif ((self.LA(1) >= u'\u0000' and self.LA(1) <= u'\ufffe')) and (True) and (True) and (True):
                    pass
                else:
                    self.raise_NoViableAlt(self.LA(1))
                
            elif ((self.LA(1) >= u'\u0000' and self.LA(1) <= u'\ufffe')) and (True) and (True) and (True):
                pass
            else:
                self.raise_NoViableAlt(self.LA(1))
            
            realc = str(int(self.text.getString(_begin),8)); self.text.setLength(_begin) ; self.text.append(realc);
        elif la1 and la1 in u'x':
            pass
            _saveIndex = self.text.length()
            self.match('x')
            self.text.setLength(_saveIndex)
            self.mHEXDIGIT(False)
            if (_tokenSet_6.member(self.LA(1))) and ((self.LA(2) >= u'\u0000' and self.LA(2) <= u'\ufffe')) and (True) and (True):
                pass
                self.mHEXDIGIT(False)
            elif ((self.LA(1) >= u'\u0000' and self.LA(1) <= u'\ufffe')) and (True) and (True) and (True):
                pass
            else:
                self.raise_NoViableAlt(self.LA(1))
            
            realc = str(int(self.text.getString(_begin),16)); self.text.setLength(_begin) ; self.text.append(realc);
        elif la1 and la1 in u'u':
            pass
            _saveIndex = self.text.length()
            self.match('u')
            self.text.setLength(_saveIndex)
            self.mHEXDIGIT(False)
            if (_tokenSet_6.member(self.LA(1))) and ((self.LA(2) >= u'\u0000' and self.LA(2) <= u'\ufffe')) and (True) and (True):
                pass
                self.mHEXDIGIT(False)
                if (_tokenSet_6.member(self.LA(1))) and ((self.LA(2) >= u'\u0000' and self.LA(2) <= u'\ufffe')) and (True) and (True):
                    pass
                    self.mHEXDIGIT(False)
                    if (_tokenSet_6.member(self.LA(1))) and ((self.LA(2) >= u'\u0000' and self.LA(2) <= u'\ufffe')) and (True) and (True):
                        pass
                        self.mHEXDIGIT(False)
                    elif ((self.LA(1) >= u'\u0000' and self.LA(1) <= u'\ufffe')) and (True) and (True) and (True):
                        pass
                    else:
                        self.raise_NoViableAlt(self.LA(1))
                    
                elif ((self.LA(1) >= u'\u0000' and self.LA(1) <= u'\ufffe')) and (True) and (True) and (True):
                    pass
                else:
                    self.raise_NoViableAlt(self.LA(1))
                
            elif ((self.LA(1) >= u'\u0000' and self.LA(1) <= u'\ufffe')) and (True) and (True) and (True):
                pass
            else:
                self.raise_NoViableAlt(self.LA(1))
            
            realc = str(int(self.text.getString(_begin),16)); self.text.setLength(_begin) ; self.text.append(realc);
        else:
                self.raise_NoViableAlt(self.LA(1))
            
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mWIDE_CHAR_LITERAL(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = WIDE_CHAR_LITERAL
        _saveIndex = 0
        pass
        _saveIndex = self.text.length()
        self.match('L')
        self.text.setLength(_saveIndex)
        self.mCHAR_LITERAL(False)
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mSTRING_LITERAL(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = STRING_LITERAL
        _saveIndex = 0
        pass
        _saveIndex = self.text.length()
        self.match('"')
        self.text.setLength(_saveIndex)
        while True:
            if (self.LA(1)==u'\\'):
                pass
                self.mESC(False)
            elif (_tokenSet_7.member(self.LA(1))):
                pass
                self.matchNot('"')
            else:
                break
            
        _saveIndex = self.text.length()
        self.match('"')
        self.text.setLength(_saveIndex)
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mWIDE_STRING_LITERAL(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = WIDE_STRING_LITERAL
        _saveIndex = 0
        pass
        _saveIndex = self.text.length()
        self.match('L')
        self.text.setLength(_saveIndex)
        self.mSTRING_LITERAL(False)
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mOCTDIGIT(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = OCTDIGIT
        _saveIndex = 0
        pass
        self.matchRange(u'0', u'7')
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mHEXDIGIT(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = HEXDIGIT
        _saveIndex = 0
        pass
        la1 = self.LA(1)
        if False:
            pass
        elif la1 and la1 in u'0123456789':
            pass
            self.matchRange(u'0', u'9')
        elif la1 and la1 in u'abcdef':
            pass
            self.matchRange(u'a', u'f')
        elif la1 and la1 in u'ABCDEF':
            pass
            self.matchRange(u'A', u'F')
        else:
                self.raise_NoViableAlt(self.LA(1))
            
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mVOCAB(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = VOCAB
        _saveIndex = 0
        pass
        self.matchRange(u'\3', u'\377')
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mDIGIT(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = DIGIT
        _saveIndex = 0
        pass
        self.matchRange(u'0', u'9')
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mNONZERODIGIT(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = NONZERODIGIT
        _saveIndex = 0
        pass
        self.matchRange(u'1', u'9')
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mHEX(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = HEX
        _saveIndex = 0
        pass
        if (self.LA(1)==u'0') and (self.LA(2)==u'x'):
            pass
            self.match("0x")
        elif (self.LA(1)==u'0') and (self.LA(2)==u'X'):
            pass
            self.match("0X")
        else:
            self.raise_NoViableAlt(self.LA(1))
        
        _cnt382= 0
        while True:
            if (_tokenSet_6.member(self.LA(1))):
                pass
                self.mHEXDIGIT(False)
            else:
                break
            
            _cnt382 += 1
        if _cnt382 < 1:
            self.raise_NoViableAlt(self.LA(1))
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mINT(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = INT
        _saveIndex = 0
        pass
        self.mNONZERODIGIT(False)
        while True:
            if ((self.LA(1) >= u'0' and self.LA(1) <= u'9')):
                pass
                self.mDIGIT(False)
            else:
                break
            
        la1 = self.LA(1)
        if False:
            pass
        elif la1 and la1 in u'.':
            pass
            self.match('.')
            while True:
                if ((self.LA(1) >= u'0' and self.LA(1) <= u'9')):
                    pass
                    self.mDIGIT(False)
                else:
                    break
                
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in u'Ee':
                pass
                pass
                la1 = self.LA(1)
                if False:
                    pass
                elif la1 and la1 in u'e':
                    pass
                    self.match('e')
                elif la1 and la1 in u'E':
                    pass
                    self.match('E')
                else:
                        self.raise_NoViableAlt(self.LA(1))
                    
                la1 = self.LA(1)
                if False:
                    pass
                elif la1 and la1 in u'+':
                    pass
                    self.match('+')
                elif la1 and la1 in u'-':
                    pass
                    self.match('-')
                elif la1 and la1 in u'0123456789':
                    pass
                else:
                        self.raise_NoViableAlt(self.LA(1))
                    
                _cnt394= 0
                while True:
                    if ((self.LA(1) >= u'0' and self.LA(1) <= u'9')):
                        pass
                        self.mDIGIT(False)
                    else:
                        break
                    
                    _cnt394 += 1
                if _cnt394 < 1:
                    self.raise_NoViableAlt(self.LA(1))
                _ttype = FLOAT;
            elif la1 and la1 in u'Dd':
                pass
                la1 = self.LA(1)
                if False:
                    pass
                elif la1 and la1 in u'd':
                    pass
                    self.match('d')
                elif la1 and la1 in u'D':
                    pass
                    self.match('D')
                else:
                        self.raise_NoViableAlt(self.LA(1))
                    
                _ttype = FIXED;
            else:
                ##<m3> <closing
                    pass
                    _ttype = FLOAT;
                
        elif la1 and la1 in u'Ee':
            pass
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in u'e':
                pass
                self.match('e')
            elif la1 and la1 in u'E':
                pass
                self.match('E')
            else:
                    self.raise_NoViableAlt(self.LA(1))
                
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in u'+':
                pass
                self.match('+')
            elif la1 and la1 in u'-':
                pass
                self.match('-')
            elif la1 and la1 in u'0123456789':
                pass
            else:
                    self.raise_NoViableAlt(self.LA(1))
                
            _cnt399= 0
            while True:
                if ((self.LA(1) >= u'0' and self.LA(1) <= u'9')):
                    pass
                    self.mDIGIT(False)
                else:
                    break
                
                _cnt399 += 1
            if _cnt399 < 1:
                self.raise_NoViableAlt(self.LA(1))
            _ttype = FLOAT;
        elif la1 and la1 in u'Dd':
            pass
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in u'd':
                pass
                self.match('d')
            elif la1 and la1 in u'D':
                pass
                self.match('D')
            else:
                    self.raise_NoViableAlt(self.LA(1))
                
            _ttype = FIXED;
        else:
            ##<m3> <closing
                pass
            
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mOCTAL(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = OCTAL
        _saveIndex = 0
        pass
        self.match('0')
        la1 = self.LA(1)
        if False:
            pass
        elif la1 and la1 in u'0123456789':
            pass
            _cnt404= 0
            while True:
                if ((self.LA(1) >= u'0' and self.LA(1) <= u'9')):
                    pass
                    self.mDIGIT(False)
                else:
                    break
                
                _cnt404 += 1
            if _cnt404 < 1:
                self.raise_NoViableAlt(self.LA(1))
        elif la1 and la1 in u'.':
            pass
            self.mFLOAT(False)
            _ttype = FLOAT;
        elif la1 and la1 in u'Dd':
            pass
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in u'd':
                pass
                self.match('d')
            elif la1 and la1 in u'D':
                pass
                self.match('D')
            else:
                    self.raise_NoViableAlt(self.LA(1))
                
            _ttype = FIXED;
        else:
            ##<m3> <closing
                pass
                _ttype = INT;
            
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mFLOAT(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = FLOAT
        _saveIndex = 0
        pass
        self.match('.')
        _cnt408= 0
        while True:
            if ((self.LA(1) >= u'0' and self.LA(1) <= u'9')):
                pass
                self.mDIGIT(False)
            else:
                break
            
            _cnt408 += 1
        if _cnt408 < 1:
            self.raise_NoViableAlt(self.LA(1))
        la1 = self.LA(1)
        if False:
            pass
        elif la1 and la1 in u'Ee':
            pass
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in u'e':
                pass
                self.match('e')
            elif la1 and la1 in u'E':
                pass
                self.match('E')
            else:
                    self.raise_NoViableAlt(self.LA(1))
                
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in u'+':
                pass
                self.match('+')
            elif la1 and la1 in u'-':
                pass
                self.match('-')
            elif la1 and la1 in u'0123456789':
                pass
            else:
                    self.raise_NoViableAlt(self.LA(1))
                
            _cnt413= 0
            while True:
                if ((self.LA(1) >= u'0' and self.LA(1) <= u'9')):
                    pass
                    self.mDIGIT(False)
                else:
                    break
                
                _cnt413 += 1
            if _cnt413 < 1:
                self.raise_NoViableAlt(self.LA(1))
        elif la1 and la1 in u'Dd':
            pass
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in u'd':
                pass
                self.match('d')
            elif la1 and la1 in u'D':
                pass
                self.match('D')
            else:
                    self.raise_NoViableAlt(self.LA(1))
                
            _ttype = FIXED;
        else:
            ##<m3> <closing
                pass
            
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mIDENT(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = IDENT
        _saveIndex = 0
        pass
        la1 = self.LA(1)
        if False:
            pass
        elif la1 and la1 in u'abcdefghijklmnopqrstuvwxyz':
            pass
            self.matchRange(u'a', u'z')
        elif la1 and la1 in u'ABCDEFGHIJKLMNOPQRSTUVWXYZ':
            pass
            self.matchRange(u'A', u'Z')
        else:
                self.raise_NoViableAlt(self.LA(1))
            
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
            elif la1 and la1 in u'_':
                pass
                self.match('_')
            elif la1 and la1 in u'0123456789':
                pass
                self.matchRange(u'0', u'9')
            else:
                    break
                
        ### option { testLiterals=true } 
        _ttype = self.testLiteralsTable(_ttype)
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mESCAPED_IDENT(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = ESCAPED_IDENT
        _saveIndex = 0
        pass
        self.match('_')
        la1 = self.LA(1)
        if False:
            pass
        elif la1 and la1 in u'abcdefghijklmnopqrstuvwxyz':
            pass
            self.matchRange(u'a', u'z')
        elif la1 and la1 in u'ABCDEFGHIJKLMNOPQRSTUVWXYZ':
            pass
            self.matchRange(u'A', u'Z')
        else:
                self.raise_NoViableAlt(self.LA(1))
            
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
            elif la1 and la1 in u'_':
                pass
                self.match('_')
            elif la1 and la1 in u'0123456789':
                pass
                self.matchRange(u'0', u'9')
            else:
                    break
                
        _begin+=1;_ttype = IDENT;
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    

### generate bit set
def mk_tokenSet_0(): 
    data = [0L] * 1025 ### init list
    data[1] =576460743847706622L
    return data
_tokenSet_0 = antlr.BitSet(mk_tokenSet_0())

### generate bit set
def mk_tokenSet_1(): 
    data = [0L] * 2048 ### init list
    data[0] =-1025L
    for x in xrange(1, 1023):
        data[x] = -1L
    data[1023] =9223372036854775807L
    return data
_tokenSet_1 = antlr.BitSet(mk_tokenSet_1())

### generate bit set
def mk_tokenSet_2(): 
    data = [0L] * 2048 ### init list
    data[0] =-140737488355329L
    for x in xrange(1, 1023):
        data[x] = -1L
    data[1023] =9223372036854775807L
    return data
_tokenSet_2 = antlr.BitSet(mk_tokenSet_2())

### generate bit set
def mk_tokenSet_3(): 
    data = [0L] * 2048 ### init list
    data[0] =-145135534867457L
    for x in xrange(1, 1023):
        data[x] = -1L
    data[1023] =9223372036854775807L
    return data
_tokenSet_3 = antlr.BitSet(mk_tokenSet_3())

### generate bit set
def mk_tokenSet_4(): 
    data = [0L] * 2048 ### init list
    data[0] =-4398046512129L
    for x in xrange(1, 1023):
        data[x] = -1L
    data[1023] =9223372036854775807L
    return data
_tokenSet_4 = antlr.BitSet(mk_tokenSet_4())

### generate bit set
def mk_tokenSet_5(): 
    data = [0L] * 2048 ### init list
    data[0] =-549755813889L
    data[1] =-268435457L
    for x in xrange(2, 1023):
        data[x] = -1L
    data[1023] =9223372036854775807L
    return data
_tokenSet_5 = antlr.BitSet(mk_tokenSet_5())

### generate bit set
def mk_tokenSet_6(): 
    data = [0L] * 1025 ### init list
    data[0] =287948901175001088L
    data[1] =541165879422L
    return data
_tokenSet_6 = antlr.BitSet(mk_tokenSet_6())

### generate bit set
def mk_tokenSet_7(): 
    data = [0L] * 2048 ### init list
    data[0] =-17179869185L
    data[1] =-268435457L
    for x in xrange(2, 1023):
        data[x] = -1L
    data[1023] =9223372036854775807L
    return data
_tokenSet_7 = antlr.BitSet(mk_tokenSet_7())
    
### __main__ header action >>> 
if __name__ == '__main__' :
    import sys
    import antlr
    import IDLLexer
    
    ### create lexer - shall read from stdin
    try:
        for token in IDLLexer.Lexer():
            print token
            
    except antlr.TokenStreamException, e:
        print "error: exception caught while lexing: ", e
### __main__ header action <<< 
