### $ANTLR 2.7.7 (20070817): "rvct_ld_l.g" -> "rvct_ld_l.py"$
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
literals[u"LAST"] = 18
literals[u"RO-DATA"] = 6
literals[u"RW-DATA"] = 9
literals[u".ANY"] = 25
literals[u"OVERLAY"] = 23
literals[u"DATA"] = 13
literals[u"RO"] = 7
literals[u"EMPTY"] = 24
literals[u"ZI"] = 14
literals[u"RELOC"] = 22
literals[u"ABSOLUTE"] = 20
literals[u"PI"] = 21
literals[u"FIRST"] = 17
literals[u"CONST"] = 10
literals[u"RO-CODE"] = 4
literals[u"RW-CODE"] = 11
literals[u"RW"] = 12
literals[u"BSS"] = 15
literals[u"NOCOMPRESS"] = 19
literals[u"CODE"] = 5
literals[u"TEXT"] = 8
literals[u"ENTRY"] = 16


### import antlr.Token 
from antlr import Token
### >>>The Known Token Types <<<
SKIP                = antlr.SKIP
INVALID_TYPE        = antlr.INVALID_TYPE
EOF_TYPE            = antlr.EOF_TYPE
EOF                 = antlr.EOF
NULL_TREE_LOOKAHEAD = antlr.NULL_TREE_LOOKAHEAD
MIN_USER_TYPE       = antlr.MIN_USER_TYPE
RO_CODE = 4
TOKEN_CODE = 5
RO_DATA = 6
TOKEN_RO = 7
TOKEN_TEXT = 8
RW_DATA = 9
TOKEN_CONST = 10
RW_CODE = 11
TOKEN_RW = 12
TOKEN_DATA = 13
TOKEN_ZI = 14
TOKEN_BSS = 15
TOKEN_ENTRY = 16
TOKEN_FIRST = 17
TOKEN_LAST = 18
TOKEN_NOCOMPRESS = 19
TOKEN_ABSOLUTE = 20
TOKEN_PI = 21
TOKEN_RELOC = 22
TOKEN_OVERLAY = 23
TOKEN_EMPTY = 24
ANY = 25
WS = 26
LPAREN = 27
RPAREN = 28
LCURLY = 29
RCURLY = 30
PLUS = 31
MINUS = 32
COMMA = 33
DIGIT = 34
INT = 35
HEX = 36
IDENT = 37
SL_PREPOCESSOR_DIRECTIVE = 38

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
                            elif la1 and la1 in u'(':
                                pass
                                self.mLPAREN(True)
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
                            elif la1 and la1 in u'+':
                                pass
                                self.mPLUS(True)
                                theRetToken = self._returnToken
                            elif la1 and la1 in u'-':
                                pass
                                self.mMINUS(True)
                                theRetToken = self._returnToken
                            elif la1 and la1 in u',':
                                pass
                                self.mCOMMA(True)
                                theRetToken = self._returnToken
                            elif la1 and la1 in u'*.ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz':
                                pass
                                self.mIDENT(True)
                                theRetToken = self._returnToken
                            elif la1 and la1 in u'#':
                                pass
                                self.mSL_PREPOCESSOR_DIRECTIVE(True)
                                theRetToken = self._returnToken
                            else:
                                if (self.LA(1)==u'0') and (self.LA(2)==u'X' or self.LA(2)==u'x'):
                                    pass
                                    self.mHEX(True)
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
    
    def mCOMMA(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = COMMA
        _saveIndex = 0
        pass
        self.match(',')
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
    
    def mINT(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = INT
        _saveIndex = 0
        pass
        _cnt13= 0
        while True:
            if ((self.LA(1) >= u'0' and self.LA(1) <= u'9')):
                pass
                self.mDIGIT(False)
            else:
                break
            
            _cnt13 += 1
        if _cnt13 < 1:
            self.raise_NoViableAlt(self.LA(1))
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
        
        _cnt17= 0
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
                
            _cnt17 += 1
        if _cnt17 < 1:
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
        elif la1 and la1 in u'*':
            pass
            self.match('*')
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
            
        while True:
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in u'.':
                pass
                self.match('.')
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
            elif la1 and la1 in u'*':
                pass
                self.match('*')
            else:
                    break
                
        ### option { testLiterals=true } 
        _ttype = self.testLiteralsTable(_ttype)
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    def mSL_PREPOCESSOR_DIRECTIVE(self, _createToken):    
        _ttype = 0
        _token = None
        _begin = self.text.length()
        _ttype = SL_PREPOCESSOR_DIRECTIVE
        _saveIndex = 0
        pass
        self.match('#')
        while True:
            if (_tokenSet_0.member(self.LA(1))):
                pass
                self.matchNot('\n')
            else:
                break
            
        self.match('\n')
        _ttype = Token.SKIP; self.newline();
        self.set_return_token(_createToken, _token, _ttype, _begin)
    
    

### generate bit set
def mk_tokenSet_0(): 
    data = [0L] * 8 ### init list
    data[0] =-1032L
    for x in xrange(1, 4):
        data[x] = -1L
    return data
_tokenSet_0 = antlr.BitSet(mk_tokenSet_0())
    
### __main__ header action >>> 
if __name__ == '__main__' :
    import sys
    import antlr
    import rvct_ld_l
    
    ### create lexer - shall read from stdin
    try:
        for token in rvct_ld_l.Lexer():
            print token
            
    except antlr.TokenStreamException, e:
        print "error: exception caught while lexing: ", e
### __main__ header action <<< 
