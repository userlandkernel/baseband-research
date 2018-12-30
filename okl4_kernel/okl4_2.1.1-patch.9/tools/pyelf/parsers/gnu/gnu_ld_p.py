### $ANTLR 2.7.6 (20060903): "gnu_ld_p.g" -> "gnu_ld_p.py"$
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

class Parser(antlr.LLkParser):
    ### user action >>>
    ### user action <<<
    
    def __init__(self, *args, **kwargs):
        antlr.LLkParser.__init__(self, *args, **kwargs)
        self.tokenNames = _tokenNames
        self.buildTokenTypeASTClassMap()
        self.astFactory = antlr.ASTFactory(self.getTokenTypeToASTClassMap())
        self.astFactory.setASTNodeClass()
        
    def ident(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        ident_AST = None
        try:      ## for error handling
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [IDENT]:
                pass
                pass
                tmp1_AST = None
                tmp1_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp1_AST)
                self.match(IDENT)
                ident_AST = currentAST.root
            elif la1 and la1 in [OP_STAR]:
                pass
                pass
                tmp2_AST = None
                tmp2_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp2_AST)
                self.match(OP_STAR)
                ident_AST = currentAST.root
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_0)
        
        self.returnAST = ident_AST
    
    def func(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        func_AST = None
        try:      ## for error handling
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [TOKEN_ASSERT]:
                pass
                tmp3_AST = None
                tmp3_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp3_AST)
                self.match(TOKEN_ASSERT)
                func_AST = currentAST.root
            elif la1 and la1 in [TOKEN_ENTRY]:
                pass
                tmp4_AST = None
                tmp4_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp4_AST)
                self.match(TOKEN_ENTRY)
                func_AST = currentAST.root
            elif la1 and la1 in [TOKEN_ABSOLUTE]:
                pass
                tmp5_AST = None
                tmp5_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp5_AST)
                self.match(TOKEN_ABSOLUTE)
                func_AST = currentAST.root
            elif la1 and la1 in [TOKEN_ADDR]:
                pass
                tmp6_AST = None
                tmp6_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp6_AST)
                self.match(TOKEN_ADDR)
                func_AST = currentAST.root
            elif la1 and la1 in [TOKEN_LOADADDR]:
                pass
                tmp7_AST = None
                tmp7_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp7_AST)
                self.match(TOKEN_LOADADDR)
                func_AST = currentAST.root
            elif la1 and la1 in [TOKEN_ALIGN]:
                pass
                tmp8_AST = None
                tmp8_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp8_AST)
                self.match(TOKEN_ALIGN)
                func_AST = currentAST.root
            elif la1 and la1 in [TOKEN_DEFINED]:
                pass
                tmp9_AST = None
                tmp9_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp9_AST)
                self.match(TOKEN_DEFINED)
                func_AST = currentAST.root
            elif la1 and la1 in [TOKEN_NEXT]:
                pass
                tmp10_AST = None
                tmp10_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp10_AST)
                self.match(TOKEN_NEXT)
                func_AST = currentAST.root
            elif la1 and la1 in [TOKEN_SIZEOF]:
                pass
                tmp11_AST = None
                tmp11_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp11_AST)
                self.match(TOKEN_SIZEOF)
                func_AST = currentAST.root
            elif la1 and la1 in [TOKEN_MAX]:
                pass
                tmp12_AST = None
                tmp12_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp12_AST)
                self.match(TOKEN_MAX)
                func_AST = currentAST.root
            elif la1 and la1 in [TOKEN_MIN]:
                pass
                tmp13_AST = None
                tmp13_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp13_AST)
                self.match(TOKEN_MIN)
                func_AST = currentAST.root
            elif la1 and la1 in [TOKEN_CONSTANT]:
                pass
                tmp14_AST = None
                tmp14_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp14_AST)
                self.match(TOKEN_CONSTANT)
                func_AST = currentAST.root
            elif la1 and la1 in [TOKEN_OUTPUT_FORMAT]:
                pass
                tmp15_AST = None
                tmp15_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp15_AST)
                self.match(TOKEN_OUTPUT_FORMAT)
                func_AST = currentAST.root
            elif la1 and la1 in [TOKEN_OUTPUT_ARCH]:
                pass
                tmp16_AST = None
                tmp16_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp16_AST)
                self.match(TOKEN_OUTPUT_ARCH)
                func_AST = currentAST.root
            elif la1 and la1 in [TOKEN_SEARCH_DIR]:
                pass
                tmp17_AST = None
                tmp17_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp17_AST)
                self.match(TOKEN_SEARCH_DIR)
                func_AST = currentAST.root
            elif la1 and la1 in [TOKEN_INPUT]:
                pass
                tmp18_AST = None
                tmp18_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp18_AST)
                self.match(TOKEN_INPUT)
                func_AST = currentAST.root
            elif la1 and la1 in [TOKEN_OUTPUT]:
                pass
                tmp19_AST = None
                tmp19_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp19_AST)
                self.match(TOKEN_OUTPUT)
                func_AST = currentAST.root
            elif la1 and la1 in [TOKEN_GROUP]:
                pass
                tmp20_AST = None
                tmp20_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp20_AST)
                self.match(TOKEN_GROUP)
                func_AST = currentAST.root
            elif la1 and la1 in [TOKEN_STARTUP]:
                pass
                tmp21_AST = None
                tmp21_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp21_AST)
                self.match(TOKEN_STARTUP)
                func_AST = currentAST.root
            elif la1 and la1 in [TOKEN_TARGET]:
                pass
                tmp22_AST = None
                tmp22_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp22_AST)
                self.match(TOKEN_TARGET)
                func_AST = currentAST.root
            elif la1 and la1 in [TOKEN_NOCROSSREFS]:
                pass
                tmp23_AST = None
                tmp23_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp23_AST)
                self.match(TOKEN_NOCROSSREFS)
                func_AST = currentAST.root
            elif la1 and la1 in [TOKEN_DATA_SEGMENT_ALIGN]:
                pass
                tmp24_AST = None
                tmp24_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp24_AST)
                self.match(TOKEN_DATA_SEGMENT_ALIGN)
                func_AST = currentAST.root
            elif la1 and la1 in [TOKEN_DATA_SEGMENT_END]:
                pass
                tmp25_AST = None
                tmp25_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp25_AST)
                self.match(TOKEN_DATA_SEGMENT_END)
                func_AST = currentAST.root
            elif la1 and la1 in [TOKEN_DATA_SEGMENT_RELRO_END]:
                pass
                tmp26_AST = None
                tmp26_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp26_AST)
                self.match(TOKEN_DATA_SEGMENT_RELRO_END)
                func_AST = currentAST.root
            elif la1 and la1 in [TOKEN_BYTE]:
                pass
                tmp27_AST = None
                tmp27_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp27_AST)
                self.match(TOKEN_BYTE)
                func_AST = currentAST.root
            elif la1 and la1 in [TOKEN_SHORT]:
                pass
                tmp28_AST = None
                tmp28_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp28_AST)
                self.match(TOKEN_SHORT)
                func_AST = currentAST.root
            elif la1 and la1 in [TOKEN_LONG]:
                pass
                tmp29_AST = None
                tmp29_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp29_AST)
                self.match(TOKEN_LONG)
                func_AST = currentAST.root
            elif la1 and la1 in [TOKEN_QUAD]:
                pass
                tmp30_AST = None
                tmp30_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp30_AST)
                self.match(TOKEN_QUAD)
                func_AST = currentAST.root
            elif la1 and la1 in [TOKEN_SQUAD]:
                pass
                tmp31_AST = None
                tmp31_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp31_AST)
                self.match(TOKEN_SQUAD)
                func_AST = currentAST.root
            elif la1 and la1 in [TOKEN_FILL]:
                pass
                tmp32_AST = None
                tmp32_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp32_AST)
                self.match(TOKEN_FILL)
                func_AST = currentAST.root
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_1)
        
        self.returnAST = func_AST
    
    def op(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        op_AST = None
        try:      ## for error handling
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [OPERATOR]:
                pass
                tmp33_AST = None
                tmp33_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp33_AST)
                self.match(OPERATOR)
                op_AST = currentAST.root
            elif la1 and la1 in [OP_MINUS]:
                pass
                tmp34_AST = None
                tmp34_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp34_AST)
                self.match(OP_MINUS)
                op_AST = currentAST.root
            elif la1 and la1 in [OP_OTHERS]:
                pass
                tmp35_AST = None
                tmp35_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp35_AST)
                self.match(OP_OTHERS)
                op_AST = currentAST.root
            elif la1 and la1 in [OP_AND]:
                pass
                tmp36_AST = None
                tmp36_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp36_AST)
                self.match(OP_AND)
                op_AST = currentAST.root
            elif la1 and la1 in [OP_OR]:
                pass
                tmp37_AST = None
                tmp37_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp37_AST)
                self.match(OP_OR)
                op_AST = currentAST.root
            elif la1 and la1 in [OP_STAR]:
                pass
                tmp38_AST = None
                tmp38_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp38_AST)
                self.match(OP_STAR)
                op_AST = currentAST.root
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_2)
        
        self.returnAST = op_AST
    
    def assign_op(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        assign_op_AST = None
        try:      ## for error handling
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [EQUALS]:
                pass
                tmp39_AST = None
                tmp39_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp39_AST)
                self.match(EQUALS)
                assign_op_AST = currentAST.root
            elif la1 and la1 in [OP_ASSIGN]:
                pass
                tmp40_AST = None
                tmp40_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp40_AST)
                self.match(OP_ASSIGN)
                assign_op_AST = currentAST.root
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_3)
        
        self.returnAST = assign_op_AST
    
    def elem(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        elem_AST = None
        try:      ## for error handling
            pass
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [OP_MINUS]:
                pass
                tmp41_AST = None
                tmp41_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp41_AST)
                self.match(OP_MINUS)
            elif la1 and la1 in [TOKEN_ENTRY,TOKEN_PROVIDE,TOKEN_PROVIDE_HIDDEN,TOKEN_ABSOLUTE,TOKEN_ADDR,TOKEN_LOADADDR,TOKEN_ALIGN,TOKEN_DEFINED,TOKEN_NEXT,TOKEN_SIZEOF,TOKEN_MAX,TOKEN_MIN,TOKEN_CONSTANT,TOKEN_OUTPUT_FORMAT,TOKEN_ASSERT,TOKEN_OUTPUT_ARCH,TOKEN_SEARCH_DIR,TOKEN_INPUT,TOKEN_OUTPUT,TOKEN_GROUP,TOKEN_STARTUP,TOKEN_TARGET,TOKEN_NOCROSSREFS,TOKEN_DATA_SEGMENT_ALIGN,TOKEN_DATA_SEGMENT_END,TOKEN_DATA_SEGMENT_RELRO_END,TOKEN_BYTE,TOKEN_SHORT,TOKEN_LONG,TOKEN_QUAD,TOKEN_SQUAD,TOKEN_FILL,LPAREN,OP_STAR,IDENT,INT,HEX]:
                pass
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [INT]:
                pass
                tmp42_AST = None
                tmp42_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp42_AST)
                self.match(INT)
            elif la1 and la1 in [HEX]:
                pass
                tmp43_AST = None
                tmp43_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp43_AST)
                self.match(HEX)
            elif la1 and la1 in [OP_STAR,IDENT]:
                pass
                self.ident()
                self.addASTChild(currentAST, self.returnAST)
            elif la1 and la1 in [TOKEN_ENTRY,TOKEN_PROVIDE,TOKEN_PROVIDE_HIDDEN,TOKEN_ABSOLUTE,TOKEN_ADDR,TOKEN_LOADADDR,TOKEN_ALIGN,TOKEN_DEFINED,TOKEN_NEXT,TOKEN_SIZEOF,TOKEN_MAX,TOKEN_MIN,TOKEN_CONSTANT,TOKEN_OUTPUT_FORMAT,TOKEN_ASSERT,TOKEN_OUTPUT_ARCH,TOKEN_SEARCH_DIR,TOKEN_INPUT,TOKEN_OUTPUT,TOKEN_GROUP,TOKEN_STARTUP,TOKEN_TARGET,TOKEN_NOCROSSREFS,TOKEN_DATA_SEGMENT_ALIGN,TOKEN_DATA_SEGMENT_END,TOKEN_DATA_SEGMENT_RELRO_END,TOKEN_BYTE,TOKEN_SHORT,TOKEN_LONG,TOKEN_QUAD,TOKEN_SQUAD,TOKEN_FILL]:
                pass
                self.macro()
                self.addASTChild(currentAST, self.returnAST)
            elif la1 and la1 in [LPAREN]:
                pass
                pass
                tmp44_AST = None
                tmp44_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp44_AST)
                self.match(LPAREN)
                self.expression()
                self.addASTChild(currentAST, self.returnAST)
                tmp45_AST = None
                tmp45_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp45_AST)
                self.match(RPAREN)
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
            elem_AST = currentAST.root
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_4)
        
        self.returnAST = elem_AST
    
    def macro(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        macro_AST = None
        try:      ## for error handling
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [TOKEN_ENTRY,TOKEN_ABSOLUTE,TOKEN_ADDR,TOKEN_LOADADDR,TOKEN_ALIGN,TOKEN_DEFINED,TOKEN_NEXT,TOKEN_SIZEOF,TOKEN_MAX,TOKEN_MIN,TOKEN_CONSTANT,TOKEN_OUTPUT_FORMAT,TOKEN_ASSERT,TOKEN_OUTPUT_ARCH,TOKEN_SEARCH_DIR,TOKEN_INPUT,TOKEN_OUTPUT,TOKEN_GROUP,TOKEN_STARTUP,TOKEN_TARGET,TOKEN_NOCROSSREFS,TOKEN_DATA_SEGMENT_ALIGN,TOKEN_DATA_SEGMENT_END,TOKEN_DATA_SEGMENT_RELRO_END,TOKEN_BYTE,TOKEN_SHORT,TOKEN_LONG,TOKEN_QUAD,TOKEN_SQUAD,TOKEN_FILL]:
                pass
                pass
                self.func()
                self.addASTChild(currentAST, self.returnAST)
                tmp46_AST = None
                tmp46_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp46_AST)
                self.match(LPAREN)
                self.expression()
                self.addASTChild(currentAST, self.returnAST)
                while True:
                    if (self.LA(1)==COMMA):
                        pass
                        tmp47_AST = None
                        tmp47_AST = self.astFactory.create(self.LT(1))
                        self.addASTChild(currentAST, tmp47_AST)
                        self.match(COMMA)
                        self.expression()
                        self.addASTChild(currentAST, self.returnAST)
                    else:
                        break
                    
                tmp48_AST = None
                tmp48_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp48_AST)
                self.match(RPAREN)
                macro_AST = currentAST.root
            elif la1 and la1 in [TOKEN_PROVIDE,TOKEN_PROVIDE_HIDDEN]:
                pass
                self.provide()
                self.addASTChild(currentAST, self.returnAST)
                macro_AST = currentAST.root
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_5)
        
        self.returnAST = macro_AST
    
    def expression(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        expression_AST = None
        try:      ## for error handling
            pass
            self.expression_i()
            self.addASTChild(currentAST, self.returnAST)
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [QMARK]:
                pass
                tmp49_AST = None
                tmp49_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp49_AST)
                self.match(QMARK)
                self.expression_i()
                self.addASTChild(currentAST, self.returnAST)
                tmp50_AST = None
                tmp50_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp50_AST)
                self.match(COLON)
                self.expression_i()
                self.addASTChild(currentAST, self.returnAST)
            elif la1 and la1 in [TOKEN_OVERLAY,TOKEN_BLOCK,TOKEN_ENTRY,TOKEN_PROVIDE,TOKEN_PROVIDE_HIDDEN,TOKEN_ABSOLUTE,TOKEN_ADDR,TOKEN_LOADADDR,TOKEN_ALIGN,TOKEN_DEFINED,TOKEN_NEXT,TOKEN_SIZEOF,TOKEN_MAX,TOKEN_MIN,TOKEN_CONSTANT,TOKEN_OUTPUT_FORMAT,TOKEN_ASSERT,TOKEN_OUTPUT_ARCH,TOKEN_SEARCH_DIR,TOKEN_INPUT,TOKEN_OUTPUT,TOKEN_GROUP,TOKEN_STARTUP,TOKEN_TARGET,TOKEN_NOCROSSREFS,TOKEN_DATA_SEGMENT_ALIGN,TOKEN_DATA_SEGMENT_END,TOKEN_DATA_SEGMENT_RELRO_END,TOKEN_BYTE,TOKEN_SHORT,TOKEN_LONG,TOKEN_QUAD,TOKEN_SQUAD,TOKEN_FILL,DISCARD,NOLOAD,RPAREN,LCURLY,RCURLY,COMMA,SEMICOLON,COLON,OP_STAR,IDENT]:
                pass
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
            expression_AST = currentAST.root
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_6)
        
        self.returnAST = expression_AST
    
    def expression_i(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        expression_i_AST = None
        try:      ## for error handling
            pass
            self.elem()
            self.addASTChild(currentAST, self.returnAST)
            while True:
                if (_tokenSet_7.member(self.LA(1))) and (_tokenSet_3.member(self.LA(2))) and (_tokenSet_8.member(self.LA(3))) and (_tokenSet_9.member(self.LA(4))) and (_tokenSet_9.member(self.LA(5))) and (_tokenSet_9.member(self.LA(6))) and (_tokenSet_9.member(self.LA(7))) and (_tokenSet_9.member(self.LA(8))) and (_tokenSet_9.member(self.LA(9))) and (_tokenSet_9.member(self.LA(10))):
                    pass
                    self.op()
                    self.addASTChild(currentAST, self.returnAST)
                    self.elem()
                    self.addASTChild(currentAST, self.returnAST)
                else:
                    break
                
            expression_i_AST = currentAST.root
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_10)
        
        self.returnAST = expression_i_AST
    
    def assignment(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        assignment_AST = None
        try:      ## for error handling
            pass
            self.ident()
            self.addASTChild(currentAST, self.returnAST)
            self.assign_op()
            self.addASTChild(currentAST, self.returnAST)
            self.expression()
            self.addASTChild(currentAST, self.returnAST)
            tmp51_AST = None
            tmp51_AST = self.astFactory.create(self.LT(1))
            self.addASTChild(currentAST, tmp51_AST)
            self.match(SEMICOLON)
            assignment_AST = currentAST.root
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_11)
        
        self.returnAST = assignment_AST
    
    def provide(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        provide_AST = None
        try:      ## for error handling
            pass
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [TOKEN_PROVIDE]:
                pass
                tmp52_AST = None
                tmp52_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp52_AST)
                self.match(TOKEN_PROVIDE)
            elif la1 and la1 in [TOKEN_PROVIDE_HIDDEN]:
                pass
                tmp53_AST = None
                tmp53_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp53_AST)
                self.match(TOKEN_PROVIDE_HIDDEN)
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
            tmp54_AST = None
            tmp54_AST = self.astFactory.create(self.LT(1))
            self.addASTChild(currentAST, tmp54_AST)
            self.match(LPAREN)
            self.ident()
            self.addASTChild(currentAST, self.returnAST)
            tmp55_AST = None
            tmp55_AST = self.astFactory.create(self.LT(1))
            self.addASTChild(currentAST, tmp55_AST)
            self.match(EQUALS)
            self.expression()
            self.addASTChild(currentAST, self.returnAST)
            tmp56_AST = None
            tmp56_AST = self.astFactory.create(self.LT(1))
            self.addASTChild(currentAST, tmp56_AST)
            self.match(RPAREN)
            provide_AST = currentAST.root
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_5)
        
        self.returnAST = provide_AST
    
    def actual_section(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        actual_section_AST = None
        section_AST = None
        try:      ## for error handling
            pass
            self.ident()
            section_AST = self.returnAST
            self.addASTChild(currentAST, self.returnAST)
            if getattr(self, "section_names", None) == None: 
               self.section_names = []
            if section_AST.getText() not in self.section_names:
               self.section_names.append(section_AST.getText())
            actual_section_AST = currentAST.root
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_12)
        
        self.returnAST = actual_section_AST
    
    def input_section(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        input_section_AST = None
        try:      ## for error handling
            pass
            self.ident()
            self.addASTChild(currentAST, self.returnAST)
            tmp57_AST = None
            tmp57_AST = self.astFactory.create(self.LT(1))
            self.addASTChild(currentAST, tmp57_AST)
            self.match(LPAREN)
            if (self.LA(1)==OP_STAR or self.LA(1)==IDENT) and (self.LA(2)==EQUALS or self.LA(2)==OP_ASSIGN):
                pass
                self.assignment()
                self.addASTChild(currentAST, self.returnAST)
            elif (self.LA(1)==OP_STAR or self.LA(1)==IDENT) and (_tokenSet_13.member(self.LA(2))):
                pass
                if (self.LA(1)==OP_STAR or self.LA(1)==IDENT) and (self.LA(2)==LPAREN):
                    pass
                    self.input_section()
                    self.addASTChild(currentAST, self.returnAST)
                elif (self.LA(1)==OP_STAR or self.LA(1)==IDENT) and (_tokenSet_12.member(self.LA(2))):
                    pass
                    self.actual_section()
                    self.addASTChild(currentAST, self.returnAST)
                else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
            else:
                raise antlr.NoViableAltException(self.LT(1), self.getFilename())
            
            while True:
                if (self.LA(1)==COMMA or self.LA(1)==OP_STAR or self.LA(1)==IDENT):
                    pass
                    la1 = self.LA(1)
                    if False:
                        pass
                    elif la1 and la1 in [COMMA]:
                        pass
                        tmp58_AST = None
                        tmp58_AST = self.astFactory.create(self.LT(1))
                        self.addASTChild(currentAST, tmp58_AST)
                        self.match(COMMA)
                    elif la1 and la1 in [OP_STAR,IDENT]:
                        pass
                    else:
                            raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                        
                    if (self.LA(1)==OP_STAR or self.LA(1)==IDENT) and (self.LA(2)==LPAREN):
                        pass
                        self.input_section()
                        self.addASTChild(currentAST, self.returnAST)
                    elif (self.LA(1)==OP_STAR or self.LA(1)==IDENT) and (_tokenSet_12.member(self.LA(2))):
                        pass
                        self.actual_section()
                        self.addASTChild(currentAST, self.returnAST)
                    else:
                        raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                    
                else:
                    break
                
            tmp59_AST = None
            tmp59_AST = self.astFactory.create(self.LT(1))
            self.addASTChild(currentAST, tmp59_AST)
            self.match(RPAREN)
            input_section_AST = currentAST.root
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_14)
        
        self.returnAST = input_section_AST
    
    def output_attributes(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        output_attributes_AST = None
        try:      ## for error handling
            pass
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [OPERATOR]:
                pass
                tmp60_AST = None
                tmp60_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp60_AST)
                self.match(OPERATOR)
                self.ident()
                self.addASTChild(currentAST, self.returnAST)
            elif la1 and la1 in [TOKEN_OVERLAY,TOKEN_ENTRY,TOKEN_PROVIDE,TOKEN_PROVIDE_HIDDEN,TOKEN_ABSOLUTE,TOKEN_ADDR,TOKEN_LOADADDR,TOKEN_ALIGN,TOKEN_DEFINED,TOKEN_NEXT,TOKEN_SIZEOF,TOKEN_MAX,TOKEN_MIN,TOKEN_CONSTANT,TOKEN_OUTPUT_FORMAT,TOKEN_ASSERT,TOKEN_OUTPUT_ARCH,TOKEN_SEARCH_DIR,TOKEN_INPUT,TOKEN_OUTPUT,TOKEN_GROUP,TOKEN_STARTUP,TOKEN_TARGET,TOKEN_NOCROSSREFS,TOKEN_DATA_SEGMENT_ALIGN,TOKEN_DATA_SEGMENT_END,TOKEN_DATA_SEGMENT_RELRO_END,TOKEN_BYTE,TOKEN_SHORT,TOKEN_LONG,TOKEN_QUAD,TOKEN_SQUAD,TOKEN_FILL,DISCARD,RCURLY,SEMICOLON,COLON,EQUALS,OP_STAR,IDENT]:
                pass
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
            while True:
                if (self.LA(1)==COLON):
                    pass
                    tmp61_AST = None
                    tmp61_AST = self.astFactory.create(self.LT(1))
                    self.addASTChild(currentAST, tmp61_AST)
                    self.match(COLON)
                    self.ident()
                    self.addASTChild(currentAST, self.returnAST)
                else:
                    break
                
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [EQUALS]:
                pass
                tmp62_AST = None
                tmp62_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp62_AST)
                self.match(EQUALS)
                self.expression()
                self.addASTChild(currentAST, self.returnAST)
            elif la1 and la1 in [TOKEN_OVERLAY,TOKEN_ENTRY,TOKEN_PROVIDE,TOKEN_PROVIDE_HIDDEN,TOKEN_ABSOLUTE,TOKEN_ADDR,TOKEN_LOADADDR,TOKEN_ALIGN,TOKEN_DEFINED,TOKEN_NEXT,TOKEN_SIZEOF,TOKEN_MAX,TOKEN_MIN,TOKEN_CONSTANT,TOKEN_OUTPUT_FORMAT,TOKEN_ASSERT,TOKEN_OUTPUT_ARCH,TOKEN_SEARCH_DIR,TOKEN_INPUT,TOKEN_OUTPUT,TOKEN_GROUP,TOKEN_STARTUP,TOKEN_TARGET,TOKEN_NOCROSSREFS,TOKEN_DATA_SEGMENT_ALIGN,TOKEN_DATA_SEGMENT_END,TOKEN_DATA_SEGMENT_RELRO_END,TOKEN_BYTE,TOKEN_SHORT,TOKEN_LONG,TOKEN_QUAD,TOKEN_SQUAD,TOKEN_FILL,DISCARD,RCURLY,SEMICOLON,OP_STAR,IDENT]:
                pass
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
            output_attributes_AST = currentAST.root
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_15)
        
        self.returnAST = output_attributes_AST
    
    def output_section(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        output_section_AST = None
        try:      ## for error handling
            pass
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [OP_STAR,IDENT]:
                pass
                self.ident()
                self.addASTChild(currentAST, self.returnAST)
            elif la1 and la1 in [DISCARD]:
                pass
                tmp63_AST = None
                tmp63_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp63_AST)
                self.match(DISCARD)
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [TOKEN_ENTRY,TOKEN_PROVIDE,TOKEN_PROVIDE_HIDDEN,TOKEN_ABSOLUTE,TOKEN_ADDR,TOKEN_LOADADDR,TOKEN_ALIGN,TOKEN_DEFINED,TOKEN_NEXT,TOKEN_SIZEOF,TOKEN_MAX,TOKEN_MIN,TOKEN_CONSTANT,TOKEN_OUTPUT_FORMAT,TOKEN_ASSERT,TOKEN_OUTPUT_ARCH,TOKEN_SEARCH_DIR,TOKEN_INPUT,TOKEN_OUTPUT,TOKEN_GROUP,TOKEN_STARTUP,TOKEN_TARGET,TOKEN_NOCROSSREFS,TOKEN_DATA_SEGMENT_ALIGN,TOKEN_DATA_SEGMENT_END,TOKEN_DATA_SEGMENT_RELRO_END,TOKEN_BYTE,TOKEN_SHORT,TOKEN_LONG,TOKEN_QUAD,TOKEN_SQUAD,TOKEN_FILL,LPAREN,OP_STAR,OP_MINUS,IDENT,INT,HEX]:
                pass
                self.expression()
                self.addASTChild(currentAST, self.returnAST)
            elif la1 and la1 in [TOKEN_BLOCK,NOLOAD,COLON]:
                pass
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [TOKEN_BLOCK]:
                pass
                tmp64_AST = None
                tmp64_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp64_AST)
                self.match(TOKEN_BLOCK)
                tmp65_AST = None
                tmp65_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp65_AST)
                self.match(LPAREN)
                self.expression()
                self.addASTChild(currentAST, self.returnAST)
                tmp66_AST = None
                tmp66_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp66_AST)
                self.match(RPAREN)
            elif la1 and la1 in [NOLOAD,COLON]:
                pass
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [NOLOAD]:
                pass
                tmp67_AST = None
                tmp67_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp67_AST)
                self.match(NOLOAD)
            elif la1 and la1 in [COLON]:
                pass
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
            tmp68_AST = None
            tmp68_AST = self.astFactory.create(self.LT(1))
            self.addASTChild(currentAST, tmp68_AST)
            self.match(COLON)
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [TOKEN_AT]:
                pass
                tmp69_AST = None
                tmp69_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp69_AST)
                self.match(TOKEN_AT)
            elif la1 and la1 in [TOKEN_ENTRY,TOKEN_PROVIDE,TOKEN_PROVIDE_HIDDEN,TOKEN_ABSOLUTE,TOKEN_ADDR,TOKEN_LOADADDR,TOKEN_ALIGN,TOKEN_DEFINED,TOKEN_NEXT,TOKEN_SIZEOF,TOKEN_MAX,TOKEN_MIN,TOKEN_CONSTANT,TOKEN_OUTPUT_FORMAT,TOKEN_ASSERT,TOKEN_OUTPUT_ARCH,TOKEN_SEARCH_DIR,TOKEN_INPUT,TOKEN_OUTPUT,TOKEN_GROUP,TOKEN_STARTUP,TOKEN_TARGET,TOKEN_NOCROSSREFS,TOKEN_DATA_SEGMENT_ALIGN,TOKEN_DATA_SEGMENT_END,TOKEN_DATA_SEGMENT_RELRO_END,TOKEN_BYTE,TOKEN_SHORT,TOKEN_LONG,TOKEN_QUAD,TOKEN_SQUAD,TOKEN_FILL,LPAREN,LCURLY,OP_STAR,OP_MINUS,IDENT,INT,HEX]:
                pass
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [TOKEN_ENTRY,TOKEN_PROVIDE,TOKEN_PROVIDE_HIDDEN,TOKEN_ABSOLUTE,TOKEN_ADDR,TOKEN_LOADADDR,TOKEN_ALIGN,TOKEN_DEFINED,TOKEN_NEXT,TOKEN_SIZEOF,TOKEN_MAX,TOKEN_MIN,TOKEN_CONSTANT,TOKEN_OUTPUT_FORMAT,TOKEN_ASSERT,TOKEN_OUTPUT_ARCH,TOKEN_SEARCH_DIR,TOKEN_INPUT,TOKEN_OUTPUT,TOKEN_GROUP,TOKEN_STARTUP,TOKEN_TARGET,TOKEN_NOCROSSREFS,TOKEN_DATA_SEGMENT_ALIGN,TOKEN_DATA_SEGMENT_END,TOKEN_DATA_SEGMENT_RELRO_END,TOKEN_BYTE,TOKEN_SHORT,TOKEN_LONG,TOKEN_QUAD,TOKEN_SQUAD,TOKEN_FILL,LPAREN,OP_STAR,OP_MINUS,IDENT,INT,HEX]:
                pass
                self.expression()
                self.addASTChild(currentAST, self.returnAST)
            elif la1 and la1 in [LCURLY]:
                pass
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
            tmp70_AST = None
            tmp70_AST = self.astFactory.create(self.LT(1))
            self.addASTChild(currentAST, tmp70_AST)
            self.match(LCURLY)
            while True:
                la1 = self.LA(1)
                if False:
                    pass
                elif la1 and la1 in [TOKEN_ENTRY,TOKEN_PROVIDE,TOKEN_PROVIDE_HIDDEN,TOKEN_ABSOLUTE,TOKEN_ADDR,TOKEN_LOADADDR,TOKEN_ALIGN,TOKEN_DEFINED,TOKEN_NEXT,TOKEN_SIZEOF,TOKEN_MAX,TOKEN_MIN,TOKEN_CONSTANT,TOKEN_OUTPUT_FORMAT,TOKEN_ASSERT,TOKEN_OUTPUT_ARCH,TOKEN_SEARCH_DIR,TOKEN_INPUT,TOKEN_OUTPUT,TOKEN_GROUP,TOKEN_STARTUP,TOKEN_TARGET,TOKEN_NOCROSSREFS,TOKEN_DATA_SEGMENT_ALIGN,TOKEN_DATA_SEGMENT_END,TOKEN_DATA_SEGMENT_RELRO_END,TOKEN_BYTE,TOKEN_SHORT,TOKEN_LONG,TOKEN_QUAD,TOKEN_SQUAD,TOKEN_FILL]:
                    pass
                    self.macro()
                    self.addASTChild(currentAST, self.returnAST)
                elif la1 and la1 in [SEMICOLON]:
                    pass
                    tmp71_AST = None
                    tmp71_AST = self.astFactory.create(self.LT(1))
                    self.addASTChild(currentAST, tmp71_AST)
                    self.match(SEMICOLON)
                else:
                    if (self.LA(1)==OP_STAR or self.LA(1)==IDENT) and (self.LA(2)==EQUALS or self.LA(2)==OP_ASSIGN):
                        pass
                        self.assignment()
                        self.addASTChild(currentAST, self.returnAST)
                    elif (self.LA(1)==OP_STAR or self.LA(1)==IDENT) and (self.LA(2)==LPAREN):
                        pass
                        self.input_section()
                        self.addASTChild(currentAST, self.returnAST)
                    else:
                        break
                    
            tmp72_AST = None
            tmp72_AST = self.astFactory.create(self.LT(1))
            self.addASTChild(currentAST, tmp72_AST)
            self.match(RCURLY)
            self.output_attributes()
            self.addASTChild(currentAST, self.returnAST)
            output_section_AST = currentAST.root
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_15)
        
        self.returnAST = output_section_AST
    
    def overlay_section(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        overlay_section_AST = None
        try:      ## for error handling
            pass
            self.ident()
            self.addASTChild(currentAST, self.returnAST)
            tmp73_AST = None
            tmp73_AST = self.astFactory.create(self.LT(1))
            self.addASTChild(currentAST, tmp73_AST)
            self.match(LCURLY)
            while True:
                la1 = self.LA(1)
                if False:
                    pass
                elif la1 and la1 in [TOKEN_ENTRY,TOKEN_PROVIDE,TOKEN_PROVIDE_HIDDEN,TOKEN_ABSOLUTE,TOKEN_ADDR,TOKEN_LOADADDR,TOKEN_ALIGN,TOKEN_DEFINED,TOKEN_NEXT,TOKEN_SIZEOF,TOKEN_MAX,TOKEN_MIN,TOKEN_CONSTANT,TOKEN_OUTPUT_FORMAT,TOKEN_ASSERT,TOKEN_OUTPUT_ARCH,TOKEN_SEARCH_DIR,TOKEN_INPUT,TOKEN_OUTPUT,TOKEN_GROUP,TOKEN_STARTUP,TOKEN_TARGET,TOKEN_NOCROSSREFS,TOKEN_DATA_SEGMENT_ALIGN,TOKEN_DATA_SEGMENT_END,TOKEN_DATA_SEGMENT_RELRO_END,TOKEN_BYTE,TOKEN_SHORT,TOKEN_LONG,TOKEN_QUAD,TOKEN_SQUAD,TOKEN_FILL]:
                    pass
                    self.macro()
                    self.addASTChild(currentAST, self.returnAST)
                elif la1 and la1 in [SEMICOLON]:
                    pass
                    tmp74_AST = None
                    tmp74_AST = self.astFactory.create(self.LT(1))
                    self.addASTChild(currentAST, tmp74_AST)
                    self.match(SEMICOLON)
                else:
                    if (self.LA(1)==OP_STAR or self.LA(1)==IDENT) and (self.LA(2)==EQUALS or self.LA(2)==OP_ASSIGN):
                        pass
                        self.assignment()
                        self.addASTChild(currentAST, self.returnAST)
                    elif (self.LA(1)==OP_STAR or self.LA(1)==IDENT) and (self.LA(2)==LPAREN):
                        pass
                        self.input_section()
                        self.addASTChild(currentAST, self.returnAST)
                    else:
                        break
                    
            tmp75_AST = None
            tmp75_AST = self.astFactory.create(self.LT(1))
            self.addASTChild(currentAST, tmp75_AST)
            self.match(RCURLY)
            while True:
                if (self.LA(1)==COLON):
                    pass
                    tmp76_AST = None
                    tmp76_AST = self.astFactory.create(self.LT(1))
                    self.addASTChild(currentAST, tmp76_AST)
                    self.match(COLON)
                    self.ident()
                    self.addASTChild(currentAST, self.returnAST)
                else:
                    break
                
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [EQUALS]:
                pass
                tmp77_AST = None
                tmp77_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp77_AST)
                self.match(EQUALS)
                self.expression()
                self.addASTChild(currentAST, self.returnAST)
            elif la1 and la1 in [RCURLY]:
                pass
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
            overlay_section_AST = currentAST.root
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_16)
        
        self.returnAST = overlay_section_AST
    
    def overlay_block(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        overlay_block_AST = None
        try:      ## for error handling
            pass
            tmp78_AST = None
            tmp78_AST = self.astFactory.create(self.LT(1))
            self.addASTChild(currentAST, tmp78_AST)
            self.match(TOKEN_OVERLAY)
            self.expression()
            self.addASTChild(currentAST, self.returnAST)
            tmp79_AST = None
            tmp79_AST = self.astFactory.create(self.LT(1))
            self.addASTChild(currentAST, tmp79_AST)
            self.match(COLON)
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [TOKEN_NOCROSSREFS]:
                pass
                tmp80_AST = None
                tmp80_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp80_AST)
                self.match(TOKEN_NOCROSSREFS)
            elif la1 and la1 in [LCURLY,OP_STAR,IDENT]:
                pass
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [OP_STAR,IDENT]:
                pass
                self.ident()
                self.addASTChild(currentAST, self.returnAST)
                tmp81_AST = None
                tmp81_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp81_AST)
                self.match(LPAREN)
                self.expression()
                self.addASTChild(currentAST, self.returnAST)
                tmp82_AST = None
                tmp82_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp82_AST)
                self.match(RPAREN)
            elif la1 and la1 in [LCURLY]:
                pass
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
            tmp83_AST = None
            tmp83_AST = self.astFactory.create(self.LT(1))
            self.addASTChild(currentAST, tmp83_AST)
            self.match(LCURLY)
            self.overlay_section()
            self.addASTChild(currentAST, self.returnAST)
            tmp84_AST = None
            tmp84_AST = self.astFactory.create(self.LT(1))
            self.addASTChild(currentAST, tmp84_AST)
            self.match(RCURLY)
            self.output_attributes()
            self.addASTChild(currentAST, self.returnAST)
            overlay_block_AST = currentAST.root
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_15)
        
        self.returnAST = overlay_block_AST
    
    def section_lines(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        section_lines_AST = None
        try:      ## for error handling
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [TOKEN_ENTRY,TOKEN_PROVIDE,TOKEN_PROVIDE_HIDDEN,TOKEN_ABSOLUTE,TOKEN_ADDR,TOKEN_LOADADDR,TOKEN_ALIGN,TOKEN_DEFINED,TOKEN_NEXT,TOKEN_SIZEOF,TOKEN_MAX,TOKEN_MIN,TOKEN_CONSTANT,TOKEN_OUTPUT_FORMAT,TOKEN_ASSERT,TOKEN_OUTPUT_ARCH,TOKEN_SEARCH_DIR,TOKEN_INPUT,TOKEN_OUTPUT,TOKEN_GROUP,TOKEN_STARTUP,TOKEN_TARGET,TOKEN_NOCROSSREFS,TOKEN_DATA_SEGMENT_ALIGN,TOKEN_DATA_SEGMENT_END,TOKEN_DATA_SEGMENT_RELRO_END,TOKEN_BYTE,TOKEN_SHORT,TOKEN_LONG,TOKEN_QUAD,TOKEN_SQUAD,TOKEN_FILL]:
                pass
                self.macro()
                self.addASTChild(currentAST, self.returnAST)
                section_lines_AST = currentAST.root
            elif la1 and la1 in [TOKEN_OVERLAY]:
                pass
                self.overlay_block()
                self.addASTChild(currentAST, self.returnAST)
                section_lines_AST = currentAST.root
            elif la1 and la1 in [SEMICOLON]:
                pass
                tmp85_AST = None
                tmp85_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp85_AST)
                self.match(SEMICOLON)
                section_lines_AST = currentAST.root
            else:
                if (self.LA(1)==OP_STAR or self.LA(1)==IDENT) and (self.LA(2)==EQUALS or self.LA(2)==OP_ASSIGN):
                    pass
                    self.assignment()
                    self.addASTChild(currentAST, self.returnAST)
                    section_lines_AST = currentAST.root
                elif (self.LA(1)==DISCARD or self.LA(1)==OP_STAR or self.LA(1)==IDENT) and (_tokenSet_17.member(self.LA(2))):
                    pass
                    self.output_section()
                    self.addASTChild(currentAST, self.returnAST)
                    section_lines_AST = currentAST.root
                else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_15)
        
        self.returnAST = section_lines_AST
    
    def section_block(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        section_block_AST = None
        try:      ## for error handling
            pass
            tmp86_AST = None
            tmp86_AST = self.astFactory.create(self.LT(1))
            self.addASTChild(currentAST, tmp86_AST)
            self.match(TOKEN_SECTIONS)
            tmp87_AST = None
            tmp87_AST = self.astFactory.create(self.LT(1))
            self.addASTChild(currentAST, tmp87_AST)
            self.match(LCURLY)
            while True:
                if (_tokenSet_18.member(self.LA(1))):
                    pass
                    self.section_lines()
                    self.addASTChild(currentAST, self.returnAST)
                else:
                    break
                
            tmp88_AST = None
            tmp88_AST = self.astFactory.create(self.LT(1))
            self.addASTChild(currentAST, tmp88_AST)
            self.match(RCURLY)
            section_block_AST = currentAST.root
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_19)
        
        self.returnAST = section_block_AST
    
    def phdrs_lines(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        phdrs_lines_AST = None
        name_AST = None
        try:      ## for error handling
            pass
            self.ident()
            name_AST = self.returnAST
            self.addASTChild(currentAST, self.returnAST)
            if getattr(self, "segment_names", None) == None: 
               self.segment_names = []
            if name_AST.getText() not in self.segment_names:
               self.segment_names.append(name_AST.getText())
            self.ident()
            self.addASTChild(currentAST, self.returnAST)
            while True:
                if (self.LA(1)==TOKEN_PHDRS or self.LA(1)==OP_STAR or self.LA(1)==IDENT):
                    pass
                    la1 = self.LA(1)
                    if False:
                        pass
                    elif la1 and la1 in [OP_STAR,IDENT]:
                        pass
                        self.ident()
                        self.addASTChild(currentAST, self.returnAST)
                    elif la1 and la1 in [TOKEN_PHDRS]:
                        pass
                        tmp89_AST = None
                        tmp89_AST = self.astFactory.create(self.LT(1))
                        self.addASTChild(currentAST, tmp89_AST)
                        self.match(TOKEN_PHDRS)
                    else:
                            raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                        
                    la1 = self.LA(1)
                    if False:
                        pass
                    elif la1 and la1 in [LPAREN]:
                        pass
                        tmp90_AST = None
                        tmp90_AST = self.astFactory.create(self.LT(1))
                        self.addASTChild(currentAST, tmp90_AST)
                        self.match(LPAREN)
                        self.expression()
                        self.addASTChild(currentAST, self.returnAST)
                        tmp91_AST = None
                        tmp91_AST = self.astFactory.create(self.LT(1))
                        self.addASTChild(currentAST, tmp91_AST)
                        self.match(RPAREN)
                    elif la1 and la1 in [TOKEN_PHDRS,SEMICOLON,OP_STAR,IDENT]:
                        pass
                    else:
                            raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                        
                else:
                    break
                
            tmp92_AST = None
            tmp92_AST = self.astFactory.create(self.LT(1))
            self.addASTChild(currentAST, tmp92_AST)
            self.match(SEMICOLON)
            phdrs_lines_AST = currentAST.root
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_20)
        
        self.returnAST = phdrs_lines_AST
    
    def phdrs_block(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        phdrs_block_AST = None
        try:      ## for error handling
            pass
            tmp93_AST = None
            tmp93_AST = self.astFactory.create(self.LT(1))
            self.addASTChild(currentAST, tmp93_AST)
            self.match(TOKEN_PHDRS)
            tmp94_AST = None
            tmp94_AST = self.astFactory.create(self.LT(1))
            self.addASTChild(currentAST, tmp94_AST)
            self.match(LCURLY)
            while True:
                if (self.LA(1)==OP_STAR or self.LA(1)==IDENT):
                    pass
                    self.phdrs_lines()
                    self.addASTChild(currentAST, self.returnAST)
                else:
                    break
                
            tmp95_AST = None
            tmp95_AST = self.astFactory.create(self.LT(1))
            self.addASTChild(currentAST, tmp95_AST)
            self.match(RCURLY)
            phdrs_block_AST = currentAST.root
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_19)
        
        self.returnAST = phdrs_block_AST
    
    def mem_lines(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        mem_lines_AST = None
        try:      ## for error handling
            pass
            self.ident()
            self.addASTChild(currentAST, self.returnAST)
            tmp96_AST = None
            tmp96_AST = self.astFactory.create(self.LT(1))
            self.addASTChild(currentAST, tmp96_AST)
            self.match(LPAREN)
            while True:
                if (_tokenSet_7.member(self.LA(1))) and (_tokenSet_21.member(self.LA(2))) and (_tokenSet_22.member(self.LA(3))) and (_tokenSet_22.member(self.LA(4))) and (_tokenSet_23.member(self.LA(5))) and (_tokenSet_24.member(self.LA(6))) and (_tokenSet_25.member(self.LA(7))) and (_tokenSet_25.member(self.LA(8))) and (_tokenSet_25.member(self.LA(9))) and (_tokenSet_25.member(self.LA(10))):
                    pass
                    self.op()
                    self.addASTChild(currentAST, self.returnAST)
                elif (self.LA(1)==OP_STAR or self.LA(1)==IDENT) and (_tokenSet_21.member(self.LA(2))) and (_tokenSet_22.member(self.LA(3))) and (_tokenSet_22.member(self.LA(4))) and (_tokenSet_23.member(self.LA(5))) and (_tokenSet_24.member(self.LA(6))) and (_tokenSet_25.member(self.LA(7))) and (_tokenSet_25.member(self.LA(8))) and (_tokenSet_25.member(self.LA(9))) and (_tokenSet_25.member(self.LA(10))):
                    pass
                    self.ident()
                    self.addASTChild(currentAST, self.returnAST)
                else:
                    break
                
            tmp97_AST = None
            tmp97_AST = self.astFactory.create(self.LT(1))
            self.addASTChild(currentAST, tmp97_AST)
            self.match(RPAREN)
            tmp98_AST = None
            tmp98_AST = self.astFactory.create(self.LT(1))
            self.addASTChild(currentAST, tmp98_AST)
            self.match(COLON)
            self.ident()
            self.addASTChild(currentAST, self.returnAST)
            tmp99_AST = None
            tmp99_AST = self.astFactory.create(self.LT(1))
            self.addASTChild(currentAST, tmp99_AST)
            self.match(EQUALS)
            self.expression()
            self.addASTChild(currentAST, self.returnAST)
            tmp100_AST = None
            tmp100_AST = self.astFactory.create(self.LT(1))
            self.addASTChild(currentAST, tmp100_AST)
            self.match(COMMA)
            self.ident()
            self.addASTChild(currentAST, self.returnAST)
            tmp101_AST = None
            tmp101_AST = self.astFactory.create(self.LT(1))
            self.addASTChild(currentAST, tmp101_AST)
            self.match(EQUALS)
            self.expression()
            self.addASTChild(currentAST, self.returnAST)
            mem_lines_AST = currentAST.root
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_20)
        
        self.returnAST = mem_lines_AST
    
    def mem_block(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        mem_block_AST = None
        try:      ## for error handling
            pass
            tmp102_AST = None
            tmp102_AST = self.astFactory.create(self.LT(1))
            self.addASTChild(currentAST, tmp102_AST)
            self.match(TOKEN_MEMORY)
            tmp103_AST = None
            tmp103_AST = self.astFactory.create(self.LT(1))
            self.addASTChild(currentAST, tmp103_AST)
            self.match(LCURLY)
            _cnt71= 0
            while True:
                if (self.LA(1)==OP_STAR or self.LA(1)==IDENT):
                    pass
                    self.mem_lines()
                    self.addASTChild(currentAST, self.returnAST)
                else:
                    break
                
                _cnt71 += 1
            if _cnt71 < 1:
                raise antlr.NoViableAltException(self.LT(1), self.getFilename())
            tmp104_AST = None
            tmp104_AST = self.astFactory.create(self.LT(1))
            self.addASTChild(currentAST, tmp104_AST)
            self.match(RCURLY)
            mem_block_AST = currentAST.root
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_19)
        
        self.returnAST = mem_block_AST
    
    def block(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        block_AST = None
        try:      ## for error handling
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [TOKEN_ENTRY,TOKEN_PROVIDE,TOKEN_PROVIDE_HIDDEN,TOKEN_ABSOLUTE,TOKEN_ADDR,TOKEN_LOADADDR,TOKEN_ALIGN,TOKEN_DEFINED,TOKEN_NEXT,TOKEN_SIZEOF,TOKEN_MAX,TOKEN_MIN,TOKEN_CONSTANT,TOKEN_OUTPUT_FORMAT,TOKEN_ASSERT,TOKEN_OUTPUT_ARCH,TOKEN_SEARCH_DIR,TOKEN_INPUT,TOKEN_OUTPUT,TOKEN_GROUP,TOKEN_STARTUP,TOKEN_TARGET,TOKEN_NOCROSSREFS,TOKEN_DATA_SEGMENT_ALIGN,TOKEN_DATA_SEGMENT_END,TOKEN_DATA_SEGMENT_RELRO_END,TOKEN_BYTE,TOKEN_SHORT,TOKEN_LONG,TOKEN_QUAD,TOKEN_SQUAD,TOKEN_FILL]:
                pass
                self.macro()
                self.addASTChild(currentAST, self.returnAST)
                block_AST = currentAST.root
            elif la1 and la1 in [OP_STAR,IDENT]:
                pass
                self.assignment()
                self.addASTChild(currentAST, self.returnAST)
                block_AST = currentAST.root
            elif la1 and la1 in [TOKEN_SECTIONS]:
                pass
                self.section_block()
                self.addASTChild(currentAST, self.returnAST)
                block_AST = currentAST.root
            elif la1 and la1 in [TOKEN_PHDRS]:
                pass
                self.phdrs_block()
                self.addASTChild(currentAST, self.returnAST)
                block_AST = currentAST.root
            elif la1 and la1 in [TOKEN_MEMORY]:
                pass
                self.mem_block()
                self.addASTChild(currentAST, self.returnAST)
                block_AST = currentAST.root
            elif la1 and la1 in [SEMICOLON]:
                pass
                tmp105_AST = None
                tmp105_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp105_AST)
                self.match(SEMICOLON)
                block_AST = currentAST.root
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_19)
        
        self.returnAST = block_AST
    
    def script(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        script_AST = None
        try:      ## for error handling
            pass
            while True:
                if (_tokenSet_26.member(self.LA(1))):
                    pass
                    self.block()
                    self.addASTChild(currentAST, self.returnAST)
                else:
                    break
                
            tmp106_AST = None
            tmp106_AST = self.astFactory.create(self.LT(1))
            self.addASTChild(currentAST, tmp106_AST)
            self.match(EOF_TYPE)
            script_AST = currentAST.root
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_27)
        
        self.returnAST = script_AST
    
    
    def buildTokenTypeASTClassMap(self):
        self.tokenTypeToASTClassMap = None

