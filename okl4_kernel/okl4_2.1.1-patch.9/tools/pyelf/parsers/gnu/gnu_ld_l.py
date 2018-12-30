### $ANTLR 2.7.6 (20060903): "gnu_ld_l.g" -> "gnu_ld_l.py"$
### import antlr and other modules ..
import sys
import antlr

version = sys.version.split()[0]
if version < '2.2.1':
    False = 0
if version < '2.3':
    True = not False
### header action >>> 
# Copyright (c) 2007 Open Kernel Labs, Inc. (Copyright Holder).
# All rights reserved.
# 
# 1. Redistribution and use of OKL4 (Software) in source and binary
# forms, with or without modification, are permitted provided that the
# following conditions are met:
# 
#     (a) Redistributions of source code must retain this clause 1
#         (including paragraphs (a), (b) and (c)), clause 2 and clause 3
#         (Licence Terms) and the above copyright notice.
# 
#     (b) Redistributions in binary form must reproduce the above
#         copyright notice and the Licence Terms in the documentation and/or
#         other materials provided with the distribution.
# 
#     (c) Redistributions in any form must be accompanied by information on
#         how to obtain complete source code for:
#        (i) the Software; and
#        (ii) all accompanying software that uses (or is intended to
#        use) the Software whether directly or indirectly.  Such source
#        code must:
#        (iii) either be included in the distribution or be available
#        for no more than the cost of distribution plus a nominal fee;
#        and
#        (iv) be licensed by each relevant holder of copyright under
#        either the Licence Terms (with an appropriate copyright notice)
#        or the terms of a licence which is approved by the Open Source
#        Initative.  For an executable file, "complete source code"
#        means the source code for all modules it contains and includes
#        associated build and other files reasonably required to produce
#        the executable.
# 
# 2. THIS SOFTWARE IS PROVIDED ``AS IS'' AND, TO THE EXTENT PERMITTED BY
# LAW, ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
# THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
# PURPOSE, OR NON-INFRINGEMENT, ARE DISCLAIMED.  WHERE ANY WARRANTY IS
# IMPLIED AND IS PREVENTED BY LAW FROM BEING DISCLAIMED THEN TO THE
# EXTENT PERMISSIBLE BY LAW: (A) THE WARRANTY IS READ DOWN IN FAVOUR OF
# THE COPYRIGHT HOLDER (AND, IN THE CASE OF A PARTICIPANT, THAT
# PARTICIPANT) AND (B) ANY LIMITATIONS PERMITTED BY LAW (INCLUDING AS TO
# THE EXTENT OF THE WARRANTY AND THE REMEDIES AVAILABLE IN THE EVENT OF
# BREACH) ARE DEEMED PART OF THIS LICENCE IN A FORM MOST FAVOURABLE TO
# THE COPYRIGHT HOLDER (AND, IN THE CASE OF A PARTICIPANT, THAT
# PARTICIPANT). IN THE LICENCE TERMS, "PARTICIPANT" INCLUDES EVERY
# PERSON WHO HAS CONTRIBUTED TO THE SOFTWARE OR WHO HAS BEEN INVOLVED IN
# THE DISTRIBUTION OR DISSEMINATION OF THE SOFTWARE.
# 
# 3. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR ANY OTHER PARTICIPANT BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
# BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
# OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
# IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
### header action <<< 
### preamble action >>> 

### preamble action <<< 
### >>>The Literals<<<
literals = {}
literals[u"ENTRY"] = 9
literals[u"MIN"] = 21
literals[u"BLOCK"] = 8
literals[u"OUTPUT_ARCH"] = 25
literals[u"QUAD"] = 39
literals[u"CONSTANT"] = 22
literals[u"DATA_SEGMENT_RELRO_END"] = 35
literals[u"INPUT"] = 27
literals[u"PHDRS"] = 5
literals[u"FILL"] = 41
literals[u"SQUAD"] = 40
literals[u"BYTE"] = 36
literals[u"STARTUP"] = 30
literals[u"GROUP"] = 29
literals[u"DATA_SEGMENT_END"] = 34
literals[u"DATA_SEGMENT_ALIGN"] = 33
literals[u"NEXT"] = 18
literals[u"PROVIDE"] = 10
literals[u"ABSOLUTE"] = 13
literals[u"SHORT"] = 37
literals[u"TARGET"] = 31
literals[u"LONG"] = 38
literals[u"DEFINED"] = 17
literals[u"OVERLAY"] = 7
literals[u"PROVIDE_HIDDEN"] = 11
literals[u"SEARCH_DIR"] = 26
literals[u"AT"] = 12
literals[u"SECTIONS"] = 4
literals[u"ADDR"] = 14
literals[u"MAX"] = 20
literals[u"ALIGN"] = 16
literals[u"NOCROSSREFS"] = 32
literals[u"SIZEOF"] = 19
literals[u"OUTPUT_FORMAT"] = 23
literals[u"MEMORY"] = 6
literals[u"ASSERT"] = 24
literals[u"OUTPUT"] = 28
literals[u"LOADADDR"] = 15


