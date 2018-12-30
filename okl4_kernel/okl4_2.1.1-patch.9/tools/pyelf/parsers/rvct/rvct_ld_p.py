### $ANTLR 2.7.7 (20070817): "rvct_ld_p.g" -> "rvct_ld_p.py"$
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
TOKEN_PADVALUE = 39
TOKEN_ZEROPAD = 40
TOKEN_UNINT = 41
STAR = 42
TOKEN_RO_CODE = 43
TOKEN_RO_DATA = 44
TOKEN_RW_DATA = 45
TOKEN_RW_CODE = 46
TOKEN_GDEF = 47

class Parser(antlr.LLkParser):
    ### user action >>>
    ### user action <<<
    
    def __init__(self, *args, **kwargs):
        antlr.LLkParser.__init__(self, *args, **kwargs)
        self.tokenNames = _tokenNames
        self.buildTokenTypeASTClassMap()
        self.astFactory = antlr.ASTFactory(self.getTokenTypeToASTClassMap())
        self.astFactory.setASTNodeClass()
        
    def scatter_description(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        scatter_description_AST = None
        try:      ## for error handling
            pass
            while True:
                if (self.LA(1)==IDENT):
                    pass
                    self.load_region_description()
                    self.addASTChild(currentAST, self.returnAST)
                else:
                    break
                
            tmp1_AST = None
            tmp1_AST = self.astFactory.create(self.LT(1))
            self.addASTChild(currentAST, tmp1_AST)
            self.match(EOF_TYPE)
            scatter_description_AST = currentAST.root
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_0)
        
        self.returnAST = scatter_description_AST
    
    def load_region_description(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        load_region_description_AST = None
        try:      ## for error handling
            pass
            self.load_region_name()
            self.addASTChild(currentAST, self.returnAST)
            self.region_address()
            self.addASTChild(currentAST, self.returnAST)
            self.attribute_list()
            self.addASTChild(currentAST, self.returnAST)
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [INT,HEX]:
                pass
                self.max_size()
                self.addASTChild(currentAST, self.returnAST)
            elif la1 and la1 in [LCURLY]:
                pass
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
            self.load_region_description_a()
            self.addASTChild(currentAST, self.returnAST)
            load_region_description_AST = currentAST.root
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_1)
        
        self.returnAST = load_region_description_AST
    
    def load_region_name(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        load_region_name_AST = None
        a = None
        a_AST = None
        try:      ## for error handling
            pass
            a = self.LT(1)
            a_AST = self.astFactory.create(a)
            self.addASTChild(currentAST, a_AST)
            self.match(IDENT)
            # print "load_region_name found ", a.getText() 
            
            if getattr(self, "segments_to_sections", None) == None:
               self.segments_to_sections = {}
            self.segments_to_sections[a.getText()] = []
            self.current_segment = a.getText()
            load_region_name_AST = currentAST.root
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_2)
        
        self.returnAST = load_region_name_AST
    
    def region_address(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        region_address_AST = None
        try:      ## for error handling
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [LPAREN,INT,HEX]:
                pass
                self.base_address()
                self.addASTChild(currentAST, self.returnAST)
                region_address_AST = currentAST.root
            elif la1 and la1 in [PLUS]:
                pass
                tmp2_AST = None
                tmp2_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp2_AST)
                self.match(PLUS)
                self.offset()
                self.addASTChild(currentAST, self.returnAST)
                region_address_AST = currentAST.root
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_3)
        
        self.returnAST = region_address_AST
    
    def attribute_list(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        attribute_list_AST = None
        try:      ## for error handling
            pass
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [TOKEN_ABSOLUTE,TOKEN_PI,TOKEN_RELOC,TOKEN_OVERLAY]:
                pass
                self.position_option()
                self.addASTChild(currentAST, self.returnAST)
            elif la1 and la1 in [TOKEN_NOCOMPRESS,LCURLY,INT,HEX]:
                pass
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [TOKEN_NOCOMPRESS]:
                pass
                self.compression_option()
                self.addASTChild(currentAST, self.returnAST)
            elif la1 and la1 in [LCURLY,INT,HEX]:
                pass
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
            attribute_list_AST = currentAST.root
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_4)
        
        self.returnAST = attribute_list_AST
    
    def max_size(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        max_size_AST = None
        try:      ## for error handling
            pass
            self.number()
            self.addASTChild(currentAST, self.returnAST)
            max_size_AST = currentAST.root
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_5)
        
        self.returnAST = max_size_AST
    
    def load_region_description_a(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        load_region_description_a_AST = None
        try:      ## for error handling
            pass
            tmp3_AST = None
            tmp3_AST = self.astFactory.create(self.LT(1))
            self.addASTChild(currentAST, tmp3_AST)
            self.match(LCURLY)
            self.execution_region_description_list()
            self.addASTChild(currentAST, self.returnAST)
            tmp4_AST = None
            tmp4_AST = self.astFactory.create(self.LT(1))
            self.addASTChild(currentAST, tmp4_AST)
            self.match(RCURLY)
            load_region_description_a_AST = currentAST.root
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_1)
        
        self.returnAST = load_region_description_a_AST
    
    def execution_region_description_list(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        execution_region_description_list_AST = None
        try:      ## for error handling
            pass
            while True:
                if (self.LA(1)==ANY or self.LA(1)==IDENT):
                    pass
                    self.execution_region_description()
                    self.addASTChild(currentAST, self.returnAST)
                else:
                    break
                
            execution_region_description_list_AST = currentAST.root
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_6)
        
        self.returnAST = execution_region_description_list_AST
    
    def base_address(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        base_address_AST = None
        try:      ## for error handling
            pass
            self.expr()
            self.addASTChild(currentAST, self.returnAST)
            base_address_AST = currentAST.root
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_3)
        
        self.returnAST = base_address_AST
    
    def offset(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        offset_AST = None
        try:      ## for error handling
            pass
            self.number()
            self.addASTChild(currentAST, self.returnAST)
            offset_AST = currentAST.root
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_3)
        
        self.returnAST = offset_AST
    
    def expr(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        expr_AST = None
        try:      ## for error handling
            pass
            self.atom()
            self.addASTChild(currentAST, self.returnAST)
            while True:
                if (self.LA(1)==PLUS):
                    pass
                    pass
                    tmp5_AST = None
                    tmp5_AST = self.astFactory.create(self.LT(1))
                    self.addASTChild(currentAST, tmp5_AST)
                    self.match(PLUS)
                    self.atom()
                    self.addASTChild(currentAST, self.returnAST)
                else:
                    break
                
            expr_AST = currentAST.root
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_7)
        
        self.returnAST = expr_AST
    
    def number(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        number_AST = None
        try:      ## for error handling
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [INT]:
                pass
                tmp6_AST = None
                tmp6_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp6_AST)
                self.match(INT)
                number_AST = currentAST.root
            elif la1 and la1 in [HEX]:
                pass
                tmp7_AST = None
                tmp7_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp7_AST)
                self.match(HEX)
                number_AST = currentAST.root
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_8)
        
        self.returnAST = number_AST
    
    def atom(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        atom_AST = None
        try:      ## for error handling
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [INT,HEX]:
                pass
                self.number()
                self.addASTChild(currentAST, self.returnAST)
                atom_AST = currentAST.root
            elif la1 and la1 in [LPAREN]:
                pass
                tmp8_AST = None
                tmp8_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp8_AST)
                self.match(LPAREN)
                self.expr()
                self.addASTChild(currentAST, self.returnAST)
                tmp9_AST = None
                tmp9_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp9_AST)
                self.match(RPAREN)
                atom_AST = currentAST.root
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_8)
        
        self.returnAST = atom_AST
    
    def position_option(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        position_option_AST = None
        try:      ## for error handling
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [TOKEN_ABSOLUTE]:
                pass
                tmp10_AST = None
                tmp10_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp10_AST)
                self.match(TOKEN_ABSOLUTE)
                position_option_AST = currentAST.root
            elif la1 and la1 in [TOKEN_PI]:
                pass
                tmp11_AST = None
                tmp11_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp11_AST)
                self.match(TOKEN_PI)
                position_option_AST = currentAST.root
            elif la1 and la1 in [TOKEN_RELOC]:
                pass
                tmp12_AST = None
                tmp12_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp12_AST)
                self.match(TOKEN_RELOC)
                position_option_AST = currentAST.root
            elif la1 and la1 in [TOKEN_OVERLAY]:
                pass
                tmp13_AST = None
                tmp13_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp13_AST)
                self.match(TOKEN_OVERLAY)
                position_option_AST = currentAST.root
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_9)
        
        self.returnAST = position_option_AST
    
    def compression_option(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        compression_option_AST = None
        try:      ## for error handling
            pass
            tmp14_AST = None
            tmp14_AST = self.astFactory.create(self.LT(1))
            self.addASTChild(currentAST, tmp14_AST)
            self.match(TOKEN_NOCOMPRESS)
            compression_option_AST = currentAST.root
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_10)
        
        self.returnAST = compression_option_AST
    
    def execution_region_description(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        execution_region_description_AST = None
        try:      ## for error handling
            pass
            self.execution_region_name()
            self.addASTChild(currentAST, self.returnAST)
            self.region_address()
            self.addASTChild(currentAST, self.returnAST)
            self.execution_attribute_list()
            self.addASTChild(currentAST, self.returnAST)
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [MINUS,INT,HEX]:
                pass
                self.size_length()
                self.addASTChild(currentAST, self.returnAST)
            elif la1 and la1 in [LCURLY]:
                pass
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
            self.execution_region_description_a()
            self.addASTChild(currentAST, self.returnAST)
            execution_region_description_AST = currentAST.root
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_11)
        
        self.returnAST = execution_region_description_AST
    
    def execution_region_name(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        execution_region_name_AST = None
        a = None
        a_AST = None
        b = None
        b_AST = None
        try:      ## for error handling
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [IDENT]:
                pass
                a = self.LT(1)
                a_AST = self.astFactory.create(a)
                self.addASTChild(currentAST, a_AST)
                self.match(IDENT)
                self.segments_to_sections[self.current_segment].append(a.getText())
                execution_region_name_AST = currentAST.root
            elif la1 and la1 in [ANY]:
                pass
                b = self.LT(1)
                b_AST = self.astFactory.create(b)
                self.addASTChild(currentAST, b_AST)
                self.match(ANY)
                self.segments_to_sections[self.current_segment].append(a.getText())
                execution_region_name_AST = currentAST.root
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_2)
        
        self.returnAST = execution_region_name_AST
    
    def execution_attribute_list(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        execution_attribute_list_AST = None
        try:      ## for error handling
            pass
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [TOKEN_ABSOLUTE,TOKEN_PI,TOKEN_RELOC,TOKEN_OVERLAY]:
                pass
                self.position_option()
                self.addASTChild(currentAST, self.returnAST)
            elif la1 and la1 in [TOKEN_NOCOMPRESS,TOKEN_EMPTY,LCURLY,MINUS,INT,HEX,TOKEN_PADVALUE,TOKEN_ZEROPAD,TOKEN_UNINT]:
                pass
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [TOKEN_NOCOMPRESS]:
                pass
                self.compression_option()
                self.addASTChild(currentAST, self.returnAST)
            elif la1 and la1 in [TOKEN_EMPTY,LCURLY,MINUS,INT,HEX,TOKEN_PADVALUE,TOKEN_ZEROPAD,TOKEN_UNINT]:
                pass
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [TOKEN_EMPTY,TOKEN_PADVALUE,TOKEN_ZEROPAD,TOKEN_UNINT]:
                pass
                self.initialisation_option()
                self.addASTChild(currentAST, self.returnAST)
            elif la1 and la1 in [LCURLY,MINUS,INT,HEX]:
                pass
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
            execution_attribute_list_AST = currentAST.root
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_12)
        
        self.returnAST = execution_attribute_list_AST
    
    def size_length(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        size_length_AST = None
        try:      ## for error handling
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [INT,HEX]:
                pass
                self.max_size()
                self.addASTChild(currentAST, self.returnAST)
                size_length_AST = currentAST.root
            elif la1 and la1 in [MINUS]:
                pass
                self.length()
                self.addASTChild(currentAST, self.returnAST)
                size_length_AST = currentAST.root
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_5)
        
        self.returnAST = size_length_AST
    
    def execution_region_description_a(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        execution_region_description_a_AST = None
        try:      ## for error handling
            pass
            tmp15_AST = None
            tmp15_AST = self.astFactory.create(self.LT(1))
            self.addASTChild(currentAST, tmp15_AST)
            self.match(LCURLY)
            while True:
                if (self.LA(1)==ANY or self.LA(1)==IDENT or self.LA(1)==STAR):
                    pass
                    self.input_section_description()
                    self.addASTChild(currentAST, self.returnAST)
                else:
                    break
                
            tmp16_AST = None
            tmp16_AST = self.astFactory.create(self.LT(1))
            self.addASTChild(currentAST, tmp16_AST)
            self.match(RCURLY)
            execution_region_description_a_AST = currentAST.root
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_11)
        
        self.returnAST = execution_region_description_a_AST
    
    def input_section_description(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        input_section_description_AST = None
        a = None
        a_AST = None
        b = None
        b_AST = None
        c = None
        c_AST = None
        try:      ## for error handling
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [IDENT]:
                pass
                a = self.LT(1)
                a_AST = self.astFactory.create(a)
                self.addASTChild(currentAST, a_AST)
                self.match(IDENT)
                self.input_section_attribute_list()
                self.addASTChild(currentAST, self.returnAST)
                # print "IDENT input_section_description found ", a.getText()
                input_section_description_AST = currentAST.root
            elif la1 and la1 in [ANY]:
                pass
                b = self.LT(1)
                b_AST = self.astFactory.create(b)
                self.addASTChild(currentAST, b_AST)
                self.match(ANY)
                self.input_section_attribute_list()
                self.addASTChild(currentAST, self.returnAST)
                # print "ANY input_section_description found ", b.getText()
                input_section_description_AST = currentAST.root
            elif la1 and la1 in [STAR]:
                pass
                c = self.LT(1)
                c_AST = self.astFactory.create(c)
                self.addASTChild(currentAST, c_AST)
                self.match(STAR)
                self.input_section_attribute_list()
                self.addASTChild(currentAST, self.returnAST)
                # print "STAR input_section_description found ", c.getText()
                input_section_description_AST = currentAST.root
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_13)
        
        self.returnAST = input_section_description_AST
    
    def initialisation_option(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        initialisation_option_AST = None
        try:      ## for error handling
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [TOKEN_EMPTY]:
                pass
                tmp17_AST = None
                tmp17_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp17_AST)
                self.match(TOKEN_EMPTY)
                initialisation_option_AST = currentAST.root
            elif la1 and la1 in [TOKEN_PADVALUE]:
                pass
                tmp18_AST = None
                tmp18_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp18_AST)
                self.match(TOKEN_PADVALUE)
                self.number()
                self.addASTChild(currentAST, self.returnAST)
                initialisation_option_AST = currentAST.root
            elif la1 and la1 in [TOKEN_ZEROPAD]:
                pass
                tmp19_AST = None
                tmp19_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp19_AST)
                self.match(TOKEN_ZEROPAD)
                initialisation_option_AST = currentAST.root
            elif la1 and la1 in [TOKEN_UNINT]:
                pass
                tmp20_AST = None
                tmp20_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp20_AST)
                self.match(TOKEN_UNINT)
                initialisation_option_AST = currentAST.root
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_12)
        
        self.returnAST = initialisation_option_AST
    
    def length(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        length_AST = None
        try:      ## for error handling
            pass
            tmp21_AST = None
            tmp21_AST = self.astFactory.create(self.LT(1))
            self.addASTChild(currentAST, tmp21_AST)
            self.match(MINUS)
            self.number()
            self.addASTChild(currentAST, self.returnAST)
            length_AST = currentAST.root
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_5)
        
        self.returnAST = length_AST
    
    def input_section_description_list(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        input_section_description_list_AST = None
        try:      ## for error handling
            pass
            _cnt40= 0
            while True:
                if (self.LA(1)==ANY or self.LA(1)==IDENT or self.LA(1)==STAR):
                    pass
                    self.input_section_description()
                    self.addASTChild(currentAST, self.returnAST)
                else:
                    break
                
                _cnt40 += 1
            if _cnt40 < 1:
                raise antlr.NoViableAltException(self.LT(1), self.getFilename())
            input_section_description_list_AST = currentAST.root
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_0)
        
        self.returnAST = input_section_description_list_AST
    
    def input_section_attribute_list(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        input_section_attribute_list_AST = None
        try:      ## for error handling
            pass
            tmp22_AST = None
            tmp22_AST = self.astFactory.create(self.LT(1))
            self.addASTChild(currentAST, tmp22_AST)
            self.match(LPAREN)
            self.input_section_attribute_list_items()
            self.addASTChild(currentAST, self.returnAST)
            tmp23_AST = None
            tmp23_AST = self.astFactory.create(self.LT(1))
            self.addASTChild(currentAST, tmp23_AST)
            self.match(RPAREN)
            input_section_attribute_list_AST = currentAST.root
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_13)
        
        self.returnAST = input_section_attribute_list_AST
    
    def input_section_attribute_list_items(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        input_section_attribute_list_items_AST = None
        try:      ## for error handling
            pass
            self.input_section_attribute_list_item()
            self.addASTChild(currentAST, self.returnAST)
            while True:
                if (self.LA(1)==COMMA):
                    pass
                    tmp24_AST = None
                    tmp24_AST = self.astFactory.create(self.LT(1))
                    self.addASTChild(currentAST, tmp24_AST)
                    self.match(COMMA)
                    self.input_section_attribute_list_item()
                    self.addASTChild(currentAST, self.returnAST)
                else:
                    break
                
            input_section_attribute_list_items_AST = currentAST.root
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_14)
        
        self.returnAST = input_section_attribute_list_items_AST
    
    def input_section_attribute_list_item(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        input_section_attribute_list_item_AST = None
        try:      ## for error handling
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [PLUS]:
                pass
                tmp25_AST = None
                tmp25_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp25_AST)
                self.match(PLUS)
                self.input_section_attribute()
                self.addASTChild(currentAST, self.returnAST)
                input_section_attribute_list_item_AST = currentAST.root
            elif la1 and la1 in [IDENT,TOKEN_GDEF]:
                pass
                self.input_symbol_pattern()
                self.addASTChild(currentAST, self.returnAST)
                input_section_attribute_list_item_AST = currentAST.root
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_15)
        
        self.returnAST = input_section_attribute_list_item_AST
    
    def input_section_attribute(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        input_section_attribute_AST = None
        try:      ## for error handling
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [TOKEN_RO_CODE]:
                pass
                tmp26_AST = None
                tmp26_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp26_AST)
                self.match(TOKEN_RO_CODE)
                input_section_attribute_AST = currentAST.root
            elif la1 and la1 in [TOKEN_CODE]:
                pass
                tmp27_AST = None
                tmp27_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp27_AST)
                self.match(TOKEN_CODE)
                input_section_attribute_AST = currentAST.root
            elif la1 and la1 in [TOKEN_RO_DATA]:
                pass
                tmp28_AST = None
                tmp28_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp28_AST)
                self.match(TOKEN_RO_DATA)
                input_section_attribute_AST = currentAST.root
            elif la1 and la1 in [TOKEN_RO]:
                pass
                tmp29_AST = None
                tmp29_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp29_AST)
                self.match(TOKEN_RO)
                input_section_attribute_AST = currentAST.root
            elif la1 and la1 in [TOKEN_TEXT]:
                pass
                tmp30_AST = None
                tmp30_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp30_AST)
                self.match(TOKEN_TEXT)
                input_section_attribute_AST = currentAST.root
            elif la1 and la1 in [TOKEN_RW_DATA]:
                pass
                tmp31_AST = None
                tmp31_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp31_AST)
                self.match(TOKEN_RW_DATA)
                input_section_attribute_AST = currentAST.root
            elif la1 and la1 in [TOKEN_CONST]:
                pass
                tmp32_AST = None
                tmp32_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp32_AST)
                self.match(TOKEN_CONST)
                input_section_attribute_AST = currentAST.root
            elif la1 and la1 in [TOKEN_RW_CODE]:
                pass
                tmp33_AST = None
                tmp33_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp33_AST)
                self.match(TOKEN_RW_CODE)
                input_section_attribute_AST = currentAST.root
            elif la1 and la1 in [TOKEN_RW]:
                pass
                tmp34_AST = None
                tmp34_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp34_AST)
                self.match(TOKEN_RW)
                input_section_attribute_AST = currentAST.root
            elif la1 and la1 in [TOKEN_DATA]:
                pass
                tmp35_AST = None
                tmp35_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp35_AST)
                self.match(TOKEN_DATA)
                input_section_attribute_AST = currentAST.root
            elif la1 and la1 in [TOKEN_ZI]:
                pass
                tmp36_AST = None
                tmp36_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp36_AST)
                self.match(TOKEN_ZI)
                input_section_attribute_AST = currentAST.root
            elif la1 and la1 in [TOKEN_BSS]:
                pass
                tmp37_AST = None
                tmp37_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp37_AST)
                self.match(TOKEN_BSS)
                input_section_attribute_AST = currentAST.root
            elif la1 and la1 in [TOKEN_ENTRY]:
                pass
                tmp38_AST = None
                tmp38_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp38_AST)
                self.match(TOKEN_ENTRY)
                input_section_attribute_AST = currentAST.root
            elif la1 and la1 in [TOKEN_FIRST]:
                pass
                tmp39_AST = None
                tmp39_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp39_AST)
                self.match(TOKEN_FIRST)
                input_section_attribute_AST = currentAST.root
            elif la1 and la1 in [TOKEN_LAST]:
                pass
                tmp40_AST = None
                tmp40_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp40_AST)
                self.match(TOKEN_LAST)
                input_section_attribute_AST = currentAST.root
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_15)
        
        self.returnAST = input_section_attribute_AST
    
    def input_symbol_pattern(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        input_symbol_pattern_AST = None
        a = None
        a_AST = None
        try:      ## for error handling
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [IDENT]:
                pass
                a = self.LT(1)
                a_AST = self.astFactory.create(a)
                self.addASTChild(currentAST, a_AST)
                self.match(IDENT)
                
                input_symbol_pattern_AST = currentAST.root
            elif la1 and la1 in [TOKEN_GDEF]:
                pass
                tmp41_AST = None
                tmp41_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp41_AST)
                self.match(TOKEN_GDEF)
                tmp42_AST = None
                tmp42_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp42_AST)
                self.match(IDENT)
                input_symbol_pattern_AST = currentAST.root
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_15)
        
        self.returnAST = input_symbol_pattern_AST
    
    
    def buildTokenTypeASTClassMap(self):
        self.tokenTypeToASTClassMap = None