_tokenNames = [
    "<0>", 
    "EOF", 
    "<2>", 
    "NULL_TREE_LOOKAHEAD", 
    "\"SECTIONS\"", 
    "\"PHDRS\"", 
    "\"MEMORY\"", 
    "\"OVERLAY\"", 
    "\"BLOCK\"", 
    "\"ENTRY\"", 
    "\"PROVIDE\"", 
    "\"PROVIDE_HIDDEN\"", 
    "\"AT\"", 
    "\"ABSOLUTE\"", 
    "\"ADDR\"", 
    "\"LOADADDR\"", 
    "\"ALIGN\"", 
    "\"DEFINED\"", 
    "\"NEXT\"", 
    "\"SIZEOF\"", 
    "\"MAX\"", 
    "\"MIN\"", 
    "\"CONSTANT\"", 
    "\"OUTPUT_FORMAT\"", 
    "\"ASSERT\"", 
    "\"OUTPUT_ARCH\"", 
    "\"SEARCH_DIR\"", 
    "\"INPUT\"", 
    "\"OUTPUT\"", 
    "\"GROUP\"", 
    "\"STARTUP\"", 
    "\"TARGET\"", 
    "\"NOCROSSREFS\"", 
    "\"DATA_SEGMENT_ALIGN\"", 
    "\"DATA_SEGMENT_END\"", 
    "\"DATA_SEGMENT_RELRO_END\"", 
    "\"BYTE\"", 
    "\"SHORT\"", 
    "\"LONG\"", 
    "\"QUAD\"", 
    "\"SQUAD\"", 
    "\"FILL\"", 
    "WS", 
    "ML_COMMENT", 
    "DISCARD", 
    "NOLOAD", 
    "'('", 
    "')'", 
    "'{'", 
    "'}'", 
    "','", 
    "';'", 
    "':'", 
    "'?'", 
    "'='", 
    "OP_STAR", 
    "OP_MINUS", 
    "OP_OTHERS", 
    "OP_AND", 
    "OP_OR", 
    "OP_EQ", 
    "OPERATOR", 
    "OP_ASSIGN", 
    "DIGIT", 
    "IDENT_FIRSTCHAR", 
    "IDENT_CHAR", 
    "an identifer", 
    "INT", 
    "a hexadecimal value"
]
    

