### $ANTLR 2.7.5 (20050403): "mig.g" -> "MIGLexer.py"$
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
literals[u"replyport"] = 87
literals[u"msgoption"] = 13
literals[u"MACH_MSG_TYPE_BOOLEAN"] = 58
literals[u"ValueOf"] = 43
literals[u"simport"] = 25
literals[u"ureplyport"] = 89
literals[u"dimport"] = 26
literals[u"serverimpl"] = 98
literals[u"MACH_MSG_TYPE_MAKE_SEND_ONCE"] = 49
literals[u"userimpl"] = 99
literals[u"cservertype"] = 38
literals[u"error"] = 16
literals[u"UserTypeLimit"] = 14
literals[u"PointerToIfNot"] = 41
literals[u"sectoken"] = 91
literals[u"outtran"] = 34
literals[u"uimport"] = 24
literals[u"notdealloc"] = 106
literals[u"MACH_MSG_TYPE_MOVE_SEND_ONCE"] = 50
literals[u"MACH_MSG_TYPE_STRING_C"] = 69
literals[u"userprefix"] = 18
literals[u"MACH_MSG_TYPE_INTEGER_32"] = 61
literals[u"waittime"] = 11
literals[u"audittoken"] = 94
literals[u"cusertype"] = 37
literals[u"auto"] = 108
literals[u"serversectoken"] = 92
literals[u"CountInOut"] = 107
literals[u"inout"] = 85
literals[u"MACH_MSG_TYPE_MAKE_SEND"] = 47
literals[u"intran"] = 30
literals[u"import"] = 23
literals[u"requestport"] = 86
literals[u"MACH_MSG_TYPE_COPY_SEND"] = 46
literals[u"MACH_MSG_TYPE_STRING"] = 68
literals[u"MACH_MSG_TYPE_MOVE_RECEIVE"] = 45
literals[u"const"] = 109
literals[u"retcode"] = 101
literals[u"MACH_MSG_TYPE_REAL_64"] = 65
literals[u"array"] = 71
literals[u"MACH_MSG_TYPE_PORT_SEND_ONCE"] = 54
literals[u"MACH_MSG_TYPE_POLYMORPHIC"] = 55
literals[u"MACH_MSG_TYPE_MOVE_SEND"] = 48
literals[u"ctype"] = 36
literals[u"type"] = 28
literals[u"in"] = 83
literals[u"MACH_MSG_TYPE_INTEGER_8"] = 59
literals[u"msgseqno"] = 97
literals[u"kerneluser"] = 7
literals[u"MACH_MSG_TYPE_PORT_SEND"] = 53
literals[u"MACH_MSG_TYPE_INTEGER_16"] = 60
literals[u"dealloc"] = 105
literals[u"physicalcopy"] = 102
literals[u"subsystem"] = 6
literals[u"simpleroutine"] = 82
literals[u"MACH_MSG_TYPE_CHAR"] = 66
literals[u"MACH_MSG_TYPE_BIT"] = 57
literals[u"routine"] = 81
literals[u"iimport"] = 27
literals[u"polymorphic"] = 44
literals[u"samecount"] = 100
literals[u"MACH_MSG_TYPE_UNSTRUCTURED"] = 56
literals[u"MACH_MSG_TYPE_REAL"] = 63
literals[u"MACH_MSG_TYPE_INTEGER_64"] = 62
literals[u"kernelserver"] = 8
literals[u"OnStackLimit"] = 15
literals[u"skip"] = 5
literals[u"out"] = 84
literals[u"MACH_MSG_TYPE_BYTE"] = 67
literals[u"PointerTo"] = 40
literals[u"MACH_MSG_TYPE_REAL_32"] = 64
literals[u"MACH_MSG_TYPE_PORT_RECEIVE"] = 52
literals[u"overwrite"] = 104
literals[u"sreplyport"] = 88
literals[u"c_string"] = 77
literals[u"MACH_MSG_TYPE_PORT_NAME"] = 51
literals[u"usersectoken"] = 93
literals[u"serverdemux"] = 19
literals[u"useraudittoken"] = 96
literals[u"struct"] = 76
literals[u"serveraudittoken"] = 95
literals[u"sendtime"] = 90
literals[u"servercopy"] = 103
literals[u"destructor"] = 35
literals[u"serverprefix"] = 17


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
LITERAL_skip = 5
LITERAL_subsystem = 6
LITERAL_kerneluser = 7
LITERAL_kernelserver = 8
IDENT = 9
NUMBER = 10
LITERAL_waittime = 11
STRING = 12
LITERAL_msgoption = 13
LITERAL_UserTypeLimit = 14
LITERAL_OnStackLimit = 15
LITERAL_error = 16
LITERAL_serverprefix = 17
LITERAL_userprefix = 18
LITERAL_serverdemux = 19
QUOTEDSTRING = 20
ANGLEQUOTEDSTRING = 21
RCSDECL = 22
LITERAL_import = 23
LITERAL_uimport = 24
LITERAL_simport = 25
LITERAL_dimport = 26
LITERAL_iimport = 27
LITERAL_type = 28
EQUAL = 29
LITERAL_intran = 30
COLON = 31
LPAREN = 32
RPAREN = 33
LITERAL_outtran = 34
LITERAL_destructor = 35
LITERAL_ctype = 36
LITERAL_cusertype = 37
LITERAL_cservertype = 38
CARET = 39
LITERAL_PointerTo = 40
LITERAL_PointerToIfNot = 41
COMMA = 42
LITERAL_ValueOf = 43
LITERAL_polymorphic = 44
LITERAL_MACH_MSG_TYPE_MOVE_RECEIVE = 45
LITERAL_MACH_MSG_TYPE_COPY_SEND = 46
LITERAL_MACH_MSG_TYPE_MAKE_SEND = 47
LITERAL_MACH_MSG_TYPE_MOVE_SEND = 48
LITERAL_MACH_MSG_TYPE_MAKE_SEND_ONCE = 49
LITERAL_MACH_MSG_TYPE_MOVE_SEND_ONCE = 50
LITERAL_MACH_MSG_TYPE_PORT_NAME = 51
LITERAL_MACH_MSG_TYPE_PORT_RECEIVE = 52
LITERAL_MACH_MSG_TYPE_PORT_SEND = 53
LITERAL_MACH_MSG_TYPE_PORT_SEND_ONCE = 54
LITERAL_MACH_MSG_TYPE_POLYMORPHIC = 55
LITERAL_MACH_MSG_TYPE_UNSTRUCTURED = 56
LITERAL_MACH_MSG_TYPE_BIT = 57
LITERAL_MACH_MSG_TYPE_BOOLEAN = 58
### "MACH_MSG_TYPE_INTEGER_8" = 59
### "MACH_MSG_TYPE_INTEGER_16" = 60
### "MACH_MSG_TYPE_INTEGER_32" = 61
### "MACH_MSG_TYPE_INTEGER_64" = 62
LITERAL_MACH_MSG_TYPE_REAL = 63
### "MACH_MSG_TYPE_REAL_32" = 64
### "MACH_MSG_TYPE_REAL_64" = 65
LITERAL_MACH_MSG_TYPE_CHAR = 66
LITERAL_MACH_MSG_TYPE_BYTE = 67
LITERAL_MACH_MSG_TYPE_STRING = 68
LITERAL_MACH_MSG_TYPE_STRING_C = 69
BAR = 70
LITERAL_array = 71
LBRACK = 72
RBRACK = 73
OF = 74
STAR = 75
LITERAL_struct = 76
LITERAL_c_string = 77
PLUS = 78
MINUS = 79
DIV = 80
LITERAL_routine = 81
LITERAL_simpleroutine = 82
LITERAL_in = 83
LITERAL_out = 84
LITERAL_inout = 85
LITERAL_requestport = 86
LITERAL_replyport = 87
LITERAL_sreplyport = 88
LITERAL_ureplyport = 89
LITERAL_sendtime = 90
LITERAL_sectoken = 91
LITERAL_serversectoken = 92
LITERAL_usersectoken = 93
LITERAL_audittoken = 94
LITERAL_serveraudittoken = 95
LITERAL_useraudittoken = 96
LITERAL_msgseqno = 97
LITERAL_serverimpl = 98
LITERAL_userimpl = 99
LITERAL_samecount = 100
LITERAL_retcode = 101
LITERAL_physicalcopy = 102
LITERAL_servercopy = 103
LITERAL_overwrite = 104
LITERAL_dealloc = 105
LITERAL_notdealloc = 106
LITERAL_CountInOut = 107
LITERAL_auto = 108
LITERAL_const = 109
TILDE = 110
LANGLE = 111
RANGLE = 112
LETTER = 113
DIGIT = 114
WS = 115
COMMENT = 116

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
                            elif la1 and la1 in u':':
                                pass
                                self.mCOLON(True)
                                theRetToken = self._returnToken
                            elif la1 and la1 in u';':
                                pass
                                self.mSEMI(True)
                                theRetToken = self._returnToken
                            elif la1 and la1 in u',':
                                pass
                                self.mCOMMA(True)
                                theRetToken = self._returnToken
                            elif la1 and la1 in u'+':
                                pass
                                self.mPLUS(True)
                                theRetToken = self._returnToken
                            elif la1 and la1 in u'-':
                                pass
                                self.mMINUS(True)
                                theRetToken = self._returnToken
                            elif la1 and la1 in u'*':
                                pass
                                self.mSTAR(True)
                                theRetToken = self._returnToken
                            elif la1 and la1 in u'(':
                                pass
                                self.mLPAREN(True)
                                theRetToken = self._returnToken
                            elif la1 and la1 in u')':
                                pass
                                self.mRPAREN(True)
                                theRetToken = self._returnToken
                            elif la1 and la1 in u'=':
                                pass
                                self.mEQUAL(True)
                                theRetToken = self._returnToken
                            elif la1 and la1 in u'^':
                                pass
                                self.mCARET(True)
                                theRetToken = self._returnToken
                            elif la1 and la1 in u'~':
                                pass
                                self.mTILDE(True)
                                theRetToken = self._returnToken
                            elif la1 and la1 in u'>':
                                pass
                                self.mRANGLE(True)
                                theRetToken = self._returnToken
                            elif la1 and la1 in u'[':
                                pass
                                self.mLBRACK(True)
                                theRetToken = self._returnToken
                            elif la1 and la1 in u']':
                                pass
                                self.mRBRACK(True)
                                theRetToken = self._returnToken
                            elif la1 and la1 in u'|':
                                pass
                                self.mBAR(True)
                                theRetToken = self._returnToken
                            elif la1 and la1 in u'0123456789':
                                pass
                                self.mNUMBER(True)
                                theRetToken = self._returnToken
                            elif la1 and la1 in u'"':
                                pass
                                self.mQUOTEDSTRING(True)
                                theRetToken = self._returnToken
                            elif la1 and la1 in u'\t\n\r ':
                                pass
                                self.mWS(True)
                                theRetToken = self._returnToken
                            else:
                                if (self.LA(1)==u'o') and (self.LA(2)==u'f'):
                                    pass
                                    self.mOF(True)
                                    theRetToken = self._returnToken
                                elif (self.LA(1)==u'<') and (_tokenSet_0.member(self.LA(2))):
                                    pass
                                    self.mANGLEQUOTEDSTRING(True)
                                    theRetToken = self._returnToken
                                elif (self.LA(1)==u'/') and (self.LA(2)==u'*'):
                                    pass
                                    self.mCOMMENT(True)
                                    theRetToken = self._returnToken
                                elif (self.LA(1)==u'/') and (True):
                                    pass
                                    self.mDIV(True)
                                    theRetToken = self._returnToken
                                elif (self.LA(1)==u'<') and (True):
                                    pass
                                    self.mLANGLE(True)
                                    theRetToken = self._returnToken
                                elif (_tokenSet_1.member(self.LA(1))) and (True):
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
        
    def mCOLON(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = COLON
        _saveIndex = 0
        pass
        self.match(':')
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mSEMI(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = SEMI
        _saveIndex = 0
        pass
        self.match(';')
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
    
    def mSTAR(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = STAR
        _saveIndex = 0
        pass
        self.match('*')
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
    
    def mEQUAL(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = EQUAL
        _saveIndex = 0
        pass
        self.match('=')
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mCARET(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = CARET
        _saveIndex = 0
        pass
        self.match('^')
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
    
    def mLANGLE(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = LANGLE
        _saveIndex = 0
        pass
        self.match('<')
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mRANGLE(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = RANGLE
        _saveIndex = 0
        pass
        self.match('>')
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
    
    def mBAR(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = BAR
        _saveIndex = 0
        pass
        self.match('|')
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mOF(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = OF
        _saveIndex = 0
        pass
        self.match("of")
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
        elif la1 and la1 in u'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz':
            pass
            self.mLETTER(False)
        elif la1 and la1 in u'_':
            pass
            self.match('_')
        else:
                self.raise_NoViableAlt(self.LA(1))
            
        while True:
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in u'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz':
                pass
                self.mLETTER(False)
            elif la1 and la1 in u'0123456789':
                pass
                self.mDIGIT(False)
            elif la1 and la1 in u'_':
                pass
                self.match('_')
            else:
                    break
                
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mLETTER(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = LETTER
        _saveIndex = 0
        la1 = self.LA(1)
        if False:
            pass
        elif la1 and la1 in u'ABCDEFGHIJKLMNOPQRSTUVWXYZ':
            pass
            pass
            self.matchRange(u'A', u'Z')
        elif la1 and la1 in u'abcdefghijklmnopqrstuvwxyz':
            pass
            pass
            self.matchRange(u'a', u'z')
        else:
                self.raise_NoViableAlt(self.LA(1))
            
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
    
    def mNUMBER(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = NUMBER
        _saveIndex = 0
        pass
        _cnt97= 0
        while True:
            if ((self.LA(1) >= u'0' and self.LA(1) <= u'9')):
                pass
                self.mDIGIT(False)
            else:
                break
            
            _cnt97 += 1
        if _cnt97 < 1:
            self.raise_NoViableAlt(self.LA(1))
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mQUOTEDSTRING(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = QUOTEDSTRING
        _saveIndex = 0
        pass
        pass
        self.match('"')
        _cnt105= 0
        while True:
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in u'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz':
                pass
                self.mLETTER(False)
            elif la1 and la1 in u'-':
                pass
                self.match('-')
            elif la1 and la1 in u'0123456789':
                pass
                self.mDIGIT(False)
            elif la1 and la1 in u'.':
                pass
                self.match('.')
            elif la1 and la1 in u'_':
                pass
                self.match('_')
            elif la1 and la1 in u'/':
                pass
                self.match('/')
            else:
                    break
                
            _cnt105 += 1
        if _cnt105 < 1:
            self.raise_NoViableAlt(self.LA(1))
        self.match('"')
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mANGLEQUOTEDSTRING(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = ANGLEQUOTEDSTRING
        _saveIndex = 0
        pass
        pass
        self.match('<')
        _cnt109= 0
        while True:
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in u'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz':
                pass
                self.mLETTER(False)
            elif la1 and la1 in u'-':
                pass
                self.match('-')
            elif la1 and la1 in u'0123456789':
                pass
                self.mDIGIT(False)
            elif la1 and la1 in u'.':
                pass
                self.match('.')
            elif la1 and la1 in u'_':
                pass
                self.match('_')
            elif la1 and la1 in u'/':
                pass
                self.match('/')
            else:
                    break
                
            _cnt109 += 1
        if _cnt109 < 1:
            self.raise_NoViableAlt(self.LA(1))
        self.match('>')
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
        elif la1 and la1 in u'\r':
            pass
            self.match('\r')
            self.match('\n')
        elif la1 and la1 in u'\n':
            pass
            self.match('\n')
        elif la1 and la1 in u'\t':
            pass
            self.match('\t')
        else:
                self.raise_NoViableAlt(self.LA(1))
            
        _ttype = Token.SKIP;
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mCOMMENT(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = COMMENT
        _saveIndex = 0
        pass
        self.match("/*")
        while True:
            ###  nongreedy exit test
            if ((self.LA(1)==u'*') and (self.LA(2)==u'/')):
                break
            if ((self.LA(1) >= u'\u0000' and self.LA(1) <= u'\u007f')) and ((self.LA(2) >= u'\u0000' and self.LA(2) <= u'\u007f')):
                pass
                self.matchNot(antlr.EOF_CHAR)
            else:
                break
            
        self.match("*/")
        _ttype = Token.SKIP;
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    

### generate bit set
def mk_tokenSet_0(): 
    ### var1
    data = [ 288195191779622912L, 576460745995190270L, 0L, 0L]
    return data
_tokenSet_0 = antlr.BitSet(mk_tokenSet_0())

### generate bit set
def mk_tokenSet_1(): 
    ### var1
    data = [ 0L, 576460745995190270L, 0L, 0L]
    return data
_tokenSet_1 = antlr.BitSet(mk_tokenSet_1())
    
### __main__ header action >>> 
if __name__ == '__main__' :
    import sys
    import antlr
    import MIGLexer
    
    ### create lexer - shall read from stdin
    try:
        for token in MIGLexer.Lexer():
            print token
            
    except antlr.TokenStreamException, e:
        print "error: exception caught while lexing: ", e
### __main__ header action <<< 