_tokenNames = [
    "<0>", 
    "EOF", 
    "<2>", 
    "NULL_TREE_LOOKAHEAD", 
    "\"RO-CODE\"", 
    "\"CODE\"", 
    "\"RO-DATA\"", 
    "\"RO\"", 
    "\"TEXT\"", 
    "\"RW-DATA\"", 
    "\"CONST\"", 
    "\"RW-CODE\"", 
    "\"RW\"", 
    "\"DATA\"", 
    "\"ZI\"", 
    "\"BSS\"", 
    "\"ENTRY\"", 
    "\"FIRST\"", 
    "\"LAST\"", 
    "\"NOCOMPRESS\"", 
    "\"ABSOLUTE\"", 
    "\"PI\"", 
    "\"RELOC\"", 
    "\"OVERLAY\"", 
    "\"EMPTY\"", 
    "\".ANY\"", 
    "WS", 
    "'('", 
    "')'", 
    "LCURLY", 
    "RCURLY", 
    "'+'", 
    "'-'", 
    "','", 
    "DIGIT", 
    "INT", 
    "a hexadecimal value value", 
    "an identifer", 
    "SL_PREPOCESSOR_DIRECTIVE", 
    "TOKEN_PADVALUE", 
    "TOKEN_ZEROPAD", 
    "TOKEN_UNINT", 
    "STAR", 
    "TOKEN_RO_CODE", 
    "TOKEN_RO_DATA", 
    "TOKEN_RW_DATA", 
    "TOKEN_RW_CODE", 
    "TOKEN_GDEF"
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
    data = [ 137438953474L, 0L]
    return data
_tokenSet_1 = antlr.BitSet(mk_tokenSet_1())

### generate bit set
def mk_tokenSet_2(): 
    ### var1
    data = [ 105360916480L, 0L]
    return data
_tokenSet_2 = antlr.BitSet(mk_tokenSet_2())

### generate bit set
def mk_tokenSet_3(): 
    ### var1
    data = [ 3956234780672L, 0L]
    return data
_tokenSet_3 = antlr.BitSet(mk_tokenSet_3())

### generate bit set
def mk_tokenSet_4(): 
    ### var1
    data = [ 103616086016L, 0L]
    return data
_tokenSet_4 = antlr.BitSet(mk_tokenSet_4())

### generate bit set
def mk_tokenSet_5(): 
    ### var1
    data = [ 536870912L, 0L]
    return data
_tokenSet_5 = antlr.BitSet(mk_tokenSet_5())

### generate bit set
def mk_tokenSet_6(): 
    ### var1
    data = [ 1073741824L, 0L]
    return data
_tokenSet_6 = antlr.BitSet(mk_tokenSet_6())

### generate bit set
def mk_tokenSet_7(): 
    ### var1
    data = [ 3956503216128L, 0L]
    return data
_tokenSet_7 = antlr.BitSet(mk_tokenSet_7())

### generate bit set
def mk_tokenSet_8(): 
    ### var1
    data = [ 3958650699776L, 0L]
    return data
_tokenSet_8 = antlr.BitSet(mk_tokenSet_8())

### generate bit set
def mk_tokenSet_9(): 
    ### var1
    data = [ 3956219052032L, 0L]
    return data
_tokenSet_9 = antlr.BitSet(mk_tokenSet_9())

### generate bit set
def mk_tokenSet_10(): 
    ### var1
    data = [ 3956218527744L, 0L]
    return data
_tokenSet_10 = antlr.BitSet(mk_tokenSet_10())

### generate bit set
def mk_tokenSet_11(): 
    ### var1
    data = [ 138546249728L, 0L]
    return data
_tokenSet_11 = antlr.BitSet(mk_tokenSet_11())

### generate bit set
def mk_tokenSet_12(): 
    ### var1
    data = [ 107911053312L, 0L]
    return data
_tokenSet_12 = antlr.BitSet(mk_tokenSet_12())

### generate bit set
def mk_tokenSet_13(): 
    ### var1
    data = [ 4536592760834L, 0L]
    return data
_tokenSet_13 = antlr.BitSet(mk_tokenSet_13())

### generate bit set
def mk_tokenSet_14(): 
    ### var1
    data = [ 268435456L, 0L]
    return data
_tokenSet_14 = antlr.BitSet(mk_tokenSet_14())

### generate bit set
def mk_tokenSet_15(): 
    ### var1
    data = [ 8858370048L, 0L]
    return data
_tokenSet_15 = antlr.BitSet(mk_tokenSet_15())
    

### __main__ header action >>> 
if __name__ == '__main__':
    import rvct_ld_l
    import rvct_ld_p
    
    
    L = rvct_ld_l.Lexer()
    P = rvct_ld_p.Parser(L)
    P.setFilename(L.getFilename())
    
    ### Parse the input expression
    try:
       P.scatter_description()
    except antlr.ANTLRException, ex:
       print "*** error(s) while parsing."
       print ">>> exit(1)"
       import sys
       sys.exit(1)
    
    ast = P.getAST()
    
    if not ast:
       print "stop - no AST generated."
       sys.exit(0)
    
    print "NB this script should only be run directly for debugging purposes\n"
    print "The segment to section names mapping as seen in the rvct linker or \"scatter gather\" script provided on stdin are:\n"
    print P.segments_to_sections
### __main__ header action <<< 