### generate bit set
def mk_tokenSet_0(): 
    ### var1
    data = [ 8070437338108391328L, 28L, 0L, 0L]
    return data
_tokenSet_0 = antlr.BitSet(mk_tokenSet_0())

### generate bit set
def mk_tokenSet_1(): 
    ### var1
    data = [ 70368744177664L, 0L]
    return data
_tokenSet_1 = antlr.BitSet(mk_tokenSet_1())

### generate bit set
def mk_tokenSet_2(): 
    ### var1
    data = [ 3422951221080616448L, 28L, 0L, 0L]
    return data
_tokenSet_2 = antlr.BitSet(mk_tokenSet_2())

### generate bit set
def mk_tokenSet_3(): 
    ### var1
    data = [ 108161157847576064L, 28L, 0L, 0L]
    return data
_tokenSet_3 = antlr.BitSet(mk_tokenSet_3())

### generate bit set
def mk_tokenSet_4(): 
    ### var1
    data = [ 3440666552427343744L, 4L, 0L, 0L]
    return data
_tokenSet_4 = antlr.BitSet(mk_tokenSet_4())

### generate bit set
def mk_tokenSet_5(): 
    ### var1
    data = [ 3440666552427343858L, 4L, 0L, 0L]
    return data
_tokenSet_5 = antlr.BitSet(mk_tokenSet_5())

