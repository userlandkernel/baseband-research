### $ANTLR 2.7.6 (20060903): "CPP_parser_py.g" -> "CPPLexer.py"$
### import antlr and other modules ..
import sys
import antlr

version = sys.version.split()[0]
if version < '2.2.1':
    False = 0
if version < '2.3':
    True = not False
### header action >>> 
# The statements in this block appear in both CPPLexer.py and CPPParser.py
from CPPDictionary import CPPDictionary
from LineObject import LineObject
import CPPSymbol
from var_types import * # "Do not use. Should be deleted in the next version"
from enum_types import *
# Auto-conversion hack - nfd
false = False
true = True
NULL = None
def printf(fmt, *args): print fmt % (args)
### header action <<< 
### preamble action >>> 

### preamble action <<< 
### >>>The Literals<<<
literals = {}
literals[u"namespace"] = 13
literals[u"public"] = 97
literals[u"_cdecl"] = 165
literals[u"__nonnull__"] = 59
literals[u"__stdcall"] = 28
literals[u"__restrict__"] = 69
literals[u"extern"] = 17
literals[u"__format__"] = 46
literals[u"case"] = 111
literals[u"delete"] = 175
literals[u"__used__"] = 52
literals[u"new"] = 164
literals[u"__declspec"] = 86
literals[u"__constructor__"] = 50
literals[u"__w64"] = 76
literals[u"__builtin_va_list"] = 87
literals[u"_declspec"] = 85
literals[u"__const__"] = 43
literals[u"_near"] = 167
literals[u"__int64"] = 75
literals[u"inline"] = 12
literals[u"unsigned"] = 81
literals[u"static_cast"] = 161
literals[u"__externally_visible__"] = 60
literals[u"return"] = 122
literals[u"mutable"] = 64
literals[u"__noreturn__"] = 37
literals[u"sizeof"] = 158
literals[u"protected"] = 98
literals[u"false"] = 94
literals[u"default"] = 112
literals[u"__signed__"] = 80
literals[u"do"] = 117
literals[u"__interrupt"] = 171
literals[u"__no_instrument_function__"] = 48
literals[u"_far"] = 169
literals[u"__malloc__"] = 56
literals[u"__sentinel__"] = 45
literals[u"_inline"] = 21
literals[u"_int64"] = 74
literals[u"union"] = 90
literals[u"catch"] = 124
literals[u"__volatile"] = 67
literals[u"__format_arg__"] = 47
literals[u"volatile"] = 66
literals[u"__unused__"] = 53
literals[u"explicit"] = 25
literals[u"_asm"] = 126
literals[u"__asm__"] = 108
literals[u"if"] = 113
literals[u"__destructor__"] = 51
literals[u"double"] = 83
literals[u"__signed"] = 79
literals[u"const_cast"] = 65
literals[u"__const"] = 36
literals[u"typedef"] = 7
literals[u"pascal"] = 172
literals[u"__mode__"] = 33
literals[u"__asm"] = 107
literals[u"dynamic_cast"] = 160
literals[u"try"] = 123
literals[u"__cdecl"] = 166
literals[u"register"] = 62
literals[u"__volatile__"] = 127
literals[u"auto"] = 61
literals[u"enum"] = 8
literals[u"goto"] = 119
literals[u"int"] = 73
literals[u"__noinline__"] = 39
literals[u"for"] = 118
literals[u"__flatten__"] = 41
literals[u"char"] = 70
literals[u"friend"] = 26
literals[u"__near"] = 168
literals[u"private"] = 99
literals[u"_stdcall"] = 27
literals[u"class"] = 88
literals[u"__section__"] = 49
literals[u"packed"] = 34
literals[u"this"] = 92
literals[u"__far"] = 170
literals[u"__nothrow__"] = 44
literals[u"static"] = 63
literals[u"__always_inline__"] = 40
literals[u"__returns_twice__"] = 38
literals[u"operator"] = 91
literals[u"using"] = 125
literals[u"__deprecated__"] = 54
literals[u"typename"] = 29
literals[u"throw"] = 110
literals[u"__alias__"] = 57
literals[u"float"] = 82
literals[u"const"] = 35
literals[u"continue"] = 120
literals[u"__attribute__"] = 30
literals[u"reinterpret_cast"] = 162
literals[u"__restrict"] = 68
literals[u"bool"] = 71
literals[u"template"] = 4
literals[u"struct"] = 89
literals[u"__pascal"] = 174
literals[u"signed"] = 78
literals[u"else"] = 114
literals[u"_pascal"] = 173
literals[u"__warn_unused_result__"] = 58
literals[u"void"] = 84
literals[u"__pure__"] = 42
literals[u"switch"] = 115
literals[u"__inline__"] = 23
literals[u"__weak__"] = 55
literals[u"true"] = 93
literals[u"__inline"] = 22
literals[u"long"] = 77
literals[u"__extension__"] = 20
literals[u"asm"] = 106
literals[u"virtual"] = 24
literals[u"break"] = 121
literals[u"while"] = 116
literals[u"short"] = 72