### import antlr.Token 
from antlr import Token
### >>>The Known Token Types <<<
SKIP                = antlr.SKIP
INVALID_TYPE        = antlr.INVALID_TYPE
EOF_TYPE            = antlr.EOF_TYPE
EOF                 = antlr.EOF
NULL_TREE_LOOKAHEAD = antlr.NULL_TREE_LOOKAHEAD
MIN_USER_TYPE       = antlr.MIN_USER_TYPE
TOKEN_SECTIONS = 4
TOKEN_PHDRS = 5
TOKEN_MEMORY = 6
TOKEN_OVERLAY = 7
TOKEN_BLOCK = 8
TOKEN_ENTRY = 9
TOKEN_PROVIDE = 10
TOKEN_PROVIDE_HIDDEN = 11
TOKEN_AT = 12
TOKEN_ABSOLUTE = 13
TOKEN_ADDR = 14
TOKEN_LOADADDR = 15
TOKEN_ALIGN = 16
TOKEN_DEFINED = 17
TOKEN_NEXT = 18
TOKEN_SIZEOF = 19
TOKEN_MAX = 20
TOKEN_MIN = 21
TOKEN_CONSTANT = 22
TOKEN_OUTPUT_FORMAT = 23
TOKEN_ASSERT = 24
TOKEN_OUTPUT_ARCH = 25
TOKEN_SEARCH_DIR = 26
TOKEN_INPUT = 27
TOKEN_OUTPUT = 28
TOKEN_GROUP = 29
TOKEN_STARTUP = 30
TOKEN_TARGET = 31
TOKEN_NOCROSSREFS = 32
TOKEN_DATA_SEGMENT_ALIGN = 33
TOKEN_DATA_SEGMENT_END = 34
TOKEN_DATA_SEGMENT_RELRO_END = 35
TOKEN_BYTE = 36
TOKEN_SHORT = 37
TOKEN_LONG = 38
TOKEN_QUAD = 39
TOKEN_SQUAD = 40
TOKEN_FILL = 41
WS = 42
ML_COMMENT = 43
DISCARD = 44
NOLOAD = 45
LPAREN = 46
RPAREN = 47
LCURLY = 48
RCURLY = 49
COMMA = 50
SEMICOLON = 51
COLON = 52
QMARK = 53
EQUALS = 54
OP_STAR = 55
OP_MINUS = 56
OP_OTHERS = 57
OP_AND = 58
OP_OR = 59
OP_EQ = 60
OPERATOR = 61
OP_ASSIGN = 62
DIGIT = 63
IDENT_FIRSTCHAR = 64
IDENT_CHAR = 65
IDENT = 66
INT = 67
HEX = 68

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
                            elif la1 and la1 in u'\t\n\r ':
                                pass
                                self.mWS(True)
                                theRetToken = self._returnToken
                            elif la1 and la1 in u')':
                                pass
                                self.mRPAREN(True)
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
                            elif la1 and la1 in u';':
                                pass
                                self.mSEMICOLON(True)
                                theRetToken = self._returnToken
                            elif la1 and la1 in u':':
                                pass
                                self.mCOLON(True)
                                theRetToken = self._returnToken
                            elif la1 and la1 in u'?':
                                pass
                                self.mQMARK(True)
                                theRetToken = self._returnToken
                            elif la1 and la1 in u'".ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz':
                                pass
                                self.mIDENT(True)
                                theRetToken = self._returnToken
                            else:
                                if (self.LA(1)==u'/') and (self.LA(2)==u'*'):
                                    pass
                                    self.mML_COMMENT(True)
                                    theRetToken = self._returnToken
                                elif (self.LA(1)==u'/') and (self.LA(2)==u'D'):
                                    pass
                                    self.mDISCARD(True)
                                    theRetToken = self._returnToken
                                elif (self.LA(1)==u'(') and (self.LA(2)==u'N'):
                                    pass
                                    self.mNOLOAD(True)
                                    theRetToken = self._returnToken
                                elif (_tokenSet_0.member(self.LA(1))) and (self.LA(2)==u'='):
                                    pass
                                    self.mOP_ASSIGN(True)
                                    theRetToken = self._returnToken
                                elif (self.LA(1)==u'0') and (self.LA(2)==u'X' or self.LA(2)==u'x'):
                                    pass
                                    self.mHEX(True)
                                    theRetToken = self._returnToken
                                elif (self.LA(1)==u'(') and (True):
                                    pass
                                    self.mLPAREN(True)
                                    theRetToken = self._returnToken
                                elif ((self.LA(1)==u'=') and (True) and (True) and (True) and (True) and (self.LA(2) != '=')):
                                    pass
                                    self.mEQUALS(True)
                                    theRetToken = self._returnToken
                                elif ((self.LA(1)==u'*') and (True) and (self.LA(2) != '=')):
                                    pass
                                    self.mOP_STAR(True)
                                    theRetToken = self._returnToken
                                elif ((self.LA(1)==u'-') and (True) and (self.LA(2) != '=')):
                                    pass
                                    self.mOP_MINUS(True)
                                    theRetToken = self._returnToken
                                elif ((self.LA(1)==u'%' or self.LA(1)==u'+' or self.LA(1)==u'/') and (True) and (self.LA(2) != '=')):
                                    pass
                                    self.mOP_OTHERS(True)
                                    theRetToken = self._returnToken
                                elif ((self.LA(1)==u'&') and (True) and (self.LA(2) != '=')):
                                    pass
                                    self.mOP_AND(True)
                                    theRetToken = self._returnToken
                                elif ((self.LA(1)==u'|') and (True) and (self.LA(2) != '=')):
                                    pass
                                    self.mOP_OR(True)
                                    theRetToken = self._returnToken
                                elif (_tokenSet_1.member(self.LA(1))) and (True) and (True) and (True) and (True):
                                    pass
                                    self.mOPERATOR(True)
                                    theRetToken = self._returnToken
                                elif ((self.LA(1) >= u'0' and self.LA(1) <= u'9')) and (True):
                                    pass
                                    self.mINT(True)
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
        elif la1 and la1 in u'\r':
            pass
            self.match('\r')
        else:
                self.raise_NoViableAlt(self.LA(1))
            
        _ttype = SKIP;
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mML_COMMENT(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = ML_COMMENT
        _saveIndex = 0
        pass
        self.match("/*")
        while True:
            if (self.LA(1)==u'\r') and (self.LA(2)==u'\n') and ((self.LA(3) >= u'\u0003' and self.LA(3) <= u'\u00ff')) and ((self.LA(4) >= u'\u0003' and self.LA(4) <= u'\u00ff')) and (True):
                pass
                self.match('\r')
                self.match('\n')
                self.newline();
            elif ((self.LA(1)==u'*') and ((self.LA(2) >= u'\u0003' and self.LA(2) <= u'\u00ff')) and ((self.LA(3) >= u'\u0003' and self.LA(3) <= u'\u00ff')) and ( self.LA(2)!='/' )):
                pass
                self.match('*')
            elif (self.LA(1)==u'\r') and ((self.LA(2) >= u'\u0003' and self.LA(2) <= u'\u00ff')) and ((self.LA(3) >= u'\u0003' and self.LA(3) <= u'\u00ff')) and (True) and (True):
                pass
                self.match('\r')
                self.newline();
            elif (self.LA(1)==u'\n'):
                pass
                self.match('\n')
                self.newline();
            elif (_tokenSet_2.member(self.LA(1))):
                pass
                self.match(_tokenSet_2)
            else:
                break
            
        self.match("*/")
        _ttype = Token.SKIP;
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mDISCARD(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = DISCARD
        _saveIndex = 0
        pass
        self.match("/DISCARD/")
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mNOLOAD(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = NOLOAD
        _saveIndex = 0
        pass
        self.match("(NOLOAD)")
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
    
    def mSEMICOLON(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = SEMICOLON
        _saveIndex = 0
        pass
        self.match(';')
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
    
    def mQMARK(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = QMARK
        _saveIndex = 0
        pass
        self.match('?')
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mEQUALS(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = EQUALS
        _saveIndex = 0
        if not self.LA(2) != '=':
            raise antlr.SemanticException("self.LA(2) != '='")
        pass
        self.match('=')
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mOP_STAR(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = OP_STAR
        _saveIndex = 0
        if not self.LA(2) != '=':
            raise antlr.SemanticException("self.LA(2) != '='")
        pass
        self.match('*')
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mOP_MINUS(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = OP_MINUS
        _saveIndex = 0
        if not self.LA(2) != '=':
            raise antlr.SemanticException("self.LA(2) != '='")
        pass
        self.match('-')
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mOP_OTHERS(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = OP_OTHERS
        _saveIndex = 0
        if not self.LA(2) != '=':
            raise antlr.SemanticException("self.LA(2) != '='")
        pass
        la1 = self.LA(1)
        if False:
            pass
        elif la1 and la1 in u'+':
            pass
            self.match('+')
        elif la1 and la1 in u'/':
            pass
            self.match('/')
        elif la1 and la1 in u'%':
            pass
            self.match('%')
        else:
                self.raise_NoViableAlt(self.LA(1))
            
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mOP_AND(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = OP_AND
        _saveIndex = 0
        if not self.LA(2) != '=':
            raise antlr.SemanticException("self.LA(2) != '='")
        pass
        self.match('&')
        if (self.LA(1)==u'&'):
            pass
            self.match('&')
        else: ## <m4>
                pass
            
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mOP_OR(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = OP_OR
        _saveIndex = 0
        if not self.LA(2) != '=':
            raise antlr.SemanticException("self.LA(2) != '='")
        pass
        self.match('|')
        if (self.LA(1)==u'|'):
            pass
            self.match('|')
        else: ## <m4>
                pass
            
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mOP_EQ(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = OP_EQ
        _saveIndex = 0
        pass
        la1 = self.LA(1)
        if False:
            pass
        elif la1 and la1 in u'!':
            pass
            self.match('!')
        elif la1 and la1 in u'~':
            pass
            self.match('~')
        elif la1 and la1 in u'<':
            pass
            self.match('<')
        elif la1 and la1 in u'>':
            pass
            self.match('>')
        else:
                self.raise_NoViableAlt(self.LA(1))
            
        if (self.LA(1)==u'='):
            pass
            self.match('=')
        else: ## <m4>
                pass
            
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mOPERATOR(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = OPERATOR
        _saveIndex = 0
        if (self.LA(1)==u'<') and (self.LA(2)==u'<'):
            pass
            self.match("<<")
        elif (self.LA(1)==u'>') and (self.LA(2)==u'>'):
            pass
            self.match(">>")
        elif (_tokenSet_3.member(self.LA(1))) and (True):
            pass
            self.mOP_EQ(False)
        elif (self.LA(1)==u'='):
            pass
            self.match("==")
        else:
            self.raise_NoViableAlt(self.LA(1))
        
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mOP_ASSIGN(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = OP_ASSIGN
        _saveIndex = 0
        la1 = self.LA(1)
        if False:
            pass
        elif la1 and la1 in u'*':
            pass
            self.match("*=")
        elif la1 and la1 in u'+':
            pass
            self.match("+=")
        elif la1 and la1 in u'/':
            pass
            self.match("/=")
        elif la1 and la1 in u'-':
            pass
            self.match("-=")
        elif la1 and la1 in u'&':
            pass
            self.match("&=")
        elif la1 and la1 in u'|':
            pass
            self.match("|=")
        elif la1 and la1 in u'%':
            pass
            self.match("%=")
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
    
    def mIDENT_FIRSTCHAR(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = IDENT_FIRSTCHAR
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
        elif la1 and la1 in u'.':
            pass
            self.match('.')
        else:
                self.raise_NoViableAlt(self.LA(1))
            
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mIDENT_CHAR(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = IDENT_CHAR
        _saveIndex = 0
        pass
        la1 = self.LA(1)
        if False:
            pass
        elif la1 and la1 in u'.ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz':
            pass
            self.mIDENT_FIRSTCHAR(False)
        elif la1 and la1 in u'0123456789':
            pass
            self.mDIGIT(False)
        elif la1 and la1 in u'*':
            pass
            self.match('*')
        elif la1 and la1 in u'-':
            pass
            self.match('-')
        elif la1 and la1 in u'?':
            pass
            self.match('?')
        else:
                self.raise_NoViableAlt(self.LA(1))
            
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
        elif la1 and la1 in u'"':
            pass
            pass
            self.match('"')
            while True:
                if (_tokenSet_4.member(self.LA(1))):
                    pass
                    self.match(_tokenSet_4)
                else:
                    break
                
            self.match('"')
        elif la1 and la1 in u'.ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz':
            pass
            pass
            self.mIDENT_FIRSTCHAR(False)
            while True:
                if (_tokenSet_5.member(self.LA(1))):
                    pass
                    self.mIDENT_CHAR(False)
                else:
                    break
                
        else:
                self.raise_NoViableAlt(self.LA(1))
            
        ### option { testLiterals=true } 
        _ttype = self.testLiteralsTable(_ttype)
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mINT(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = INT
        _saveIndex = 0
        pass
        _cnt47= 0
        while True:
            if ((self.LA(1) >= u'0' and self.LA(1) <= u'9')):
                pass
                self.mDIGIT(False)
            else:
                break
            
            _cnt47 += 1
        if _cnt47 < 1:
            self.raise_NoViableAlt(self.LA(1))
        la1 = self.LA(1)
        if False:
            pass
        elif la1 and la1 in u'K':
            pass
            self.match('K')
        elif la1 and la1 in u'M':
            pass
            self.match('M')
        elif la1 and la1 in u'k':
            pass
            self.match('k')
        elif la1 and la1 in u'm':
            pass
            self.match('m')
        else:
            ##<m3> <closing
                pass
            
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
        
        _cnt52= 0
        while True:
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
                    break
                
            _cnt52 += 1
        if _cnt52 < 1:
            self.raise_NoViableAlt(self.LA(1))
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    

### generate bit set
def mk_tokenSet_0(): 
    ### var1
    data = [ 189528316837888L, 1152921504606846976L, 0L, 0L, 0L]
    return data
_tokenSet_0 = antlr.BitSet(mk_tokenSet_0())

### generate bit set
def mk_tokenSet_1(): 
    ### var1
    data = [ 8070450540837863424L, 4611686018427387904L, 0L, 0L, 0L]
    return data
_tokenSet_1 = antlr.BitSet(mk_tokenSet_1())

### generate bit set
def mk_tokenSet_2(): 
    data = [0L] * 8 ### init list
    data[0] =-4398046520328L
    for x in xrange(1, 4):
        data[x] = -1L
    return data
_tokenSet_2 = antlr.BitSet(mk_tokenSet_2())

### generate bit set
def mk_tokenSet_3(): 
    ### var1
    data = [ 5764607531624169472L, 4611686018427387904L, 0L, 0L, 0L]
    return data
_tokenSet_3 = antlr.BitSet(mk_tokenSet_3())

### generate bit set
def mk_tokenSet_4(): 
    data = [0L] * 8 ### init list
    data[0] =-17179869192L
    for x in xrange(1, 4):
        data[x] = -1L
    return data
_tokenSet_4 = antlr.BitSet(mk_tokenSet_4())

### generate bit set
def mk_tokenSet_5(): 
    ### var1
    data = [ -8935313184516997120L, 576460745995190270L, 0L, 0L, 0L]
    return data
_tokenSet_5 = antlr.BitSet(mk_tokenSet_5())
    
### __main__ header action >>> 
if __name__ == '__main__' :
    import sys
    import antlr
    import gnu_ld_l
    
    ### create lexer - shall read from stdin
    try:
        for token in gnu_ld_l.Lexer():
            print token
            
    except antlr.TokenStreamException, e:
        print "error: exception caught while lexing: ", e
### __main__ header action <<< 