### generate bit set
def mk_tokenSet_6(): 
    ### var1
    data = [ 44952433389989760L, 4L, 0L, 0L]
    return data
_tokenSet_6 = antlr.BitSet(mk_tokenSet_6())

### generate bit set
def mk_tokenSet_7(): 
    ### var1
    data = [ 3422735716801576960L, 0L]
    return data
_tokenSet_7 = antlr.BitSet(mk_tokenSet_7())

### generate bit set
def mk_tokenSet_8(): 
    ### var1
    data = [ 3440736921171521408L, 28L, 0L, 0L]
    return data
_tokenSet_8 = antlr.BitSet(mk_tokenSet_8())

### generate bit set
def mk_tokenSet_9(): 
    ### var1
    data = [ 8070437338108395506L, 28L, 0L, 0L]
    return data
_tokenSet_9 = antlr.BitSet(mk_tokenSet_9())

### generate bit set
def mk_tokenSet_10(): 
    ### var1
    data = [ 53959632644730752L, 4L, 0L, 0L]
    return data
_tokenSet_10 = antlr.BitSet(mk_tokenSet_10())

### generate bit set
def mk_tokenSet_11(): 
    ### var1
    data = [ 40132174413819634L, 4L, 0L, 0L]
    return data
_tokenSet_11 = antlr.BitSet(mk_tokenSet_11())