### import antlr.Token 
from antlr import Token
### >>>The Known Token Types <<<
SKIP                = antlr.SKIP
INVALID_TYPE        = antlr.INVALID_TYPE
EOF_TYPE            = antlr.EOF_TYPE
EOF                 = antlr.EOF
NULL_TREE_LOOKAHEAD = antlr.NULL_TREE_LOOKAHEAD
MIN_USER_TYPE       = antlr.MIN_USER_TYPE
LITERAL_template = 4
LESSTHAN = 5
GREATERTHAN = 6
LITERAL_typedef = 7
LITERAL_enum = 8
ID = 9
LCURLY = 10
SEMICOLON = 11
LITERAL_inline = 12
LITERAL_namespace = 13
RCURLY = 14
ASSIGNEQUAL = 15
COLON = 16
LITERAL_extern = 17
StringLiteral = 18
COMMA = 19
LITERAL___extension__ = 20
LITERAL__inline = 21
LITERAL___inline = 22
LITERAL___inline__ = 23
LITERAL_virtual = 24
LITERAL_explicit = 25
LITERAL_friend = 26
LITERAL__stdcall = 27
LITERAL___stdcall = 28
LITERAL_typename = 29
LITERAL___attribute__ = 30
LPAREN = 31
RPAREN = 32
LITERAL___mode__ = 33
LITERAL_packed = 34
LITERAL_const = 35
LITERAL___const = 36
LITERAL___noreturn__ = 37
LITERAL___returns_twice__ = 38
LITERAL___noinline__ = 39
LITERAL___always_inline__ = 40
LITERAL___flatten__ = 41
LITERAL___pure__ = 42
LITERAL___const__ = 43
LITERAL___nothrow__ = 44
LITERAL___sentinel__ = 45
LITERAL___format__ = 46
LITERAL___format_arg__ = 47
LITERAL___no_instrument_function__ = 48
LITERAL___section__ = 49
LITERAL___constructor__ = 50
LITERAL___destructor__ = 51
LITERAL___used__ = 52
LITERAL___unused__ = 53
LITERAL___deprecated__ = 54
LITERAL___weak__ = 55
LITERAL___malloc__ = 56
LITERAL___alias__ = 57
LITERAL___warn_unused_result__ = 58
LITERAL___nonnull__ = 59
LITERAL___externally_visible__ = 60
LITERAL_auto = 61
LITERAL_register = 62
LITERAL_static = 63
LITERAL_mutable = 64
LITERAL_const_cast = 65
LITERAL_volatile = 66
LITERAL___volatile = 67
LITERAL___restrict = 68
LITERAL___restrict__ = 69
LITERAL_char = 70
LITERAL_bool = 71
LITERAL_short = 72
LITERAL_int = 73
### "_int64" = 74
### "__int64" = 75
### "__w64" = 76
LITERAL_long = 77
LITERAL_signed = 78
LITERAL___signed = 79
LITERAL___signed__ = 80
LITERAL_unsigned = 81
LITERAL_float = 82
LITERAL_double = 83
LITERAL_void = 84
LITERAL__declspec = 85
LITERAL___declspec = 86
LITERAL___builtin_va_list = 87
LITERAL_class = 88
LITERAL_struct = 89
LITERAL_union = 90
OPERATOR = 91
LITERAL_this = 92
LITERAL_true = 93
LITERAL_false = 94
DOT = 95
EQUALS = 96
LITERAL_public = 97
LITERAL_protected = 98
LITERAL_private = 99
OCTALINT = 100
STAR = 101
AMPERSAND = 102
LSQUARE = 103
RSQUARE = 104
TILDE = 105
LITERAL_asm = 106
LITERAL___asm = 107
LITERAL___asm__ = 108
ELLIPSIS = 109
LITERAL_throw = 110
LITERAL_case = 111
LITERAL_default = 112
LITERAL_if = 113
LITERAL_else = 114
LITERAL_switch = 115
LITERAL_while = 116
LITERAL_do = 117
LITERAL_for = 118
LITERAL_goto = 119
LITERAL_continue = 120
LITERAL_break = 121
LITERAL_return = 122
LITERAL_try = 123
LITERAL_catch = 124
LITERAL_using = 125
LITERAL__asm = 126
LITERAL___volatile__ = 127
SCOPE = 128
TIMESEQUAL = 129
DIVIDEEQUAL = 130
MINUSEQUAL = 131
PLUSEQUAL = 132
MODEQUAL = 133
SHIFTLEFTEQUAL = 134
SHIFTRIGHTEQUAL = 135
BITWISEANDEQUAL = 136
BITWISEXOREQUAL = 137
BITWISEOREQUAL = 138
QUESTIONMARK = 139
OR = 140
AND = 141
BITWISEOR = 142
BITWISEXOR = 143
NOTEQUAL = 144
EQUAL = 145
LESSTHANOREQUALTO = 146
GREATERTHANOREQUALTO = 147
SHIFTLEFT = 148
SHIFTRIGHT = 149
PLUS = 150
MINUS = 151
DIVIDE = 152
MOD = 153
DOTMBR = 154
POINTERTOMBR = 155
PLUSPLUS = 156
MINUSMINUS = 157
LITERAL_sizeof = 158
POINTERTO = 159
LITERAL_dynamic_cast = 160
LITERAL_static_cast = 161
LITERAL_reinterpret_cast = 162
NOT = 163
LITERAL_new = 164
LITERAL__cdecl = 165
LITERAL___cdecl = 166
LITERAL__near = 167
LITERAL___near = 168
LITERAL__far = 169
LITERAL___far = 170
LITERAL___interrupt = 171
LITERAL_pascal = 172
LITERAL__pascal = 173
LITERAL___pascal = 174
LITERAL_delete = 175
DECIMALINT = 176
HEXADECIMALINT = 177
CharLiteral = 178
FLOATONE = 179
FLOATTWO = 180
Whitespace = 181
Comment = 182
CPPComment = 183
PREPROC_DIRECTIVE = 184
ALL_TO_NL = 185
LineDirective = 186
Space = 187
Pragma = 188
Error = 189
EndOfLine = 190
Escape = 191
Digit = 192
Decimal = 193
LongSuffix = 194
UnsignedSuffix = 195
FloatSuffix = 196
Exponent = 197
Vocabulary = 198
Number = 199