### generate bit set
def mk_tokenSet_12(): 
    ### var1
    data = [ 37295434414161920L, 4L, 0L, 0L]
    return data
_tokenSet_12 = antlr.BitSet(mk_tokenSet_12())

### generate bit set
def mk_tokenSet_13(): 
    ### var1
    data = [ 37365803158339584L, 4L, 0L, 0L]
    return data
_tokenSet_13 = antlr.BitSet(mk_tokenSet_13())

### generate bit set
def mk_tokenSet_14(): 
    ### var1
    data = [ 40114582227774976L, 4L, 0L, 0L]
    return data
_tokenSet_14 = antlr.BitSet(mk_tokenSet_14())

### generate bit set
def mk_tokenSet_15(): 
    ### var1
    data = [ 38865537018621568L, 4L, 0L, 0L]
    return data
_tokenSet_15 = antlr.BitSet(mk_tokenSet_15())

### generate bit set
def mk_tokenSet_16(): 
    ### var1
    data = [ 562949953421312L, 0L]
    return data
_tokenSet_16 = antlr.BitSet(mk_tokenSet_16())

### generate bit set
def mk_tokenSet_17(): 
    ### var1
    data = [ 112699941847035648L, 28L, 0L, 0L]
    return data
_tokenSet_17 = antlr.BitSet(mk_tokenSet_17())

### generate bit set
def mk_tokenSet_18(): 
    ### var1
    data = [ 38302587065200256L, 4L, 0L, 0L]
    return data
_tokenSet_18 = antlr.BitSet(mk_tokenSet_18())

### generate bit set
def mk_tokenSet_19(): 
    ### var1
    data = [ 38284994879155826L, 4L, 0L, 0L]
    return data
_tokenSet_19 = antlr.BitSet(mk_tokenSet_19())

### generate bit set
def mk_tokenSet_20(): 
    ### var1
    data = [ 36591746972385280L, 4L, 0L, 0L]
    return data
_tokenSet_20 = antlr.BitSet(mk_tokenSet_20())

### generate bit set
def mk_tokenSet_21(): 
    ### var1
    data = [ 3422876454289932288L, 4L, 0L, 0L]
    return data
_tokenSet_21 = antlr.BitSet(mk_tokenSet_21())

### generate bit set
def mk_tokenSet_22(): 
    ### var1
    data = [ 3427380053917302784L, 4L, 0L, 0L]
    return data
_tokenSet_22 = antlr.BitSet(mk_tokenSet_22())

### generate bit set
def mk_tokenSet_23(): 
    ### var1
    data = [ 3445394452426784768L, 4L, 0L, 0L]
    return data