class Lexer(antlr.CharScanner) :
    ### user action >>>
    def setOriginalSource(src):
       originalSource = src
       lineObject.setSource(src)
    
    def setSource(src):
       lineObject.setSource(src)
    ### user action <<<
    def __init__(self, *argv, **kwargs) :
        antlr.CharScanner.__init__(self, *argv, **kwargs)
        self.caseSensitiveLiterals = True
        self.setCaseSensitive(True)
        self.literals = literals
        ### __init__ header action >>> 
        self.line_directive_handler = kwargs.get("line_directive_handler")
        self.lineObject = LineObject()
        self.originalSource = ""
        
        self._line = 0
        ### __init__ header action <<< 
    
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
                            elif la1 and la1 in u',':
                                pass
                                self.mCOMMA(True)
                                theRetToken = self._returnToken
                            elif la1 and la1 in u'?':
                                pass
                                self.mQUESTIONMARK(True)
                                theRetToken = self._returnToken
                            elif la1 and la1 in u';':
                                pass
                                self.mSEMICOLON(True)
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
                                self.mLSQUARE(True)
                                theRetToken = self._returnToken
                            elif la1 and la1 in u']':
                                pass
                                self.mRSQUARE(True)
                                theRetToken = self._returnToken
                            elif la1 and la1 in u'{':
                                pass
                                self.mLCURLY(True)
                                theRetToken = self._returnToken
                            elif la1 and la1 in u'}':
                                pass
                                self.mRCURLY(True)
                                theRetToken = self._returnToken
                            elif la1 and la1 in u'~':
                                pass
                                self.mTILDE(True)
                                theRetToken = self._returnToken
                            elif la1 and la1 in u'\t\n\u000c\r \\':
                                pass
                                self.mWhitespace(True)
                                theRetToken = self._returnToken
                            elif la1 and la1 in u'\'':
                                pass
                                self.mCharLiteral(True)
                                theRetToken = self._returnToken
                            elif la1 and la1 in u'"':
                                pass
                                self.mStringLiteral(True)
                                theRetToken = self._returnToken
                            elif la1 and la1 in u'ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz':
                                pass
                                self.mID(True)
                                theRetToken = self._returnToken
                            else:
                                if (self.LA(1)==u'>') and (self.LA(2)==u'>') and (self.LA(3)==u'='):
                                    pass
                                    self.mSHIFTRIGHTEQUAL(True)
                                    theRetToken = self._returnToken
                                elif (self.LA(1)==u'<') and (self.LA(2)==u'<') and (self.LA(3)==u'='):
                                    pass
                                    self.mSHIFTLEFTEQUAL(True)
                                    theRetToken = self._returnToken
                                elif (self.LA(1)==u'-') and (self.LA(2)==u'>') and (self.LA(3)==u'*'):
                                    pass
                                    self.mPOINTERTOMBR(True)
                                    theRetToken = self._returnToken
                                elif (self.LA(1)==u'-') and (self.LA(2)==u'>') and (True):
                                    pass
                                    self.mPOINTERTO(True)
                                    theRetToken = self._returnToken
                                elif (self.LA(1)==u'=') and (self.LA(2)==u'='):
                                    pass
                                    self.mEQUAL(True)
                                    theRetToken = self._returnToken
                                elif (self.LA(1)==u'!') and (self.LA(2)==u'='):
                                    pass
                                    self.mNOTEQUAL(True)
                                    theRetToken = self._returnToken
                                elif (self.LA(1)==u'<') and (self.LA(2)==u'='):
                                    pass
                                    self.mLESSTHANOREQUALTO(True)
                                    theRetToken = self._returnToken
                                elif (self.LA(1)==u'>') and (self.LA(2)==u'='):
                                    pass
                                    self.mGREATERTHANOREQUALTO(True)
                                    theRetToken = self._returnToken
                                elif (self.LA(1)==u'/') and (self.LA(2)==u'='):
                                    pass
                                    self.mDIVIDEEQUAL(True)
                                    theRetToken = self._returnToken
                                elif (self.LA(1)==u'+') and (self.LA(2)==u'='):
                                    pass
                                    self.mPLUSEQUAL(True)
                                    theRetToken = self._returnToken
                                elif (self.LA(1)==u'+') and (self.LA(2)==u'+'):
                                    pass
                                    self.mPLUSPLUS(True)
                                    theRetToken = self._returnToken
                                elif (self.LA(1)==u'-') and (self.LA(2)==u'='):
                                    pass
                                    self.mMINUSEQUAL(True)
                                    theRetToken = self._returnToken
                                elif (self.LA(1)==u'-') and (self.LA(2)==u'-'):
                                    pass
                                    self.mMINUSMINUS(True)
                                    theRetToken = self._returnToken
                                elif (self.LA(1)==u'*') and (self.LA(2)==u'='):
                                    pass
                                    self.mTIMESEQUAL(True)
                                    theRetToken = self._returnToken
                                elif (self.LA(1)==u'%') and (self.LA(2)==u'='):
                                    pass
                                    self.mMODEQUAL(True)
                                    theRetToken = self._returnToken
                                elif (self.LA(1)==u'>') and (self.LA(2)==u'>') and (True):
                                    pass
                                    self.mSHIFTRIGHT(True)
                                    theRetToken = self._returnToken
                                elif (self.LA(1)==u'<') and (self.LA(2)==u'<') and (True):
                                    pass
                                    self.mSHIFTLEFT(True)
                                    theRetToken = self._returnToken
                                elif (self.LA(1)==u'&') and (self.LA(2)==u'&'):
                                    pass
                                    self.mAND(True)
                                    theRetToken = self._returnToken
                                elif (self.LA(1)==u'|') and (self.LA(2)==u'|'):
                                    pass
                                    self.mOR(True)
                                    theRetToken = self._returnToken
                                elif (self.LA(1)==u'&') and (self.LA(2)==u'='):
                                    pass
                                    self.mBITWISEANDEQUAL(True)
                                    theRetToken = self._returnToken
                                elif (self.LA(1)==u'|') and (self.LA(2)==u'='):
                                    pass
                                    self.mBITWISEOREQUAL(True)
                                    theRetToken = self._returnToken
                                elif (self.LA(1)==u'^') and (self.LA(2)==u'='):
                                    pass
                                    self.mBITWISEXOREQUAL(True)
                                    theRetToken = self._returnToken
                                elif (self.LA(1)==u'.') and (self.LA(2)==u'*'):
                                    pass
                                    self.mDOTMBR(True)
                                    theRetToken = self._returnToken
                                elif (self.LA(1)==u':') and (self.LA(2)==u':'):
                                    pass
                                    self.mSCOPE(True)
                                    theRetToken = self._returnToken
                                elif (self.LA(1)==u'/') and (self.LA(2)==u'*'):
                                    pass
                                    self.mComment(True)
                                    theRetToken = self._returnToken
                                elif (self.LA(1)==u'/') and (self.LA(2)==u'/'):
                                    pass
                                    self.mCPPComment(True)
                                    theRetToken = self._returnToken
                                elif (self.LA(1)==u'#') and (_tokenSet_0.member(self.LA(2))):
                                    pass
                                    self.mPREPROC_DIRECTIVE(True)
                                    theRetToken = self._returnToken
                                elif (self.LA(1)==u'#') and (self.LA(2)==u'p'):
                                    pass
                                    self.mPragma(True)
                                    theRetToken = self._returnToken
                                elif (self.LA(1)==u'#') and (self.LA(2)==u'e'):
                                    pass
                                    self.mError(True)
                                    theRetToken = self._returnToken
                                elif (self.LA(1)==u'=') and (True):
                                    pass
                                    self.mASSIGNEQUAL(True)
                                    theRetToken = self._returnToken
                                elif (self.LA(1)==u':') and (True):
                                    pass
                                    self.mCOLON(True)
                                    theRetToken = self._returnToken
                                elif (self.LA(1)==u'<') and (True):
                                    pass
                                    self.mLESSTHAN(True)
                                    theRetToken = self._returnToken
                                elif (self.LA(1)==u'>') and (True):
                                    pass
                                    self.mGREATERTHAN(True)
                                    theRetToken = self._returnToken
                                elif (self.LA(1)==u'/') and (True):
                                    pass
                                    self.mDIVIDE(True)
                                    theRetToken = self._returnToken
                                elif (self.LA(1)==u'+') and (True):
                                    pass
                                    self.mPLUS(True)
                                    theRetToken = self._returnToken
                                elif (self.LA(1)==u'-') and (True):
                                    pass
                                    self.mMINUS(True)
                                    theRetToken = self._returnToken
                                elif (self.LA(1)==u'*') and (True):
                                    pass
                                    self.mSTAR(True)
                                    theRetToken = self._returnToken
                                elif (self.LA(1)==u'%') and (True):
                                    pass
                                    self.mMOD(True)
                                    theRetToken = self._returnToken
                                elif (self.LA(1)==u'!') and (True):
                                    pass
                                    self.mNOT(True)
                                    theRetToken = self._returnToken
                                elif (self.LA(1)==u'&') and (True):
                                    pass
                                    self.mAMPERSAND(True)
                                    theRetToken = self._returnToken
                                elif (self.LA(1)==u'|') and (True):
                                    pass
                                    self.mBITWISEOR(True)
                                    theRetToken = self._returnToken
                                elif (self.LA(1)==u'^') and (True):
                                    pass
                                    self.mBITWISEXOR(True)
                                    theRetToken = self._returnToken
                                elif (_tokenSet_1.member(self.LA(1))) and (True):
                                    pass
                                    self.mNumber(True)
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
        
    def mASSIGNEQUAL(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = ASSIGNEQUAL
        _saveIndex = 0
        pass
        self.match('=')
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
    
    def mQUESTIONMARK(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = QUESTIONMARK
        _saveIndex = 0
        pass
        self.match('?')
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
    
    def mPOINTERTO(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = POINTERTO
        _saveIndex = 0
        pass
        self.match("->")
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
    
    def mEQUAL(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = EQUAL
        _saveIndex = 0
        pass
        self.match("==")
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mNOTEQUAL(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = NOTEQUAL
        _saveIndex = 0
        pass
        self.match("!=")
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mLESSTHANOREQUALTO(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = LESSTHANOREQUALTO
        _saveIndex = 0
        pass
        self.match("<=")
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
    
    def mGREATERTHANOREQUALTO(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = GREATERTHANOREQUALTO
        _saveIndex = 0
        pass
        self.match(">=")
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
    
    def mDIVIDE(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = DIVIDE
        _saveIndex = 0
        pass
        self.match('/')
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mDIVIDEEQUAL(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = DIVIDEEQUAL
        _saveIndex = 0
        pass
        self.match("/=")
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
    
    def mPLUSEQUAL(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = PLUSEQUAL
        _saveIndex = 0
        pass
        self.match("+=")
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mPLUSPLUS(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = PLUSPLUS
        _saveIndex = 0
        pass
        self.match("++")
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
    
    def mMINUSEQUAL(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = MINUSEQUAL
        _saveIndex = 0
        pass
        self.match("-=")
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mMINUSMINUS(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = MINUSMINUS
        _saveIndex = 0
        pass
        self.match("--")
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
    
    def mTIMESEQUAL(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = TIMESEQUAL
        _saveIndex = 0
        pass
        self.match("*=")
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
    
    def mMODEQUAL(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = MODEQUAL
        _saveIndex = 0
        pass
        self.match("%=")
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mSHIFTRIGHT(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = SHIFTRIGHT
        _saveIndex = 0
        pass
        self.match(">>")
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mSHIFTRIGHTEQUAL(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = SHIFTRIGHTEQUAL
        _saveIndex = 0
        pass
        self.match(">>=")
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mSHIFTLEFT(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = SHIFTLEFT
        _saveIndex = 0
        pass
        self.match("<<")
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mSHIFTLEFTEQUAL(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = SHIFTLEFTEQUAL
        _saveIndex = 0
        pass
        self.match("<<=")
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mAND(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = AND
        _saveIndex = 0
        pass
        self.match("&&")
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
    
    def mOR(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = OR
        _saveIndex = 0
        pass
        self.match("||")
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mAMPERSAND(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = AMPERSAND
        _saveIndex = 0
        pass
        self.match('&')
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mBITWISEANDEQUAL(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = BITWISEANDEQUAL
        _saveIndex = 0
        pass
        self.match("&=")
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
    
    def mBITWISEOR(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = BITWISEOR
        _saveIndex = 0
        pass
        self.match('|')
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mBITWISEOREQUAL(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = BITWISEOREQUAL
        _saveIndex = 0
        pass
        self.match("|=")
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mBITWISEXOR(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = BITWISEXOR
        _saveIndex = 0
        pass
        self.match('^')
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mBITWISEXOREQUAL(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = BITWISEXOREQUAL
        _saveIndex = 0
        pass
        self.match("^=")
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mPOINTERTOMBR(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = POINTERTOMBR
        _saveIndex = 0
        pass
        self.match("->*")
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mDOTMBR(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = DOTMBR
        _saveIndex = 0
        pass
        self.match(".*")
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mSCOPE(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = SCOPE
        _saveIndex = 0
        pass
        self.match("::")
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mWhitespace(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = Whitespace
        _saveIndex = 0
        pass
        la1 = self.LA(1)
        if False:
            pass
        elif la1 and la1 in u'\t\u000c ':
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
            elif la1 and la1 in u'\u000c':
                pass
                self.match('\f')
            else:
                    self.raise_NoViableAlt(self.LA(1))
                
        elif la1 and la1 in u'\n\r':
            pass
            if (self.LA(1)==u'\r') and (self.LA(2)==u'\n'):
                pass
                self.match("\r\n")
            elif (self.LA(1)==u'\r') and (True):
                pass
                self.match('\r')
            elif (self.LA(1)==u'\n'):
                pass
                self.match('\n')
            else:
                self.raise_NoViableAlt(self.LA(1))
            
            if not self.inputState.guessing:
                self.newline();
        elif la1 and la1 in u'\\':
            pass
            if (self.LA(1)==u'\\') and (self.LA(2)==u'\r') and (self.LA(3)==u'\n'):
                pass
                self.match("\\\r\n")
            elif (self.LA(1)==u'\\') and (self.LA(2)==u'\r') and (True):
                pass
                self.match("\\\r")
            elif (self.LA(1)==u'\\') and (self.LA(2)==u'\n'):
                pass
                self.match("\\\n")
            else:
                self.raise_NoViableAlt(self.LA(1))
            
            if not self.inputState.guessing:
                self.newline();
        else:
                self.raise_NoViableAlt(self.LA(1))
            
        if not self.inputState.guessing:
            _ttype = SKIP
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mComment(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = Comment
        _saveIndex = 0
        pass
        self.match("/*")
        while True:
            if ((self.LA(1)==u'*') and ((self.LA(2) >= u'\u0000' and self.LA(2) <= u'\u00ff')) and ((self.LA(3) >= u'\u0000' and self.LA(3) <= u'\u00ff')) and (self.LA(2) != '/')):
                pass
                self.match('*')
            elif (self.LA(1)==u'\n' or self.LA(1)==u'\r'):
                pass
                self.mEndOfLine(False)
                if not self.inputState.guessing:
                    self.newline();
            elif (_tokenSet_2.member(self.LA(1))):
                pass
                self.match(_tokenSet_2)
            else:
                break
            
        self.match("*/")
        if not self.inputState.guessing:
            _ttype = SKIP
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mEndOfLine(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = EndOfLine
        _saveIndex = 0
        pass
        if (self.LA(1)==u'\r') and (self.LA(2)==u'\n') and (True):
            pass
            self.match("\r\n")
        elif (self.LA(1)==u'\r') and (True) and (True):
            pass
            self.match('\r')
        elif (self.LA(1)==u'\n'):
            pass
            self.match('\n')
        else:
            self.raise_NoViableAlt(self.LA(1))
        
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mCPPComment(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = CPPComment
        _saveIndex = 0
        pass
        self.match("//")
        while True:
            if (_tokenSet_3.member(self.LA(1))):
                pass
                self.match(_tokenSet_3)
            else:
                break
            
        self.mEndOfLine(False)
        if not self.inputState.guessing:
            _ttype = SKIP; self.newline();
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mPREPROC_DIRECTIVE(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = PREPROC_DIRECTIVE
        _saveIndex = 0
        pass
        self.match('#')
        self.mLineDirective(False)
        if not self.inputState.guessing:
            _ttype = SKIP; self.newline();
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mLineDirective(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = LineDirective
        _saveIndex = 0
        n = None
        sl = None
        pass
        la1 = self.LA(1)
        if False:
            pass
        elif la1 and la1 in u'l':
            pass
            self.match("line")
        elif la1 and la1 in u'\t\u000c ':
            pass
        else:
                self.raise_NoViableAlt(self.LA(1))
            
        _cnt584= 0
        while True:
            if (self.LA(1)==u'\t' or self.LA(1)==u'\u000c' or self.LA(1)==u' '):
                pass
                self.mSpace(False)
            else:
                break
            
            _cnt584 += 1
        if _cnt584 < 1:
            self.raise_NoViableAlt(self.LA(1))
        self.mDecimal(True)
        n = self._returnToken
        _cnt586= 0
        while True:
            if (self.LA(1)==u'\t' or self.LA(1)==u'\u000c' or self.LA(1)==u' '):
                pass
                self.mSpace(False)
            else:
                break
            
            _cnt586 += 1
        if _cnt586 < 1:
            self.raise_NoViableAlt(self.LA(1))
        pass
        self.mALL_TO_NL(True)
        sl = self._returnToken
        if not self.inputState.guessing:
            self.line_directive_handler(sl.getText(), n.getText())  # see main()
        self.mEndOfLine(False)
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mALL_TO_NL(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = ALL_TO_NL
        _saveIndex = 0
        pass
        pass
        self.match(_tokenSet_4)
        while True:
            if (_tokenSet_3.member(self.LA(1))):
                pass
                self.match(_tokenSet_3)
            else:
                break
            
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mSpace(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = Space
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
        elif la1 and la1 in u'\u000c':
            pass
            self.match('\f')
        else:
                self.raise_NoViableAlt(self.LA(1))
            
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mDecimal(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = Decimal
        _saveIndex = 0
        pass
        _cnt622= 0
        while True:
            if ((self.LA(1) >= u'0' and self.LA(1) <= u'9')):
                pass
                self.matchRange(u'0', u'9')
            else:
                break
            
            _cnt622 += 1
        if _cnt622 < 1:
            self.raise_NoViableAlt(self.LA(1))
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mPragma(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = Pragma
        _saveIndex = 0
        pass
        pass
        self.match('#')
        self.match("pragma")
        while True:
            if (_tokenSet_3.member(self.LA(1))):
                pass
                self.match(_tokenSet_3)
            else:
                break
            
        self.mEndOfLine(False)
        if not self.inputState.guessing:
            _ttype = SKIP; self.newline();
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mError(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = Error
        _saveIndex = 0
        pass
        pass
        self.match('#')
        self.match("error")
        while True:
            if (_tokenSet_3.member(self.LA(1))):
                pass
                self.match(_tokenSet_3)
            else:
                break
            
        self.mEndOfLine(False)
        if not self.inputState.guessing:
            _ttype = SKIP; self.newline();
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mCharLiteral(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = CharLiteral
        _saveIndex = 0
        pass
        self.match('\'')
        if (self.LA(1)==u'\\') and (_tokenSet_5.member(self.LA(2))) and (_tokenSet_6.member(self.LA(3))):
            pass
            self.mEscape(False)
        elif (_tokenSet_7.member(self.LA(1))) and (self.LA(2)==u'\'') and (True):
            pass
            self.match(_tokenSet_7)
        else:
            self.raise_NoViableAlt(self.LA(1))
        
        self.match('\'')
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mEscape(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = Escape
        _saveIndex = 0
        pass
        self.match('\\')
        la1 = self.LA(1)
        if False:
            pass
        elif la1 and la1 in u'a':
            pass
            self.match('a')
        elif la1 and la1 in u'b':
            pass
            self.match('b')
        elif la1 and la1 in u'f':
            pass
            self.match('f')
        elif la1 and la1 in u'n':
            pass
            self.match('n')
        elif la1 and la1 in u'r':
            pass
            self.match('r')
        elif la1 and la1 in u't':
            pass
            self.match('t')
        elif la1 and la1 in u'v':
            pass
            self.match('v')
        elif la1 and la1 in u'"':
            pass
            self.match('"')
        elif la1 and la1 in u'\'':
            pass
            self.match('\'')
        elif la1 and la1 in u'\\':
            pass
            self.match('\\')
        elif la1 and la1 in u'?':
            pass
            self.match('?')
        elif la1 and la1 in u'0123':
            pass
            pass
            self.matchRange(u'0', u'3')
            if ((self.LA(1) >= u'0' and self.LA(1) <= u'9')) and (_tokenSet_3.member(self.LA(2))) and (True):
                pass
                self.mDigit(False)
                if ((self.LA(1) >= u'0' and self.LA(1) <= u'9')) and (_tokenSet_3.member(self.LA(2))) and (True):
                    pass
                    self.mDigit(False)
                elif (_tokenSet_3.member(self.LA(1))) and (True) and (True):
                    pass
                else:
                    self.raise_NoViableAlt(self.LA(1))
                
            elif (_tokenSet_3.member(self.LA(1))) and (True) and (True):
                pass
            else:
                self.raise_NoViableAlt(self.LA(1))
            
        elif la1 and la1 in u'4567':
            pass
            pass
            self.matchRange(u'4', u'7')
            if ((self.LA(1) >= u'0' and self.LA(1) <= u'9')) and (_tokenSet_3.member(self.LA(2))) and (True):
                pass
                self.mDigit(False)
            elif (_tokenSet_3.member(self.LA(1))) and (True) and (True):
                pass
            else:
                self.raise_NoViableAlt(self.LA(1))
            
        elif la1 and la1 in u'x':
            pass
            self.match('x')
            _cnt618= 0
            while True:
                if ((self.LA(1) >= u'0' and self.LA(1) <= u'9')) and (_tokenSet_3.member(self.LA(2))) and (True):
                    pass
                    self.mDigit(False)
                elif ((self.LA(1) >= u'a' and self.LA(1) <= u'f')) and (_tokenSet_3.member(self.LA(2))) and (True):
                    pass
                    self.matchRange(u'a', u'f')
                elif ((self.LA(1) >= u'A' and self.LA(1) <= u'F')) and (_tokenSet_3.member(self.LA(2))) and (True):
                    pass
                    self.matchRange(u'A', u'F')
                else:
                    break
                
                _cnt618 += 1
            if _cnt618 < 1:
                self.raise_NoViableAlt(self.LA(1))
        else:
                self.raise_NoViableAlt(self.LA(1))
            
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mStringLiteral(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = StringLiteral
        _saveIndex = 0
        pass
        self.match('"')
        while True:
            if (self.LA(1)==u'\\') and (_tokenSet_5.member(self.LA(2))):
                pass
                self.mEscape(False)
            elif (self.LA(1)==u'\\') and (self.LA(2)==u'\n' or self.LA(2)==u'\r'):
                pass
                if (self.LA(1)==u'\\') and (self.LA(2)==u'\r') and (self.LA(3)==u'\n'):
                    pass
                    self.match("\\\r\n")
                elif (self.LA(1)==u'\\') and (self.LA(2)==u'\r') and (_tokenSet_3.member(self.LA(3))):
                    pass
                    self.match("\\\r")
                elif (self.LA(1)==u'\\') and (self.LA(2)==u'\n'):
                    pass
                    self.match("\\\n")
                else:
                    self.raise_NoViableAlt(self.LA(1))
                
                if not self.inputState.guessing:
                    self.newline();
            elif (_tokenSet_8.member(self.LA(1))):
                pass
                self.match(_tokenSet_8)
            else:
                break
            
        self.match('"')
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mDigit(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = Digit
        _saveIndex = 0
        pass
        self.matchRange(u'0', u'9')
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mLongSuffix(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = LongSuffix
        _saveIndex = 0
        la1 = self.LA(1)
        if False:
            pass
        elif la1 and la1 in u'l':
            pass
            self.match('l')
        elif la1 and la1 in u'L':
            pass
            self.match('L')
        else:
                self.raise_NoViableAlt(self.LA(1))
            
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mUnsignedSuffix(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = UnsignedSuffix
        _saveIndex = 0
        la1 = self.LA(1)
        if False:
            pass
        elif la1 and la1 in u'u':
            pass
            self.match('u')
        elif la1 and la1 in u'U':
            pass
            self.match('U')
        else:
                self.raise_NoViableAlt(self.LA(1))
            
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mFloatSuffix(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = FloatSuffix
        _saveIndex = 0
        la1 = self.LA(1)
        if False:
            pass
        elif la1 and la1 in u'f':
            pass
            self.match('f')
        elif la1 and la1 in u'F':
            pass
            self.match('F')
        else:
                self.raise_NoViableAlt(self.LA(1))
            
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mExponent(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = Exponent
        _saveIndex = 0
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
            
        _cnt630= 0
        while True:
            if ((self.LA(1) >= u'0' and self.LA(1) <= u'9')):
                pass
                self.mDigit(False)
            else:
                break
            
            _cnt630 += 1
        if _cnt630 < 1:
            self.raise_NoViableAlt(self.LA(1))
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mVocabulary(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = Vocabulary
        _saveIndex = 0
        pass
        self.matchRange(u'\3', u'\377')
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mNumber(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = Number
        _saveIndex = 0
        synPredMatched637 = False
        if ((self.LA(1) >= u'0' and self.LA(1) <= u'9')) and (_tokenSet_9.member(self.LA(2))) and (True):
            _m637 = self.mark()
            synPredMatched637 = True
            self.inputState.guessing += 1
            try:
                pass
                _cnt635= 0
                while True:
                    if ((self.LA(1) >= u'0' and self.LA(1) <= u'9')):
                        pass
                        self.mDigit(False)
                    else:
                        break
                    
                    _cnt635 += 1
                if _cnt635 < 1:
                    self.raise_NoViableAlt(self.LA(1))
                la1 = self.LA(1)
                if False:
                    pass
                elif la1 and la1 in u'.':
                    pass
                    self.match('.')
                elif la1 and la1 in u'e':
                    pass
                    self.match('e')
                elif la1 and la1 in u'E':
                    pass
                    self.match('E')
                else:
                        self.raise_NoViableAlt(self.LA(1))
                    
            except antlr.RecognitionException, pe:
                synPredMatched637 = False
            self.rewind(_m637)
            self.inputState.guessing -= 1
        if synPredMatched637:
            pass
            _cnt639= 0
            while True:
                if ((self.LA(1) >= u'0' and self.LA(1) <= u'9')):
                    pass
                    self.mDigit(False)
                else:
                    break
                
                _cnt639 += 1
            if _cnt639 < 1:
                self.raise_NoViableAlt(self.LA(1))
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in u'.':
                pass
                self.match('.')
                while True:
                    if ((self.LA(1) >= u'0' and self.LA(1) <= u'9')):
                        pass
                        self.mDigit(False)
                    else:
                        break
                    
                if (self.LA(1)==u'E' or self.LA(1)==u'e'):
                    pass
                    self.mExponent(False)
                else: ## <m4>
                        pass
                    
                if not self.inputState.guessing:
                    _ttype = FLOATONE;
            elif la1 and la1 in u'Ee':
                pass
                self.mExponent(False)
                if not self.inputState.guessing:
                    _ttype = FLOATTWO;
            else:
                    self.raise_NoViableAlt(self.LA(1))
                
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in u'Ff':
                pass
                self.mFloatSuffix(False)
            elif la1 and la1 in u'Ll':
                pass
                self.mLongSuffix(False)
            else:
                ##<m3> <closing
                    pass
                
        else:
            synPredMatched646 = False
            if (self.LA(1)==u'.') and (self.LA(2)==u'.'):
                _m646 = self.mark()
                synPredMatched646 = True
                self.inputState.guessing += 1
                try:
                    pass
                    self.match("...")
                except antlr.RecognitionException, pe:
                    synPredMatched646 = False
                self.rewind(_m646)
                self.inputState.guessing -= 1
            if synPredMatched646:
                pass
                self.match("...")
                if not self.inputState.guessing:
                    _ttype = ELLIPSIS;
            elif (self.LA(1)==u'0') and (self.LA(2)==u'X' or self.LA(2)==u'x'):
                pass
                self.match('0')
                la1 = self.LA(1)
                if False:
                    pass
                elif la1 and la1 in u'x':
                    pass
                    self.match('x')
                elif la1 and la1 in u'X':
                    pass
                    self.match('X')
                else:
                        self.raise_NoViableAlt(self.LA(1))
                    
                _cnt662= 0
                while True:
                    la1 = self.LA(1)
                    if False:
                        pass
                    elif la1 and la1 in u'abcdef':
                        pass
                        self.matchRange(u'a', u'f')
                    elif la1 and la1 in u'ABCDEF':
                        pass
                        self.matchRange(u'A', u'F')
                    elif la1 and la1 in u'0123456789':
                        pass
                        self.mDigit(False)
                    else:
                            break
                        
                    _cnt662 += 1
                if _cnt662 < 1:
                    self.raise_NoViableAlt(self.LA(1))
                while True:
                    la1 = self.LA(1)
                    if False:
                        pass
                    elif la1 and la1 in u'Ll':
                        pass
                        self.mLongSuffix(False)
                    elif la1 and la1 in u'Uu':
                        pass
                        self.mUnsignedSuffix(False)
                    else:
                            break
                        
                if not self.inputState.guessing:
                    _ttype = HEXADECIMALINT;
            elif (self.LA(1)==u'.') and (True):
                pass
                self.match('.')
                if not self.inputState.guessing:
                    _ttype = DOT;
                if ((self.LA(1) >= u'0' and self.LA(1) <= u'9')):
                    pass
                    _cnt649= 0
                    while True:
                        if ((self.LA(1) >= u'0' and self.LA(1) <= u'9')):
                            pass
                            self.mDigit(False)
                        else:
                            break
                        
                        _cnt649 += 1
                    if _cnt649 < 1:
                        self.raise_NoViableAlt(self.LA(1))
                    if (self.LA(1)==u'E' or self.LA(1)==u'e'):
                        pass
                        self.mExponent(False)
                    else: ## <m4>
                            pass
                        
                    if not self.inputState.guessing:
                        _ttype = FLOATONE;
                    la1 = self.LA(1)
                    if False:
                        pass
                    elif la1 and la1 in u'Ff':
                        pass
                        self.mFloatSuffix(False)
                    elif la1 and la1 in u'Ll':
                        pass
                        self.mLongSuffix(False)
                    else:
                        ##<m3> <closing
                            pass
                        
                else: ## <m4>
                        pass
                    
            elif (self.LA(1)==u'0') and (True) and (True):
                pass
                self.match('0')
                while True:
                    if ((self.LA(1) >= u'0' and self.LA(1) <= u'7')):
                        pass
                        self.matchRange(u'0', u'7')
                    else:
                        break
                    
                while True:
                    la1 = self.LA(1)
                    if False:
                        pass
                    elif la1 and la1 in u'Ll':
                        pass
                        self.mLongSuffix(False)
                    elif la1 and la1 in u'Uu':
                        pass
                        self.mUnsignedSuffix(False)
                    else:
                            break
                        
                if not self.inputState.guessing:
                    _ttype = OCTALINT;
            elif ((self.LA(1) >= u'1' and self.LA(1) <= u'9')) and (True) and (True):
                pass
                self.matchRange(u'1', u'9')
                while True:
                    if ((self.LA(1) >= u'0' and self.LA(1) <= u'9')):
                        pass
                        self.mDigit(False)
                    else:
                        break
                    
                while True:
                    la1 = self.LA(1)
                    if False:
                        pass
                    elif la1 and la1 in u'Ll':
                        pass
                        self.mLongSuffix(False)
                    elif la1 and la1 in u'Uu':
                        pass
                        self.mUnsignedSuffix(False)
                    else:
                            break
                        
                if not self.inputState.guessing:
                    _ttype = DECIMALINT;
            else:
                self.raise_NoViableAlt(self.LA(1))
            
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mID(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = ID
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
        elif la1 and la1 in u'_':
            pass
            self.match('_')
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
    
    

### generate bit set
def mk_tokenSet_0(): 
    ### var1
    data = [ 4294971904L, 17592186044416L, 0L, 0L, 0L]
    return data
_tokenSet_0 = antlr.BitSet(mk_tokenSet_0())

### generate bit set
def mk_tokenSet_1(): 
    ### var1
    data = [ 288019269919178752L, 0L, 0L, 0L, 0L]
    return data
_tokenSet_1 = antlr.BitSet(mk_tokenSet_1())

### generate bit set
def mk_tokenSet_2(): 
    data = [0L] * 8 ### init list
    data[0] =-4398046520321L
    for x in xrange(1, 4):
        data[x] = -1L
    return data
_tokenSet_2 = antlr.BitSet(mk_tokenSet_2())

### generate bit set
def mk_tokenSet_3(): 
    data = [0L] * 8 ### init list
    data[0] =-9217L
    for x in xrange(1, 4):
        data[x] = -1L
    return data
_tokenSet_3 = antlr.BitSet(mk_tokenSet_3())

### generate bit set
def mk_tokenSet_4(): 
    data = [0L] * 8 ### init list
    data[0] =-4294971905L
    for x in xrange(1, 4):
        data[x] = -1L
    return data
_tokenSet_4 = antlr.BitSet(mk_tokenSet_4())

### generate bit set
def mk_tokenSet_5(): 
    ### var1
    data = [ -9151595350857875456L, 95772161741946880L, 0L, 0L, 0L]
    return data
_tokenSet_5 = antlr.BitSet(mk_tokenSet_5())

### generate bit set
def mk_tokenSet_6(): 
    ### var1
    data = [ 287949450930814976L, 541165879422L, 0L, 0L, 0L]
    return data
_tokenSet_6 = antlr.BitSet(mk_tokenSet_6())

### generate bit set
def mk_tokenSet_7(): 
    data = [0L] * 8 ### init list
    data[0] =-549755813889L
    for x in xrange(1, 4):
        data[x] = -1L
    return data
_tokenSet_7 = antlr.BitSet(mk_tokenSet_7())

### generate bit set
def mk_tokenSet_8(): 
    data = [0L] * 8 ### init list
    data[0] =-17179878401L
    data[1] =-268435457L
    for x in xrange(2, 4):
        data[x] = -1L
    return data
_tokenSet_8 = antlr.BitSet(mk_tokenSet_8())

### generate bit set
def mk_tokenSet_9(): 
    ### var1
    data = [ 288019269919178752L, 137438953504L, 0L, 0L, 0L]
    return data
_tokenSet_9 = antlr.BitSet(mk_tokenSet_9())
    
### __main__ header action >>> 
if __name__ == '__main__' :
    import sys
    import antlr
    import CPPLexer
    
    ### create lexer - shall read from stdin
    try:
        for token in CPPLexer.Lexer():
            print token
            
    except antlr.TokenStreamException, e:
        print "error: exception caught while lexing: ", e
### __main__ header action <<< 