_tokenSet_23 = antlr.BitSet(mk_tokenSet_23())

### generate bit set
def mk_tokenSet_24(): 
    ### var1
    data = [ 3445469219217468928L, 28L, 0L, 0L]
    return data
_tokenSet_24 = antlr.BitSet(mk_tokenSet_24())

### generate bit set
def mk_tokenSet_25(): 
    ### var1
    data = [ 3455602318379052544L, 28L, 0L, 0L]
    return data
_tokenSet_25 = antlr.BitSet(mk_tokenSet_25())

### generate bit set
def mk_tokenSet_26(): 
    ### var1
    data = [ 38284994879155824L, 4L, 0L, 0L]
    return data
_tokenSet_26 = antlr.BitSet(mk_tokenSet_26())

### generate bit set
def mk_tokenSet_27(): 
    ### var1
    data = [ 2L, 0L]
    return data
_tokenSet_27 = antlr.BitSet(mk_tokenSet_27())
    

### __main__ header action >>> 
if __name__ == '__main__':
    import gnu_ld_l
    import gnu_ld_p
    
    L = gnu_ld_l.Lexer()
    P = gnu_ld_p.Parser(L)
    P.setFilename(L.getFilename())
    
    # Parse the input expression
    try:
       P.script()
    except antlr.RecognitionException, ex:
       print "ERROR\n"
    
    ast = P.getAST()
    
    if not ast:
       print "stop - no AST generated."
       sys.exit(0)
    
    print "NB this script should only be run directly for debugging purposes"
    print "segment names seen in the gnu linker or \"scatter gather\" script provided on stdin are:"
    print getattr(P, "segment_names", [])
    print getattr(P, "section_names", [])
### __main__ header action <<< 
