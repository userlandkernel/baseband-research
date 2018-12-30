### $ANTLR 2.7.6 (20060903): "CPP_parser_py.g" -> "CPPParser.py"$
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
### preamble action>>>
#        The statements in this block appear only in CPPParser.py and not in CPPLexer.py

# Used to control selected (level) tracing (see support.cpp)
# 1 Shows which external and member statements selected
# 2 Shows above plus all declarations/definitions
# 3 reserved for future use
# 4 and above available for user
statementTrace = 0

def trace1(s):
   if statementTrace >= 1:
       print s

def checknode(node):
   assert node is not None
   return node

CPPParser_MaxQualifiedItemSize = 500


# Limit lookahead for qualifiedItemIs()
MaxTemplateTokenScan = 200
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

class Parser(antlr.LLkParser):
    ### user action >>>
    ### user action <<<
    
    def __init__(self, *args, **kwargs):
        antlr.LLkParser.__init__(self, *args, **kwargs)
        self.tokenNames = _tokenNames
        ### __init__ header action >>> 
        #antlrTrace(False);    // This is a dynamic trace facility for use with -traceParser etc.
        # It requires modification in LLkParser.cpp and LLkParser.hpp
        # otherwise it should be commented out (see MyReadMe.txt)
        # true shows antlr trace (can be set and reset during parsing)
        # false stops showing antlr trace 
        # Provided the parser is always generated with -traceParser this
        # facility allows trace output to be turned on or off by changing
        # the setting here from false to true or vice versa and then
        # recompiling and linking CPPParser only thus avoiding the need
        # to use antlr.Tool to re-generate the lexer and parser again. 
        # 
        
        
        # Creates a dictionary to hold symbols with 4001 buckets, 200 scopes and 800,000 characters
        # These can be changed to suit the size of program(s) being parsed
        # nfd: The Python version uses Python's native dict, which has "infinite" capacity, so
        # the above params aren't passed in.
        self.symbols = CPPDictionary()
        
        # Set template parameter and external scopes
        # Set template parameter scope to 0
        self.templateParameterScope = self.symbols.getCurrentScopeIndex()
        self.symbols.saveScope()    # Advance currentScope from 0 to 1
        self.externalScope = self.symbols.getCurrentScopeIndex()    # Set external scope to 1
        
        # Declare predefined scope "std" in external scope
        a = CPPSymbol.CPPSymbol("std", CPPSymbol.otTypedef)
        self.symbols.define("std", a)
        
        # Global flags to allow for nested declarations
        self._td = False        # For typedef
        self._fd = False        # For friend
        self._ts = None         # For TypeSpecifier
        self._ds = dsInvalid    # For DeclSpecifier
        
        # 0 = Function definition not being parsed
        # 1 = Parsing function name
        # 2 = Parsing function parameter list
        # 3 = Parsing function block
        self.functionDefinition = 0
        self.qualifierPrefix = "" # [0] = '\0';
        self.enclosingClass = "";
        self.assign_stmt_RHS_found = 0;
        self.in_parameter_list = False;
        self.K_and_R = False;    # used to distinguish old K & R parameter definitions
        self.in_return = False;
        self.is_address = False;
        self.is_pointer = False;
        
        # "protected:"
        ### __init__ header action <<< 
        
    def translation_unit(self):    
        node = None
        
        try:      ## for error handling
            pass
            if not self.inputState.guessing:
                self.enterExternalScope();
            if not self.inputState.guessing:
                node = self.node("translation_unit");
            _cnt3= 0
            while True:
                if (_tokenSet_0.member(self.LA(1))):
                    pass
                    ed=self.external_declaration()
                    if not self.inputState.guessing:
                        node.add_child(ed);
                else:
                    break
                
                _cnt3 += 1
            if _cnt3 < 1:
                raise antlr.NoViableAltException(self.LT(1), self.getFilename())
            self.match(EOF_TYPE)
            if not self.inputState.guessing:
                self.exitExternalScope();
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_1)
            else:
                raise ex
        
        return node
    
    def external_declaration(self):    
        node = None
        
        self.K_and_R = false;
        try:      ## for error handling
            pass
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [LITERAL_namespace]:
                pass
                dn=self.decl_namespace()
                if not self.inputState.guessing:
                    node = self.node("unfinished_namespace_decl");
            elif la1 and la1 in [SEMICOLON]:
                pass
                self.match(SEMICOLON)
                if not self.inputState.guessing:
                    self.end_of_stmt();
            else:
                synPredMatched7 = False
                if (self.LA(1)==LITERAL_template) and (self.LA(2)==LESSTHAN):
                    _m7 = self.mark()
                    synPredMatched7 = True
                    self.inputState.guessing += 1
                    try:
                        pass
                        self.match(LITERAL_template)
                        self.match(LESSTHAN)
                        self.match(GREATERTHAN)
                    except antlr.RecognitionException, pe:
                        synPredMatched7 = False
                    self.rewind(_m7)
                    self.inputState.guessing -= 1
                if synPredMatched7:
                    pass
                    self.match(LITERAL_template)
                    self.match(LESSTHAN)
                    self.match(GREATERTHAN)
                    d=self.declaration()
                    if not self.inputState.guessing:
                        node = self.node("template_specialisation"); node.add_child(d);
                else:
                    synPredMatched10 = False
                    if (_tokenSet_2.member(self.LA(1))) and (_tokenSet_3.member(self.LA(2))):
                        _m10 = self.mark()
                        synPredMatched10 = True
                        self.inputState.guessing += 1
                        try:
                            pass
                            la1 = self.LA(1)
                            if False:
                                pass
                            elif la1 and la1 in [LITERAL_typedef]:
                                pass
                                self.match(LITERAL_typedef)
                            elif la1 and la1 in [LITERAL_class,LITERAL_struct,LITERAL_union]:
                                pass
                            else:
                                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                                
                            self.class_head()
                        except antlr.RecognitionException, pe:
                            synPredMatched10 = False
                        self.rewind(_m10)
                        self.inputState.guessing -= 1
                    if synPredMatched10:
                        pass
                        d=self.declaration()
                        if not self.inputState.guessing:
                            node = d;
                    else:
                        synPredMatched12 = False
                        if (self.LA(1)==LITERAL_template) and (self.LA(2)==LESSTHAN):
                            _m12 = self.mark()
                            synPredMatched12 = True
                            self.inputState.guessing += 1
                            try:
                                pass
                                self.template_head()
                                self.class_head()
                            except antlr.RecognitionException, pe:
                                synPredMatched12 = False
                            self.rewind(_m12)
                            self.inputState.guessing -= 1
                        if synPredMatched12:
                            pass
                            th=self.template_head()
                            d=self.declaration()
                            if not self.inputState.guessing:
                                node = self.node("template"); node.add_child(th); node.add_child(d);
                        else:
                            synPredMatched15 = False
                            if (self.LA(1)==LITERAL_enum) and (self.LA(2)==ID or self.LA(2)==LCURLY):
                                _m15 = self.mark()
                                synPredMatched15 = True
                                self.inputState.guessing += 1
                                try:
                                    pass
                                    self.match(LITERAL_enum)
                                    la1 = self.LA(1)
                                    if False:
                                        pass
                                    elif la1 and la1 in [ID]:
                                        pass
                                        self.match(ID)
                                    elif la1 and la1 in [LCURLY]:
                                        pass
                                    else:
                                            raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                                        
                                    self.match(LCURLY)
                                except antlr.RecognitionException, pe:
                                    synPredMatched15 = False
                                self.rewind(_m15)
                                self.inputState.guessing -= 1
                            if synPredMatched15:
                                pass
                                if not self.inputState.guessing:
                                    node = self.node("enum");
                                es=self.enum_specifier()
                                if not self.inputState.guessing:
                                    node.add_child(es);
                                la1 = self.LA(1)
                                if False:
                                    pass
                                elif la1 and la1 in [ID,LITERAL__stdcall,LITERAL___stdcall,LPAREN,OPERATOR,LITERAL_this,LITERAL_true,LITERAL_false,STAR,AMPERSAND,TILDE,SCOPE,LITERAL__cdecl,LITERAL___cdecl,LITERAL__near,LITERAL___near,LITERAL__far,LITERAL___far,LITERAL___interrupt,LITERAL_pascal,LITERAL__pascal,LITERAL___pascal]:
                                    pass
                                    idl=self.init_declarator_list()
                                    if not self.inputState.guessing:
                                        node.add_child(idl);
                                elif la1 and la1 in [SEMICOLON]:
                                    pass
                                else:
                                        raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                                    
                                self.match(SEMICOLON)
                                if not self.inputState.guessing:
                                    self.end_of_stmt();
                            else:
                                synPredMatched19 = False
                                if (_tokenSet_4.member(self.LA(1))) and (_tokenSet_5.member(self.LA(2))):
                                    _m19 = self.mark()
                                    synPredMatched19 = True
                                    self.inputState.guessing += 1
                                    try:
                                        pass
                                        la1 = self.LA(1)
                                        if False:
                                            pass
                                        elif la1 and la1 in [LITERAL_template]:
                                            pass
                                            self.template_head()
                                        elif la1 and la1 in [ID,LITERAL_inline,LITERAL__inline,LITERAL___inline,LITERAL_virtual,TILDE,SCOPE]:
                                            pass
                                        else:
                                                raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                                            
                                        self.dtor_head(1)
                                        self.match(LCURLY)
                                    except antlr.RecognitionException, pe:
                                        synPredMatched19 = False
                                    self.rewind(_m19)
                                    self.inputState.guessing -= 1
                                if synPredMatched19:
                                    pass
                                    la1 = self.LA(1)
                                    if False:
                                        pass
                                    elif la1 and la1 in [LITERAL_template]:
                                        pass
                                        th=self.template_head()
                                    elif la1 and la1 in [ID,LITERAL_inline,LITERAL__inline,LITERAL___inline,LITERAL_virtual,TILDE,SCOPE]:
                                        pass
                                    else:
                                            raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                                        
                                    dh=self.dtor_head(1)
                                    db=self.dtor_body()
                                    if not self.inputState.guessing:
                                        node = self.node("unfinished_destructor");
                                else:
                                    synPredMatched23 = False
                                    if (_tokenSet_6.member(self.LA(1))) and (_tokenSet_7.member(self.LA(2))):
                                        _m23 = self.mark()
                                        synPredMatched23 = True
                                        self.inputState.guessing += 1
                                        try:
                                            pass
                                            if (True) and (True):
                                                pass
                                                cds=self.ctor_decl_spec()
                                            else: ## <m4>
                                                    pass
                                                
                                            if not self.qualifiedItemIsOneOf(qiCtor):
                                                raise antlr.SemanticException("self.qualifiedItemIsOneOf(qiCtor)")
                                        except antlr.RecognitionException, pe:
                                            synPredMatched23 = False
                                        self.rewind(_m23)
                                        self.inputState.guessing -= 1
                                    if synPredMatched23:
                                        pass
                                        cd=self.ctor_definition()
                                        if not self.inputState.guessing:
                                            node = self.node("unfinished_non-templated constructor");
                                    else:
                                        synPredMatched26 = False
                                        if (_tokenSet_8.member(self.LA(1))) and (_tokenSet_9.member(self.LA(2))):
                                            _m26 = self.mark()
                                            synPredMatched26 = True
                                            self.inputState.guessing += 1
                                            try:
                                                pass
                                                la1 = self.LA(1)
                                                if False:
                                                    pass
                                                elif la1 and la1 in [LITERAL_inline]:
                                                    pass
                                                    self.match(LITERAL_inline)
                                                elif la1 and la1 in [ID,OPERATOR,SCOPE]:
                                                    pass
                                                else:
                                                        raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                                                    
                                                self.scope_override()
                                                self.conversion_function_decl_or_def()
                                            except antlr.RecognitionException, pe:
                                                synPredMatched26 = False
                                            self.rewind(_m26)
                                            self.inputState.guessing -= 1
                                        if synPredMatched26:
                                            pass
                                            la1 = self.LA(1)
                                            if False:
                                                pass
                                            elif la1 and la1 in [LITERAL_inline]:
                                                pass
                                                self.match(LITERAL_inline)
                                            elif la1 and la1 in [ID,OPERATOR,SCOPE]:
                                                pass
                                            else:
                                                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                                                
                                            s=self.scope_override()
                                            cfdd=self.conversion_function_decl_or_def()
                                            if not self.inputState.guessing:
                                                node = self.node("unfinished_typecast");
                                        else:
                                            synPredMatched29 = False
                                            if (_tokenSet_2.member(self.LA(1))) and (_tokenSet_3.member(self.LA(2))):
                                                _m29 = self.mark()
                                                synPredMatched29 = True
                                                self.inputState.guessing += 1
                                                try:
                                                    pass
                                                    self.declaration_specifiers()
                                                    self.function_declarator(0)
                                                    self.match(SEMICOLON)
                                                except antlr.RecognitionException, pe:
                                                    synPredMatched29 = False
                                                self.rewind(_m29)
                                                self.inputState.guessing -= 1
                                            if synPredMatched29:
                                                pass
                                                d=self.declaration()
                                                if not self.inputState.guessing:
                                                    node = d;
                                            else:
                                                synPredMatched31 = False
                                                if (_tokenSet_10.member(self.LA(1))) and (_tokenSet_11.member(self.LA(2))):
                                                    _m31 = self.mark()
                                                    synPredMatched31 = True
                                                    self.inputState.guessing += 1
                                                    try:
                                                        pass
                                                        self.declaration_specifiers()
                                                        self.function_declarator(1)
                                                        self.match(LCURLY)
                                                    except antlr.RecognitionException, pe:
                                                        synPredMatched31 = False
                                                    self.rewind(_m31)
                                                    self.inputState.guessing -= 1
                                                if synPredMatched31:
                                                    pass
                                                    fd=self.function_definition()
                                                    if not self.inputState.guessing:
                                                        node = fd;
                                                else:
                                                    synPredMatched33 = False
                                                    if (_tokenSet_10.member(self.LA(1))) and (_tokenSet_11.member(self.LA(2))):
                                                        _m33 = self.mark()
                                                        synPredMatched33 = True
                                                        self.inputState.guessing += 1
                                                        try:
                                                            pass
                                                            self.declaration_specifiers()
                                                            self.function_declarator(1)
                                                            self.declaration()
                                                        except antlr.RecognitionException, pe:
                                                            synPredMatched33 = False
                                                        self.rewind(_m33)
                                                        self.inputState.guessing -= 1
                                                    if synPredMatched33:
                                                        pass
                                                        if not self.inputState.guessing:
                                                            self.K_and_R = true;
                                                        fd=self.function_definition()
                                                        if not self.inputState.guessing:
                                                            node = fd;
                                                    else:
                                                        synPredMatched36 = False
                                                        if (self.LA(1)==LITERAL_template) and (self.LA(2)==LESSTHAN):
                                                            _m36 = self.mark()
                                                            synPredMatched36 = True
                                                            self.inputState.guessing += 1
                                                            try:
                                                                pass
                                                                self.template_head()
                                                                self.declaration_specifiers()
                                                                la1 = self.LA(1)
                                                                if False:
                                                                    pass
                                                                elif la1 and la1 in [ID,LITERAL__stdcall,LITERAL___stdcall,LPAREN,OPERATOR,LITERAL_this,LITERAL_true,LITERAL_false,STAR,AMPERSAND,TILDE,SCOPE,LITERAL__cdecl,LITERAL___cdecl,LITERAL__near,LITERAL___near,LITERAL__far,LITERAL___far,LITERAL___interrupt,LITERAL_pascal,LITERAL__pascal,LITERAL___pascal]:
                                                                    pass
                                                                    self.init_declarator_list()
                                                                elif la1 and la1 in [SEMICOLON]:
                                                                    pass
                                                                else:
                                                                        raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                                                                    
                                                                self.match(SEMICOLON)
                                                                if not self.inputState.guessing:
                                                                    self.end_of_stmt();
                                                            except antlr.RecognitionException, pe:
                                                                synPredMatched36 = False
                                                            self.rewind(_m36)
                                                            self.inputState.guessing -= 1
                                                        if synPredMatched36:
                                                            pass
                                                            if not self.inputState.guessing:
                                                                beginTemplateDeclaration();
                                                            th=self.template_head()
                                                            ds=self.declaration_specifiers()
                                                            la1 = self.LA(1)
                                                            if False:
                                                                pass
                                                            elif la1 and la1 in [ID,LITERAL__stdcall,LITERAL___stdcall,LPAREN,OPERATOR,LITERAL_this,LITERAL_true,LITERAL_false,STAR,AMPERSAND,TILDE,SCOPE,LITERAL__cdecl,LITERAL___cdecl,LITERAL__near,LITERAL___near,LITERAL__far,LITERAL___far,LITERAL___interrupt,LITERAL_pascal,LITERAL__pascal,LITERAL___pascal]:
                                                                pass
                                                                idl=self.init_declarator_list()
                                                            elif la1 and la1 in [SEMICOLON]:
                                                                pass
                                                            else:
                                                                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                                                                
                                                            self.match(SEMICOLON)
                                                            if not self.inputState.guessing:
                                                                self.end_of_stmt();
                                                            if not self.inputState.guessing:
                                                                endTemplateDeclaration();
                                                            if not self.inputState.guessing:
                                                                node = self.node("unfinished_templated_forward_class_decl");
                                                        elif (self.LA(1)==LITERAL_template) and (self.LA(2)==LESSTHAN):
                                                            pass
                                                            if not self.inputState.guessing:
                                                                beginTemplateDefinition();
                                                            th=self.template_head()
                                                            if not self.inputState.guessing:
                                                                node = self.node("templated_function");
                                                            synPredMatched40 = False
                                                            if (_tokenSet_6.member(self.LA(1))) and (_tokenSet_7.member(self.LA(2))):
                                                                _m40 = self.mark()
                                                                synPredMatched40 = True
                                                                self.inputState.guessing += 1
                                                                try:
                                                                    pass
                                                                    self.ctor_decl_spec()
                                                                    if not self.qualifiedItemIsOneOf(qiCtor):
                                                                        raise antlr.SemanticException("self.qualifiedItemIsOneOf(qiCtor)")
                                                                except antlr.RecognitionException, pe:
                                                                    synPredMatched40 = False
                                                                self.rewind(_m40)
                                                                self.inputState.guessing -= 1
                                                            if synPredMatched40:
                                                                pass
                                                                cd=self.ctor_definition()
                                                                if not self.inputState.guessing:
                                                                    node.add_child(cd);
                                                            else:
                                                                synPredMatched42 = False
                                                                if (_tokenSet_2.member(self.LA(1))) and (_tokenSet_3.member(self.LA(2))):
                                                                    _m42 = self.mark()
                                                                    synPredMatched42 = True
                                                                    self.inputState.guessing += 1
                                                                    try:
                                                                        pass
                                                                        self.declaration_specifiers()
                                                                        self.function_declarator(0)
                                                                        self.match(SEMICOLON)
                                                                    except antlr.RecognitionException, pe:
                                                                        synPredMatched42 = False
                                                                    self.rewind(_m42)
                                                                    self.inputState.guessing -= 1
                                                                if synPredMatched42:
                                                                    pass
                                                                    d=self.declaration()
                                                                    if not self.inputState.guessing:
                                                                        node.add_child(d);
                                                                else:
                                                                    synPredMatched44 = False
                                                                    if (_tokenSet_10.member(self.LA(1))) and (_tokenSet_11.member(self.LA(2))):
                                                                        _m44 = self.mark()
                                                                        synPredMatched44 = True
                                                                        self.inputState.guessing += 1
                                                                        try:
                                                                            pass
                                                                            self.declaration_specifiers()
                                                                            self.function_declarator(1)
                                                                            self.match(LCURLY)
                                                                        except antlr.RecognitionException, pe:
                                                                            synPredMatched44 = False
                                                                        self.rewind(_m44)
                                                                        self.inputState.guessing -= 1
                                                                    if synPredMatched44:
                                                                        pass
                                                                        fd=self.function_definition()
                                                                        if not self.inputState.guessing:
                                                                            node.add_child(fd);
                                                                    else:
                                                                        raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                                                                    
                                                            if not self.inputState.guessing:
                                                                endTemplateDefinition();
                                                        elif (_tokenSet_2.member(self.LA(1))) and (_tokenSet_3.member(self.LA(2))):
                                                            pass
                                                            d=self.declaration()
                                                            if not self.inputState.guessing:
                                                                node = d;
                                                        else:
                                                            raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                                                        
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_12)
            else:
                raise ex
        
        return node
    
    def declaration(self):    
        node = None
        
        try:      ## for error handling
            synPredMatched104 = False
            if (self.LA(1)==LITERAL_extern) and (self.LA(2)==StringLiteral):
                _m104 = self.mark()
                synPredMatched104 = True
                self.inputState.guessing += 1
                try:
                    pass
                    self.match(LITERAL_extern)
                    self.match(StringLiteral)
                except antlr.RecognitionException, pe:
                    synPredMatched104 = False
                self.rewind(_m104)
                self.inputState.guessing -= 1
            if synPredMatched104:
                pass
                ls=self.linkage_specification()
                if not self.inputState.guessing:
                    node = ls;
            elif (_tokenSet_13.member(self.LA(1))) and (_tokenSet_14.member(self.LA(2))):
                pass
                if not self.inputState.guessing:
                    self.beginDeclaration(); node = self.node("declaration");
                ds=self.declaration_specifiers()
                if not self.inputState.guessing:
                    node.add_child(ds);
                la1 = self.LA(1)
                if False:
                    pass
                elif la1 and la1 in [ID,COMMA,LITERAL__stdcall,LITERAL___stdcall,LITERAL___attribute__,LPAREN,OPERATOR,LITERAL_this,LITERAL_true,LITERAL_false,STAR,AMPERSAND,TILDE,SCOPE,LITERAL__cdecl,LITERAL___cdecl,LITERAL__near,LITERAL___near,LITERAL__far,LITERAL___far,LITERAL___interrupt,LITERAL_pascal,LITERAL__pascal,LITERAL___pascal]:
                    pass
                    la1 = self.LA(1)
                    if False:
                        pass
                    elif la1 and la1 in [COMMA]:
                        pass
                        self.match(COMMA)
                    elif la1 and la1 in [ID,LITERAL__stdcall,LITERAL___stdcall,LITERAL___attribute__,LPAREN,OPERATOR,LITERAL_this,LITERAL_true,LITERAL_false,STAR,AMPERSAND,TILDE,SCOPE,LITERAL__cdecl,LITERAL___cdecl,LITERAL__near,LITERAL___near,LITERAL__far,LITERAL___far,LITERAL___interrupt,LITERAL_pascal,LITERAL__pascal,LITERAL___pascal]:
                        pass
                    else:
                            raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                        
                    la1 = self.LA(1)
                    if False:
                        pass
                    elif la1 and la1 in [LITERAL___attribute__]:
                        pass
                        a=self.attribute()
                        if not self.inputState.guessing:
                            node.add_child(a);
                    elif la1 and la1 in [ID,LITERAL__stdcall,LITERAL___stdcall,LPAREN,OPERATOR,LITERAL_this,LITERAL_true,LITERAL_false,STAR,AMPERSAND,TILDE,SCOPE,LITERAL__cdecl,LITERAL___cdecl,LITERAL__near,LITERAL___near,LITERAL__far,LITERAL___far,LITERAL___interrupt,LITERAL_pascal,LITERAL__pascal,LITERAL___pascal]:
                        pass
                    else:
                            raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                        
                    idl=self.init_declarator_list()
                    if not self.inputState.guessing:
                        node.add_child(idl);
                elif la1 and la1 in [SEMICOLON]:
                    pass
                else:
                        raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                    
                self.match(SEMICOLON)
                if not self.inputState.guessing:
                    self.end_of_stmt();
                if not self.inputState.guessing:
                    self.endDeclaration();
            elif (self.LA(1)==LITERAL_using):
                pass
                ud=self.using_declaration()
                if not self.inputState.guessing:
                    node = ud;
            else:
                raise antlr.NoViableAltException(self.LT(1), self.getFilename())
            
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_15)
            else:
                raise ex
        
        return node
    
    def class_head(self):    
        node = None
        
        id = None
        node = self.node("class_head") ;
        try:      ## for error handling
            pass
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [LITERAL_struct]:
                pass
                self.match(LITERAL_struct)
                if not self.inputState.guessing:
                    node.add_child(self.node("struct"));
            elif la1 and la1 in [LITERAL_union]:
                pass
                self.match(LITERAL_union)
                if not self.inputState.guessing:
                    node.add_child(self.node("union"));
            elif la1 and la1 in [LITERAL_class]:
                pass
                self.match(LITERAL_class)
                if not self.inputState.guessing:
                    node.add_child(self.node("class"));
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [ID]:
                pass
                id = self.LT(1)
                self.match(ID)
                if not self.inputState.guessing:
                    node.set_leaf(id.getText());
                la1 = self.LA(1)
                if False:
                    pass
                elif la1 and la1 in [LESSTHAN]:
                    pass
                    self.match(LESSTHAN)
                    tal=self.template_argument_list()
                    if not self.inputState.guessing:
                        node.add_child(tal);
                    self.match(GREATERTHAN)
                elif la1 and la1 in [LCURLY,COLON]:
                    pass
                else:
                        raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                    
                la1 = self.LA(1)
                if False:
                    pass
                elif la1 and la1 in [COLON]:
                    pass
                    bc=self.base_clause()
                    if not self.inputState.guessing:
                        node.add_child(bc);
                elif la1 and la1 in [LCURLY]:
                    pass
                else:
                        raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                    
            elif la1 and la1 in [LCURLY]:
                pass
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
            self.match(LCURLY)
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_1)
            else:
                raise ex
        
        return node
    
    def template_head(self):    
        node = None
        
        node = self.node("template_head");
        try:      ## for error handling
            pass
            self.match(LITERAL_template)
            self.match(LESSTHAN)
            tpl=self.template_parameter_list()
            if not self.inputState.guessing:
                node.add_child(tpl);
            self.match(GREATERTHAN)
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_16)
            else:
                raise ex
        
        return node
    
    def enum_specifier(self):    
        node = None
        
        id = None
        node = self.node("enum_specifier");
        try:      ## for error handling
            pass
            self.match(LITERAL_enum)
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [LCURLY]:
                pass
                self.match(LCURLY)
                el=self.enumerator_list()
                if not self.inputState.guessing:
                    node.add_child(el);
                self.match(RCURLY)
            elif la1 and la1 in [ID]:
                pass
                id = self.LT(1)
                self.match(ID)
                if not self.inputState.guessing:
                    node.set_leaf(id.getText());
                if not self.inputState.guessing:
                    self.beginEnumDefinition(id.getText());
                la1 = self.LA(1)
                if False:
                    pass
                elif la1 and la1 in [LCURLY]:
                    pass
                    self.match(LCURLY)
                    el=self.enumerator_list()
                    if not self.inputState.guessing:
                        node.add_child(el);
                    self.match(RCURLY)
                elif la1 and la1 in [LESSTHAN,GREATERTHAN,ID,SEMICOLON,RCURLY,ASSIGNEQUAL,COLON,COMMA,LITERAL__stdcall,LITERAL___stdcall,LITERAL___attribute__,LPAREN,RPAREN,OPERATOR,LITERAL_this,LITERAL_true,LITERAL_false,STAR,AMPERSAND,LSQUARE,RSQUARE,TILDE,ELLIPSIS,SCOPE,TIMESEQUAL,DIVIDEEQUAL,MINUSEQUAL,PLUSEQUAL,MODEQUAL,SHIFTLEFTEQUAL,SHIFTRIGHTEQUAL,BITWISEANDEQUAL,BITWISEXOREQUAL,BITWISEOREQUAL,QUESTIONMARK,OR,AND,BITWISEOR,BITWISEXOR,NOTEQUAL,EQUAL,LESSTHANOREQUALTO,GREATERTHANOREQUALTO,SHIFTLEFT,SHIFTRIGHT,PLUS,MINUS,DIVIDE,MOD,DOTMBR,POINTERTOMBR,LITERAL__cdecl,LITERAL___cdecl,LITERAL__near,LITERAL___near,LITERAL__far,LITERAL___far,LITERAL___interrupt,LITERAL_pascal,LITERAL__pascal,LITERAL___pascal]:
                    pass
                else:
                        raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                    
                if not self.inputState.guessing:
                    self.endEnumDefinition();
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_17)
            else:
                raise ex
        
        return node
    
    def init_declarator_list(self):    
        node = None
        
        node = self.node("init_declarator_list");
        try:      ## for error handling
            pass
            id=self.init_declarator()
            if not self.inputState.guessing:
                node.add_child(id);
            while True:
                if (self.LA(1)==COMMA):
                    pass
                    self.match(COMMA)
                    id=self.init_declarator()
                    if not self.inputState.guessing:
                        node.add_child(id);
                else:
                    break
                
            while True:
                if (self.LA(1)==LITERAL___attribute__):
                    pass
                    a=self.attribute()
                    if not self.inputState.guessing:
                        node.add_child(a);
                else:
                    break
                
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_18)
            else:
                raise ex
        
        return node
    
    def dtor_head(self,
        definition
    ):    
        node = None
        
        node = self.node("dtor_head");
        try:      ## for error handling
            pass
            dds=self.dtor_decl_spec()
            if not self.inputState.guessing:
                node.add_child(dds);
            dd=self.dtor_declarator(definition)
            if not self.inputState.guessing:
                node.add_child(dd);
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_19)
            else:
                raise ex
        
        return node
    
    def dtor_body(self):    
        node = None
        
        node = self.node("dtor_body");
        try:      ## for error handling
            pass
            cs=self.compound_statement()
            if not self.inputState.guessing:
                node.add_child(cs);
            if not self.inputState.guessing:
                endDestructorDefinition();
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_20)
            else:
                raise ex
        
        return node
    
    def ctor_decl_spec(self):    
        node = None
        
        node = self.node("ctor_decl_spec");
        try:      ## for error handling
            pass
            while True:
                la1 = self.LA(1)
                if False:
                    pass
                elif la1 and la1 in [LITERAL_inline,LITERAL__inline,LITERAL___inline]:
                    pass
                    la1 = self.LA(1)
                    if False:
                        pass
                    elif la1 and la1 in [LITERAL_inline]:
                        pass
                        self.match(LITERAL_inline)
                    elif la1 and la1 in [LITERAL__inline]:
                        pass
                        self.match(LITERAL__inline)
                    elif la1 and la1 in [LITERAL___inline]:
                        pass
                        self.match(LITERAL___inline)
                    else:
                            raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                        
                    if not self.inputState.guessing:
                        node.add_child(self.node("inline"));
                elif la1 and la1 in [LITERAL_explicit]:
                    pass
                    self.match(LITERAL_explicit)
                    if not self.inputState.guessing:
                        node.add_child(self.node("explicit"));
                else:
                        break
                    
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_21)
            else:
                raise ex
        
        return node
    
    def ctor_definition(self):    
        node = None
        
        node = self.node("ctor_definition");
        try:      ## for error handling
            pass
            ch=self.ctor_head()
            if not self.inputState.guessing:
                node.add_child(ch);
            cb=self.ctor_body()
            if not self.inputState.guessing:
                node.add_child(cb);
            if not self.inputState.guessing:
                endConstructorDefinition();
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_20)
            else:
                raise ex
        
        return node
    
    def scope_override(self):    
        s = None
        
        id = None
        sitem = ""
        try:      ## for error handling
            pass
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [SCOPE]:
                pass
                self.match(SCOPE)
                if not self.inputState.guessing:
                    sitem += "::";
            elif la1 and la1 in [ID,OPERATOR,LITERAL_this,LITERAL_true,LITERAL_false,STAR,TILDE]:
                pass
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
            while True:
                if ((self.LA(1)==ID) and (self.LA(2)==LESSTHAN or self.LA(2)==SCOPE) and (self.scopedItem())):
                    pass
                    id = self.LT(1)
                    self.match(ID)
                    la1 = self.LA(1)
                    if False:
                        pass
                    elif la1 and la1 in [LESSTHAN]:
                        pass
                        self.match(LESSTHAN)
                        tal=self.template_argument_list()
                        self.match(GREATERTHAN)
                    elif la1 and la1 in [SCOPE]:
                        pass
                    else:
                            raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                        
                    self.match(SCOPE)
                    if not self.inputState.guessing:
                        #printf("scope_override entered\n");
                                        sitem += id.getText();
                                        sitem += "::";
                else:
                    break
                
            if not self.inputState.guessing:
                s = sitem;
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_22)
            else:
                raise ex
        
        return s
    
    def conversion_function_decl_or_def(self):    
        node = None
        
        node = self.node("unfinished_conversion_function_decl_or_def");
        try:      ## for error handling
            pass
            self.match(OPERATOR)
            ds=self.declaration_specifiers()
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [STAR]:
                pass
                self.match(STAR)
            elif la1 and la1 in [AMPERSAND]:
                pass
                self.match(AMPERSAND)
            elif la1 and la1 in [LESSTHAN,LPAREN]:
                pass
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [LESSTHAN]:
                pass
                self.match(LESSTHAN)
                tpl=self.template_parameter_list()
                self.match(GREATERTHAN)
            elif la1 and la1 in [LPAREN]:
                pass
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
            self.match(LPAREN)
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [LITERAL_typedef,LITERAL_enum,ID,LITERAL_inline,LITERAL_extern,LITERAL___extension__,LITERAL__inline,LITERAL___inline,LITERAL___inline__,LITERAL_virtual,LITERAL_explicit,LITERAL_friend,LITERAL__stdcall,LITERAL___stdcall,LITERAL_typename,LPAREN,LITERAL_const,LITERAL___const,LITERAL_auto,LITERAL_register,LITERAL_static,LITERAL_mutable,LITERAL_const_cast,LITERAL_volatile,LITERAL___volatile,LITERAL___restrict,LITERAL___restrict__,LITERAL_char,LITERAL_bool,LITERAL_short,LITERAL_int,74,75,76,LITERAL_long,LITERAL_signed,LITERAL___signed,LITERAL___signed__,LITERAL_unsigned,LITERAL_float,LITERAL_double,LITERAL_void,LITERAL__declspec,LITERAL___declspec,LITERAL___builtin_va_list,LITERAL_class,LITERAL_struct,LITERAL_union,OPERATOR,LITERAL_this,LITERAL_true,LITERAL_false,STAR,AMPERSAND,TILDE,ELLIPSIS,SCOPE,LITERAL__cdecl,LITERAL___cdecl,LITERAL__near,LITERAL___near,LITERAL__far,LITERAL___far,LITERAL___interrupt,LITERAL_pascal,LITERAL__pascal,LITERAL___pascal]:
                pass
                pl=self.parameter_list()
            elif la1 and la1 in [RPAREN]:
                pass
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
            self.match(RPAREN)
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [LITERAL_const,LITERAL___const,LITERAL_const_cast,LITERAL_volatile,LITERAL___volatile,LITERAL___restrict,LITERAL___restrict__]:
                pass
                tq=self.type_qualifier()
            elif la1 and la1 in [LCURLY,SEMICOLON,LITERAL_throw]:
                pass
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [LITERAL_throw]:
                pass
                es=self.exception_specification()
            elif la1 and la1 in [LCURLY,SEMICOLON]:
                pass
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [LCURLY]:
                pass
                cs=self.compound_statement()
            elif la1 and la1 in [SEMICOLON]:
                pass
                self.match(SEMICOLON)
                if not self.inputState.guessing:
                    self.end_of_stmt();
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_20)
            else:
                raise ex
        
        return node
    
    def declaration_specifiers(self):    
        node = None
        
        # Global flags to allow for nested declarations
        self._td = false;                # For typedef
        self._fd = false;                # For friend
        self._tq = None;        # For TypeQualifier
        self._ts = None;        # For TypeSpecifier
        self._ds = None;        # For DeclSpecifier
        
         # Locals
        td = false;        # For typedef
        fd = false;        # For friend
        sc = "";        # auto,register,static,extern,mutable
        tq = "";        # const,const_cast,volatile,cdecl
        ts =  NULL;        # char,int,double, etc., class,struct,union
        # inline,virtual,explicit:
        ds = None
        try:      ## for error handling
            pass
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [LITERAL_typedef,LITERAL_enum,ID,LITERAL_inline,LITERAL_extern,LITERAL___extension__,LITERAL__inline,LITERAL___inline,LITERAL___inline__,LITERAL_virtual,LITERAL_explicit,LITERAL_friend,LITERAL__stdcall,LITERAL___stdcall,LITERAL_const,LITERAL___const,LITERAL_auto,LITERAL_register,LITERAL_static,LITERAL_mutable,LITERAL_const_cast,LITERAL_volatile,LITERAL___volatile,LITERAL___restrict,LITERAL___restrict__,LITERAL_char,LITERAL_bool,LITERAL_short,LITERAL_int,74,75,76,LITERAL_long,LITERAL_signed,LITERAL___signed,LITERAL___signed__,LITERAL_unsigned,LITERAL_float,LITERAL_double,LITERAL_void,LITERAL__declspec,LITERAL___declspec,LITERAL___builtin_va_list,LITERAL_class,LITERAL_struct,LITERAL_union,SCOPE]:
                pass
                if not self.inputState.guessing:
                    node = self.node("declaration_specifiers");
                while True:
                    if (_tokenSet_23.member(self.LA(1))):
                        pass
                        la1 = self.LA(1)
                        if False:
                            pass
                        elif la1 and la1 in [LITERAL_extern,LITERAL_auto,LITERAL_register,LITERAL_static,LITERAL_mutable]:
                            pass
                            sc=self.storage_class_specifier()
                            if not self.inputState.guessing:
                                specname = sc;
                        elif la1 and la1 in [LITERAL_const,LITERAL___const,LITERAL_const_cast,LITERAL_volatile,LITERAL___volatile,LITERAL___restrict,LITERAL___restrict__]:
                            pass
                            tq=self.type_qualifier()
                            if not self.inputState.guessing:
                                specname = tq;
                        elif la1 and la1 in [LITERAL___extension__]:
                            pass
                            self.match(LITERAL___extension__)
                            if not self.inputState.guessing:
                                specname = "__extension__";
                        elif la1 and la1 in [LITERAL_inline,LITERAL__inline,LITERAL___inline,LITERAL___inline__]:
                            pass
                            la1 = self.LA(1)
                            if False:
                                pass
                            elif la1 and la1 in [LITERAL_inline]:
                                pass
                                self.match(LITERAL_inline)
                            elif la1 and la1 in [LITERAL__inline]:
                                pass
                                self.match(LITERAL__inline)
                            elif la1 and la1 in [LITERAL___inline]:
                                pass
                                self.match(LITERAL___inline)
                            elif la1 and la1 in [LITERAL___inline__]:
                                pass
                                self.match(LITERAL___inline__)
                            else:
                                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                                
                            if not self.inputState.guessing:
                                ds = dsINLINE; specname = "inline";
                        elif la1 and la1 in [LITERAL_virtual]:
                            pass
                            self.match(LITERAL_virtual)
                            if not self.inputState.guessing:
                                ds = dsVIRTUAL; specname = "virtual";
                        elif la1 and la1 in [LITERAL_explicit]:
                            pass
                            self.match(LITERAL_explicit)
                            if not self.inputState.guessing:
                                ds = dsEXPLICIT; specname = "explicit";
                        elif la1 and la1 in [LITERAL_typedef]:
                            pass
                            self.match(LITERAL_typedef)
                            if not self.inputState.guessing:
                                td=true;
                            if not self.inputState.guessing:
                                specname = "typedef";
                        elif la1 and la1 in [LITERAL_friend]:
                            pass
                            self.match(LITERAL_friend)
                            if not self.inputState.guessing:
                                fd=true;
                            if not self.inputState.guessing:
                                specname = "friend";
                        elif la1 and la1 in [LITERAL__stdcall,LITERAL___stdcall]:
                            pass
                            la1 = self.LA(1)
                            if False:
                                pass
                            elif la1 and la1 in [LITERAL__stdcall]:
                                pass
                                self.match(LITERAL__stdcall)
                            elif la1 and la1 in [LITERAL___stdcall]:
                                pass
                                self.match(LITERAL___stdcall)
                            else:
                                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                                
                            if not self.inputState.guessing:
                                specname = "stdcall";
                        else:
                                raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                            
                        if not self.inputState.guessing:
                            node.add_child(self.node(specname));
                    else:
                        break
                    
                ts=self.type_specifier(ds)
                if not self.inputState.guessing:
                    node.add_child(ts);
            elif la1 and la1 in [LITERAL_typename]:
                pass
                self.match(LITERAL_typename)
                if not self.inputState.guessing:
                    td=true;
                if not self.inputState.guessing:
                    node = self.node("declaration_specifiers");
                dd=self.direct_declarator()
                if not self.inputState.guessing:
                    node.add_child(dd); node.set_leaf("typename");
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
            if not self.inputState.guessing:
                self.declarationSpecifier(td,fd,sc,tq,ts,ds);
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_17)
            else:
                raise ex
        
        return node
    
    def function_declarator(self,
        definition
    ):    
        node = None
        
        node = self.node("function_declarator");
        try:      ## for error handling
            synPredMatched241 = False
            if (_tokenSet_24.member(self.LA(1))) and (_tokenSet_25.member(self.LA(2))):
                _m241 = self.mark()
                synPredMatched241 = True
                self.inputState.guessing += 1
                try:
                    pass
                    self.ptr_operator()
                except antlr.RecognitionException, pe:
                    synPredMatched241 = False
                self.rewind(_m241)
                self.inputState.guessing -= 1
            if synPredMatched241:
                pass
                po=self.ptr_operator()
                if not self.inputState.guessing:
                    node.set_leaf(po);
                fd=self.function_declarator(definition)
                if not self.inputState.guessing:
                    node.add_child(fd);
            elif (_tokenSet_26.member(self.LA(1))) and (_tokenSet_27.member(self.LA(2))):
                pass
                fdd=self.function_direct_declarator(definition)
                if not self.inputState.guessing:
                    node.add_child(fdd);
            else:
                raise antlr.NoViableAltException(self.LT(1), self.getFilename())
            
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_28)
            else:
                raise ex
        
        return node
    
    def function_definition(self):    
        node = None
        
        try:      ## for error handling
            pass
            if not self.inputState.guessing:
                self.beginFunctionDefinition(); node = self.node("function_definition");
            if ((_tokenSet_13.member(self.LA(1))) and (_tokenSet_29.member(self.LA(2))) and (( not(self.LA(1)==SCOPE or self.LA(1)==ID) or self.qualifiedItemIsOneOf(qiType|qiCtor) ))):
                pass
                ds=self.declaration_specifiers()
                if not self.inputState.guessing:
                    node.add_child(ds);
                fd=self.function_declarator(1)
                if not self.inputState.guessing:
                    node.add_child(fd);
                if (_tokenSet_30.member(self.LA(1))) and (_tokenSet_31.member(self.LA(2))):
                    pass
                    if not self.inputState.guessing:
                        decl_node = self.node("declaration_list");
                    while True:
                        if (_tokenSet_2.member(self.LA(1))):
                            pass
                            d=self.declaration()
                            if not self.inputState.guessing:
                                decl_node.add_child(d);
                        else:
                            break
                        
                    if not self.inputState.guessing:
                        self.in_parameter_list = false;
                    if not self.inputState.guessing:
                        node.add_child(decl_node);
                elif (self.LA(1)==LCURLY) and (_tokenSet_32.member(self.LA(2))):
                    pass
                else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
                cs=self.compound_statement()
                if not self.inputState.guessing:
                    node.add_child(cs);
            elif (_tokenSet_33.member(self.LA(1))) and (_tokenSet_34.member(self.LA(2))):
                pass
                fd=self.function_declarator(1)
                if not self.inputState.guessing:
                    node.add_child(fd);
                if (_tokenSet_30.member(self.LA(1))) and (_tokenSet_31.member(self.LA(2))):
                    pass
                    if not self.inputState.guessing:
                        decl_node = self.node("declaration_list");
                    while True:
                        if (_tokenSet_2.member(self.LA(1))):
                            pass
                            d=self.declaration()
                        else:
                            break
                        
                    if not self.inputState.guessing:
                        decl_node.add_child(d);
                    if not self.inputState.guessing:
                        self.in_parameter_list = false;
                    if not self.inputState.guessing:
                        node.add_child(decl_node);
                elif (self.LA(1)==LCURLY) and (_tokenSet_32.member(self.LA(2))):
                    pass
                else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
                cs=self.compound_statement()
                if not self.inputState.guessing:
                    node.add_child(cs);
            else:
                raise antlr.NoViableAltException(self.LT(1), self.getFilename())
            
            if not self.inputState.guessing:
                self.endFunctionDefinition();
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_20)
            else:
                raise ex
        
        return node
    
    def decl_namespace(self):    
        node = None
        
        ns = None
        ns2 = None
        try:      ## for error handling
            pass
            self.match(LITERAL_namespace)
            if (self.LA(1)==ID or self.LA(1)==LCURLY) and (_tokenSet_35.member(self.LA(2))):
                pass
                if not self.inputState.guessing:
                    node = self.node("namespace");
                la1 = self.LA(1)
                if False:
                    pass
                elif la1 and la1 in [ID]:
                    pass
                    ns = self.LT(1)
                    self.match(ID)
                    if not self.inputState.guessing:
                        node.set_leaf(ns.getText());
                    if not self.inputState.guessing:
                        self._td = true; self.declaratorID(ns.getText(), qiType);
                elif la1 and la1 in [LCURLY]:
                    pass
                else:
                        raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                    
                self.match(LCURLY)
                if not self.inputState.guessing:
                    self.enterNewLocalScope();
                while True:
                    if (_tokenSet_0.member(self.LA(1))):
                        pass
                        ed=self.external_declaration()
                        if not self.inputState.guessing:
                            node.add_child(ed);
                    else:
                        break
                    
                if not self.inputState.guessing:
                    self.exitLocalScope();
                self.match(RCURLY)
            elif (self.LA(1)==ID) and (self.LA(2)==ASSIGNEQUAL):
                pass
                if not self.inputState.guessing:
                    node = self.node("unfinished_namespace_assignment");
                ns2 = self.LT(1)
                self.match(ID)
                if not self.inputState.guessing:
                    self._td = true; self.declaratorID(ns2.getText(), qiType);
                self.match(ASSIGNEQUAL)
                qid=self.qualified_id()
                self.match(SEMICOLON)
                if not self.inputState.guessing:
                    self.end_of_stmt();
            else:
                raise antlr.NoViableAltException(self.LT(1), self.getFilename())
            
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_12)
            else:
                raise ex
        
        return node
    
    def qualified_id(self):    
        q = None
        
        id = None
        try:      ## for error handling
            pass
            so=self.scope_override()
            if not self.inputState.guessing:
                qitem = so;
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [ID]:
                pass
                id = self.LT(1)
                self.match(ID)
                if (self.LA(1)==LESSTHAN) and (_tokenSet_36.member(self.LA(2))):
                    pass
                    self.match(LESSTHAN)
                    tal=self.template_argument_list()
                    self.match(GREATERTHAN)
                elif (_tokenSet_37.member(self.LA(1))) and (_tokenSet_38.member(self.LA(2))):
                    pass
                else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
                if not self.inputState.guessing:
                    qitem += id.getText();
            elif la1 and la1 in [OPERATOR]:
                pass
                self.match(OPERATOR)
                o=self.optor()
                if not self.inputState.guessing:
                    qitem += "operator"; qitem +=  o;
            elif la1 and la1 in [LITERAL_this]:
                pass
                self.match(LITERAL_this)
            elif la1 and la1 in [LITERAL_true,LITERAL_false]:
                pass
                la1 = self.LA(1)
                if False:
                    pass
                elif la1 and la1 in [LITERAL_true]:
                    pass
                    self.match(LITERAL_true)
                elif la1 and la1 in [LITERAL_false]:
                    pass
                    self.match(LITERAL_false)
                else:
                        raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                    
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
            if not self.inputState.guessing:
                q = qitem;
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_37)
            else:
                raise ex
        
        return q
    
    def member_declaration(self):    
        node = None
        
        try:      ## for error handling
            pass
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [LITERAL_public,LITERAL_protected,LITERAL_private]:
                pass
                as=self.access_specifier()
                self.match(COLON)
                if not self.inputState.guessing:
                    node = as;
            elif la1 and la1 in [SEMICOLON]:
                pass
                self.match(SEMICOLON)
                if not self.inputState.guessing:
                    self.end_of_stmt();
                if not self.inputState.guessing:
                    node = self.node("unfinished_member-decl-524");
            else:
                synPredMatched54 = False
                if (_tokenSet_2.member(self.LA(1))) and (_tokenSet_3.member(self.LA(2))):
                    _m54 = self.mark()
                    synPredMatched54 = True
                    self.inputState.guessing += 1
                    try:
                        pass
                        la1 = self.LA(1)
                        if False:
                            pass
                        elif la1 and la1 in [LITERAL_typedef]:
                            pass
                            self.match(LITERAL_typedef)
                        elif la1 and la1 in [LITERAL_class,LITERAL_struct,LITERAL_union]:
                            pass
                        else:
                                raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                            
                        self.class_head()
                    except antlr.RecognitionException, pe:
                        synPredMatched54 = False
                    self.rewind(_m54)
                    self.inputState.guessing -= 1
                if synPredMatched54:
                    pass
                    d=self.declaration()
                    if not self.inputState.guessing:
                        node = d;
                else:
                    synPredMatched57 = False
                    if (self.LA(1)==LITERAL_enum) and (self.LA(2)==ID or self.LA(2)==LCURLY):
                        _m57 = self.mark()
                        synPredMatched57 = True
                        self.inputState.guessing += 1
                        try:
                            pass
                            self.match(LITERAL_enum)
                            la1 = self.LA(1)
                            if False:
                                pass
                            elif la1 and la1 in [ID]:
                                pass
                                self.match(ID)
                            elif la1 and la1 in [LCURLY]:
                                pass
                            else:
                                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                                
                            self.match(LCURLY)
                        except antlr.RecognitionException, pe:
                            synPredMatched57 = False
                        self.rewind(_m57)
                        self.inputState.guessing -= 1
                    if synPredMatched57:
                        pass
                        if not self.inputState.guessing:
                            node = self.node("member_declaration");
                        es=self.enum_specifier()
                        if not self.inputState.guessing:
                            node.add_child(es);
                        la1 = self.LA(1)
                        if False:
                            pass
                        elif la1 and la1 in [ID,COLON,LITERAL__stdcall,LITERAL___stdcall,LPAREN,OPERATOR,LITERAL_this,LITERAL_true,LITERAL_false,STAR,AMPERSAND,TILDE,SCOPE,LITERAL__cdecl,LITERAL___cdecl,LITERAL__near,LITERAL___near,LITERAL__far,LITERAL___far,LITERAL___interrupt,LITERAL_pascal,LITERAL__pascal,LITERAL___pascal]:
                            pass
                            mdl=self.member_declarator_list()
                            if not self.inputState.guessing:
                                node.add_child(mdl);
                        elif la1 and la1 in [SEMICOLON]:
                            pass
                        else:
                                raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                            
                        self.match(SEMICOLON)
                        if not self.inputState.guessing:
                            self.end_of_stmt();
                    else:
                        synPredMatched60 = False
                        if (_tokenSet_6.member(self.LA(1))) and (_tokenSet_7.member(self.LA(2))):
                            _m60 = self.mark()
                            synPredMatched60 = True
                            self.inputState.guessing += 1
                            try:
                                pass
                                self.ctor_decl_spec()
                                if not self.qualifiedItemIsOneOf(qiCtor):
                                    raise antlr.SemanticException("self.qualifiedItemIsOneOf(qiCtor)")
                                self.ctor_declarator(0)
                                self.match(SEMICOLON)
                            except antlr.RecognitionException, pe:
                                synPredMatched60 = False
                            self.rewind(_m60)
                            self.inputState.guessing -= 1
                        if synPredMatched60:
                            pass
                            if not self.inputState.guessing:
                                node = self.node("member_declaration");
                            cds=self.ctor_decl_spec()
                            if not self.inputState.guessing:
                                node.add_child(cds);
                            cd=self.ctor_declarator(0)
                            if not self.inputState.guessing:
                                node.add_child(cd);
                            self.match(SEMICOLON)
                            if not self.inputState.guessing:
                                self.end_of_stmt();
                        else:
                            synPredMatched63 = False
                            if (_tokenSet_6.member(self.LA(1))) and (_tokenSet_7.member(self.LA(2))):
                                _m63 = self.mark()
                                synPredMatched63 = True
                                self.inputState.guessing += 1
                                try:
                                    pass
                                    self.ctor_decl_spec()
                                    if not self.qualifiedItemIsOneOf(qiCtor):
                                        raise antlr.SemanticException("self.qualifiedItemIsOneOf(qiCtor)")
                                    self.ctor_declarator(1)
                                    la1 = self.LA(1)
                                    if False:
                                        pass
                                    elif la1 and la1 in [COLON]:
                                        pass
                                        self.match(COLON)
                                    elif la1 and la1 in [LCURLY]:
                                        pass
                                        self.match(LCURLY)
                                    else:
                                            raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                                        
                                except antlr.RecognitionException, pe:
                                    synPredMatched63 = False
                                self.rewind(_m63)
                                self.inputState.guessing -= 1
                            if synPredMatched63:
                                pass
                                if not self.inputState.guessing:
                                    node = self.node("member_declaration");
                                cd=self.ctor_definition()
                                if not self.inputState.guessing:
                                    node.add_child(cd);
                            else:
                                synPredMatched65 = False
                                if (_tokenSet_39.member(self.LA(1))) and (_tokenSet_5.member(self.LA(2))):
                                    _m65 = self.mark()
                                    synPredMatched65 = True
                                    self.inputState.guessing += 1
                                    try:
                                        pass
                                        self.dtor_head(0)
                                        self.match(SEMICOLON)
                                    except antlr.RecognitionException, pe:
                                        synPredMatched65 = False
                                    self.rewind(_m65)
                                    self.inputState.guessing -= 1
                                if synPredMatched65:
                                    pass
                                    if not self.inputState.guessing:
                                        node = self.node("member_declaration");
                                    dh=self.dtor_head(0)
                                    if not self.inputState.guessing:
                                        node.add_child(dh);
                                    self.match(SEMICOLON)
                                    if not self.inputState.guessing:
                                        self.end_of_stmt();
                                else:
                                    synPredMatched67 = False
                                    if (_tokenSet_39.member(self.LA(1))) and (_tokenSet_5.member(self.LA(2))):
                                        _m67 = self.mark()
                                        synPredMatched67 = True
                                        self.inputState.guessing += 1
                                        try:
                                            pass
                                            self.dtor_head(1)
                                            self.match(LCURLY)
                                        except antlr.RecognitionException, pe:
                                            synPredMatched67 = False
                                        self.rewind(_m67)
                                        self.inputState.guessing -= 1
                                    if synPredMatched67:
                                        pass
                                        if not self.inputState.guessing:
                                            node = self.node("member_declaration");
                                        dh=self.dtor_head(1)
                                        if not self.inputState.guessing:
                                            node.add_child(dh);
                                        db=self.dtor_body()
                                        if not self.inputState.guessing:
                                            node.add_child(db);
                                    else:
                                        synPredMatched69 = False
                                        if (_tokenSet_2.member(self.LA(1))) and (_tokenSet_3.member(self.LA(2))):
                                            _m69 = self.mark()
                                            synPredMatched69 = True
                                            self.inputState.guessing += 1
                                            try:
                                                pass
                                                self.declaration_specifiers()
                                                self.function_declarator(0)
                                                self.match(SEMICOLON)
                                            except antlr.RecognitionException, pe:
                                                synPredMatched69 = False
                                            self.rewind(_m69)
                                            self.inputState.guessing -= 1
                                        if synPredMatched69:
                                            pass
                                            d=self.declaration()
                                            if not self.inputState.guessing:
                                                node = d;
                                        else:
                                            synPredMatched71 = False
                                            if (_tokenSet_10.member(self.LA(1))) and (_tokenSet_11.member(self.LA(2))):
                                                _m71 = self.mark()
                                                synPredMatched71 = True
                                                self.inputState.guessing += 1
                                                try:
                                                    pass
                                                    self.declaration_specifiers()
                                                    self.function_declarator(1)
                                                    self.match(LCURLY)
                                                except antlr.RecognitionException, pe:
                                                    synPredMatched71 = False
                                                self.rewind(_m71)
                                                self.inputState.guessing -= 1
                                            if synPredMatched71:
                                                pass
                                                if not self.inputState.guessing:
                                                    self.beginFieldDeclaration();
                                                fd=self.function_definition()
                                                if not self.inputState.guessing:
                                                    node = fd;
                                            else:
                                                synPredMatched74 = False
                                                if (self.LA(1)==LITERAL_inline or self.LA(1)==OPERATOR) and (_tokenSet_40.member(self.LA(2))):
                                                    _m74 = self.mark()
                                                    synPredMatched74 = True
                                                    self.inputState.guessing += 1
                                                    try:
                                                        pass
                                                        la1 = self.LA(1)
                                                        if False:
                                                            pass
                                                        elif la1 and la1 in [LITERAL_inline]:
                                                            pass
                                                            self.match(LITERAL_inline)
                                                        elif la1 and la1 in [OPERATOR]:
                                                            pass
                                                        else:
                                                                raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                                                            
                                                        self.conversion_function_decl_or_def()
                                                    except antlr.RecognitionException, pe:
                                                        synPredMatched74 = False
                                                    self.rewind(_m74)
                                                    self.inputState.guessing -= 1
                                                if synPredMatched74:
                                                    pass
                                                    la1 = self.LA(1)
                                                    if False:
                                                        pass
                                                    elif la1 and la1 in [LITERAL_inline]:
                                                        pass
                                                        self.match(LITERAL_inline)
                                                    elif la1 and la1 in [OPERATOR]:
                                                        pass
                                                    else:
                                                            raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                                                        
                                                    cfdd=self.conversion_function_decl_or_def()
                                                    if not self.inputState.guessing:
                                                        node = self.node("unfinished_udtc-439");
                                                else:
                                                    synPredMatched77 = False
                                                    if (_tokenSet_41.member(self.LA(1))) and (_tokenSet_42.member(self.LA(2))):
                                                        _m77 = self.mark()
                                                        synPredMatched77 = True
                                                        self.inputState.guessing += 1
                                                        try:
                                                            pass
                                                            self.qualified_id()
                                                            self.match(SEMICOLON)
                                                        except antlr.RecognitionException, pe:
                                                            synPredMatched77 = False
                                                        self.rewind(_m77)
                                                        self.inputState.guessing -= 1
                                                    if synPredMatched77:
                                                        pass
                                                        q=self.qualified_id()
                                                        self.match(SEMICOLON)
                                                        if not self.inputState.guessing:
                                                            self.end_of_stmt();
                                                        if not self.inputState.guessing:
                                                            node = self.node("unfinished_declhack-446");
                                                    else:
                                                        synPredMatched79 = False
                                                        if (_tokenSet_13.member(self.LA(1))) and (_tokenSet_43.member(self.LA(2))):
                                                            _m79 = self.mark()
                                                            synPredMatched79 = True
                                                            self.inputState.guessing += 1
                                                            try:
                                                                pass
                                                                self.declaration_specifiers()
                                                            except antlr.RecognitionException, pe:
                                                                synPredMatched79 = False
                                                            self.rewind(_m79)
                                                            self.inputState.guessing -= 1
                                                        if synPredMatched79:
                                                            pass
                                                            if not self.inputState.guessing:
                                                                self.beginFieldDeclaration();
                                                            if not self.inputState.guessing:
                                                                node = self.node("member_declaration");
                                                            ds=self.declaration_specifiers()
                                                            if not self.inputState.guessing:
                                                                node.add_child(ds);
                                                            la1 = self.LA(1)
                                                            if False:
                                                                pass
                                                            elif la1 and la1 in [ID,COLON,LITERAL__stdcall,LITERAL___stdcall,LPAREN,OPERATOR,LITERAL_this,LITERAL_true,LITERAL_false,STAR,AMPERSAND,TILDE,SCOPE,LITERAL__cdecl,LITERAL___cdecl,LITERAL__near,LITERAL___near,LITERAL__far,LITERAL___far,LITERAL___interrupt,LITERAL_pascal,LITERAL__pascal,LITERAL___pascal]:
                                                                pass
                                                                md=self.member_declarator_list()
                                                                if not self.inputState.guessing:
                                                                    node.add_child(md);
                                                            elif la1 and la1 in [SEMICOLON]:
                                                                pass
                                                            else:
                                                                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                                                                
                                                            self.match(SEMICOLON)
                                                            if not self.inputState.guessing:
                                                                self.end_of_stmt();
                                                        else:
                                                            synPredMatched82 = False
                                                            if (_tokenSet_33.member(self.LA(1))) and (_tokenSet_34.member(self.LA(2))):
                                                                _m82 = self.mark()
                                                                synPredMatched82 = True
                                                                self.inputState.guessing += 1
                                                                try:
                                                                    pass
                                                                    self.function_declarator(0)
                                                                    self.match(SEMICOLON)
                                                                except antlr.RecognitionException, pe:
                                                                    synPredMatched82 = False
                                                                self.rewind(_m82)
                                                                self.inputState.guessing -= 1
                                                            if synPredMatched82:
                                                                pass
                                                                if not self.inputState.guessing:
                                                                    self.beginFieldDeclaration();
                                                                fd=self.function_declarator(0)
                                                                self.match(SEMICOLON)
                                                                if not self.inputState.guessing:
                                                                    self.end_of_stmt();
                                                                if not self.inputState.guessing:
                                                                    node = fd;
                                                            elif (_tokenSet_33.member(self.LA(1))) and (_tokenSet_34.member(self.LA(2))):
                                                                pass
                                                                fd=self.function_declarator(1)
                                                                cs=self.compound_statement()
                                                                if not self.inputState.guessing:
                                                                    self.endFunctionDefinition();
                                                                if not self.inputState.guessing:
                                                                    node = self.node("member_declaration"); node.add_child(fd); node.add_child(cs);
                                                            else:
                                                                synPredMatched85 = False
                                                                if (self.LA(1)==LITERAL_template) and (self.LA(2)==LESSTHAN):
                                                                    _m85 = self.mark()
                                                                    synPredMatched85 = True
                                                                    self.inputState.guessing += 1
                                                                    try:
                                                                        pass
                                                                        self.template_head()
                                                                        ds=self.declaration_specifiers()
                                                                        la1 = self.LA(1)
                                                                        if False:
                                                                            pass
                                                                        elif la1 and la1 in [ID,LITERAL__stdcall,LITERAL___stdcall,LPAREN,OPERATOR,LITERAL_this,LITERAL_true,LITERAL_false,STAR,AMPERSAND,TILDE,SCOPE,LITERAL__cdecl,LITERAL___cdecl,LITERAL__near,LITERAL___near,LITERAL__far,LITERAL___far,LITERAL___interrupt,LITERAL_pascal,LITERAL__pascal,LITERAL___pascal]:
                                                                            pass
                                                                            self.init_declarator_list()
                                                                        elif la1 and la1 in [SEMICOLON]:
                                                                            pass
                                                                        else:
                                                                                raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                                                                            
                                                                        self.match(SEMICOLON)
                                                                    except antlr.RecognitionException, pe:
                                                                        synPredMatched85 = False
                                                                    self.rewind(_m85)
                                                                    self.inputState.guessing -= 1
                                                                if synPredMatched85:
                                                                    pass
                                                                    if not self.inputState.guessing:
                                                                        beginTemplateDeclaration(); node = self.node("member_declaration");
                                                                    th=self.template_head()
                                                                    if not self.inputState.guessing:
                                                                        node.add_child(th);
                                                                    ds=self.declaration_specifiers()
                                                                    if not self.inputState.guessing:
                                                                        node.add_child(ds);
                                                                    la1 = self.LA(1)
                                                                    if False:
                                                                        pass
                                                                    elif la1 and la1 in [ID,LITERAL__stdcall,LITERAL___stdcall,LPAREN,OPERATOR,LITERAL_this,LITERAL_true,LITERAL_false,STAR,AMPERSAND,TILDE,SCOPE,LITERAL__cdecl,LITERAL___cdecl,LITERAL__near,LITERAL___near,LITERAL__far,LITERAL___far,LITERAL___interrupt,LITERAL_pascal,LITERAL__pascal,LITERAL___pascal]:
                                                                        pass
                                                                        idl=self.init_declarator_list()
                                                                        if not self.inputState.guessing:
                                                                            node.add_child(idl);
                                                                    elif la1 and la1 in [SEMICOLON]:
                                                                        pass
                                                                    else:
                                                                            raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                                                                        
                                                                    self.match(SEMICOLON)
                                                                    if not self.inputState.guessing:
                                                                        self.end_of_stmt();
                                                                    if not self.inputState.guessing:
                                                                        endTemplateDeclaration();
                                                                elif (self.LA(1)==LITERAL_template) and (self.LA(2)==LESSTHAN):
                                                                    pass
                                                                    if not self.inputState.guessing:
                                                                        beginTemplateDefinition();
                                                                    th=self.template_head()
                                                                    if not self.inputState.guessing:
                                                                        node = self.node("templated_function"); node.add_child(th);
                                                                    synPredMatched89 = False
                                                                    if (_tokenSet_6.member(self.LA(1))) and (_tokenSet_7.member(self.LA(2))):
                                                                        _m89 = self.mark()
                                                                        synPredMatched89 = True
                                                                        self.inputState.guessing += 1
                                                                        try:
                                                                            pass
                                                                            self.ctor_decl_spec()
                                                                            if not self.qualifiedItemIsOneOf(qiCtor):
                                                                                raise antlr.SemanticException("self.qualifiedItemIsOneOf(qiCtor)")
                                                                        except antlr.RecognitionException, pe:
                                                                            synPredMatched89 = False
                                                                        self.rewind(_m89)
                                                                        self.inputState.guessing -= 1
                                                                    if synPredMatched89:
                                                                        pass
                                                                        cd=self.ctor_definition()
                                                                        if not self.inputState.guessing:
                                                                            node.add_child(cd);
                                                                    else:
                                                                        synPredMatched91 = False
                                                                        if (_tokenSet_2.member(self.LA(1))) and (_tokenSet_3.member(self.LA(2))):
                                                                            _m91 = self.mark()
                                                                            synPredMatched91 = True
                                                                            self.inputState.guessing += 1
                                                                            try:
                                                                                pass
                                                                                self.declaration_specifiers()
                                                                                self.function_declarator(0)
                                                                                self.match(SEMICOLON)
                                                                            except antlr.RecognitionException, pe:
                                                                                synPredMatched91 = False
                                                                            self.rewind(_m91)
                                                                            self.inputState.guessing -= 1
                                                                        if synPredMatched91:
                                                                            pass
                                                                            d=self.declaration()
                                                                            if not self.inputState.guessing:
                                                                                node.add_child(d);
                                                                        else:
                                                                            synPredMatched93 = False
                                                                            if (_tokenSet_10.member(self.LA(1))) and (_tokenSet_11.member(self.LA(2))):
                                                                                _m93 = self.mark()
                                                                                synPredMatched93 = True
                                                                                self.inputState.guessing += 1
                                                                                try:
                                                                                    pass
                                                                                    self.declaration_specifiers()
                                                                                    self.function_declarator(1)
                                                                                    self.match(LCURLY)
                                                                                except antlr.RecognitionException, pe:
                                                                                    synPredMatched93 = False
                                                                                self.rewind(_m93)
                                                                                self.inputState.guessing -= 1
                                                                            if synPredMatched93:
                                                                                pass
                                                                                fd=self.function_definition()
                                                                                if not self.inputState.guessing:
                                                                                    node.add_child(fd);
                                                                            elif (self.LA(1)==OPERATOR) and (_tokenSet_13.member(self.LA(2))):
                                                                                pass
                                                                                cfdd=self.conversion_function_decl_or_def()
                                                                                if not self.inputState.guessing:
                                                                                    node.add_child(cfdd);
                                                                            else:
                                                                                raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                                                                            
                                                                    if not self.inputState.guessing:
                                                                        endTemplateDefinition();
                                                                else:
                                                                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                                                                
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_44)
            else:
                raise ex
        
        return node
    
    def member_declarator_list(self):    
        node = None
        
        node = self.node("init_declarator_list");
        try:      ## for error handling
            pass
            md=self.member_declarator()
            if not self.inputState.guessing:
                node.add_child(md);
            while True:
                if (self.LA(1)==COMMA):
                    pass
                    self.match(COMMA)
                    md=self.member_declarator()
                    if not self.inputState.guessing:
                        node.add_child(md);
                else:
                    break
                
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_18)
            else:
                raise ex
        
        return node
    
    def ctor_declarator(self,
        definition
    ):    
        node = None
        
        node = self.node("ctor_declarator");
        try:      ## for error handling
            pass
            q=self.qualified_ctor_id()
            if not self.inputState.guessing:
                node.set_leaf(q);
            if not self.inputState.guessing:
                self.declaratorParameterList(definition);
            self.match(LPAREN)
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [LITERAL_typedef,LITERAL_enum,ID,LITERAL_inline,LITERAL_extern,LITERAL___extension__,LITERAL__inline,LITERAL___inline,LITERAL___inline__,LITERAL_virtual,LITERAL_explicit,LITERAL_friend,LITERAL__stdcall,LITERAL___stdcall,LITERAL_typename,LPAREN,LITERAL_const,LITERAL___const,LITERAL_auto,LITERAL_register,LITERAL_static,LITERAL_mutable,LITERAL_const_cast,LITERAL_volatile,LITERAL___volatile,LITERAL___restrict,LITERAL___restrict__,LITERAL_char,LITERAL_bool,LITERAL_short,LITERAL_int,74,75,76,LITERAL_long,LITERAL_signed,LITERAL___signed,LITERAL___signed__,LITERAL_unsigned,LITERAL_float,LITERAL_double,LITERAL_void,LITERAL__declspec,LITERAL___declspec,LITERAL___builtin_va_list,LITERAL_class,LITERAL_struct,LITERAL_union,OPERATOR,LITERAL_this,LITERAL_true,LITERAL_false,STAR,AMPERSAND,TILDE,ELLIPSIS,SCOPE,LITERAL__cdecl,LITERAL___cdecl,LITERAL__near,LITERAL___near,LITERAL__far,LITERAL___far,LITERAL___interrupt,LITERAL_pascal,LITERAL__pascal,LITERAL___pascal]:
                pass
                pl=self.parameter_list()
                if not self.inputState.guessing:
                    node.add_child(pl);
            elif la1 and la1 in [RPAREN]:
                pass
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
            self.match(RPAREN)
            if not self.inputState.guessing:
                self.declaratorEndParameterList(definition);
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [LITERAL_throw]:
                pass
                es=self.exception_specification()
                if not self.inputState.guessing:
                    node.add_child(es);
            elif la1 and la1 in [LCURLY,SEMICOLON,COLON]:
                pass
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_45)
            else:
                raise ex
        
        return node
    
    def compound_statement(self):    
        node = None
        
        node = self.node("compound_statement");
        try:      ## for error handling
            pass
            self.match(LCURLY)
            if not self.inputState.guessing:
                self.end_of_stmt(); self.enterNewLocalScope();
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [LITERAL_typedef,LITERAL_enum,ID,LCURLY,SEMICOLON,LITERAL_inline,LITERAL_extern,StringLiteral,LITERAL___extension__,LITERAL__inline,LITERAL___inline,LITERAL___inline__,LITERAL_virtual,LITERAL_explicit,LITERAL_friend,LITERAL__stdcall,LITERAL___stdcall,LITERAL_typename,LPAREN,LITERAL_const,LITERAL___const,LITERAL_auto,LITERAL_register,LITERAL_static,LITERAL_mutable,LITERAL_const_cast,LITERAL_volatile,LITERAL___volatile,LITERAL___restrict,LITERAL___restrict__,LITERAL_char,LITERAL_bool,LITERAL_short,LITERAL_int,74,75,76,LITERAL_long,LITERAL_signed,LITERAL___signed,LITERAL___signed__,LITERAL_unsigned,LITERAL_float,LITERAL_double,LITERAL_void,LITERAL__declspec,LITERAL___declspec,LITERAL___builtin_va_list,LITERAL_class,LITERAL_struct,LITERAL_union,OPERATOR,LITERAL_this,LITERAL_true,LITERAL_false,OCTALINT,STAR,AMPERSAND,TILDE,LITERAL_asm,LITERAL___asm,LITERAL___asm__,LITERAL_throw,LITERAL_case,LITERAL_default,LITERAL_if,LITERAL_switch,LITERAL_while,LITERAL_do,LITERAL_for,LITERAL_goto,LITERAL_continue,LITERAL_break,LITERAL_return,LITERAL_try,LITERAL_using,LITERAL__asm,SCOPE,PLUS,MINUS,PLUSPLUS,MINUSMINUS,LITERAL_sizeof,LITERAL_dynamic_cast,LITERAL_static_cast,LITERAL_reinterpret_cast,NOT,LITERAL_new,LITERAL_delete,DECIMALINT,HEXADECIMALINT,CharLiteral,FLOATONE,FLOATTWO]:
                pass
                sl=self.statement_list()
                if not self.inputState.guessing:
                    node.add_child(sl);
            elif la1 and la1 in [RCURLY]:
                pass
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
            self.match(RCURLY)
            if not self.inputState.guessing:
                self.exitLocalScope();
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_46)
            else:
                raise ex
        
        return node
    
    def access_specifier(self):    
        node = None
        
        try:      ## for error handling
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [LITERAL_public]:
                pass
                self.match(LITERAL_public)
                if not self.inputState.guessing:
                    node = self.node("public");
            elif la1 and la1 in [LITERAL_protected]:
                pass
                self.match(LITERAL_protected)
                if not self.inputState.guessing:
                    node = self.node("protected");
            elif la1 and la1 in [LITERAL_private]:
                pass
                self.match(LITERAL_private)
                if not self.inputState.guessing:
                    node = self.node("private");
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_47)
            else:
                raise ex
        
        return node
    
    def linkage_specification(self):    
        node = None
        
        node = self.node("linkage_specification");
        try:      ## for error handling
            pass
            self.match(LITERAL_extern)
            self.match(StringLiteral)
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [LCURLY]:
                pass
                self.match(LCURLY)
                while True:
                    if (_tokenSet_0.member(self.LA(1))):
                        pass
                        ed=self.external_declaration()
                        if not self.inputState.guessing:
                            node.add_child(ed);
                    else:
                        break
                    
                self.match(RCURLY)
            elif la1 and la1 in [LITERAL_typedef,LITERAL_enum,ID,LITERAL_inline,LITERAL_extern,LITERAL___extension__,LITERAL__inline,LITERAL___inline,LITERAL___inline__,LITERAL_virtual,LITERAL_explicit,LITERAL_friend,LITERAL__stdcall,LITERAL___stdcall,LITERAL_typename,LITERAL_const,LITERAL___const,LITERAL_auto,LITERAL_register,LITERAL_static,LITERAL_mutable,LITERAL_const_cast,LITERAL_volatile,LITERAL___volatile,LITERAL___restrict,LITERAL___restrict__,LITERAL_char,LITERAL_bool,LITERAL_short,LITERAL_int,74,75,76,LITERAL_long,LITERAL_signed,LITERAL___signed,LITERAL___signed__,LITERAL_unsigned,LITERAL_float,LITERAL_double,LITERAL_void,LITERAL__declspec,LITERAL___declspec,LITERAL___builtin_va_list,LITERAL_class,LITERAL_struct,LITERAL_union,LITERAL_using,SCOPE]:
                pass
                d=self.declaration()
                if not self.inputState.guessing:
                    node.add_child(d);
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_15)
            else:
                raise ex
        
        return node
    
    def attribute(self):    
        a = None
        
        a = self.node("attribute");
        try:      ## for error handling
            pass
            self.match(LITERAL___attribute__)
            self.match(LPAREN)
            self.match(LPAREN)
            adl=self.attribute_decl_list()
            if not self.inputState.guessing:
                a.add_child(adl);
            self.match(RPAREN)
            self.match(RPAREN)
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_17)
            else:
                raise ex
        
        return a
    
    def using_declaration(self):    
        node = None
        
        node = self.node("unfinished_using_declaration");
        try:      ## for error handling
            pass
            self.match(LITERAL_using)
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [LITERAL_namespace]:
                pass
                self.match(LITERAL_namespace)
                qid=self.qualified_id()
            elif la1 and la1 in [ID,OPERATOR,LITERAL_this,LITERAL_true,LITERAL_false,SCOPE]:
                pass
                qid=self.qualified_id()
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
            self.match(SEMICOLON)
            if not self.inputState.guessing:
                self.end_of_stmt();
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_15)
            else:
                raise ex
        
        return node
    
    def storage_class_specifier(self):    
        s = None
        
        try:      ## for error handling
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [LITERAL_auto]:
                pass
                self.match(LITERAL_auto)
                if not self.inputState.guessing:
                    s = "auto";
            elif la1 and la1 in [LITERAL_register]:
                pass
                self.match(LITERAL_register)
                if not self.inputState.guessing:
                    s = "register";
            elif la1 and la1 in [LITERAL_static]:
                pass
                self.match(LITERAL_static)
                if not self.inputState.guessing:
                    s = "static";
            elif la1 and la1 in [LITERAL_extern]:
                pass
                self.match(LITERAL_extern)
                if not self.inputState.guessing:
                    s = "extern";
            elif la1 and la1 in [LITERAL_mutable]:
                pass
                self.match(LITERAL_mutable)
                if not self.inputState.guessing:
                    s = "mutable";
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_48)
            else:
                raise ex
        
        return s
    
    def type_qualifier(self):    
        s = None
        
        try:      ## for error handling
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [LITERAL_const,LITERAL___const,LITERAL_const_cast]:
                pass
                la1 = self.LA(1)
                if False:
                    pass
                elif la1 and la1 in [LITERAL_const]:
                    pass
                    self.match(LITERAL_const)
                elif la1 and la1 in [LITERAL_const_cast]:
                    pass
                    self.match(LITERAL_const_cast)
                elif la1 and la1 in [LITERAL___const]:
                    pass
                    self.match(LITERAL___const)
                else:
                        raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                    
                if not self.inputState.guessing:
                    s = "const";
            elif la1 and la1 in [LITERAL_volatile,LITERAL___volatile]:
                pass
                la1 = self.LA(1)
                if False:
                    pass
                elif la1 and la1 in [LITERAL_volatile]:
                    pass
                    self.match(LITERAL_volatile)
                elif la1 and la1 in [LITERAL___volatile]:
                    pass
                    self.match(LITERAL___volatile)
                else:
                        raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                    
                if not self.inputState.guessing:
                    s = "volatile";
            elif la1 and la1 in [LITERAL___restrict,LITERAL___restrict__]:
                pass
                la1 = self.LA(1)
                if False:
                    pass
                elif la1 and la1 in [LITERAL___restrict]:
                    pass
                    self.match(LITERAL___restrict)
                elif la1 and la1 in [LITERAL___restrict__]:
                    pass
                    self.match(LITERAL___restrict__)
                else:
                        raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                    
                if not self.inputState.guessing:
                    s = "__restrict";
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_49)
            else:
                raise ex
        
        return s
    
    def type_specifier(self,
         ds
    ):    
        node = None
        
        try:      ## for error handling
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [ID,LITERAL_char,LITERAL_bool,LITERAL_short,LITERAL_int,74,75,76,LITERAL_long,LITERAL_signed,LITERAL___signed,LITERAL___signed__,LITERAL_unsigned,LITERAL_float,LITERAL_double,LITERAL_void,LITERAL__declspec,LITERAL___declspec,LITERAL___builtin_va_list,SCOPE]:
                pass
                node=self.simple_type_specifier()
            elif la1 and la1 in [LITERAL_class,LITERAL_struct,LITERAL_union]:
                pass
                node=self.class_specifier(ds)
            elif la1 and la1 in [LITERAL_enum]:
                pass
                node=self.enum_specifier()
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_17)
            else:
                raise ex
        
        return node
    
    def direct_declarator(self):    
        node = None
        
        dtor = None
        try:      ## for error handling
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [TILDE]:
                pass
                if not self.inputState.guessing:
                    node = self.node("direct_declarator");
                self.match(TILDE)
                dtor = self.LT(1)
                self.match(ID)
                if not self.inputState.guessing:
                    self.declaratorID(dtor.getText(),qiDtor);
                if not self.inputState.guessing:
                    printf("%d warning direct_declarator5 entered unexpectedly with %s\n",
                                            LT(1).getLine(),dtor.getText());
                self.match(LPAREN)
                if not self.inputState.guessing:
                    self.declaratorParameterList(0);
                la1 = self.LA(1)
                if False:
                    pass
                elif la1 and la1 in [LITERAL_typedef,LITERAL_enum,ID,LITERAL_inline,LITERAL_extern,LITERAL___extension__,LITERAL__inline,LITERAL___inline,LITERAL___inline__,LITERAL_virtual,LITERAL_explicit,LITERAL_friend,LITERAL__stdcall,LITERAL___stdcall,LITERAL_typename,LPAREN,LITERAL_const,LITERAL___const,LITERAL_auto,LITERAL_register,LITERAL_static,LITERAL_mutable,LITERAL_const_cast,LITERAL_volatile,LITERAL___volatile,LITERAL___restrict,LITERAL___restrict__,LITERAL_char,LITERAL_bool,LITERAL_short,LITERAL_int,74,75,76,LITERAL_long,LITERAL_signed,LITERAL___signed,LITERAL___signed__,LITERAL_unsigned,LITERAL_float,LITERAL_double,LITERAL_void,LITERAL__declspec,LITERAL___declspec,LITERAL___builtin_va_list,LITERAL_class,LITERAL_struct,LITERAL_union,OPERATOR,LITERAL_this,LITERAL_true,LITERAL_false,STAR,AMPERSAND,TILDE,ELLIPSIS,SCOPE,LITERAL__cdecl,LITERAL___cdecl,LITERAL__near,LITERAL___near,LITERAL__far,LITERAL___far,LITERAL___interrupt,LITERAL_pascal,LITERAL__pascal,LITERAL___pascal]:
                    pass
                    pl=self.parameter_list()
                elif la1 and la1 in [RPAREN]:
                    pass
                else:
                        raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                    
                self.match(RPAREN)
                if not self.inputState.guessing:
                    self.declaratorEndParameterList(0);
                if not self.inputState.guessing:
                    node.add_child(self.node("unfinished_direct_declarator"));
            elif la1 and la1 in [LPAREN]:
                pass
                self.match(LPAREN)
                d=self.declarator()
                if not self.inputState.guessing:
                    node = d;
                self.match(RPAREN)
                ds=self.declarator_suffixes()
                if not self.inputState.guessing:
                    node.add_child(ds);
            else:
                synPredMatched217 = False
                if (_tokenSet_41.member(self.LA(1))) and (_tokenSet_27.member(self.LA(2))):
                    _m217 = self.mark()
                    synPredMatched217 = True
                    self.inputState.guessing += 1
                    try:
                        pass
                        self.qualified_id()
                        self.match(LPAREN)
                        la1 = self.LA(1)
                        if False:
                            pass
                        elif la1 and la1 in [RPAREN]:
                            pass
                            self.match(RPAREN)
                        elif la1 and la1 in [LITERAL_typedef,LITERAL_enum,ID,LITERAL_inline,LITERAL_extern,LITERAL___extension__,LITERAL__inline,LITERAL___inline,LITERAL___inline__,LITERAL_virtual,LITERAL_explicit,LITERAL_friend,LITERAL__stdcall,LITERAL___stdcall,LITERAL_typename,LITERAL_const,LITERAL___const,LITERAL_auto,LITERAL_register,LITERAL_static,LITERAL_mutable,LITERAL_const_cast,LITERAL_volatile,LITERAL___volatile,LITERAL___restrict,LITERAL___restrict__,LITERAL_char,LITERAL_bool,LITERAL_short,LITERAL_int,74,75,76,LITERAL_long,LITERAL_signed,LITERAL___signed,LITERAL___signed__,LITERAL_unsigned,LITERAL_float,LITERAL_double,LITERAL_void,LITERAL__declspec,LITERAL___declspec,LITERAL___builtin_va_list,LITERAL_class,LITERAL_struct,LITERAL_union,SCOPE]:
                            pass
                            self.declaration_specifiers()
                        else:
                                raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                            
                    except antlr.RecognitionException, pe:
                        synPredMatched217 = False
                    self.rewind(_m217)
                    self.inputState.guessing -= 1
                if synPredMatched217:
                    pass
                    if not self.inputState.guessing:
                        node = self.node("direct_declarator");
                    id=self.qualified_id()
                    if not self.inputState.guessing:
                        node.set_leaf(id);
                    if not self.inputState.guessing:
                        self.declaratorID(id, qiFun);
                    self.match(LPAREN)
                    if not self.inputState.guessing:
                        self.declaratorParameterList(0);
                    la1 = self.LA(1)
                    if False:
                        pass
                    elif la1 and la1 in [LITERAL_typedef,LITERAL_enum,ID,LITERAL_inline,LITERAL_extern,LITERAL___extension__,LITERAL__inline,LITERAL___inline,LITERAL___inline__,LITERAL_virtual,LITERAL_explicit,LITERAL_friend,LITERAL__stdcall,LITERAL___stdcall,LITERAL_typename,LPAREN,LITERAL_const,LITERAL___const,LITERAL_auto,LITERAL_register,LITERAL_static,LITERAL_mutable,LITERAL_const_cast,LITERAL_volatile,LITERAL___volatile,LITERAL___restrict,LITERAL___restrict__,LITERAL_char,LITERAL_bool,LITERAL_short,LITERAL_int,74,75,76,LITERAL_long,LITERAL_signed,LITERAL___signed,LITERAL___signed__,LITERAL_unsigned,LITERAL_float,LITERAL_double,LITERAL_void,LITERAL__declspec,LITERAL___declspec,LITERAL___builtin_va_list,LITERAL_class,LITERAL_struct,LITERAL_union,OPERATOR,LITERAL_this,LITERAL_true,LITERAL_false,STAR,AMPERSAND,TILDE,ELLIPSIS,SCOPE,LITERAL__cdecl,LITERAL___cdecl,LITERAL__near,LITERAL___near,LITERAL__far,LITERAL___far,LITERAL___interrupt,LITERAL_pascal,LITERAL__pascal,LITERAL___pascal]:
                        pass
                        pl=self.parameter_list()
                        if not self.inputState.guessing:
                            node.add_child(pl);
                    elif la1 and la1 in [RPAREN]:
                        pass
                    else:
                            raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                        
                    self.match(RPAREN)
                    if not self.inputState.guessing:
                        self.declaratorEndParameterList(0);
                    while True:
                        if (_tokenSet_50.member(self.LA(1))):
                            pass
                            tq=self.type_qualifier()
                            if not self.inputState.guessing:
                                node.add_child(self.node(tq));
                        else:
                            break
                        
                    la1 = self.LA(1)
                    if False:
                        pass
                    elif la1 and la1 in [LITERAL_throw]:
                        pass
                        es=self.exception_specification()
                        if not self.inputState.guessing:
                            node.add_child(es);
                    elif la1 and la1 in [LESSTHAN,GREATERTHAN,ID,SEMICOLON,RCURLY,ASSIGNEQUAL,COLON,COMMA,LITERAL__stdcall,LITERAL___stdcall,LITERAL___attribute__,LPAREN,RPAREN,OPERATOR,LITERAL_this,LITERAL_true,LITERAL_false,STAR,AMPERSAND,LSQUARE,RSQUARE,TILDE,LITERAL_asm,LITERAL___asm,LITERAL___asm__,ELLIPSIS,SCOPE,TIMESEQUAL,DIVIDEEQUAL,MINUSEQUAL,PLUSEQUAL,MODEQUAL,SHIFTLEFTEQUAL,SHIFTRIGHTEQUAL,BITWISEANDEQUAL,BITWISEXOREQUAL,BITWISEOREQUAL,QUESTIONMARK,OR,AND,BITWISEOR,BITWISEXOR,NOTEQUAL,EQUAL,LESSTHANOREQUALTO,GREATERTHANOREQUALTO,SHIFTLEFT,SHIFTRIGHT,PLUS,MINUS,DIVIDE,MOD,DOTMBR,POINTERTOMBR,LITERAL__cdecl,LITERAL___cdecl,LITERAL__near,LITERAL___near,LITERAL__far,LITERAL___far,LITERAL___interrupt,LITERAL_pascal,LITERAL__pascal,LITERAL___pascal]:
                        pass
                    else:
                            raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                        
                else:
                    synPredMatched223 = False
                    if (_tokenSet_41.member(self.LA(1))) and (_tokenSet_27.member(self.LA(2))):
                        _m223 = self.mark()
                        synPredMatched223 = True
                        self.inputState.guessing += 1
                        try:
                            pass
                            self.qualified_id()
                            self.match(LPAREN)
                            self.qualified_id()
                        except antlr.RecognitionException, pe:
                            synPredMatched223 = False
                        self.rewind(_m223)
                        self.inputState.guessing -= 1
                    if synPredMatched223:
                        pass
                        if not self.inputState.guessing:
                            node = self.node("direct_declarator");
                        id=self.qualified_id()
                        if not self.inputState.guessing:
                            node.set_leaf(id);
                        if not self.inputState.guessing:
                            self.declaratorID(id,qiVar);
                        self.match(LPAREN)
                        el=self.expression_list()
                        if not self.inputState.guessing:
                            node.add_child(el);
                        self.match(RPAREN)
                    else:
                        synPredMatched225 = False
                        if (_tokenSet_41.member(self.LA(1))) and (_tokenSet_27.member(self.LA(2))):
                            _m225 = self.mark()
                            synPredMatched225 = True
                            self.inputState.guessing += 1
                            try:
                                pass
                                self.qualified_id()
                                self.match(LSQUARE)
                            except antlr.RecognitionException, pe:
                                synPredMatched225 = False
                            self.rewind(_m225)
                            self.inputState.guessing -= 1
                        if synPredMatched225:
                            pass
                            if not self.inputState.guessing:
                                node = self.node("direct_declarator");
                            id=self.qualified_id()
                            if not self.inputState.guessing:
                                node.set_leaf(id);
                            if not self.inputState.guessing:
                                if (self._td == true): self.declaratorID(id, qiType);
                            if not self.inputState.guessing:
                                if (self._td != true): self.declaratorID(id, qiVar);
                            if not self.inputState.guessing:
                                self.is_address = false; self.is_pointer = false;
                            _cnt228= 0
                            while True:
                                if (self.LA(1)==LSQUARE) and (_tokenSet_51.member(self.LA(2))):
                                    pass
                                    self.match(LSQUARE)
                                    la1 = self.LA(1)
                                    if False:
                                        pass
                                    elif la1 and la1 in [ID,StringLiteral,LPAREN,LITERAL_const_cast,LITERAL_char,LITERAL_bool,LITERAL_short,LITERAL_int,74,75,76,LITERAL_long,LITERAL_signed,LITERAL___signed,LITERAL___signed__,LITERAL_unsigned,LITERAL_float,LITERAL_double,LITERAL_void,LITERAL__declspec,LITERAL___declspec,LITERAL___builtin_va_list,OPERATOR,LITERAL_this,LITERAL_true,LITERAL_false,OCTALINT,STAR,AMPERSAND,TILDE,SCOPE,PLUS,MINUS,PLUSPLUS,MINUSMINUS,LITERAL_sizeof,LITERAL_dynamic_cast,LITERAL_static_cast,LITERAL_reinterpret_cast,NOT,LITERAL_new,LITERAL_delete,DECIMALINT,HEXADECIMALINT,CharLiteral,FLOATONE,FLOATTWO]:
                                        pass
                                        ce=self.constant_expression()
                                        if not self.inputState.guessing:
                                            node.add_child(ce);
                                    elif la1 and la1 in [RSQUARE]:
                                        pass
                                    else:
                                            raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                                        
                                    self.match(RSQUARE)
                                else:
                                    break
                                
                                _cnt228 += 1
                            if _cnt228 < 1:
                                raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                            if not self.inputState.guessing:
                                self.declaratorArray();
                        elif (_tokenSet_41.member(self.LA(1))) and (_tokenSet_52.member(self.LA(2))):
                            pass
                            if not self.inputState.guessing:
                                node = self.node("direct_declarator");
                            id=self.qualified_id()
                            if not self.inputState.guessing:
                                node.set_leaf(id);
                            if not self.inputState.guessing:
                                if (self._td == true): self.declaratorID(id,qiType);
                            if not self.inputState.guessing:
                                if (self._td != true): self.declaratorID(id,qiVar);
                            if not self.inputState.guessing:
                                self.is_address = false; self.is_pointer = false;
                        else:
                            raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                        
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_53)
            else:
                raise ex
        
        return node
    
    def attribute_decl_list(self):    
        node = None
        
        node = self.node("attribute_decl_list");
        try:      ## for error handling
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [LITERAL___mode__]:
                pass
                self.match(LITERAL___mode__)
                self.match(LPAREN)
                self.match(ID)
                self.match(RPAREN)
                if not self.inputState.guessing:
                    node.set_name("mode");
            elif la1 and la1 in [LITERAL_packed]:
                pass
                self.match(LITERAL_packed)
                if not self.inputState.guessing:
                    node.set_name("packed");
            elif la1 and la1 in [LITERAL_const]:
                pass
                self.match(LITERAL_const)
                if not self.inputState.guessing:
                    node.set_name("const");
            elif la1 and la1 in [LITERAL___const]:
                pass
                self.match(LITERAL___const)
                if not self.inputState.guessing:
                    node.set_name("const");
            elif la1 and la1 in [LITERAL___noreturn__]:
                pass
                self.match(LITERAL___noreturn__)
                if not self.inputState.guessing:
                    node.set_name("noreturn");
            elif la1 and la1 in [LITERAL___returns_twice__]:
                pass
                self.match(LITERAL___returns_twice__)
                if not self.inputState.guessing:
                    node.set_name("returns_twice");
            elif la1 and la1 in [LITERAL___noinline__]:
                pass
                self.match(LITERAL___noinline__)
                if not self.inputState.guessing:
                    node.set_name("noinline");
            elif la1 and la1 in [LITERAL___always_inline__]:
                pass
                self.match(LITERAL___always_inline__)
                if not self.inputState.guessing:
                    node.set_name("always_inline");
            elif la1 and la1 in [LITERAL___flatten__]:
                pass
                self.match(LITERAL___flatten__)
                if not self.inputState.guessing:
                    node.set_name("flatten");
            elif la1 and la1 in [LITERAL___pure__]:
                pass
                self.match(LITERAL___pure__)
                if not self.inputState.guessing:
                    node.set_name("pure");
            elif la1 and la1 in [LITERAL___const__]:
                pass
                self.match(LITERAL___const__)
                if not self.inputState.guessing:
                    node.set_name("const");
            elif la1 and la1 in [LITERAL___nothrow__]:
                pass
                self.match(LITERAL___nothrow__)
                if not self.inputState.guessing:
                    node.set_name("nothrow");
            elif la1 and la1 in [LITERAL___sentinel__]:
                pass
                self.match(LITERAL___sentinel__)
                if not self.inputState.guessing:
                    node.set_name("sentinel");
            elif la1 and la1 in [LITERAL___format__]:
                pass
                self.match(LITERAL___format__)
                if not self.inputState.guessing:
                    node.set_name("format");
                self.match(LPAREN)
                self.match(ID)
                self.match(COMMA)
                c1=self.constant()
                if not self.inputState.guessing:
                    node.add_child(c1);
                self.match(COMMA)
                c2=self.constant()
                if not self.inputState.guessing:
                    node.add_child(c2);
                self.match(RPAREN)
            elif la1 and la1 in [LITERAL___format_arg__]:
                pass
                self.match(LITERAL___format_arg__)
                if not self.inputState.guessing:
                    node.set_name("format_arg");
            elif la1 and la1 in [LITERAL___no_instrument_function__]:
                pass
                self.match(LITERAL___no_instrument_function__)
                if not self.inputState.guessing:
                    node.set_name("no_instrument_function");
            elif la1 and la1 in [LITERAL___section__]:
                pass
                self.match(LITERAL___section__)
                if not self.inputState.guessing:
                    node.set_name("section");
            elif la1 and la1 in [LITERAL___constructor__]:
                pass
                self.match(LITERAL___constructor__)
                if not self.inputState.guessing:
                    node.set_name("constructor");
            elif la1 and la1 in [LITERAL___destructor__]:
                pass
                self.match(LITERAL___destructor__)
                if not self.inputState.guessing:
                    node.set_name("destructor");
            elif la1 and la1 in [LITERAL___used__]:
                pass
                self.match(LITERAL___used__)
                if not self.inputState.guessing:
                    node.set_name("used");
            elif la1 and la1 in [LITERAL___unused__]:
                pass
                self.match(LITERAL___unused__)
                if not self.inputState.guessing:
                    node.set_name("unused");
            elif la1 and la1 in [LITERAL___deprecated__]:
                pass
                self.match(LITERAL___deprecated__)
                if not self.inputState.guessing:
                    node.set_name("deprecated");
            elif la1 and la1 in [LITERAL___weak__]:
                pass
                self.match(LITERAL___weak__)
                if not self.inputState.guessing:
                    node.set_name("weak");
            elif la1 and la1 in [LITERAL___malloc__]:
                pass
                self.match(LITERAL___malloc__)
                if not self.inputState.guessing:
                    node.set_name("malloc");
            elif la1 and la1 in [LITERAL___alias__]:
                pass
                self.match(LITERAL___alias__)
                if not self.inputState.guessing:
                    node.set_name("alias");
            elif la1 and la1 in [LITERAL___warn_unused_result__]:
                pass
                self.match(LITERAL___warn_unused_result__)
                if not self.inputState.guessing:
                    node.set_name("warn_unused_result");
            elif la1 and la1 in [LITERAL___nonnull__]:
                pass
                self.match(LITERAL___nonnull__)
                if not self.inputState.guessing:
                    node.set_name("nonnull");
                self.match(LPAREN)
                c1=self.constant()
                if not self.inputState.guessing:
                    node.add_child(c1);
                while True:
                    if (self.LA(1)==COMMA):
                        pass
                        self.match(COMMA)
                        c2=self.constant()
                        if not self.inputState.guessing:
                            node.add_child(c2);
                    else:
                        break
                    
                self.match(RPAREN)
            elif la1 and la1 in [LITERAL___externally_visible__]:
                pass
                self.match(LITERAL___externally_visible__)
                if not self.inputState.guessing:
                    node.set_name("externally_visible");
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_54)
            else:
                raise ex
        
        return node
    
    def constant(self):    
        node = None
        
        oi = None
        di = None
        hi = None
        cl = None
        sl = None
        f1 = None
        f2 = None
        node = self.node("expression"); node.set_leaf("literal");
        litnode = self.node("literal"); node.add_child(litnode);
        try:      ## for error handling
            pass
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [OCTALINT]:
                pass
                oi = self.LT(1)
                self.match(OCTALINT)
                if not self.inputState.guessing:
                    litnode.set_leaf(oi.getText()); litnode.set_name("int");
            elif la1 and la1 in [DECIMALINT]:
                pass
                di = self.LT(1)
                self.match(DECIMALINT)
                if not self.inputState.guessing:
                    litnode.set_leaf(di.getText()); litnode.set_name("int");
            elif la1 and la1 in [HEXADECIMALINT]:
                pass
                hi = self.LT(1)
                self.match(HEXADECIMALINT)
                if not self.inputState.guessing:
                    litnode.set_leaf(hi.getText()); litnode.set_name("int");
            elif la1 and la1 in [CharLiteral]:
                pass
                cl = self.LT(1)
                self.match(CharLiteral)
                if not self.inputState.guessing:
                    litnode.set_leaf(cl.getText()); litnode.set_name("char");
            elif la1 and la1 in [StringLiteral]:
                pass
                _cnt508= 0
                while True:
                    if (self.LA(1)==StringLiteral):
                        pass
                        sl = self.LT(1)
                        self.match(StringLiteral)
                        if not self.inputState.guessing:
                            litnode.extend_leaf(sl.getText());
                    else:
                        break
                    
                    _cnt508 += 1
                if _cnt508 < 1:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                if not self.inputState.guessing:
                    litnode.set_name("string");
            elif la1 and la1 in [FLOATONE]:
                pass
                f1 = self.LT(1)
                self.match(FLOATONE)
                if not self.inputState.guessing:
                    litnode.set_leaf(f1.getText()); litnode.set_name("float");
            elif la1 and la1 in [FLOATTWO]:
                pass
                f2 = self.LT(1)
                self.match(FLOATTWO)
                if not self.inputState.guessing:
                    litnode.set_leaf(f2.getText()); litnode.set_name("float");
            elif la1 and la1 in [LITERAL_true]:
                pass
                self.match(LITERAL_true)
                if not self.inputState.guessing:
                    litnode.set_leaf("true"); litnode.set_name("bool");
            elif la1 and la1 in [LITERAL_false]:
                pass
                self.match(LITERAL_false)
                if not self.inputState.guessing:
                    litnode.set_leaf("false"); litnode.set_name("bool");
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_55)
            else:
                raise ex
        
        return node
    
    def simple_type_specifier(self):    
        node = None
        
        node = self.node("simple_type_specifier");
        try:      ## for error handling
            pass
            if ((self.LA(1)==ID or self.LA(1)==SCOPE) and (_tokenSet_17.member(self.LA(2))) and (self.qualifiedItemIsOneOf(qiType|qiCtor))):
                pass
                s=self.qualified_type()
                if not self.inputState.guessing:
                    node.add_child(self.node(s));
            elif ((self.LA(1) >= LITERAL_char and self.LA(1) <= LITERAL___builtin_va_list)):
                pass
                _cnt135= 0
                while True:
                    la1 = self.LA(1)
                    if False:
                        pass
                    elif la1 and la1 in [LITERAL_char]:
                        pass
                        self.match(LITERAL_char)
                        if not self.inputState.guessing:
                            node.add_child(self.node("char"));
                    elif la1 and la1 in [LITERAL_bool]:
                        pass
                        self.match(LITERAL_bool)
                        if not self.inputState.guessing:
                            node.add_child(self.node("bool"));
                    elif la1 and la1 in [LITERAL_short]:
                        pass
                        self.match(LITERAL_short)
                        if not self.inputState.guessing:
                            node.add_child(self.node("short"));
                    elif la1 and la1 in [LITERAL_int]:
                        pass
                        self.match(LITERAL_int)
                        if not self.inputState.guessing:
                            node.add_child(self.node("int"));
                    elif la1 and la1 in [74,75]:
                        pass
                        la1 = self.LA(1)
                        if False:
                            pass
                        elif la1 and la1 in [74]:
                            pass
                            self.match(74)
                        elif la1 and la1 in [75]:
                            pass
                            self.match(75)
                        else:
                                raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                            
                        if not self.inputState.guessing:
                            node.add_child(self.node("__int64"));
                    elif la1 and la1 in [76]:
                        pass
                        self.match(76)
                        if not self.inputState.guessing:
                            node.add_child(self.node("__w64"));
                    elif la1 and la1 in [LITERAL_long]:
                        pass
                        self.match(LITERAL_long)
                        if not self.inputState.guessing:
                            node.add_child(self.node("long"));
                    elif la1 and la1 in [LITERAL_signed,LITERAL___signed,LITERAL___signed__]:
                        pass
                        la1 = self.LA(1)
                        if False:
                            pass
                        elif la1 and la1 in [LITERAL_signed]:
                            pass
                            self.match(LITERAL_signed)
                        elif la1 and la1 in [LITERAL___signed]:
                            pass
                            self.match(LITERAL___signed)
                        elif la1 and la1 in [LITERAL___signed__]:
                            pass
                            self.match(LITERAL___signed__)
                        else:
                                raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                            
                        if not self.inputState.guessing:
                            node.add_child(self.node("signed"));
                    elif la1 and la1 in [LITERAL_unsigned]:
                        pass
                        self.match(LITERAL_unsigned)
                        if not self.inputState.guessing:
                            node.add_child(self.node("unsigned"));
                    elif la1 and la1 in [LITERAL_float]:
                        pass
                        self.match(LITERAL_float)
                        if not self.inputState.guessing:
                            node.add_child(self.node("float"));
                    elif la1 and la1 in [LITERAL_double]:
                        pass
                        self.match(LITERAL_double)
                        if not self.inputState.guessing:
                            node.add_child(self.node("double"));
                    elif la1 and la1 in [LITERAL_void]:
                        pass
                        self.match(LITERAL_void)
                        if not self.inputState.guessing:
                            node.add_child(self.node("void"));
                    elif la1 and la1 in [LITERAL__declspec,LITERAL___declspec]:
                        pass
                        la1 = self.LA(1)
                        if False:
                            pass
                        elif la1 and la1 in [LITERAL__declspec]:
                            pass
                            self.match(LITERAL__declspec)
                        elif la1 and la1 in [LITERAL___declspec]:
                            pass
                            self.match(LITERAL___declspec)
                        else:
                                raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                            
                        self.match(LPAREN)
                        self.match(ID)
                        self.match(RPAREN)
                    elif la1 and la1 in [LITERAL___builtin_va_list]:
                        pass
                        self.match(LITERAL___builtin_va_list)
                        if not self.inputState.guessing:
                            node.add_child(self.node("__builtin_va_list"));
                    else:
                            break
                        
                    _cnt135 += 1
                if _cnt135 < 1:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
            else:
                synPredMatched137 = False
                if (self.LA(1)==ID or self.LA(1)==SCOPE) and (_tokenSet_17.member(self.LA(2))):
                    _m137 = self.mark()
                    synPredMatched137 = True
                    self.inputState.guessing += 1
                    try:
                        pass
                        self.qualified_type()
                        self.qualified_id()
                    except antlr.RecognitionException, pe:
                        synPredMatched137 = False
                    self.rewind(_m137)
                    self.inputState.guessing -= 1
                if synPredMatched137:
                    pass
                    s=self.qualified_type()
                    if not self.inputState.guessing:
                        node.add_child(self.node(s));
                else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_17)
            else:
                raise ex
        
        return node
    
    def class_specifier(self,
         ds
    ):    
        node = None
        
        try:      ## for error handling
            pass
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [LITERAL_class]:
                pass
                self.match(LITERAL_class)
                if not self.inputState.guessing:
                    ts = "struct"; node = self.node("class");
            elif la1 and la1 in [LITERAL_struct]:
                pass
                self.match(LITERAL_struct)
                if not self.inputState.guessing:
                    ts = "union"; node = self.node("struct");
            elif la1 and la1 in [LITERAL_union]:
                pass
                self.match(LITERAL_union)
                if not self.inputState.guessing:
                    ts = "class"; node = self.node("union");
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [ID,OPERATOR,LITERAL_this,LITERAL_true,LITERAL_false,SCOPE]:
                pass
                id=self.qualified_id()
                if not self.inputState.guessing:
                    node.set_leaf(id);
                if (self.LA(1)==LCURLY or self.LA(1)==COLON) and (_tokenSet_44.member(self.LA(2))):
                    pass
                    if not self.inputState.guessing:
                        saveClass = self.enclosingClass; self.enclosingClass = id;
                    la1 = self.LA(1)
                    if False:
                        pass
                    elif la1 and la1 in [COLON]:
                        pass
                        bases=self.base_clause()
                        if not self.inputState.guessing:
                            node.add_child(bases);
                    elif la1 and la1 in [LCURLY]:
                        pass
                    else:
                            raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                        
                    self.match(LCURLY)
                    if not self.inputState.guessing:
                        self.beginClassDefinition(ts, id);
                    if not self.inputState.guessing:
                        members = self.node("members");
                    while True:
                        if (_tokenSet_56.member(self.LA(1))):
                            pass
                            md=self.member_declaration()
                            if not self.inputState.guessing:
                                members.add_child(md);
                        else:
                            break
                        
                    if not self.inputState.guessing:
                        self.endClassDefinition();
                    self.match(RCURLY)
                    if not self.inputState.guessing:
                        node.add_child(members); self.enclosingClass = saveClass;
                elif (_tokenSet_17.member(self.LA(1))) and (_tokenSet_38.member(self.LA(2))):
                    pass
                    if not self.inputState.guessing:
                        self.classForwardDeclaration(ts, ds, id);
                else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
            elif la1 and la1 in [LCURLY]:
                pass
                self.match(LCURLY)
                if not self.inputState.guessing:
                    saveClass = self.enclosingClass; self.enclosingClass = "__anonymous";
                if not self.inputState.guessing:
                    self.beginClassDefinition(ts, "anonymous");
                if not self.inputState.guessing:
                    members = self.node("members");
                while True:
                    if (_tokenSet_56.member(self.LA(1))):
                        pass
                        md=self.member_declaration()
                        if not self.inputState.guessing:
                            members.add_child(md);
                    else:
                        break
                    
                if not self.inputState.guessing:
                    self.endClassDefinition();
                if not self.inputState.guessing:
                    node.add_child(members);
                self.match(RCURLY)
                if not self.inputState.guessing:
                    self.enclosingClass = saveClass;
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
            if (self.LA(1)==LITERAL___attribute__) and (self.LA(2)==LPAREN):
                pass
                a=self.attribute()
                if not self.inputState.guessing:
                    node.add_child(a);
            elif (_tokenSet_17.member(self.LA(1))) and (_tokenSet_38.member(self.LA(2))):
                pass
            else:
                raise antlr.NoViableAltException(self.LT(1), self.getFilename())
            
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_17)
            else:
                raise ex
        
        return node
    
    def qualified_type(self):    
        q = None
        
        id = None
        try:      ## for error handling
            pass
            s=self.scope_override()
            id = self.LT(1)
            self.match(ID)
            if (self.LA(1)==LESSTHAN) and (_tokenSet_36.member(self.LA(2))):
                pass
                self.match(LESSTHAN)
                tal=self.template_argument_list()
                self.match(GREATERTHAN)
            elif (_tokenSet_57.member(self.LA(1))) and (_tokenSet_38.member(self.LA(2))):
                pass
            else:
                raise antlr.NoViableAltException(self.LT(1), self.getFilename())
            
            if not self.inputState.guessing:
                qitem = s;
            if not self.inputState.guessing:
                qitem +=  id.getText();
            if not self.inputState.guessing:
                q = qitem;
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_57)
            else:
                raise ex
        
        return q
    
    def template_argument_list(self):    
        node = None
        
        node = self.node("template_argument_list");
        try:      ## for error handling
            pass
            ta=self.template_argument()
            if not self.inputState.guessing:
                node.add_child(ta);
            while True:
                if (self.LA(1)==COMMA):
                    pass
                    self.match(COMMA)
                    ta=self.template_argument()
                    if not self.inputState.guessing:
                        node.add_child(ta);
                else:
                    break
                
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_58)
            else:
                raise ex
        
        return node
    
    def base_clause(self):    
        node = None
        
        node = self.node("base_clause");
        try:      ## for error handling
            pass
            self.match(COLON)
            bs=self.base_specifier()
            if not self.inputState.guessing:
                node.add_child(bs);
            while True:
                if (self.LA(1)==COMMA):
                    pass
                    self.match(COMMA)
                    bs=self.base_specifier()
                    if not self.inputState.guessing:
                        node.add_child(bs);
                else:
                    break
                
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_59)
            else:
                raise ex
        
        return node
    
    def enumerator_list(self):    
        node = None
        
        node = self.node("enumerator_list");
        try:      ## for error handling
            pass
            e=self.enumerator()
            if not self.inputState.guessing:
                node.add_child(e);
            while True:
                if (self.LA(1)==COMMA) and (self.LA(2)==ID):
                    pass
                    self.match(COMMA)
                    e=self.enumerator()
                    if not self.inputState.guessing:
                        node.add_child(e);
                else:
                    break
                
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [COMMA]:
                pass
                self.match(COMMA)
            elif la1 and la1 in [RCURLY]:
                pass
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_60)
            else:
                raise ex
        
        return node
    
    def enumerator(self):    
        node = None
        
        id = None
        node = self.node("enumerator");
        try:      ## for error handling
            pass
            id = self.LT(1)
            self.match(ID)
            if not self.inputState.guessing:
                node.set_leaf(id.getText());
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [ASSIGNEQUAL]:
                pass
                self.match(ASSIGNEQUAL)
                cx=self.constant_expression()
                if not self.inputState.guessing:
                    node.add_child(cx);
            elif la1 and la1 in [RCURLY,COMMA]:
                pass
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
            if not self.inputState.guessing:
                self.enumElement(id.getText());
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_61)
            else:
                raise ex
        
        return node
    
    def constant_expression(self):    
        node = None
        
        try:      ## for error handling
            pass
            cs=self.conditional_expression()
            if not self.inputState.guessing:
                node = cs;
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_62)
            else:
                raise ex
        
        return node
    
    def optor(self):    
        out = None
        
        try:      ## for error handling
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [LITERAL_new]:
                pass
                self.match(LITERAL_new)
                if not self.inputState.guessing:
                    out = "new";
                if (self.LA(1)==LSQUARE) and (self.LA(2)==RSQUARE):
                    pass
                    self.match(LSQUARE)
                    self.match(RSQUARE)
                elif (_tokenSet_63.member(self.LA(1))) and (_tokenSet_38.member(self.LA(2))):
                    pass
                else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
            elif la1 and la1 in [LITERAL_delete]:
                pass
                self.match(LITERAL_delete)
                if not self.inputState.guessing:
                    out = "delete";
                if (self.LA(1)==LSQUARE) and (self.LA(2)==RSQUARE):
                    pass
                    self.match(LSQUARE)
                    self.match(RSQUARE)
                elif (_tokenSet_63.member(self.LA(1))) and (_tokenSet_38.member(self.LA(2))):
                    pass
                else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
            elif la1 and la1 in [LPAREN]:
                pass
                self.match(LPAREN)
                self.match(RPAREN)
                if not self.inputState.guessing:
                    out = "()";
            elif la1 and la1 in [LSQUARE]:
                pass
                self.match(LSQUARE)
                self.match(RSQUARE)
                if not self.inputState.guessing:
                    out = "arrayindex";
            elif la1 and la1 in [LESSTHAN,GREATERTHAN,ASSIGNEQUAL,COMMA,STAR,AMPERSAND,TILDE,TIMESEQUAL,DIVIDEEQUAL,MINUSEQUAL,PLUSEQUAL,MODEQUAL,SHIFTLEFTEQUAL,SHIFTRIGHTEQUAL,BITWISEANDEQUAL,BITWISEXOREQUAL,BITWISEOREQUAL,OR,AND,BITWISEOR,BITWISEXOR,NOTEQUAL,EQUAL,LESSTHANOREQUALTO,GREATERTHANOREQUALTO,SHIFTLEFT,SHIFTRIGHT,PLUS,MINUS,DIVIDE,MOD,POINTERTOMBR,PLUSPLUS,MINUSMINUS,POINTERTO,NOT]:
                pass
                x=self.optor_simple_tokclass()
                if not self.inputState.guessing:
                    out = x;
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_63)
            else:
                raise ex
        
        return out
    
    def typeID(self):    
        
        try:      ## for error handling
            if not self.isTypeName(self.LT(1).getText()):
                raise antlr.SemanticException("self.isTypeName(self.LT(1).getText())")
            pass
            self.match(ID)
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_1)
            else:
                raise ex
        
    
    def init_declarator(self):    
        node = None
        
        node = self.node("init_declarator");
        try:      ## for error handling
            pass
            d=self.declarator()
            if not self.inputState.guessing:
                node.add_child(d);
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [LITERAL_asm,LITERAL___asm,LITERAL___asm__]:
                pass
                an=self.func_asm_name()
                if not self.inputState.guessing:
                    node.add_child(an);
            elif la1 and la1 in [SEMICOLON,ASSIGNEQUAL,COMMA,LITERAL___attribute__,LPAREN]:
                pass
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [ASSIGNEQUAL]:
                pass
                self.match(ASSIGNEQUAL)
                i=self.initializer()
                if not self.inputState.guessing:
                    node.add_child(i);
            elif la1 and la1 in [LPAREN]:
                pass
                self.match(LPAREN)
                el=self.expression_list()
                self.match(RPAREN)
                if not self.inputState.guessing:
                    node.add_child(el);
            elif la1 and la1 in [SEMICOLON,COMMA,LITERAL___attribute__]:
                pass
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_64)
            else:
                raise ex
        
        return node
    
    def declarator(self):    
        node = None
        
        try:      ## for error handling
            synPredMatched213 = False
            if (_tokenSet_24.member(self.LA(1))) and (_tokenSet_65.member(self.LA(2))):
                _m213 = self.mark()
                synPredMatched213 = True
                self.inputState.guessing += 1
                try:
                    pass
                    self.ptr_operator()
                except antlr.RecognitionException, pe:
                    synPredMatched213 = False
                self.rewind(_m213)
                self.inputState.guessing -= 1
            if synPredMatched213:
                pass
                if not self.inputState.guessing:
                    node = self.node("declarator");
                p=self.ptr_operator()
                if not self.inputState.guessing:
                    node.set_leaf(p);
                d=self.declarator()
                if not self.inputState.guessing:
                    node.add_child(d);
            elif (_tokenSet_66.member(self.LA(1))) and (_tokenSet_67.member(self.LA(2))):
                pass
                if not self.inputState.guessing:
                    node = self.node("declarator");
                d=self.direct_declarator()
                if not self.inputState.guessing:
                    node.add_child(d);
            else:
                raise antlr.NoViableAltException(self.LT(1), self.getFilename())
            
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_68)
            else:
                raise ex
        
        return node
    
    def func_asm_name(self):    
        node = None
        
        sl = None
        try:      ## for error handling
            pass
            if not self.inputState.guessing:
                node = self.node("func_asm_name");
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [LITERAL_asm]:
                pass
                self.match(LITERAL_asm)
            elif la1 and la1 in [LITERAL___asm]:
                pass
                self.match(LITERAL___asm)
            elif la1 and la1 in [LITERAL___asm__]:
                pass
                self.match(LITERAL___asm__)
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
            self.match(LPAREN)
            _cnt255= 0
            while True:
                if (self.LA(1)==StringLiteral):
                    pass
                    sl = self.LT(1)
                    self.match(StringLiteral)
                    if not self.inputState.guessing:
                        node.extend_leaf(sl.getText());
                else:
                    break
                
                _cnt255 += 1
            if _cnt255 < 1:
                raise antlr.NoViableAltException(self.LT(1), self.getFilename())
            self.match(RPAREN)
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_69)
            else:
                raise ex
        
        return node
    
    def initializer(self):    
        node = None
        
        node = self.node("initializer");
        try:      ## for error handling
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [LCURLY]:
                pass
                self.match(LCURLY)
                i1=self.initializer()
                if not self.inputState.guessing:
                    node.add_child(i1);
                while True:
                    if (self.LA(1)==COMMA):
                        pass
                        self.match(COMMA)
                        i2=self.initializer()
                        if not self.inputState.guessing:
                            node.add_child(i2);
                    else:
                        break
                    
                self.match(RCURLY)
            elif la1 and la1 in [DOT]:
                pass
                d2=self.c99_designator()
                if not self.inputState.guessing:
                    node.add_child(d2);
                i4=self.initializer()
                if not self.inputState.guessing:
                    node.add_child(i4);
            else:
                if (_tokenSet_70.member(self.LA(1))) and (_tokenSet_71.member(self.LA(2))):
                    pass
                    rx=self.remainder_expression()
                    if not self.inputState.guessing:
                        node.add_child(rx);
                elif (self.LA(1)==ID) and (self.LA(2)==COLON):
                    pass
                    d1=self.gcc_designator()
                    if not self.inputState.guessing:
                        node.add_child(d1);
                    i3=self.initializer()
                    if not self.inputState.guessing:
                        node.add_child(i3);
                else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_72)
            else:
                raise ex
        
        return node
    
    def expression_list(self):    
        node = None
        
        node = self.node("expression");
        try:      ## for error handling
            pass
            x=self.assignment_expression()
            if not self.inputState.guessing:
                node.add_child(x);
            while True:
                if (self.LA(1)==COMMA):
                    pass
                    self.match(COMMA)
                    x=self.assignment_expression()
                    if not self.inputState.guessing:
                        node = self.exprnode_acc(node, x, "comma");
                else:
                    break
                
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_54)
            else:
                raise ex
        
        return node
    
    def remainder_expression(self):    
        node = None
        
        try:      ## for error handling
            pass
            synPredMatched390 = False
            if (_tokenSet_70.member(self.LA(1))) and (_tokenSet_73.member(self.LA(2))):
                _m390 = self.mark()
                synPredMatched390 = True
                self.inputState.guessing += 1
                try:
                    pass
                    self.conditional_expression()
                    la1 = self.LA(1)
                    if False:
                        pass
                    elif la1 and la1 in [COMMA]:
                        pass
                        self.match(COMMA)
                    elif la1 and la1 in [SEMICOLON]:
                        pass
                        self.match(SEMICOLON)
                    elif la1 and la1 in [RPAREN]:
                        pass
                        self.match(RPAREN)
                    else:
                            raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                        
                except antlr.RecognitionException, pe:
                    synPredMatched390 = False
                self.rewind(_m390)
                self.inputState.guessing -= 1
            if synPredMatched390:
                pass
                if not self.inputState.guessing:
                    self.assign_stmt_RHS_found += 1;
                x=self.assignment_expression()
                if not self.inputState.guessing:
                    node = x;
                if not self.inputState.guessing:
                    if (self.assign_stmt_RHS_found > 0): self.assign_stmt_RHS_found -= 1;
                if not self.inputState.guessing:
                    if (self.assign_stmt_RHS_found <= 0): assert("Error in remainder expr");
            elif (_tokenSet_70.member(self.LA(1))) and (_tokenSet_73.member(self.LA(2))):
                pass
                x=self.assignment_expression()
                if not self.inputState.guessing:
                    node = x;
            else:
                raise antlr.NoViableAltException(self.LT(1), self.getFilename())
            
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_74)
            else:
                raise ex
        
        return node
    
    def gcc_designator(self):    
        node = None
        
        id = None
        node = self.node("designator");
        try:      ## for error handling
            pass
            id = self.LT(1)
            self.match(ID)
            self.match(COLON)
            if not self.inputState.guessing:
                node.set_leaf(id.getText());
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_75)
            else:
                raise ex
        
        return node
    
    def c99_designator(self):    
        node = None
        
        id = None
        node = self.node("designator");
        try:      ## for error handling
            pass
            self.match(DOT)
            id = self.LT(1)
            self.match(ID)
            self.match(EQUALS)
            if not self.inputState.guessing:
                node.set_leaf(id.getText());
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_75)
            else:
                raise ex
        
        return node
    
    def base_specifier(self):    
        node = None
        
        try:      ## for error handling
            pass
            if not self.inputState.guessing:
                node = self.node("base_specifier");
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [LITERAL_virtual]:
                pass
                self.match(LITERAL_virtual)
                if not self.inputState.guessing:
                    node.add_child(self.node("virtual"));
                la1 = self.LA(1)
                if False:
                    pass
                elif la1 and la1 in [LITERAL_public,LITERAL_protected,LITERAL_private]:
                    pass
                    as=self.access_specifier()
                    if not self.inputState.guessing:
                        node.add_child(as);
                elif la1 and la1 in [ID,SCOPE]:
                    pass
                else:
                        raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                    
                qt=self.qualified_type()
                if not self.inputState.guessing:
                    node.add_child(self.node(qt));
            elif la1 and la1 in [LITERAL_public,LITERAL_protected,LITERAL_private]:
                pass
                as=self.access_specifier()
                if not self.inputState.guessing:
                    node.add_child(as);
                la1 = self.LA(1)
                if False:
                    pass
                elif la1 and la1 in [LITERAL_virtual]:
                    pass
                    self.match(LITERAL_virtual)
                    if not self.inputState.guessing:
                        node.add_child(self.node("virtual"));
                elif la1 and la1 in [ID,SCOPE]:
                    pass
                else:
                        raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                    
                qt=self.qualified_type()
                if not self.inputState.guessing:
                    node.add_child(self.node(qt));
            elif la1 and la1 in [ID,SCOPE]:
                pass
                qt=self.qualified_type()
                if not self.inputState.guessing:
                    node.add_child(self.node(qt));
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_76)
            else:
                raise ex
        
        return node
    
    def member_declarator(self):    
        node = None
        
        id = None
        try:      ## for error handling
            synPredMatched196 = False
            if (self.LA(1)==ID or self.LA(1)==COLON) and (_tokenSet_77.member(self.LA(2))):
                _m196 = self.mark()
                synPredMatched196 = True
                self.inputState.guessing += 1
                try:
                    pass
                    la1 = self.LA(1)
                    if False:
                        pass
                    elif la1 and la1 in [ID]:
                        pass
                        self.match(ID)
                    elif la1 and la1 in [COLON]:
                        pass
                    else:
                            raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                        
                    self.match(COLON)
                    self.constant_expression()
                except antlr.RecognitionException, pe:
                    synPredMatched196 = False
                self.rewind(_m196)
                self.inputState.guessing -= 1
            if synPredMatched196:
                pass
                if not self.inputState.guessing:
                    node = self.node("init_declarator_bitfield");
                la1 = self.LA(1)
                if False:
                    pass
                elif la1 and la1 in [ID]:
                    pass
                    id = self.LT(1)
                    self.match(ID)
                    if not self.inputState.guessing:
                        node.set_leaf(id.getText());
                elif la1 and la1 in [COLON]:
                    pass
                else:
                        raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                    
                self.match(COLON)
                c=self.constant_expression()
                if not self.inputState.guessing:
                    node.add_child(c);
                la1 = self.LA(1)
                if False:
                    pass
                elif la1 and la1 in [ASSIGNEQUAL]:
                    pass
                    self.match(ASSIGNEQUAL)
                    self.match(OCTALINT)
                    if not self.inputState.guessing:
                        node.add_child(self.node("purevirtual"));
                elif la1 and la1 in [SEMICOLON,COMMA]:
                    pass
                else:
                        raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                    
            elif (_tokenSet_78.member(self.LA(1))) and (_tokenSet_79.member(self.LA(2))):
                pass
                d=self.declarator()
                if not self.inputState.guessing:
                    node = self.node("init_declarator"); node.add_child(d);
                la1 = self.LA(1)
                if False:
                    pass
                elif la1 and la1 in [LITERAL___attribute__]:
                    pass
                    a=self.attribute()
                    if not self.inputState.guessing:
                        node.add_child(a);
                elif la1 and la1 in [SEMICOLON,ASSIGNEQUAL,COMMA]:
                    pass
                else:
                        raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                    
                la1 = self.LA(1)
                if False:
                    pass
                elif la1 and la1 in [ASSIGNEQUAL]:
                    pass
                    self.match(ASSIGNEQUAL)
                    self.match(OCTALINT)
                    if not self.inputState.guessing:
                        node.add_child(self.node("purevirtual"));
                elif la1 and la1 in [SEMICOLON,COMMA]:
                    pass
                else:
                        raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                    
            else:
                raise antlr.NoViableAltException(self.LT(1), self.getFilename())
            
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_80)
            else:
                raise ex
        
        return node
    
    def template_parameter_list(self):    
        node = None
        
        node = self.node("template_parameter_list");
        try:      ## for error handling
            pass
            if not self.inputState.guessing:
                beginTemplateParameterList();
            tp=self.template_parameter()
            if not self.inputState.guessing:
                node.add_child(tp);
            while True:
                if (self.LA(1)==COMMA):
                    pass
                    self.match(COMMA)
                    tp=self.template_parameter()
                    if not self.inputState.guessing:
                        node.add_child(tp);
                else:
                    break
                
            if not self.inputState.guessing:
                endTemplateParameterList();
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_58)
            else:
                raise ex
        
        return node
    
    def parameter_list(self):    
        node = None
        
        node = self.node("parameter_list");
        try:      ## for error handling
            pass
            pdl=self.parameter_declaration_list()
            if not self.inputState.guessing:
                node.add_child(pdl);
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [ELLIPSIS]:
                pass
                self.match(ELLIPSIS)
                if not self.inputState.guessing:
                    node.set_leaf("...");
            elif la1 and la1 in [RPAREN]:
                pass
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_54)
            else:
                raise ex
        
        return node
    
    def exception_specification(self):    
        node = None
        
        node = self.node("unfinished_exception_specification");
        try:      ## for error handling
            pass
            self.match(LITERAL_throw)
            self.match(LPAREN)
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [ID,RPAREN,SCOPE]:
                pass
                la1 = self.LA(1)
                if False:
                    pass
                elif la1 and la1 in [ID,SCOPE]:
                    pass
                    so=self.scope_override()
                    self.match(ID)
                    la1 = self.LA(1)
                    if False:
                        pass
                    elif la1 and la1 in [COMMA]:
                        pass
                        self.match(COMMA)
                        so=self.scope_override()
                        self.match(ID)
                    elif la1 and la1 in [RPAREN]:
                        pass
                    else:
                            raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                        
                elif la1 and la1 in [RPAREN]:
                    pass
                else:
                        raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                    
            elif la1 and la1 in [ELLIPSIS]:
                pass
                self.match(ELLIPSIS)
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
            self.match(RPAREN)
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_81)
            else:
                raise ex
        
        return node
    
    def cv_qualifier_seq(self):    
        s = None
        
        seq = ""
        try:      ## for error handling
            pass
            while True:
                if (_tokenSet_50.member(self.LA(1))):
                    pass
                    tq=self.type_qualifier()
                    if not self.inputState.guessing:
                        seq +=  tq;
                else:
                    break
                
            if not self.inputState.guessing:
                s = seq;
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_82)
            else:
                raise ex
        
        return s
    
    def ptr_operator(self):    
        oper = None
        
        oper = "UNKNOWN";
        try:      ## for error handling
            pass
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [AMPERSAND]:
                pass
                self.match(AMPERSAND)
                if not self.inputState.guessing:
                    oper = "&";
                if not self.inputState.guessing:
                    self.is_address = true;
            elif la1 and la1 in [LITERAL__cdecl,LITERAL___cdecl]:
                pass
                la1 = self.LA(1)
                if False:
                    pass
                elif la1 and la1 in [LITERAL__cdecl]:
                    pass
                    self.match(LITERAL__cdecl)
                elif la1 and la1 in [LITERAL___cdecl]:
                    pass
                    self.match(LITERAL___cdecl)
                else:
                        raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                    
                if not self.inputState.guessing:
                    oper = "_cdecl";
            elif la1 and la1 in [LITERAL__near,LITERAL___near]:
                pass
                la1 = self.LA(1)
                if False:
                    pass
                elif la1 and la1 in [LITERAL__near]:
                    pass
                    self.match(LITERAL__near)
                elif la1 and la1 in [LITERAL___near]:
                    pass
                    self.match(LITERAL___near)
                else:
                        raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                    
                if not self.inputState.guessing:
                    oper = "_near";
            elif la1 and la1 in [LITERAL__far,LITERAL___far]:
                pass
                la1 = self.LA(1)
                if False:
                    pass
                elif la1 and la1 in [LITERAL__far]:
                    pass
                    self.match(LITERAL__far)
                elif la1 and la1 in [LITERAL___far]:
                    pass
                    self.match(LITERAL___far)
                else:
                        raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                    
                if not self.inputState.guessing:
                    oper = "_far";
            elif la1 and la1 in [LITERAL___interrupt]:
                pass
                self.match(LITERAL___interrupt)
                if not self.inputState.guessing:
                    oper = "__interrupt";
            elif la1 and la1 in [LITERAL_pascal,LITERAL__pascal,LITERAL___pascal]:
                pass
                la1 = self.LA(1)
                if False:
                    pass
                elif la1 and la1 in [LITERAL_pascal]:
                    pass
                    self.match(LITERAL_pascal)
                elif la1 and la1 in [LITERAL__pascal]:
                    pass
                    self.match(LITERAL__pascal)
                elif la1 and la1 in [LITERAL___pascal]:
                    pass
                    self.match(LITERAL___pascal)
                else:
                        raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                    
                if not self.inputState.guessing:
                    oper = "pascal";
            elif la1 and la1 in [LITERAL__stdcall,LITERAL___stdcall]:
                pass
                la1 = self.LA(1)
                if False:
                    pass
                elif la1 and la1 in [LITERAL__stdcall]:
                    pass
                    self.match(LITERAL__stdcall)
                elif la1 and la1 in [LITERAL___stdcall]:
                    pass
                    self.match(LITERAL___stdcall)
                else:
                        raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                    
                if not self.inputState.guessing:
                    oper = "_stdcall";
            elif la1 and la1 in [ID,STAR,SCOPE]:
                pass
                ptm=self.ptr_to_member()
                if not self.inputState.guessing:
                    oper = ptm;
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_17)
            else:
                raise ex
        
        return oper
    
    def declarator_suffixes(self):    
        node = None
        
        node = self.node("declarator_suffixes");
        try:      ## for error handling
            pass
            if (self.LA(1)==LSQUARE):
                pass
                _cnt234= 0
                while True:
                    if (self.LA(1)==LSQUARE) and (_tokenSet_51.member(self.LA(2))):
                        pass
                        self.match(LSQUARE)
                        la1 = self.LA(1)
                        if False:
                            pass
                        elif la1 and la1 in [ID,StringLiteral,LPAREN,LITERAL_const_cast,LITERAL_char,LITERAL_bool,LITERAL_short,LITERAL_int,74,75,76,LITERAL_long,LITERAL_signed,LITERAL___signed,LITERAL___signed__,LITERAL_unsigned,LITERAL_float,LITERAL_double,LITERAL_void,LITERAL__declspec,LITERAL___declspec,LITERAL___builtin_va_list,OPERATOR,LITERAL_this,LITERAL_true,LITERAL_false,OCTALINT,STAR,AMPERSAND,TILDE,SCOPE,PLUS,MINUS,PLUSPLUS,MINUSMINUS,LITERAL_sizeof,LITERAL_dynamic_cast,LITERAL_static_cast,LITERAL_reinterpret_cast,NOT,LITERAL_new,LITERAL_delete,DECIMALINT,HEXADECIMALINT,CharLiteral,FLOATONE,FLOATTWO]:
                            pass
                            cx=self.constant_expression()
                            if not self.inputState.guessing:
                                node.add_child(cx);
                        elif la1 and la1 in [RSQUARE]:
                            pass
                        else:
                                raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                            
                        self.match(RSQUARE)
                    else:
                        break
                    
                    _cnt234 += 1
                if _cnt234 < 1:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                if not self.inputState.guessing:
                    self.declaratorArray();
            elif ((self.LA(1)==LPAREN) and ((not((self.LA(1)==LPAREN) and (self.LA(2)==ID))or(self.qualifiedItemIsOneOf(qiType|qiCtor,1))))):
                pass
                self.match(LPAREN)
                if not self.inputState.guessing:
                    self.declaratorParameterList(0);
                la1 = self.LA(1)
                if False:
                    pass
                elif la1 and la1 in [LITERAL_typedef,LITERAL_enum,ID,LITERAL_inline,LITERAL_extern,LITERAL___extension__,LITERAL__inline,LITERAL___inline,LITERAL___inline__,LITERAL_virtual,LITERAL_explicit,LITERAL_friend,LITERAL__stdcall,LITERAL___stdcall,LITERAL_typename,LPAREN,LITERAL_const,LITERAL___const,LITERAL_auto,LITERAL_register,LITERAL_static,LITERAL_mutable,LITERAL_const_cast,LITERAL_volatile,LITERAL___volatile,LITERAL___restrict,LITERAL___restrict__,LITERAL_char,LITERAL_bool,LITERAL_short,LITERAL_int,74,75,76,LITERAL_long,LITERAL_signed,LITERAL___signed,LITERAL___signed__,LITERAL_unsigned,LITERAL_float,LITERAL_double,LITERAL_void,LITERAL__declspec,LITERAL___declspec,LITERAL___builtin_va_list,LITERAL_class,LITERAL_struct,LITERAL_union,OPERATOR,LITERAL_this,LITERAL_true,LITERAL_false,STAR,AMPERSAND,TILDE,ELLIPSIS,SCOPE,LITERAL__cdecl,LITERAL___cdecl,LITERAL__near,LITERAL___near,LITERAL__far,LITERAL___far,LITERAL___interrupt,LITERAL_pascal,LITERAL__pascal,LITERAL___pascal]:
                    pass
                    pl=self.parameter_list()
                    if not self.inputState.guessing:
                        node.add_child(pl);
                elif la1 and la1 in [RPAREN]:
                    pass
                else:
                        raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                    
                self.match(RPAREN)
                if not self.inputState.guessing:
                    self.declaratorEndParameterList(0);
                while True:
                    if (_tokenSet_50.member(self.LA(1))):
                        pass
                        tq=self.type_qualifier()
                        if not self.inputState.guessing:
                            node.add_child(self.node(tq));
                    else:
                        break
                    
                la1 = self.LA(1)
                if False:
                    pass
                elif la1 and la1 in [LITERAL_throw]:
                    pass
                    es=self.exception_specification()
                    if not self.inputState.guessing:
                        node.add_child(es);
                elif la1 and la1 in [LESSTHAN,GREATERTHAN,ID,SEMICOLON,RCURLY,ASSIGNEQUAL,COLON,COMMA,LITERAL__stdcall,LITERAL___stdcall,LITERAL___attribute__,LPAREN,RPAREN,OPERATOR,LITERAL_this,LITERAL_true,LITERAL_false,STAR,AMPERSAND,LSQUARE,RSQUARE,TILDE,LITERAL_asm,LITERAL___asm,LITERAL___asm__,ELLIPSIS,SCOPE,TIMESEQUAL,DIVIDEEQUAL,MINUSEQUAL,PLUSEQUAL,MODEQUAL,SHIFTLEFTEQUAL,SHIFTRIGHTEQUAL,BITWISEANDEQUAL,BITWISEXOREQUAL,BITWISEOREQUAL,QUESTIONMARK,OR,AND,BITWISEOR,BITWISEXOR,NOTEQUAL,EQUAL,LESSTHANOREQUALTO,GREATERTHANOREQUALTO,SHIFTLEFT,SHIFTRIGHT,PLUS,MINUS,DIVIDE,MOD,DOTMBR,POINTERTOMBR,LITERAL__cdecl,LITERAL___cdecl,LITERAL__near,LITERAL___near,LITERAL__far,LITERAL___far,LITERAL___interrupt,LITERAL_pascal,LITERAL__pascal,LITERAL___pascal]:
                    pass
                else:
                        raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                    
            else:
                raise antlr.NoViableAltException(self.LT(1), self.getFilename())
            
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_53)
            else:
                raise ex
        
        return node
    
    def function_direct_declarator(self,
        definition
    ):    
        node = None
        
        node = self.node("function_direct_declarator");
        try:      ## for error handling
            pass
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [LPAREN]:
                pass
                self.match(LPAREN)
                q=self.qualified_id()
                if not self.inputState.guessing:
                    node.set_leaf(q);
                if not self.inputState.guessing:
                    self.declaratorID(q,qiFun);
                self.match(RPAREN)
            elif la1 and la1 in [ID,LITERAL___attribute__,OPERATOR,LITERAL_this,LITERAL_true,LITERAL_false,SCOPE]:
                pass
                while True:
                    if (self.LA(1)==LITERAL___attribute__):
                        pass
                        a=self.attribute()
                        if not self.inputState.guessing:
                            node.add_child(a);
                    else:
                        break
                    
                q=self.qualified_id()
                if not self.inputState.guessing:
                    node.set_leaf(q);
                if not self.inputState.guessing:
                    self.declaratorID(q,qiFun);
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
            self.match(LPAREN)
            if not self.inputState.guessing:
                self.functionParameterList();
            if not self.inputState.guessing:
                if (self.K_and_R == false): self.in_parameter_list = true;
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [LITERAL_typedef,LITERAL_enum,ID,LITERAL_inline,LITERAL_extern,LITERAL___extension__,LITERAL__inline,LITERAL___inline,LITERAL___inline__,LITERAL_virtual,LITERAL_explicit,LITERAL_friend,LITERAL__stdcall,LITERAL___stdcall,LITERAL_typename,LPAREN,LITERAL_const,LITERAL___const,LITERAL_auto,LITERAL_register,LITERAL_static,LITERAL_mutable,LITERAL_const_cast,LITERAL_volatile,LITERAL___volatile,LITERAL___restrict,LITERAL___restrict__,LITERAL_char,LITERAL_bool,LITERAL_short,LITERAL_int,74,75,76,LITERAL_long,LITERAL_signed,LITERAL___signed,LITERAL___signed__,LITERAL_unsigned,LITERAL_float,LITERAL_double,LITERAL_void,LITERAL__declspec,LITERAL___declspec,LITERAL___builtin_va_list,LITERAL_class,LITERAL_struct,LITERAL_union,OPERATOR,LITERAL_this,LITERAL_true,LITERAL_false,STAR,AMPERSAND,TILDE,ELLIPSIS,SCOPE,LITERAL__cdecl,LITERAL___cdecl,LITERAL__near,LITERAL___near,LITERAL__far,LITERAL___far,LITERAL___interrupt,LITERAL_pascal,LITERAL__pascal,LITERAL___pascal]:
                pass
                pl=self.parameter_list()
                if not self.inputState.guessing:
                    node.add_child(pl);
            elif la1 and la1 in [RPAREN]:
                pass
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
            if not self.inputState.guessing:
                if (self.K_and_R == false): self.in_parameter_list = false;
            if not self.inputState.guessing:
                if (self.K_and_R != false): self.in_parameter_list = true;
            self.match(RPAREN)
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [LITERAL_asm,LITERAL___asm,LITERAL___asm__]:
                pass
                an=self.func_asm_name()
                if not self.inputState.guessing:
                    node.add_child(an);
            elif la1 and la1 in [LITERAL_typedef,LITERAL_enum,ID,LCURLY,SEMICOLON,LITERAL_inline,ASSIGNEQUAL,LITERAL_extern,LITERAL___extension__,LITERAL__inline,LITERAL___inline,LITERAL___inline__,LITERAL_virtual,LITERAL_explicit,LITERAL_friend,LITERAL__stdcall,LITERAL___stdcall,LITERAL_typename,LITERAL_const,LITERAL___const,LITERAL_auto,LITERAL_register,LITERAL_static,LITERAL_mutable,LITERAL_const_cast,LITERAL_volatile,LITERAL___volatile,LITERAL___restrict,LITERAL___restrict__,LITERAL_char,LITERAL_bool,LITERAL_short,LITERAL_int,74,75,76,LITERAL_long,LITERAL_signed,LITERAL___signed,LITERAL___signed__,LITERAL_unsigned,LITERAL_float,LITERAL_double,LITERAL_void,LITERAL__declspec,LITERAL___declspec,LITERAL___builtin_va_list,LITERAL_class,LITERAL_struct,LITERAL_union,LITERAL_throw,LITERAL_using,SCOPE]:
                pass
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
            while True:
                if (_tokenSet_50.member(self.LA(1))) and (_tokenSet_83.member(self.LA(2))):
                    pass
                    tq=self.type_qualifier()
                    if not self.inputState.guessing:
                        node.add_child(self.node(tq));
                else:
                    break
                
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [ASSIGNEQUAL]:
                pass
                self.match(ASSIGNEQUAL)
                self.match(OCTALINT)
                if not self.inputState.guessing:
                    node.add_child(self.node("purevirtual"));
            elif la1 and la1 in [LITERAL_typedef,LITERAL_enum,ID,LCURLY,SEMICOLON,LITERAL_inline,LITERAL_extern,LITERAL___extension__,LITERAL__inline,LITERAL___inline,LITERAL___inline__,LITERAL_virtual,LITERAL_explicit,LITERAL_friend,LITERAL__stdcall,LITERAL___stdcall,LITERAL_typename,LITERAL_const,LITERAL___const,LITERAL_auto,LITERAL_register,LITERAL_static,LITERAL_mutable,LITERAL_const_cast,LITERAL_volatile,LITERAL___volatile,LITERAL___restrict,LITERAL___restrict__,LITERAL_char,LITERAL_bool,LITERAL_short,LITERAL_int,74,75,76,LITERAL_long,LITERAL_signed,LITERAL___signed,LITERAL___signed__,LITERAL_unsigned,LITERAL_float,LITERAL_double,LITERAL_void,LITERAL__declspec,LITERAL___declspec,LITERAL___builtin_va_list,LITERAL_class,LITERAL_struct,LITERAL_union,LITERAL_throw,LITERAL_using,SCOPE]:
                pass
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
            if not self.inputState.guessing:
                self.functionEndParameterList(definition);
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [LITERAL_throw]:
                pass
                es=self.exception_specification()
                if not self.inputState.guessing:
                    node.add_child(es);
            elif la1 and la1 in [LITERAL_typedef,LITERAL_enum,ID,LCURLY,SEMICOLON,LITERAL_inline,LITERAL_extern,LITERAL___extension__,LITERAL__inline,LITERAL___inline,LITERAL___inline__,LITERAL_virtual,LITERAL_explicit,LITERAL_friend,LITERAL__stdcall,LITERAL___stdcall,LITERAL_typename,LITERAL_const,LITERAL___const,LITERAL_auto,LITERAL_register,LITERAL_static,LITERAL_mutable,LITERAL_const_cast,LITERAL_volatile,LITERAL___volatile,LITERAL___restrict,LITERAL___restrict__,LITERAL_char,LITERAL_bool,LITERAL_short,LITERAL_int,74,75,76,LITERAL_long,LITERAL_signed,LITERAL___signed,LITERAL___signed__,LITERAL_unsigned,LITERAL_float,LITERAL_double,LITERAL_void,LITERAL__declspec,LITERAL___declspec,LITERAL___builtin_va_list,LITERAL_class,LITERAL_struct,LITERAL_union,LITERAL_using,SCOPE]:
                pass
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_28)
            else:
                raise ex
        
        return node
    
    def ctor_head(self):    
        node = None
        
        node = self.node("ctor_head");
        try:      ## for error handling
            pass
            cds=self.ctor_decl_spec()
            if not self.inputState.guessing:
                node.add_child(cds);
            cd=self.ctor_declarator(1)
            if not self.inputState.guessing:
                node.add_child(cd);
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_84)
            else:
                raise ex
        
        return node
    
    def ctor_body(self):    
        node = None
        
        node = self.node("ctor_body");
        try:      ## for error handling
            pass
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [COLON]:
                pass
                ci=self.ctor_initializer()
                if not self.inputState.guessing:
                    node.add_child(ci);
            elif la1 and la1 in [LCURLY]:
                pass
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
            cs=self.compound_statement()
            if not self.inputState.guessing:
                node.add_child(cs);
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_20)
            else:
                raise ex
        
        return node
    
    def qualified_ctor_id(self):    
        q = None
        
        id = None
        try:      ## for error handling
            pass
            so=self.scope_override()
            if not self.inputState.guessing:
                qitem = so;
            id = self.LT(1)
            self.match(ID)
            if not self.inputState.guessing:
                qitem += id.getText();
            if not self.inputState.guessing:
                q = qitem;
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_85)
            else:
                raise ex
        
        return q
    
    def ctor_initializer(self):    
        node = None
        
        node = self.node("unfinished_ctor_body");
        try:      ## for error handling
            pass
            self.match(COLON)
            self.superclass_init()
            while True:
                if (self.LA(1)==COMMA):
                    pass
                    self.match(COMMA)
                    self.superclass_init()
                else:
                    break
                
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_59)
            else:
                raise ex
        
        return node
    
    def superclass_init(self):    
        
        try:      ## for error handling
            pass
            q=self.qualified_id()
            self.match(LPAREN)
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [ID,StringLiteral,LPAREN,LITERAL_const_cast,LITERAL_char,LITERAL_bool,LITERAL_short,LITERAL_int,74,75,76,LITERAL_long,LITERAL_signed,LITERAL___signed,LITERAL___signed__,LITERAL_unsigned,LITERAL_float,LITERAL_double,LITERAL_void,LITERAL__declspec,LITERAL___declspec,LITERAL___builtin_va_list,OPERATOR,LITERAL_this,LITERAL_true,LITERAL_false,OCTALINT,STAR,AMPERSAND,TILDE,SCOPE,PLUS,MINUS,PLUSPLUS,MINUSMINUS,LITERAL_sizeof,LITERAL_dynamic_cast,LITERAL_static_cast,LITERAL_reinterpret_cast,NOT,LITERAL_new,LITERAL_delete,DECIMALINT,HEXADECIMALINT,CharLiteral,FLOATONE,FLOATTWO]:
                pass
                el=self.expression_list()
            elif la1 and la1 in [RPAREN]:
                pass
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
            self.match(RPAREN)
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_76)
            else:
                raise ex
        
    
    def dtor_decl_spec(self):    
        node = None
        
        node = self.node("dtor_decl_spec");
        try:      ## for error handling
            pass
            while True:
                la1 = self.LA(1)
                if False:
                    pass
                elif la1 and la1 in [LITERAL_inline,LITERAL__inline,LITERAL___inline]:
                    pass
                    la1 = self.LA(1)
                    if False:
                        pass
                    elif la1 and la1 in [LITERAL_inline]:
                        pass
                        self.match(LITERAL_inline)
                    elif la1 and la1 in [LITERAL__inline]:
                        pass
                        self.match(LITERAL__inline)
                    elif la1 and la1 in [LITERAL___inline]:
                        pass
                        self.match(LITERAL___inline)
                    else:
                            raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                        
                    if not self.inputState.guessing:
                        node.add_child(self.node("inline"));
                elif la1 and la1 in [LITERAL_virtual]:
                    pass
                    self.match(LITERAL_virtual)
                    if not self.inputState.guessing:
                        node.add_child(self.node("virtual"));
                else:
                        break
                    
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_86)
            else:
                raise ex
        
        return node
    
    def dtor_declarator(self,
        definition
    ):    
        node = None
        
        node = self.node("unfinished_dtor_declaration");
        try:      ## for error handling
            pass
            s=self.scope_override()
            self.match(TILDE)
            self.match(ID)
            if not self.inputState.guessing:
                self.declaratorParameterList(definition);
            self.match(LPAREN)
            self.match(RPAREN)
            if not self.inputState.guessing:
                self.declaratorEndParameterList(definition);
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [LITERAL_throw]:
                pass
                es=self.exception_specification()
            elif la1 and la1 in [LCURLY,SEMICOLON]:
                pass
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_19)
            else:
                raise ex
        
        return node
    
    def parameter_declaration_list(self):    
        node = None
        
        try:      ## for error handling
            pass
            if not self.inputState.guessing:
                node = self.node("parameter_declaration_list");
            pass
            p=self.parameter_declaration()
            if not self.inputState.guessing:
                node.add_child(p);
            while True:
                if (self.LA(1)==COMMA):
                    pass
                    self.match(COMMA)
                    p=self.parameter_declaration()
                    if not self.inputState.guessing:
                        node.add_child(p);
                else:
                    break
                
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_87)
            else:
                raise ex
        
        return node
    
    def parameter_declaration(self):    
        node = None
        
        try:      ## for error handling
            pass
            if not self.inputState.guessing:
                node = self.node("parameter_declaration"); self.beginParameterDeclaration();
            if ((_tokenSet_13.member(self.LA(1))) and (_tokenSet_88.member(self.LA(2))) and (not((self.LA(1)==SCOPE) and (self.LA(2)==STAR or self.LA(2)==OPERATOR)) and ( not(self.LA(1)==SCOPE or self.LA(1)==ID) or self.qualifiedItemIsOneOf(qiType|qiCtor) ))):
                pass
                ds=self.declaration_specifiers()
                if not self.inputState.guessing:
                    node.add_child(ds);
                synPredMatched291 = False
                if (_tokenSet_78.member(self.LA(1))) and (_tokenSet_89.member(self.LA(2))):
                    _m291 = self.mark()
                    synPredMatched291 = True
                    self.inputState.guessing += 1
                    try:
                        pass
                        self.declarator()
                    except antlr.RecognitionException, pe:
                        synPredMatched291 = False
                    self.rewind(_m291)
                    self.inputState.guessing -= 1
                if synPredMatched291:
                    pass
                    d=self.declarator()
                    if not self.inputState.guessing:
                        node.add_child(d);
                elif (_tokenSet_90.member(self.LA(1))) and (_tokenSet_91.member(self.LA(2))):
                    pass
                    ad=self.abstract_declarator()
                    if not self.inputState.guessing:
                        node.add_child(ad);
                else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
            else:
                synPredMatched293 = False
                if (_tokenSet_78.member(self.LA(1))) and (_tokenSet_89.member(self.LA(2))):
                    _m293 = self.mark()
                    synPredMatched293 = True
                    self.inputState.guessing += 1
                    try:
                        pass
                        self.declarator()
                    except antlr.RecognitionException, pe:
                        synPredMatched293 = False
                    self.rewind(_m293)
                    self.inputState.guessing -= 1
                if synPredMatched293:
                    pass
                    d=self.declarator()
                    if not self.inputState.guessing:
                        node.add_child(d);
                elif (self.LA(1)==ELLIPSIS):
                    pass
                    self.match(ELLIPSIS)
                    if not self.inputState.guessing:
                        node.set_leaf("ellipsis");
                else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [ASSIGNEQUAL]:
                pass
                self.match(ASSIGNEQUAL)
                re=self.remainder_expression()
                if not self.inputState.guessing:
                    node.add_child(re);
            elif la1 and la1 in [GREATERTHAN,COMMA,RPAREN,ELLIPSIS]:
                pass
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_92)
            else:
                raise ex
        
        return node
    
    def abstract_declarator(self):    
        node = None
        
        node = self.node("abstract_declarator");
        try:      ## for error handling
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [ID,LITERAL__stdcall,LITERAL___stdcall,STAR,AMPERSAND,SCOPE,LITERAL__cdecl,LITERAL___cdecl,LITERAL__near,LITERAL___near,LITERAL__far,LITERAL___far,LITERAL___interrupt,LITERAL_pascal,LITERAL__pascal,LITERAL___pascal]:
                pass
                po=self.ptr_operator()
                if not self.inputState.guessing:
                    node.set_leaf(po);
                ad=self.abstract_declarator()
                if not self.inputState.guessing:
                    node.add_child(ad);
            elif la1 and la1 in [LPAREN]:
                pass
                self.match(LPAREN)
                ad2=self.abstract_declarator()
                self.match(RPAREN)
                if not self.inputState.guessing:
                    node.add_child(ad2);
                _cnt298= 0
                while True:
                    if (self.LA(1)==LPAREN or self.LA(1)==LSQUARE):
                        pass
                        ads=self.abstract_declarator_suffix()
                        if not self.inputState.guessing:
                            node.add_child(ads);
                    else:
                        break
                    
                    _cnt298 += 1
                if _cnt298 < 1:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
            elif la1 and la1 in [LSQUARE]:
                pass
                _cnt301= 0
                while True:
                    if (self.LA(1)==LSQUARE):
                        pass
                        self.match(LSQUARE)
                        la1 = self.LA(1)
                        if False:
                            pass
                        elif la1 and la1 in [ID,StringLiteral,LPAREN,LITERAL_const_cast,LITERAL_char,LITERAL_bool,LITERAL_short,LITERAL_int,74,75,76,LITERAL_long,LITERAL_signed,LITERAL___signed,LITERAL___signed__,LITERAL_unsigned,LITERAL_float,LITERAL_double,LITERAL_void,LITERAL__declspec,LITERAL___declspec,LITERAL___builtin_va_list,OPERATOR,LITERAL_this,LITERAL_true,LITERAL_false,OCTALINT,STAR,AMPERSAND,TILDE,SCOPE,PLUS,MINUS,PLUSPLUS,MINUSMINUS,LITERAL_sizeof,LITERAL_dynamic_cast,LITERAL_static_cast,LITERAL_reinterpret_cast,NOT,LITERAL_new,LITERAL_delete,DECIMALINT,HEXADECIMALINT,CharLiteral,FLOATONE,FLOATTWO]:
                            pass
                            cx=self.constant_expression()
                            if not self.inputState.guessing:
                                node.add_child(cx);
                        elif la1 and la1 in [RSQUARE]:
                            pass
                        else:
                                raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                            
                        self.match(RSQUARE)
                        if not self.inputState.guessing:
                            self.declaratorArray();
                    else:
                        break
                    
                    _cnt301 += 1
                if _cnt301 < 1:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
            elif la1 and la1 in [GREATERTHAN,ASSIGNEQUAL,COMMA,RPAREN,ELLIPSIS]:
                pass
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_93)
            else:
                raise ex
        
        return node
    
    def type_name(self):    
        node = None
        
        node = self.node("type_name");
        try:      ## for error handling
            pass
            ds=self.declaration_specifiers()
            if not self.inputState.guessing:
                node.add_child(ds);
            ad=self.abstract_declarator()
            if not self.inputState.guessing:
                node.add_child(ad);
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_94)
            else:
                raise ex
        
        return node
    
    def abstract_declarator_suffix(self):    
        node = None
        
        node = self.node("abstract_declarator");
        try:      ## for error handling
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [LSQUARE]:
                pass
                self.match(LSQUARE)
                la1 = self.LA(1)
                if False:
                    pass
                elif la1 and la1 in [ID,StringLiteral,LPAREN,LITERAL_const_cast,LITERAL_char,LITERAL_bool,LITERAL_short,LITERAL_int,74,75,76,LITERAL_long,LITERAL_signed,LITERAL___signed,LITERAL___signed__,LITERAL_unsigned,LITERAL_float,LITERAL_double,LITERAL_void,LITERAL__declspec,LITERAL___declspec,LITERAL___builtin_va_list,OPERATOR,LITERAL_this,LITERAL_true,LITERAL_false,OCTALINT,STAR,AMPERSAND,TILDE,SCOPE,PLUS,MINUS,PLUSPLUS,MINUSMINUS,LITERAL_sizeof,LITERAL_dynamic_cast,LITERAL_static_cast,LITERAL_reinterpret_cast,NOT,LITERAL_new,LITERAL_delete,DECIMALINT,HEXADECIMALINT,CharLiteral,FLOATONE,FLOATTWO]:
                    pass
                    cx=self.constant_expression()
                    if not self.inputState.guessing:
                        node.add_child(cx);
                elif la1 and la1 in [RSQUARE]:
                    pass
                else:
                        raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                    
                self.match(RSQUARE)
                if not self.inputState.guessing:
                    self.declaratorArray();
            elif la1 and la1 in [LPAREN]:
                pass
                self.match(LPAREN)
                if not self.inputState.guessing:
                    self.declaratorParameterList(0);
                la1 = self.LA(1)
                if False:
                    pass
                elif la1 and la1 in [LITERAL_typedef,LITERAL_enum,ID,LITERAL_inline,LITERAL_extern,LITERAL___extension__,LITERAL__inline,LITERAL___inline,LITERAL___inline__,LITERAL_virtual,LITERAL_explicit,LITERAL_friend,LITERAL__stdcall,LITERAL___stdcall,LITERAL_typename,LPAREN,LITERAL_const,LITERAL___const,LITERAL_auto,LITERAL_register,LITERAL_static,LITERAL_mutable,LITERAL_const_cast,LITERAL_volatile,LITERAL___volatile,LITERAL___restrict,LITERAL___restrict__,LITERAL_char,LITERAL_bool,LITERAL_short,LITERAL_int,74,75,76,LITERAL_long,LITERAL_signed,LITERAL___signed,LITERAL___signed__,LITERAL_unsigned,LITERAL_float,LITERAL_double,LITERAL_void,LITERAL__declspec,LITERAL___declspec,LITERAL___builtin_va_list,LITERAL_class,LITERAL_struct,LITERAL_union,OPERATOR,LITERAL_this,LITERAL_true,LITERAL_false,STAR,AMPERSAND,TILDE,ELLIPSIS,SCOPE,LITERAL__cdecl,LITERAL___cdecl,LITERAL__near,LITERAL___near,LITERAL__far,LITERAL___far,LITERAL___interrupt,LITERAL_pascal,LITERAL__pascal,LITERAL___pascal]:
                    pass
                    pl=self.parameter_list()
                    if not self.inputState.guessing:
                        node.add_child(pl);
                elif la1 and la1 in [RPAREN]:
                    pass
                else:
                        raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                    
                self.match(RPAREN)
                cvs=self.cv_qualifier_seq()
                if not self.inputState.guessing:
                    node.add_child(self.node(cvs));
                if not self.inputState.guessing:
                    self.declaratorEndParameterList(0);
                la1 = self.LA(1)
                if False:
                    pass
                elif la1 and la1 in [LITERAL_throw]:
                    pass
                    es=self.exception_specification()
                    if not self.inputState.guessing:
                        node.add_child(es);
                elif la1 and la1 in [GREATERTHAN,ASSIGNEQUAL,COMMA,LPAREN,RPAREN,LSQUARE,ELLIPSIS]:
                    pass
                else:
                        raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                    
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_95)
            else:
                raise ex
        
        return node
    
    def template_parameter(self):    
        node = None
        
        id = None
        node = self.node("unfinished_template_parameter");
        try:      ## for error handling
            pass
            if (self.LA(1)==LITERAL_typename or self.LA(1)==LITERAL_class) and (_tokenSet_96.member(self.LA(2))):
                pass
                la1 = self.LA(1)
                if False:
                    pass
                elif la1 and la1 in [LITERAL_class]:
                    pass
                    self.match(LITERAL_class)
                elif la1 and la1 in [LITERAL_typename]:
                    pass
                    self.match(LITERAL_typename)
                else:
                        raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                    
                la1 = self.LA(1)
                if False:
                    pass
                elif la1 and la1 in [ID]:
                    pass
                    id = self.LT(1)
                    self.match(ID)
                    la1 = self.LA(1)
                    if False:
                        pass
                    elif la1 and la1 in [ASSIGNEQUAL]:
                        pass
                        self.match(ASSIGNEQUAL)
                        self.assigned_type_name()
                    elif la1 and la1 in [GREATERTHAN,COMMA]:
                        pass
                    else:
                            raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                        
                elif la1 and la1 in [GREATERTHAN,COMMA]:
                    pass
                else:
                        raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                    
                if not self.inputState.guessing:
                    templateTypeParameter(id.getText());
            elif (_tokenSet_97.member(self.LA(1))) and (_tokenSet_98.member(self.LA(2))):
                pass
                p=self.parameter_declaration()
            else:
                raise antlr.NoViableAltException(self.LT(1), self.getFilename())
            
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_99)
            else:
                raise ex
        
        return node
    
    def assigned_type_name(self):    
        
        try:      ## for error handling
            pass
            if (self.LA(1)==ID or self.LA(1)==SCOPE) and (_tokenSet_100.member(self.LA(2))):
                pass
                s=self.qualified_type()
                ad=self.abstract_declarator()
            elif (_tokenSet_101.member(self.LA(1))) and (_tokenSet_102.member(self.LA(2))):
                pass
                ts=self.simple_type_specifier()
                ad=self.abstract_declarator()
            else:
                raise antlr.NoViableAltException(self.LT(1), self.getFilename())
            
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_99)
            else:
                raise ex
        
    
    def template_id(self):    
        
        try:      ## for error handling
            pass
            self.match(ID)
            self.match(LESSTHAN)
            tal=self.template_argument_list()
            self.match(GREATERTHAN)
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_1)
            else:
                raise ex
        
    
    def template_argument(self):    
        node = None
        
        node = self.node("template_argument");
        try:      ## for error handling
            if ((_tokenSet_13.member(self.LA(1))) and (_tokenSet_103.member(self.LA(2))) and (( not(self.LA(1)==SCOPE or self.LA(1)==ID) or self.qualifiedItemIsOneOf(qiType|qiCtor) ))):
                pass
                tn=self.type_name()
                if not self.inputState.guessing:
                    node.add_child(tn);
            elif (_tokenSet_70.member(self.LA(1))) and (_tokenSet_104.member(self.LA(2))):
                pass
                sx=self.shift_expression()
                if not self.inputState.guessing:
                    node.add_child(sx);
            else:
                raise antlr.NoViableAltException(self.LT(1), self.getFilename())
            
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_99)
            else:
                raise ex
        
        return node
    
    def shift_expression(self):    
        node = None
        
        try:      ## for error handling
            pass
            x=self.additive_expression()
            if not self.inputState.guessing:
                node = x;
            while True:
                if (self.LA(1)==SHIFTLEFT or self.LA(1)==SHIFTRIGHT):
                    pass
                    la1 = self.LA(1)
                    if False:
                        pass
                    elif la1 and la1 in [SHIFTLEFT]:
                        pass
                        self.match(SHIFTLEFT)
                        if not self.inputState.guessing:
                            leaf = "shiftleft";
                    elif la1 and la1 in [SHIFTRIGHT]:
                        pass
                        self.match(SHIFTRIGHT)
                        if not self.inputState.guessing:
                            leaf = "shiftright";
                    else:
                            raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                        
                    x=self.additive_expression()
                    if not self.inputState.guessing:
                        node = self.exprnode_acc(node, x, leaf);
                else:
                    break
                
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_105)
            else:
                raise ex
        
        return node
    
    def statement_list(self):    
        node = None
        
        node = self.node("statement_list");
        try:      ## for error handling
            pass
            _cnt328= 0
            while True:
                if (_tokenSet_106.member(self.LA(1))):
                    pass
                    s=self.statement()
                    if not self.inputState.guessing:
                        node.add_child(s);
                else:
                    break
                
                _cnt328 += 1
            if _cnt328 < 1:
                raise antlr.NoViableAltException(self.LT(1), self.getFilename())
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_60)
            else:
                raise ex
        
        return node
    
    def statement(self):    
        node = None
        
        node = self.node("statement");
        try:      ## for error handling
            pass
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [LITERAL_case]:
                pass
                s=self.case_statement()
                if not self.inputState.guessing:
                    node.add_child(s);
            elif la1 and la1 in [LITERAL_default]:
                pass
                ds=self.default_statement()
                if not self.inputState.guessing:
                    node.add_child(ds);
            elif la1 and la1 in [LCURLY]:
                pass
                compstat=self.compound_statement()
                if not self.inputState.guessing:
                    node.add_child(compstat);
            elif la1 and la1 in [LITERAL_if,LITERAL_switch]:
                pass
                sstat=self.selection_statement()
                if not self.inputState.guessing:
                    node.add_child(sstat);
            elif la1 and la1 in [LITERAL_while,LITERAL_do,LITERAL_for]:
                pass
                iterstat=self.iteration_statement()
                if not self.inputState.guessing:
                    node.add_child(iterstat);
            elif la1 and la1 in [LITERAL_goto,LITERAL_continue,LITERAL_break,LITERAL_return]:
                pass
                jumpstat=self.jump_statement()
                if not self.inputState.guessing:
                    node.add_child(jumpstat);
            elif la1 and la1 in [SEMICOLON]:
                pass
                self.match(SEMICOLON)
                if not self.inputState.guessing:
                    self.end_of_stmt();
            elif la1 and la1 in [LITERAL_try]:
                pass
                self.try_block()
                if not self.inputState.guessing:
                    node.add_child(self.node("unfinished_try_block"));
            elif la1 and la1 in [LITERAL_throw]:
                pass
                self.throw_statement()
                if not self.inputState.guessing:
                    node.add_child(self.node("unfinished_throw_stmt"));
            elif la1 and la1 in [LITERAL_asm,LITERAL___asm,LITERAL___asm__,LITERAL__asm]:
                pass
                asmnode=self.asm_block()
                if not self.inputState.guessing:
                    node.add_child(asmnode);
            else:
                synPredMatched332 = False
                if (_tokenSet_2.member(self.LA(1))) and (_tokenSet_3.member(self.LA(2))):
                    _m332 = self.mark()
                    synPredMatched332 = True
                    self.inputState.guessing += 1
                    try:
                        pass
                        self.declaration()
                    except antlr.RecognitionException, pe:
                        synPredMatched332 = False
                    self.rewind(_m332)
                    self.inputState.guessing -= 1
                if synPredMatched332:
                    pass
                    d=self.declaration()
                    if not self.inputState.guessing:
                        node.add_child(d);
                elif (self.LA(1)==ID) and (self.LA(2)==COLON):
                    pass
                    self.labeled_statement()
                    if not self.inputState.guessing:
                        node.add_child(self.node("unfinished_labeled_statement"));
                elif (_tokenSet_70.member(self.LA(1))) and (_tokenSet_107.member(self.LA(2))):
                    pass
                    e=self.expression()
                    if not self.inputState.guessing:
                        node.add_child(e);
                    self.match(SEMICOLON)
                    if not self.inputState.guessing:
                        self.end_of_stmt();
                else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_108)
            else:
                raise ex
        
        return node
    
    def labeled_statement(self):    
        
        try:      ## for error handling
            pass
            self.match(ID)
            self.match(COLON)
            s=self.statement()
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_108)
            else:
                raise ex
        
    
    def case_statement(self):    
        node = None
        
        node = self.node("case_statement");
        try:      ## for error handling
            pass
            self.match(LITERAL_case)
            cx=self.constant_expression()
            if not self.inputState.guessing:
                node.add_child(cx);
            self.match(COLON)
            s=self.statement()
            if not self.inputState.guessing:
                node.add_child(s);
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_108)
            else:
                raise ex
        
        return node
    
    def default_statement(self):    
        node = None
        
        node = self.node("default_statement");
        try:      ## for error handling
            pass
            self.match(LITERAL_default)
            self.match(COLON)
            s=self.statement()
            if not self.inputState.guessing:
                node.add_child(s);
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_108)
            else:
                raise ex
        
        return node
    
    def expression(self):    
        node = None
        
        try:      ## for error handling
            pass
            x=self.assignment_expression()
            if not self.inputState.guessing:
                node = x;
            while True:
                if (self.LA(1)==COMMA):
                    pass
                    self.match(COMMA)
                    x=self.assignment_expression()
                    if not self.inputState.guessing:
                        node = self.exprnode_acc(node, x, "comma");
                else:
                    break
                
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_109)
            else:
                raise ex
        
        return node
    
    def selection_statement(self):    
        node = None
        
        try:      ## for error handling
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [LITERAL_if]:
                pass
                self.match(LITERAL_if)
                if not self.inputState.guessing:
                    node = self.node("if");
                self.match(LPAREN)
                x1=self.expression()
                if not self.inputState.guessing:
                    node.add_child(x1);
                self.match(RPAREN)
                s1=self.statement()
                if not self.inputState.guessing:
                    node.add_child(s1);
                if (self.LA(1)==LITERAL_else) and (_tokenSet_106.member(self.LA(2))):
                    pass
                    self.match(LITERAL_else)
                    s2=self.statement()
                    if not self.inputState.guessing:
                        elsenode =  self.node("else"); elsenode.add_child(s2); node.add_child(elsenode);
                elif (_tokenSet_108.member(self.LA(1))) and (_tokenSet_110.member(self.LA(2))):
                    pass
                else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
            elif la1 and la1 in [LITERAL_switch]:
                pass
                self.match(LITERAL_switch)
                if not self.inputState.guessing:
                    node = self.node("switch");
                self.match(LPAREN)
                x3=self.expression()
                if not self.inputState.guessing:
                    node.add_child(x3);
                self.match(RPAREN)
                s3=self.statement()
                if not self.inputState.guessing:
                    node.add_child(s3);
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_108)
            else:
                raise ex
        
        return node
    
    def iteration_statement(self):    
        node = None
        
        taken_x4 = 0; taken_x5 = 0;
        try:      ## for error handling
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [LITERAL_while]:
                pass
                self.match(LITERAL_while)
                if not self.inputState.guessing:
                    node = self.node("while");
                self.match(LPAREN)
                x1=self.expression()
                self.match(RPAREN)
                if not self.inputState.guessing:
                    node.add_child(x1);
                s1=self.statement()
                if not self.inputState.guessing:
                    node.add_child(s1);
            elif la1 and la1 in [LITERAL_do]:
                pass
                self.match(LITERAL_do)
                if not self.inputState.guessing:
                    node = self.node("do");
                s2=self.statement()
                self.match(LITERAL_while)
                if not self.inputState.guessing:
                    node.add_child(s2);
                self.match(LPAREN)
                x2=self.expression()
                self.match(RPAREN)
                if not self.inputState.guessing:
                    node.add_child(x2);
                self.match(SEMICOLON)
                if not self.inputState.guessing:
                    self.end_of_stmt();
            elif la1 and la1 in [LITERAL_for]:
                pass
                self.match(LITERAL_for)
                if not self.inputState.guessing:
                    node = self.node("for");
                self.match(LPAREN)
                synPredMatched343 = False
                if (_tokenSet_2.member(self.LA(1))) and (_tokenSet_3.member(self.LA(2))):
                    _m343 = self.mark()
                    synPredMatched343 = True
                    self.inputState.guessing += 1
                    try:
                        pass
                        self.declaration()
                    except antlr.RecognitionException, pe:
                        synPredMatched343 = False
                    self.rewind(_m343)
                    self.inputState.guessing -= 1
                if synPredMatched343:
                    pass
                    d1=self.declaration()
                    if not self.inputState.guessing:
                        node.add_child(d1);
                elif (_tokenSet_70.member(self.LA(1))) and (_tokenSet_107.member(self.LA(2))):
                    pass
                    x3=self.expression()
                    self.match(SEMICOLON)
                    if not self.inputState.guessing:
                        self.end_of_stmt();
                    if not self.inputState.guessing:
                        node.add_child(x3);
                elif (self.LA(1)==SEMICOLON):
                    pass
                    self.match(SEMICOLON)
                    if not self.inputState.guessing:
                        self.end_of_stmt(); node.add_child(self.node("blank"));
                else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
                la1 = self.LA(1)
                if False:
                    pass
                elif la1 and la1 in [ID,StringLiteral,LPAREN,LITERAL_const_cast,LITERAL_char,LITERAL_bool,LITERAL_short,LITERAL_int,74,75,76,LITERAL_long,LITERAL_signed,LITERAL___signed,LITERAL___signed__,LITERAL_unsigned,LITERAL_float,LITERAL_double,LITERAL_void,LITERAL__declspec,LITERAL___declspec,LITERAL___builtin_va_list,OPERATOR,LITERAL_this,LITERAL_true,LITERAL_false,OCTALINT,STAR,AMPERSAND,TILDE,SCOPE,PLUS,MINUS,PLUSPLUS,MINUSMINUS,LITERAL_sizeof,LITERAL_dynamic_cast,LITERAL_static_cast,LITERAL_reinterpret_cast,NOT,LITERAL_new,LITERAL_delete,DECIMALINT,HEXADECIMALINT,CharLiteral,FLOATONE,FLOATTWO]:
                    pass
                    x4=self.expression()
                    if not self.inputState.guessing:
                        taken_x4 = 1; node.add_child(x4);
                elif la1 and la1 in [SEMICOLON]:
                    pass
                else:
                        raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                    
                self.match(SEMICOLON)
                if not self.inputState.guessing:
                    self.end_of_stmt();
                if not self.inputState.guessing:
                    if (taken_x4 == 0): node.add_child(self.node("blank"));
                la1 = self.LA(1)
                if False:
                    pass
                elif la1 and la1 in [ID,StringLiteral,LPAREN,LITERAL_const_cast,LITERAL_char,LITERAL_bool,LITERAL_short,LITERAL_int,74,75,76,LITERAL_long,LITERAL_signed,LITERAL___signed,LITERAL___signed__,LITERAL_unsigned,LITERAL_float,LITERAL_double,LITERAL_void,LITERAL__declspec,LITERAL___declspec,LITERAL___builtin_va_list,OPERATOR,LITERAL_this,LITERAL_true,LITERAL_false,OCTALINT,STAR,AMPERSAND,TILDE,SCOPE,PLUS,MINUS,PLUSPLUS,MINUSMINUS,LITERAL_sizeof,LITERAL_dynamic_cast,LITERAL_static_cast,LITERAL_reinterpret_cast,NOT,LITERAL_new,LITERAL_delete,DECIMALINT,HEXADECIMALINT,CharLiteral,FLOATONE,FLOATTWO]:
                    pass
                    x5=self.expression()
                    if not self.inputState.guessing:
                        taken_x5 = 1; node.add_child(x5);
                elif la1 and la1 in [RPAREN]:
                    pass
                else:
                        raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                    
                if not self.inputState.guessing:
                    if (taken_x5 == 0): node.add_child(self.node("blank"));
                self.match(RPAREN)
                s3=self.statement()
                if not self.inputState.guessing:
                    node.add_child(s3);
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_108)
            else:
                raise ex
        
        return node
    
    def jump_statement(self):    
        node = None
        
        gotoid = None
        try:      ## for error handling
            pass
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [LITERAL_goto]:
                pass
                self.match(LITERAL_goto)
                gotoid = self.LT(1)
                self.match(ID)
                self.match(SEMICOLON)
                if not self.inputState.guessing:
                    self.end_of_stmt();
                if not self.inputState.guessing:
                    node = self.node("jump_goto"); node.set_leaf(gotoid.getText());
            elif la1 and la1 in [LITERAL_continue]:
                pass
                self.match(LITERAL_continue)
                self.match(SEMICOLON)
                if not self.inputState.guessing:
                    self.end_of_stmt();
                if not self.inputState.guessing:
                    node = self.node("jump_statement"); node.set_leaf("continue");
            elif la1 and la1 in [LITERAL_break]:
                pass
                self.match(LITERAL_break)
                self.match(SEMICOLON)
                if not self.inputState.guessing:
                    self.end_of_stmt();
                if not self.inputState.guessing:
                    node = self.node("jump_statement"); node.set_leaf("break");
            elif la1 and la1 in [LITERAL_return]:
                pass
                self.match(LITERAL_return)
                if not self.inputState.guessing:
                    self.in_return = true;
                if not self.inputState.guessing:
                    node = self.node("jump_statement"); node.set_leaf("return");
                la1 = self.LA(1)
                if False:
                    pass
                elif la1 and la1 in [ID,StringLiteral,LPAREN,LITERAL_const_cast,LITERAL_char,LITERAL_bool,LITERAL_short,LITERAL_int,74,75,76,LITERAL_long,LITERAL_signed,LITERAL___signed,LITERAL___signed__,LITERAL_unsigned,LITERAL_float,LITERAL_double,LITERAL_void,LITERAL__declspec,LITERAL___declspec,LITERAL___builtin_va_list,OPERATOR,LITERAL_this,LITERAL_true,LITERAL_false,OCTALINT,STAR,AMPERSAND,TILDE,SCOPE,PLUS,MINUS,PLUSPLUS,MINUSMINUS,LITERAL_sizeof,LITERAL_dynamic_cast,LITERAL_static_cast,LITERAL_reinterpret_cast,NOT,LITERAL_new,LITERAL_delete,DECIMALINT,HEXADECIMALINT,CharLiteral,FLOATONE,FLOATTWO]:
                    pass
                    x=self.expression()
                    if not self.inputState.guessing:
                        node.add_child(x);
                elif la1 and la1 in [SEMICOLON]:
                    pass
                else:
                        raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                    
                self.match(SEMICOLON)
                if not self.inputState.guessing:
                    self.in_return = false; self.end_of_stmt();
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_108)
            else:
                raise ex
        
        return node
    
    def try_block(self):    
        
        try:      ## for error handling
            pass
            self.match(LITERAL_try)
            compstat=self.compound_statement()
            while True:
                if (self.LA(1)==LITERAL_catch):
                    pass
                    self.handler()
                else:
                    break
                
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_108)
            else:
                raise ex
        
    
    def throw_statement(self):    
        
        try:      ## for error handling
            pass
            self.match(LITERAL_throw)
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [ID,StringLiteral,LPAREN,LITERAL_const_cast,LITERAL_char,LITERAL_bool,LITERAL_short,LITERAL_int,74,75,76,LITERAL_long,LITERAL_signed,LITERAL___signed,LITERAL___signed__,LITERAL_unsigned,LITERAL_float,LITERAL_double,LITERAL_void,LITERAL__declspec,LITERAL___declspec,LITERAL___builtin_va_list,OPERATOR,LITERAL_this,LITERAL_true,LITERAL_false,OCTALINT,STAR,AMPERSAND,TILDE,SCOPE,PLUS,MINUS,PLUSPLUS,MINUSMINUS,LITERAL_sizeof,LITERAL_dynamic_cast,LITERAL_static_cast,LITERAL_reinterpret_cast,NOT,LITERAL_new,LITERAL_delete,DECIMALINT,HEXADECIMALINT,CharLiteral,FLOATONE,FLOATTWO]:
                pass
                ax=self.assignment_expression()
            elif la1 and la1 in [SEMICOLON]:
                pass
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
            self.match(SEMICOLON)
            if not self.inputState.guessing:
                self.end_of_stmt();
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_108)
            else:
                raise ex
        
    
    def asm_block(self):    
        node = None
        
        node = self.node("asm_block");
        try:      ## for error handling
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [LITERAL___asm,LITERAL__asm]:
                pass
                self.std_asm_block()
            elif la1 and la1 in [LITERAL_asm,LITERAL___asm__]:
                pass
                self.gcc_asm_block()
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_108)
            else:
                raise ex
        
        return node
    
    def handler(self):    
        
        try:      ## for error handling
            pass
            self.match(LITERAL_catch)
            if not self.inputState.guessing:
                exceptionBeginHandler();
            if not self.inputState.guessing:
                self.declaratorParameterList(1);
            self.match(LPAREN)
            self.exception_declaration()
            self.match(RPAREN)
            if not self.inputState.guessing:
                self.declaratorEndParameterList(1);
            compstat=self.compound_statement()
            if not self.inputState.guessing:
                exceptionEndHandler();
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_111)
            else:
                raise ex
        
    
    def exception_declaration(self):    
        
        try:      ## for error handling
            pass
            pdl=self.parameter_declaration_list()
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_54)
            else:
                raise ex
        
    
    def assignment_expression(self):    
        node = None
        
        try:      ## for error handling
            pass
            cx=self.conditional_expression()
            if not self.inputState.guessing:
                node = cx;
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [ASSIGNEQUAL,TIMESEQUAL,DIVIDEEQUAL,MINUSEQUAL,PLUSEQUAL,MODEQUAL,SHIFTLEFTEQUAL,SHIFTRIGHTEQUAL,BITWISEANDEQUAL,BITWISEXOREQUAL,BITWISEOREQUAL]:
                pass
                la1 = self.LA(1)
                if False:
                    pass
                elif la1 and la1 in [ASSIGNEQUAL]:
                    pass
                    self.match(ASSIGNEQUAL)
                    if not self.inputState.guessing:
                        leaf="assignequals";
                elif la1 and la1 in [TIMESEQUAL]:
                    pass
                    self.match(TIMESEQUAL)
                    if not self.inputState.guessing:
                        leaf="timesequals";
                elif la1 and la1 in [DIVIDEEQUAL]:
                    pass
                    self.match(DIVIDEEQUAL)
                    if not self.inputState.guessing:
                        leaf="dividesequals";
                elif la1 and la1 in [MINUSEQUAL]:
                    pass
                    self.match(MINUSEQUAL)
                    if not self.inputState.guessing:
                        leaf="minusequals";
                elif la1 and la1 in [PLUSEQUAL]:
                    pass
                    self.match(PLUSEQUAL)
                    if not self.inputState.guessing:
                        leaf="plusequals";
                elif la1 and la1 in [MODEQUAL]:
                    pass
                    self.match(MODEQUAL)
                    if not self.inputState.guessing:
                        leaf="modequals";
                elif la1 and la1 in [SHIFTLEFTEQUAL]:
                    pass
                    self.match(SHIFTLEFTEQUAL)
                    if not self.inputState.guessing:
                        leaf="shiftleftequals";
                elif la1 and la1 in [SHIFTRIGHTEQUAL]:
                    pass
                    self.match(SHIFTRIGHTEQUAL)
                    if not self.inputState.guessing:
                        leaf="shiftrightequals";
                elif la1 and la1 in [BITWISEANDEQUAL]:
                    pass
                    self.match(BITWISEANDEQUAL)
                    if not self.inputState.guessing:
                        leaf="bitwiseandequals";
                elif la1 and la1 in [BITWISEXOREQUAL]:
                    pass
                    self.match(BITWISEXOREQUAL)
                    if not self.inputState.guessing:
                        leaf="bitwisexorequals";
                elif la1 and la1 in [BITWISEOREQUAL]:
                    pass
                    self.match(BITWISEOREQUAL)
                    if not self.inputState.guessing:
                        leaf="bitwiseorequals";
                else:
                        raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                    
                rx=self.remainder_expression()
                if not self.inputState.guessing:
                    node = self.exprnode_acc(node, rx, leaf);
            elif la1 and la1 in [GREATERTHAN,SEMICOLON,RCURLY,COLON,COMMA,LITERAL___attribute__,RPAREN,RSQUARE,ELLIPSIS]:
                pass
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_74)
            else:
                raise ex
        
        return node
    
    def std_asm_block(self):    
        
        try:      ## for error handling
            pass
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [LITERAL__asm]:
                pass
                self.match(LITERAL__asm)
            elif la1 and la1 in [LITERAL___asm]:
                pass
                self.match(LITERAL___asm)
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
            self.match(LCURLY)
            while True:
                if (_tokenSet_112.member(self.LA(1))):
                    pass
                    self.matchNot(RCURLY)
                else:
                    break
                
            self.match(RCURLY)
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_108)
            else:
                raise ex
        
    
    def gcc_asm_block(self):    
        
        try:      ## for error handling
            pass
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [LITERAL___asm__]:
                pass
                self.match(LITERAL___asm__)
            elif la1 and la1 in [LITERAL_asm]:
                pass
                self.match(LITERAL_asm)
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [LITERAL_volatile]:
                pass
                self.match(LITERAL_volatile)
            elif la1 and la1 in [LITERAL___volatile__]:
                pass
                self.match(LITERAL___volatile__)
            elif la1 and la1 in [LITERAL___volatile]:
                pass
                self.match(LITERAL___volatile)
            elif la1 and la1 in [LPAREN]:
                pass
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
            self.match(LPAREN)
            self.asm_code()
            while True:
                if (self.LA(1)==COLON or self.LA(1)==SCOPE):
                    pass
                    la1 = self.LA(1)
                    if False:
                        pass
                    elif la1 and la1 in [COLON]:
                        pass
                        self.match(COLON)
                    elif la1 and la1 in [SCOPE]:
                        pass
                        self.match(SCOPE)
                    else:
                            raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                        
                    la1 = self.LA(1)
                    if False:
                        pass
                    elif la1 and la1 in [StringLiteral,LSQUARE]:
                        pass
                        self.asm_reg_spec()
                    elif la1 and la1 in [COLON,RPAREN,SCOPE]:
                        pass
                    else:
                            raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                        
                else:
                    break
                
            self.match(RPAREN)
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_108)
            else:
                raise ex
        
    
    def asm_code(self):    
        
        try:      ## for error handling
            pass
            _cnt372= 0
            while True:
                if (self.LA(1)==StringLiteral):
                    pass
                    self.match(StringLiteral)
                else:
                    break
                
                _cnt372 += 1
            if _cnt372 < 1:
                raise antlr.NoViableAltException(self.LT(1), self.getFilename())
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_113)
            else:
                raise ex
        
    
    def asm_reg_spec(self):    
        
        try:      ## for error handling
            pass
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [LSQUARE]:
                pass
                self.match(LSQUARE)
                self.match(ID)
                self.match(RSQUARE)
            elif la1 and la1 in [StringLiteral]:
                pass
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
            self.match(StringLiteral)
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [LPAREN]:
                pass
                self.match(LPAREN)
                ex1=self.expression()
                self.match(RPAREN)
            elif la1 and la1 in [COLON,COMMA,RPAREN,SCOPE]:
                pass
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
            while True:
                if (self.LA(1)==COMMA):
                    pass
                    self.match(COMMA)
                    la1 = self.LA(1)
                    if False:
                        pass
                    elif la1 and la1 in [LSQUARE]:
                        pass
                        self.match(LSQUARE)
                        self.match(ID)
                        self.match(RSQUARE)
                    elif la1 and la1 in [StringLiteral]:
                        pass
                    else:
                            raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                        
                    self.match(StringLiteral)
                    la1 = self.LA(1)
                    if False:
                        pass
                    elif la1 and la1 in [LPAREN]:
                        pass
                        self.match(LPAREN)
                        ex2=self.expression()
                        self.match(RPAREN)
                    elif la1 and la1 in [COLON,COMMA,RPAREN,SCOPE]:
                        pass
                    else:
                            raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                        
                else:
                    break
                
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_113)
            else:
                raise ex
        
    
    def conditional_expression(self):    
        node = None
        
        try:      ## for error handling
            pass
            lx=self.logical_or_expression()
            if not self.inputState.guessing:
                node = lx;
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [QUESTIONMARK]:
                pass
                self.match(QUESTIONMARK)
                e=self.expression()
                self.match(COLON)
                cx=self.conditional_expression()
                if not self.inputState.guessing:
                    node = self.node("expression"); node.add_child(lx);
                if not self.inputState.guessing:
                    node.add_child(e); node.add_child(cx);
                if not self.inputState.guessing:
                    node.set_leaf("c_ternary");
            elif la1 and la1 in [GREATERTHAN,SEMICOLON,RCURLY,ASSIGNEQUAL,COLON,COMMA,LITERAL___attribute__,RPAREN,RSQUARE,ELLIPSIS,TIMESEQUAL,DIVIDEEQUAL,MINUSEQUAL,PLUSEQUAL,MODEQUAL,SHIFTLEFTEQUAL,SHIFTRIGHTEQUAL,BITWISEANDEQUAL,BITWISEXOREQUAL,BITWISEOREQUAL]:
                pass
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_114)
            else:
                raise ex
        
        return node
    
    def logical_or_expression(self):    
        node = None
        
        try:      ## for error handling
            pass
            x=self.logical_and_expression()
            if not self.inputState.guessing:
                node = x;
            while True:
                if (self.LA(1)==OR):
                    pass
                    self.match(OR)
                    x=self.logical_and_expression()
                    if not self.inputState.guessing:
                        node = self.exprnode_acc(node, x, "logicalor");
                else:
                    break
                
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_115)
            else:
                raise ex
        
        return node
    
    def logical_and_expression(self):    
        node = None
        
        try:      ## for error handling
            pass
            x=self.inclusive_or_expression()
            if not self.inputState.guessing:
                node = x;
            while True:
                if (self.LA(1)==AND):
                    pass
                    self.match(AND)
                    x=self.inclusive_or_expression()
                    if not self.inputState.guessing:
                        node = self.exprnode_acc(node, x, "logicaland");
                else:
                    break
                
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_116)
            else:
                raise ex
        
        return node
    
    def inclusive_or_expression(self):    
        node = None
        
        try:      ## for error handling
            pass
            x=self.exclusive_or_expression()
            if not self.inputState.guessing:
                node = x;
            while True:
                if (self.LA(1)==BITWISEOR):
                    pass
                    self.match(BITWISEOR)
                    x=self.exclusive_or_expression()
                    if not self.inputState.guessing:
                        node = self.exprnode_acc(node, x, "bitwiseor");
                else:
                    break
                
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_117)
            else:
                raise ex
        
        return node
    
    def exclusive_or_expression(self):    
        node = None
        
        try:      ## for error handling
            pass
            x=self.and_expression()
            if not self.inputState.guessing:
                node = x;
            while True:
                if (self.LA(1)==BITWISEXOR):
                    pass
                    self.match(BITWISEXOR)
                    x=self.and_expression()
                    if not self.inputState.guessing:
                        node = self.exprnode_acc(node, x, "bitwisexor");
                else:
                    break
                
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_118)
            else:
                raise ex
        
        return node
    
    def and_expression(self):    
        node = None
        
        try:      ## for error handling
            pass
            x=self.equality_expression()
            if not self.inputState.guessing:
                node = x;
            while True:
                if (self.LA(1)==AMPERSAND):
                    pass
                    self.match(AMPERSAND)
                    x=self.equality_expression()
                    if not self.inputState.guessing:
                        node = self.exprnode_acc(node, x, "bitwiseand");
                else:
                    break
                
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_119)
            else:
                raise ex
        
        return node
    
    def equality_expression(self):    
        node = None
        
        try:      ## for error handling
            pass
            x=self.relational_expression()
            if not self.inputState.guessing:
                node = x;
            while True:
                if (self.LA(1)==NOTEQUAL or self.LA(1)==EQUAL):
                    pass
                    la1 = self.LA(1)
                    if False:
                        pass
                    elif la1 and la1 in [NOTEQUAL]:
                        pass
                        self.match(NOTEQUAL)
                        if not self.inputState.guessing:
                            leaf = "notequals";
                    elif la1 and la1 in [EQUAL]:
                        pass
                        self.match(EQUAL)
                        if not self.inputState.guessing:
                            leaf = "equals";
                    else:
                            raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                        
                    x=self.relational_expression()
                    if not self.inputState.guessing:
                        node = self.exprnode_acc(node, x, leaf);
                else:
                    break
                
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_120)
            else:
                raise ex
        
        return node
    
    def relational_expression(self):    
        node = None
        
        try:      ## for error handling
            pass
            x=self.shift_expression()
            if not self.inputState.guessing:
                node = x;
            while True:
                if (_tokenSet_121.member(self.LA(1))) and (_tokenSet_70.member(self.LA(2))):
                    pass
                    la1 = self.LA(1)
                    if False:
                        pass
                    elif la1 and la1 in [LESSTHAN]:
                        pass
                        self.match(LESSTHAN)
                        if not self.inputState.guessing:
                            leaf = "lessthan";
                    elif la1 and la1 in [GREATERTHAN]:
                        pass
                        self.match(GREATERTHAN)
                        if not self.inputState.guessing:
                            leaf = "greaterthan";
                    elif la1 and la1 in [LESSTHANOREQUALTO]:
                        pass
                        self.match(LESSTHANOREQUALTO)
                        if not self.inputState.guessing:
                            leaf = "lessthanorequals";
                    elif la1 and la1 in [GREATERTHANOREQUALTO]:
                        pass
                        self.match(GREATERTHANOREQUALTO)
                        if not self.inputState.guessing:
                            leaf = "greaterthanorequals";
                    else:
                            raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                        
                    x=self.shift_expression()
                    if not self.inputState.guessing:
                        node = self.exprnode_acc(node, x, leaf);
                else:
                    break
                
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_122)
            else:
                raise ex
        
        return node
    
    def additive_expression(self):    
        node = None
        
        try:      ## for error handling
            pass
            x=self.multiplicative_expression()
            if not self.inputState.guessing:
                node = x;
            while True:
                if (self.LA(1)==PLUS or self.LA(1)==MINUS):
                    pass
                    la1 = self.LA(1)
                    if False:
                        pass
                    elif la1 and la1 in [PLUS]:
                        pass
                        self.match(PLUS)
                        if not self.inputState.guessing:
                            leaf = "add";
                    elif la1 and la1 in [MINUS]:
                        pass
                        self.match(MINUS)
                        if not self.inputState.guessing:
                            leaf = "subtract";
                    else:
                            raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                        
                    x=self.multiplicative_expression()
                    if not self.inputState.guessing:
                        node = self.exprnode_acc(node, x, leaf);
                else:
                    break
                
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_123)
            else:
                raise ex
        
        return node
    
    def multiplicative_expression(self):    
        node = None
        
        try:      ## for error handling
            pass
            x=self.pm_expression()
            if not self.inputState.guessing:
                node = x;
            while True:
                if (_tokenSet_124.member(self.LA(1))):
                    pass
                    la1 = self.LA(1)
                    if False:
                        pass
                    elif la1 and la1 in [STAR]:
                        pass
                        self.match(STAR)
                        if not self.inputState.guessing:
                            leaf = "multiply";
                    elif la1 and la1 in [DIVIDE]:
                        pass
                        self.match(DIVIDE)
                        if not self.inputState.guessing:
                            leaf = "divide";
                    elif la1 and la1 in [MOD]:
                        pass
                        self.match(MOD)
                        if not self.inputState.guessing:
                            leaf = "modulus";
                    else:
                            raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                        
                    x=self.pm_expression()
                    if not self.inputState.guessing:
                        node = self.exprnode_acc(node, x, leaf);
                else:
                    break
                
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_125)
            else:
                raise ex
        
        return node
    
    def pm_expression(self):    
        node = None
        
        try:      ## for error handling
            pass
            x=self.cast_expression()
            if not self.inputState.guessing:
                node = x;
            while True:
                if (self.LA(1)==DOTMBR or self.LA(1)==POINTERTOMBR):
                    pass
                    la1 = self.LA(1)
                    if False:
                        pass
                    elif la1 and la1 in [DOTMBR]:
                        pass
                        self.match(DOTMBR)
                        if not self.inputState.guessing:
                            leaf = ".*";
                    elif la1 and la1 in [POINTERTOMBR]:
                        pass
                        self.match(POINTERTOMBR)
                        if not self.inputState.guessing:
                            leaf = "->*";
                    else:
                            raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                        
                    x=self.cast_expression()
                    if not self.inputState.guessing:
                        node = self.exprnode_acc(node, x, leaf);
                else:
                    break
                
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_126)
            else:
                raise ex
        
        return node
    
    def cast_expression(self):    
        node = None
        
        try:      ## for error handling
            synPredMatched435 = False
            if (self.LA(1)==LPAREN) and (_tokenSet_13.member(self.LA(2))):
                _m435 = self.mark()
                synPredMatched435 = True
                self.inputState.guessing += 1
                try:
                    pass
                    self.compound_literal()
                except antlr.RecognitionException, pe:
                    synPredMatched435 = False
                self.rewind(_m435)
                self.inputState.guessing -= 1
            if synPredMatched435:
                pass
                x=self.compound_literal()
                if not self.inputState.guessing:
                    node = x;
            else:
                synPredMatched440 = False
                if (self.LA(1)==LPAREN) and (_tokenSet_127.member(self.LA(2))):
                    _m440 = self.mark()
                    synPredMatched440 = True
                    self.inputState.guessing += 1
                    try:
                        pass
                        self.match(LPAREN)
                        la1 = self.LA(1)
                        if False:
                            pass
                        elif la1 and la1 in [LITERAL_const,LITERAL___const,LITERAL_const_cast,LITERAL_volatile,LITERAL___volatile,LITERAL___restrict,LITERAL___restrict__]:
                            pass
                            self.type_qualifier()
                        elif la1 and la1 in [LITERAL_enum,ID,LITERAL_char,LITERAL_bool,LITERAL_short,LITERAL_int,74,75,76,LITERAL_long,LITERAL_signed,LITERAL___signed,LITERAL___signed__,LITERAL_unsigned,LITERAL_float,LITERAL_double,LITERAL_void,LITERAL__declspec,LITERAL___declspec,LITERAL___builtin_va_list,LITERAL_class,LITERAL_struct,LITERAL_union,SCOPE]:
                            pass
                        else:
                                raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                            
                        self.type_specifier(None)
                        while True:
                            if (_tokenSet_24.member(self.LA(1))):
                                pass
                                self.ptr_operator()
                            else:
                                break
                            
                        self.match(RPAREN)
                    except antlr.RecognitionException, pe:
                        synPredMatched440 = False
                    self.rewind(_m440)
                    self.inputState.guessing -= 1
                if synPredMatched440:
                    pass
                    if not self.inputState.guessing:
                        node = self.node("expression");
                    if not self.inputState.guessing:
                        node.set_leaf("cast");
                    self.match(LPAREN)
                    la1 = self.LA(1)
                    if False:
                        pass
                    elif la1 and la1 in [LITERAL_const,LITERAL___const,LITERAL_const_cast,LITERAL_volatile,LITERAL___volatile,LITERAL___restrict,LITERAL___restrict__]:
                        pass
                        tq=self.type_qualifier()
                        if not self.inputState.guessing:
                            node.add_child(self.node(tq));
                    elif la1 and la1 in [LITERAL_enum,ID,LITERAL_char,LITERAL_bool,LITERAL_short,LITERAL_int,74,75,76,LITERAL_long,LITERAL_signed,LITERAL___signed,LITERAL___signed__,LITERAL_unsigned,LITERAL_float,LITERAL_double,LITERAL_void,LITERAL__declspec,LITERAL___declspec,LITERAL___builtin_va_list,LITERAL_class,LITERAL_struct,LITERAL_union,SCOPE]:
                        pass
                    else:
                            raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                        
                    ts=self.type_specifier(None)
                    if not self.inputState.guessing:
                        node.add_child(ts);
                    if not self.inputState.guessing:
                        indirection = self.node("indirection"); node.add_child(indirection);
                    while True:
                        if (_tokenSet_24.member(self.LA(1))):
                            pass
                            po=self.ptr_operator()
                            if not self.inputState.guessing:
                                indirection.extend_leaf(po);
                        else:
                            break
                        
                    self.match(RPAREN)
                    cx=self.cast_expression()
                    if not self.inputState.guessing:
                        node.add_child(cx);
                elif (_tokenSet_70.member(self.LA(1))) and (_tokenSet_73.member(self.LA(2))):
                    pass
                    x=self.unary_expression()
                    if not self.inputState.guessing:
                        node = x;
                else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_128)
            else:
                raise ex
        
        return node
    
    def compound_literal(self):    
        node = None
        
        node = self.node("compound_literal");
        try:      ## for error handling
            pass
            self.match(LPAREN)
            tn=self.type_name()
            if not self.inputState.guessing:
                node.add_child(tn);
            self.match(RPAREN)
            self.match(LCURLY)
            i=self.initializer()
            if not self.inputState.guessing:
                node.add_child(i);
            self.match(RCURLY)
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_128)
            else:
                raise ex
        
        return node
    
    def unary_expression(self):    
        node = None
        
        s = None
        try:      ## for error handling
            pass
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [PLUSPLUS]:
                pass
                self.match(PLUSPLUS)
                x=self.unary_expression()
                if not self.inputState.guessing:
                    node = self.node("expression"); node.set_leaf("++."); node.add_child(x);
            elif la1 and la1 in [MINUSMINUS]:
                pass
                self.match(MINUSMINUS)
                x=self.unary_expression()
                if not self.inputState.guessing:
                    node = self.node("expression"); node.set_leaf("--."); node.add_child(x);
            elif la1 and la1 in [STAR,AMPERSAND,TILDE,PLUS,MINUS,NOT]:
                pass
                op=self.unary_operator()
                x=self.cast_expression()
                if not self.inputState.guessing:
                    node = self.node("expression"); node.set_leaf(op); node.add_child(x);
            elif la1 and la1 in [LITERAL_sizeof]:
                pass
                self.match(LITERAL_sizeof)
                if not self.inputState.guessing:
                    node = self.node("expression"); node.set_leaf("sizeof");
                if ((self.LA(1)==LPAREN) and (_tokenSet_13.member(self.LA(2))) and ((not(((self.LA(1)==LPAREN and (self.LA(2)==ID))))or(self.isTypeName((self.LT(2).getText())))))):
                    pass
                    self.match(LPAREN)
                    x=self.type_name()
                    self.match(RPAREN)
                    if not self.inputState.guessing:
                        node.add_child(x);
                elif (_tokenSet_70.member(self.LA(1))) and (_tokenSet_73.member(self.LA(2))):
                    pass
                    x=self.unary_expression()
                    if not self.inputState.guessing:
                        node.add_child(x);
                else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
            else:
                synPredMatched447 = False
                if (_tokenSet_129.member(self.LA(1))) and (_tokenSet_130.member(self.LA(2))):
                    _m447 = self.mark()
                    synPredMatched447 = True
                    self.inputState.guessing += 1
                    try:
                        pass
                        self.postfix_expression()
                    except antlr.RecognitionException, pe:
                        synPredMatched447 = False
                    self.rewind(_m447)
                    self.inputState.guessing -= 1
                if synPredMatched447:
                    pass
                    x=self.postfix_expression()
                    if not self.inputState.guessing:
                        node = x;
                elif (_tokenSet_131.member(self.LA(1))) and (_tokenSet_132.member(self.LA(2))):
                    pass
                    if not self.inputState.guessing:
                        node = self.node("expression");
                    la1 = self.LA(1)
                    if False:
                        pass
                    elif la1 and la1 in [SCOPE]:
                        pass
                        s = self.LT(1)
                        self.match(SCOPE)
                        if not self.inputState.guessing:
                            node.set_leaf(s.getText());
                    elif la1 and la1 in [LITERAL_new,LITERAL_delete]:
                        pass
                    else:
                            raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                        
                    la1 = self.LA(1)
                    if False:
                        pass
                    elif la1 and la1 in [LITERAL_new]:
                        pass
                        x=self.new_expression()
                        if not self.inputState.guessing:
                            node.add_child(x);
                    elif la1 and la1 in [LITERAL_delete]:
                        pass
                        x=self.delete_expression()
                        if not self.inputState.guessing:
                            node.add_child(x);
                    else:
                            raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                        
                else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_128)
            else:
                raise ex
        
        return node
    
    def postfix_expression(self):    
        node = None
        
        # Purpose?
        ds = None
        try:      ## for error handling
            pass
            synPredMatched454 = False
            if ((_tokenSet_101.member(self.LA(1))) and (_tokenSet_133.member(self.LA(2))) and (not (self.LA(1) == LPAREN))):
                _m454 = self.mark()
                synPredMatched454 = True
                self.inputState.guessing += 1
                try:
                    pass
                    ts=self.simple_type_specifier()
                    self.match(LPAREN)
                    self.match(RPAREN)
                    self.match(LPAREN)
                except antlr.RecognitionException, pe:
                    synPredMatched454 = False
                self.rewind(_m454)
                self.inputState.guessing -= 1
            if synPredMatched454:
                pass
                ts=self.simple_type_specifier()
                self.match(LPAREN)
                self.match(RPAREN)
                self.match(LPAREN)
                la1 = self.LA(1)
                if False:
                    pass
                elif la1 and la1 in [ID,StringLiteral,LPAREN,LITERAL_const_cast,LITERAL_char,LITERAL_bool,LITERAL_short,LITERAL_int,74,75,76,LITERAL_long,LITERAL_signed,LITERAL___signed,LITERAL___signed__,LITERAL_unsigned,LITERAL_float,LITERAL_double,LITERAL_void,LITERAL__declspec,LITERAL___declspec,LITERAL___builtin_va_list,OPERATOR,LITERAL_this,LITERAL_true,LITERAL_false,OCTALINT,STAR,AMPERSAND,TILDE,SCOPE,PLUS,MINUS,PLUSPLUS,MINUSMINUS,LITERAL_sizeof,LITERAL_dynamic_cast,LITERAL_static_cast,LITERAL_reinterpret_cast,NOT,LITERAL_new,LITERAL_delete,DECIMALINT,HEXADECIMALINT,CharLiteral,FLOATONE,FLOATTWO]:
                    pass
                    el=self.expression_list()
                elif la1 and la1 in [RPAREN]:
                    pass
                else:
                        raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                    
                self.match(RPAREN)
                if not self.inputState.guessing:
                    node = self.node("unfinished_postfix-expr");
            else:
                synPredMatched457 = False
                if ((_tokenSet_101.member(self.LA(1))) and (_tokenSet_133.member(self.LA(2))) and ((self.LA(1) != LPAREN))):
                    _m457 = self.mark()
                    synPredMatched457 = True
                    self.inputState.guessing += 1
                    try:
                        pass
                        ts=self.simple_type_specifier()
                        self.match(LPAREN)
                    except antlr.RecognitionException, pe:
                        synPredMatched457 = False
                    self.rewind(_m457)
                    self.inputState.guessing -= 1
                if synPredMatched457:
                    pass
                    ts=self.simple_type_specifier()
                    self.match(LPAREN)
                    la1 = self.LA(1)
                    if False:
                        pass
                    elif la1 and la1 in [ID,StringLiteral,LPAREN,LITERAL_const_cast,LITERAL_char,LITERAL_bool,LITERAL_short,LITERAL_int,74,75,76,LITERAL_long,LITERAL_signed,LITERAL___signed,LITERAL___signed__,LITERAL_unsigned,LITERAL_float,LITERAL_double,LITERAL_void,LITERAL__declspec,LITERAL___declspec,LITERAL___builtin_va_list,OPERATOR,LITERAL_this,LITERAL_true,LITERAL_false,OCTALINT,STAR,AMPERSAND,TILDE,SCOPE,PLUS,MINUS,PLUSPLUS,MINUSMINUS,LITERAL_sizeof,LITERAL_dynamic_cast,LITERAL_static_cast,LITERAL_reinterpret_cast,NOT,LITERAL_new,LITERAL_delete,DECIMALINT,HEXADECIMALINT,CharLiteral,FLOATONE,FLOATTWO]:
                        pass
                        el=self.expression_list()
                    elif la1 and la1 in [RPAREN]:
                        pass
                    else:
                            raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                        
                    self.match(RPAREN)
                    if not self.inputState.guessing:
                        node = self.node("unfinished_postfix-expr-2");
                elif (_tokenSet_134.member(self.LA(1))) and (_tokenSet_130.member(self.LA(2))):
                    pass
                    px=self.primary_expression()
                    if not self.inputState.guessing:
                        node = px;
                    if not self.inputState.guessing:
                        prevnode = node;
                    while True:
                        la1 = self.LA(1)
                        if False:
                            pass
                        elif la1 and la1 in [LSQUARE]:
                            pass
                            self.match(LSQUARE)
                            x=self.expression()
                            self.match(RSQUARE)
                            if not self.inputState.guessing:
                                node = self.node("expression"); node.set_leaf("arrayindex"); node.add_child(prevnode); node.add_child(x); prevnode = node;
                        elif la1 and la1 in [LPAREN]:
                            pass
                            self.match(LPAREN)
                            if not self.inputState.guessing:
                                node = self.node("expression"); node.set_leaf("call"); node.add_child(prevnode);
                            la1 = self.LA(1)
                            if False:
                                pass
                            elif la1 and la1 in [ID,StringLiteral,LPAREN,LITERAL_const_cast,LITERAL_char,LITERAL_bool,LITERAL_short,LITERAL_int,74,75,76,LITERAL_long,LITERAL_signed,LITERAL___signed,LITERAL___signed__,LITERAL_unsigned,LITERAL_float,LITERAL_double,LITERAL_void,LITERAL__declspec,LITERAL___declspec,LITERAL___builtin_va_list,OPERATOR,LITERAL_this,LITERAL_true,LITERAL_false,OCTALINT,STAR,AMPERSAND,TILDE,SCOPE,PLUS,MINUS,PLUSPLUS,MINUSMINUS,LITERAL_sizeof,LITERAL_dynamic_cast,LITERAL_static_cast,LITERAL_reinterpret_cast,NOT,LITERAL_new,LITERAL_delete,DECIMALINT,HEXADECIMALINT,CharLiteral,FLOATONE,FLOATTWO]:
                                pass
                                x2=self.expression_list()
                                if not self.inputState.guessing:
                                    node.add_child(x2);
                            elif la1 and la1 in [RPAREN]:
                                pass
                            else:
                                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                                
                            if not self.inputState.guessing:
                                prevnode = node;
                            self.match(RPAREN)
                        elif la1 and la1 in [DOT]:
                            pass
                            self.match(DOT)
                            idxpr=self.id_expression()
                            if not self.inputState.guessing:
                                node = self.node("expression"); node.set_leaf("postfixdot"); node.add_child(prevnode); node.add_child(idxpr);
                            if not self.inputState.guessing:
                                prevnode = node;
                        elif la1 and la1 in [POINTERTO]:
                            pass
                            self.match(POINTERTO)
                            idxpr2=self.id_expression()
                            if not self.inputState.guessing:
                                node = self.node("expression"); node.set_leaf("postfixptr");
                            if not self.inputState.guessing:
                                node.add_child(prevnode); node.add_child(idxpr2); prevnode = node;
                        elif la1 and la1 in [PLUSPLUS]:
                            pass
                            self.match(PLUSPLUS)
                            if not self.inputState.guessing:
                                node = self.node("expression"); node.set_leaf("plusplus"); node.add_child(prevnode);
                            if not self.inputState.guessing:
                                prevnode = node;
                        elif la1 and la1 in [MINUSMINUS]:
                            pass
                            self.match(MINUSMINUS)
                            if not self.inputState.guessing:
                                node = self.node("expression"); node.set_leaf("minusminus"); node.add_child(prevnode);
                            if not self.inputState.guessing:
                                prevnode = node;
                        else:
                                break
                            
                elif (_tokenSet_135.member(self.LA(1))):
                    pass
                    la1 = self.LA(1)
                    if False:
                        pass
                    elif la1 and la1 in [LITERAL_dynamic_cast]:
                        pass
                        self.match(LITERAL_dynamic_cast)
                    elif la1 and la1 in [LITERAL_static_cast]:
                        pass
                        self.match(LITERAL_static_cast)
                    elif la1 and la1 in [LITERAL_reinterpret_cast]:
                        pass
                        self.match(LITERAL_reinterpret_cast)
                    elif la1 and la1 in [LITERAL_const_cast]:
                        pass
                        self.match(LITERAL_const_cast)
                    else:
                            raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                        
                    self.match(LESSTHAN)
                    ts=self.type_specifier(ds)
                    la1 = self.LA(1)
                    if False:
                        pass
                    elif la1 and la1 in [ID,LITERAL__stdcall,LITERAL___stdcall,STAR,AMPERSAND,SCOPE,LITERAL__cdecl,LITERAL___cdecl,LITERAL__near,LITERAL___near,LITERAL__far,LITERAL___far,LITERAL___interrupt,LITERAL_pascal,LITERAL__pascal,LITERAL___pascal]:
                        pass
                        po=self.ptr_operator()
                    elif la1 and la1 in [GREATERTHAN]:
                        pass
                    else:
                            raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                        
                    self.match(GREATERTHAN)
                    self.match(LPAREN)
                    x=self.expression()
                    self.match(RPAREN)
                    if not self.inputState.guessing:
                        node = self.node("unfinished_postfix-expr-3");
                else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_128)
            else:
                raise ex
        
        return node
    
    def unary_operator(self):    
        op = None
        
        try:      ## for error handling
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [AMPERSAND]:
                pass
                self.match(AMPERSAND)
                if not self.inputState.guessing:
                    op = "unary_ampersand";
            elif la1 and la1 in [STAR]:
                pass
                self.match(STAR)
                if not self.inputState.guessing:
                    op = "unary_pointsto";
            elif la1 and la1 in [PLUS]:
                pass
                self.match(PLUS)
                if not self.inputState.guessing:
                    op = "unary_plus";
            elif la1 and la1 in [MINUS]:
                pass
                self.match(MINUS)
                if not self.inputState.guessing:
                    op = "unary_minus";
            elif la1 and la1 in [TILDE]:
                pass
                self.match(TILDE)
                if not self.inputState.guessing:
                    op = "unary_tilde";
            elif la1 and la1 in [NOT]:
                pass
                self.match(NOT)
                if not self.inputState.guessing:
                    op = "unary_not";
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_70)
            else:
                raise ex
        
        return op
    
    def new_expression(self):    
        node = None
        
        try:      ## for error handling
            pass
            pass
            self.match(LITERAL_new)
            synPredMatched473 = False
            if (self.LA(1)==LPAREN) and (_tokenSet_70.member(self.LA(2))):
                _m473 = self.mark()
                synPredMatched473 = True
                self.inputState.guessing += 1
                try:
                    pass
                    self.match(LPAREN)
                    self.expression_list()
                    self.match(RPAREN)
                except antlr.RecognitionException, pe:
                    synPredMatched473 = False
                self.rewind(_m473)
                self.inputState.guessing -= 1
            if synPredMatched473:
                pass
                self.match(LPAREN)
                el=self.expression_list()
                self.match(RPAREN)
            elif (_tokenSet_136.member(self.LA(1))) and (_tokenSet_137.member(self.LA(2))):
                pass
            else:
                raise antlr.NoViableAltException(self.LT(1), self.getFilename())
            
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [LITERAL_typedef,LITERAL_enum,ID,LITERAL_inline,LITERAL_extern,LITERAL___extension__,LITERAL__inline,LITERAL___inline,LITERAL___inline__,LITERAL_virtual,LITERAL_explicit,LITERAL_friend,LITERAL__stdcall,LITERAL___stdcall,LITERAL_typename,LITERAL_const,LITERAL___const,LITERAL_auto,LITERAL_register,LITERAL_static,LITERAL_mutable,LITERAL_const_cast,LITERAL_volatile,LITERAL___volatile,LITERAL___restrict,LITERAL___restrict__,LITERAL_char,LITERAL_bool,LITERAL_short,LITERAL_int,74,75,76,LITERAL_long,LITERAL_signed,LITERAL___signed,LITERAL___signed__,LITERAL_unsigned,LITERAL_float,LITERAL_double,LITERAL_void,LITERAL__declspec,LITERAL___declspec,LITERAL___builtin_va_list,LITERAL_class,LITERAL_struct,LITERAL_union,SCOPE]:
                pass
                self.new_type_id()
            elif la1 and la1 in [LPAREN]:
                pass
                self.match(LPAREN)
                tn=self.type_name()
                self.match(RPAREN)
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [LPAREN]:
                pass
                self.new_initializer()
            elif la1 and la1 in [LESSTHAN,GREATERTHAN,SEMICOLON,RCURLY,ASSIGNEQUAL,COLON,COMMA,LITERAL___attribute__,RPAREN,STAR,AMPERSAND,RSQUARE,ELLIPSIS,TIMESEQUAL,DIVIDEEQUAL,MINUSEQUAL,PLUSEQUAL,MODEQUAL,SHIFTLEFTEQUAL,SHIFTRIGHTEQUAL,BITWISEANDEQUAL,BITWISEXOREQUAL,BITWISEOREQUAL,QUESTIONMARK,OR,AND,BITWISEOR,BITWISEXOR,NOTEQUAL,EQUAL,LESSTHANOREQUALTO,GREATERTHANOREQUALTO,SHIFTLEFT,SHIFTRIGHT,PLUS,MINUS,DIVIDE,MOD,DOTMBR,POINTERTOMBR]:
                pass
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
            if not self.inputState.guessing:
                node = self.node("unfinished_new-expr");
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_128)
            else:
                raise ex
        
        return node
    
    def delete_expression(self):    
        node = None
        
        node = self.node("delete");
        try:      ## for error handling
            pass
            self.match(LITERAL_delete)
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [LSQUARE]:
                pass
                self.match(LSQUARE)
                self.match(RSQUARE)
                if not self.inputState.guessing:
                    node.set_leaf("arrayindex");
            elif la1 and la1 in [ID,StringLiteral,LPAREN,LITERAL_const_cast,LITERAL_char,LITERAL_bool,LITERAL_short,LITERAL_int,74,75,76,LITERAL_long,LITERAL_signed,LITERAL___signed,LITERAL___signed__,LITERAL_unsigned,LITERAL_float,LITERAL_double,LITERAL_void,LITERAL__declspec,LITERAL___declspec,LITERAL___builtin_va_list,OPERATOR,LITERAL_this,LITERAL_true,LITERAL_false,OCTALINT,STAR,AMPERSAND,TILDE,SCOPE,PLUS,MINUS,PLUSPLUS,MINUSMINUS,LITERAL_sizeof,LITERAL_dynamic_cast,LITERAL_static_cast,LITERAL_reinterpret_cast,NOT,LITERAL_new,LITERAL_delete,DECIMALINT,HEXADECIMALINT,CharLiteral,FLOATONE,FLOATTWO]:
                pass
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
            cx=self.cast_expression()
            if not self.inputState.guessing:
                node.add_child(cx);
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_128)
            else:
                raise ex
        
        return node
    
    def primary_expression(self):    
        node = None
        
        try:      ## for error handling
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [ID,OPERATOR,SCOPE]:
                pass
                x=self.id_expression()
                if not self.inputState.guessing:
                    node = x;
            elif la1 and la1 in [StringLiteral,LITERAL_true,LITERAL_false,OCTALINT,DECIMALINT,HEXADECIMALINT,CharLiteral,FLOATONE,FLOATTWO]:
                pass
                x=self.constant()
                if not self.inputState.guessing:
                    node = x;
            elif la1 and la1 in [LITERAL_this]:
                pass
                self.match(LITERAL_this)
                if not self.inputState.guessing:
                    node = self.node("expression"), node.set_leaf("this");
            elif la1 and la1 in [LPAREN]:
                pass
                self.match(LPAREN)
                x=self.expression()
                self.match(RPAREN)
                if not self.inputState.guessing:
                    node = x;
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_55)
            else:
                raise ex
        
        return node
    
    def id_expression(self):    
        node = None
        
        id1 = None
        node = self.node("expression");
        node.set_leaf("id_expression");
        try:      ## for error handling
            pass
            s=self.scope_override()
            if not self.inputState.guessing:
                node.set_value(s);
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [ID]:
                pass
                id1 = self.LT(1)
                self.match(ID)
                if not self.inputState.guessing:
                    node.extend_value(id1.getText());
            elif la1 and la1 in [OPERATOR]:
                pass
                self.match(OPERATOR)
                o=self.optor()
                if not self.inputState.guessing:
                    node.extend_value("operator"); node.extend_value(o);
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_55)
            else:
                raise ex
        
        return node
    
    def new_type_id(self):    
        
        try:      ## for error handling
            pass
            ds=self.declaration_specifiers()
            if (_tokenSet_138.member(self.LA(1))) and (_tokenSet_139.member(self.LA(2))):
                pass
                self.new_declarator()
            elif (_tokenSet_140.member(self.LA(1))) and (_tokenSet_38.member(self.LA(2))):
                pass
            else:
                raise antlr.NoViableAltException(self.LT(1), self.getFilename())
            
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_140)
            else:
                raise ex
        
    
    def new_initializer(self):    
        
        try:      ## for error handling
            pass
            self.match(LPAREN)
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [ID,StringLiteral,LPAREN,LITERAL_const_cast,LITERAL_char,LITERAL_bool,LITERAL_short,LITERAL_int,74,75,76,LITERAL_long,LITERAL_signed,LITERAL___signed,LITERAL___signed__,LITERAL_unsigned,LITERAL_float,LITERAL_double,LITERAL_void,LITERAL__declspec,LITERAL___declspec,LITERAL___builtin_va_list,OPERATOR,LITERAL_this,LITERAL_true,LITERAL_false,OCTALINT,STAR,AMPERSAND,TILDE,SCOPE,PLUS,MINUS,PLUSPLUS,MINUSMINUS,LITERAL_sizeof,LITERAL_dynamic_cast,LITERAL_static_cast,LITERAL_reinterpret_cast,NOT,LITERAL_new,LITERAL_delete,DECIMALINT,HEXADECIMALINT,CharLiteral,FLOATONE,FLOATTWO]:
                pass
                el=self.expression_list()
            elif la1 and la1 in [RPAREN]:
                pass
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
            self.match(RPAREN)
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_128)
            else:
                raise ex
        
    
    def new_declarator(self):    
        
        try:      ## for error handling
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [ID,LITERAL__stdcall,LITERAL___stdcall,STAR,AMPERSAND,SCOPE,LITERAL__cdecl,LITERAL___cdecl,LITERAL__near,LITERAL___near,LITERAL__far,LITERAL___far,LITERAL___interrupt,LITERAL_pascal,LITERAL__pascal,LITERAL___pascal]:
                pass
                po=self.ptr_operator()
                if (_tokenSet_138.member(self.LA(1))) and (_tokenSet_139.member(self.LA(2))):
                    pass
                    self.new_declarator()
                elif (_tokenSet_140.member(self.LA(1))) and (_tokenSet_38.member(self.LA(2))):
                    pass
                else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
            elif la1 and la1 in [LSQUARE]:
                pass
                self.direct_new_declarator()
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_140)
            else:
                raise ex
        
    
    def direct_new_declarator(self):    
        
        try:      ## for error handling
            pass
            _cnt499= 0
            while True:
                if (self.LA(1)==LSQUARE):
                    pass
                    self.match(LSQUARE)
                    x=self.expression()
                    self.match(RSQUARE)
                else:
                    break
                
                _cnt499 += 1
            if _cnt499 < 1:
                raise antlr.NoViableAltException(self.LT(1), self.getFilename())
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_140)
            else:
                raise ex
        
    
    def ptr_to_member(self):    
        ptm = None
        
        s = ""
        try:      ## for error handling
            pass
            so=self.scope_override()
            if not self.inputState.guessing:
                s +=  so;
            self.match(STAR)
            if not self.inputState.guessing:
                s +=  "*";
            if not self.inputState.guessing:
                self.is_pointer = true;
            cvs=self.cv_qualifier_seq()
            if not self.inputState.guessing:
                s +=  cvs;
            if not self.inputState.guessing:
                ptm = s;
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_17)
            else:
                raise ex
        
        return ptm
    
    def optor_simple_tokclass(self):    
        out = None
        
        try:      ## for error handling
            pass
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [PLUS]:
                pass
                self.match(PLUS)
            elif la1 and la1 in [MINUS]:
                pass
                self.match(MINUS)
            elif la1 and la1 in [STAR]:
                pass
                self.match(STAR)
            elif la1 and la1 in [DIVIDE]:
                pass
                self.match(DIVIDE)
            elif la1 and la1 in [MOD]:
                pass
                self.match(MOD)
            elif la1 and la1 in [BITWISEXOR]:
                pass
                self.match(BITWISEXOR)
            elif la1 and la1 in [AMPERSAND]:
                pass
                self.match(AMPERSAND)
            elif la1 and la1 in [BITWISEOR]:
                pass
                self.match(BITWISEOR)
            elif la1 and la1 in [TILDE]:
                pass
                self.match(TILDE)
            elif la1 and la1 in [NOT]:
                pass
                self.match(NOT)
            elif la1 and la1 in [SHIFTLEFT]:
                pass
                self.match(SHIFTLEFT)
            elif la1 and la1 in [SHIFTRIGHT]:
                pass
                self.match(SHIFTRIGHT)
            elif la1 and la1 in [ASSIGNEQUAL]:
                pass
                self.match(ASSIGNEQUAL)
            elif la1 and la1 in [TIMESEQUAL]:
                pass
                self.match(TIMESEQUAL)
            elif la1 and la1 in [DIVIDEEQUAL]:
                pass
                self.match(DIVIDEEQUAL)
            elif la1 and la1 in [MODEQUAL]:
                pass
                self.match(MODEQUAL)
            elif la1 and la1 in [PLUSEQUAL]:
                pass
                self.match(PLUSEQUAL)
            elif la1 and la1 in [MINUSEQUAL]:
                pass
                self.match(MINUSEQUAL)
            elif la1 and la1 in [SHIFTLEFTEQUAL]:
                pass
                self.match(SHIFTLEFTEQUAL)
            elif la1 and la1 in [SHIFTRIGHTEQUAL]:
                pass
                self.match(SHIFTRIGHTEQUAL)
            elif la1 and la1 in [BITWISEANDEQUAL]:
                pass
                self.match(BITWISEANDEQUAL)
            elif la1 and la1 in [BITWISEXOREQUAL]:
                pass
                self.match(BITWISEXOREQUAL)
            elif la1 and la1 in [BITWISEOREQUAL]:
                pass
                self.match(BITWISEOREQUAL)
            elif la1 and la1 in [EQUAL]:
                pass
                self.match(EQUAL)
            elif la1 and la1 in [NOTEQUAL]:
                pass
                self.match(NOTEQUAL)
            elif la1 and la1 in [LESSTHAN]:
                pass
                self.match(LESSTHAN)
            elif la1 and la1 in [GREATERTHAN]:
                pass
                self.match(GREATERTHAN)
            elif la1 and la1 in [LESSTHANOREQUALTO]:
                pass
                self.match(LESSTHANOREQUALTO)
            elif la1 and la1 in [GREATERTHANOREQUALTO]:
                pass
                self.match(GREATERTHANOREQUALTO)
            elif la1 and la1 in [OR]:
                pass
                self.match(OR)
            elif la1 and la1 in [AND]:
                pass
                self.match(AND)
            elif la1 and la1 in [PLUSPLUS]:
                pass
                self.match(PLUSPLUS)
            elif la1 and la1 in [MINUSMINUS]:
                pass
                self.match(MINUSMINUS)
            elif la1 and la1 in [COMMA]:
                pass
                self.match(COMMA)
            elif la1 and la1 in [POINTERTO]:
                pass
                self.match(POINTERTO)
            elif la1 and la1 in [POINTERTOMBR]:
                pass
                self.match(POINTERTOMBR)
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
            if not self.inputState.guessing:
                out = LT(1).getText();
        
        except antlr.RecognitionException, ex:
            if not self.inputState.guessing:
                self.reportError(ex)
                self.consume()
                self.consumeUntil(_tokenSet_63)
            else:
                raise ex
        
        return out
    

_tokenNames = [
    "<0>", 
    "EOF", 
    "<2>", 
    "NULL_TREE_LOOKAHEAD", 
    "\"template\"", 
    "LESSTHAN", 
    "GREATERTHAN", 
    "\"typedef\"", 
    "\"enum\"", 
    "ID", 
    "LCURLY", 
    "SEMICOLON", 
    "\"inline\"", 
    "\"namespace\"", 
    "RCURLY", 
    "ASSIGNEQUAL", 
    "COLON", 
    "\"extern\"", 
    "StringLiteral", 
    "COMMA", 
    "\"__extension__\"", 
    "\"_inline\"", 
    "\"__inline\"", 
    "\"__inline__\"", 
    "\"virtual\"", 
    "\"explicit\"", 
    "\"friend\"", 
    "\"_stdcall\"", 
    "\"__stdcall\"", 
    "\"typename\"", 
    "\"__attribute__\"", 
    "LPAREN", 
    "RPAREN", 
    "\"__mode__\"", 
    "\"packed\"", 
    "\"const\"", 
    "\"__const\"", 
    "\"__noreturn__\"", 
    "\"__returns_twice__\"", 
    "\"__noinline__\"", 
    "\"__always_inline__\"", 
    "\"__flatten__\"", 
    "\"__pure__\"", 
    "\"__const__\"", 
    "\"__nothrow__\"", 
    "\"__sentinel__\"", 
    "\"__format__\"", 
    "\"__format_arg__\"", 
    "\"__no_instrument_function__\"", 
    "\"__section__\"", 
    "\"__constructor__\"", 
    "\"__destructor__\"", 
    "\"__used__\"", 
    "\"__unused__\"", 
    "\"__deprecated__\"", 
    "\"__weak__\"", 
    "\"__malloc__\"", 
    "\"__alias__\"", 
    "\"__warn_unused_result__\"", 
    "\"__nonnull__\"", 
    "\"__externally_visible__\"", 
    "\"auto\"", 
    "\"register\"", 
    "\"static\"", 
    "\"mutable\"", 
    "\"const_cast\"", 
    "\"volatile\"", 
    "\"__volatile\"", 
    "\"__restrict\"", 
    "\"__restrict__\"", 
    "\"char\"", 
    "\"bool\"", 
    "\"short\"", 
    "\"int\"", 
    "\"_int64\"", 
    "\"__int64\"", 
    "\"__w64\"", 
    "\"long\"", 
    "\"signed\"", 
    "\"__signed\"", 
    "\"__signed__\"", 
    "\"unsigned\"", 
    "\"float\"", 
    "\"double\"", 
    "\"void\"", 
    "\"_declspec\"", 
    "\"__declspec\"", 
    "\"__builtin_va_list\"", 
    "\"class\"", 
    "\"struct\"", 
    "\"union\"", 
    "\"operator\"", 
    "\"this\"", 
    "\"true\"", 
    "\"false\"", 
    "DOT", 
    "EQUALS", 
    "\"public\"", 
    "\"protected\"", 
    "\"private\"", 
    "OCTALINT", 
    "STAR", 
    "AMPERSAND", 
    "LSQUARE", 
    "RSQUARE", 
    "TILDE", 
    "\"asm\"", 
    "\"__asm\"", 
    "\"__asm__\"", 
    "ELLIPSIS", 
    "\"throw\"", 
    "\"case\"", 
    "\"default\"", 
    "\"if\"", 
    "\"else\"", 
    "\"switch\"", 
    "\"while\"", 
    "\"do\"", 
    "\"for\"", 
    "\"goto\"", 
    "\"continue\"", 
    "\"break\"", 
    "\"return\"", 
    "\"try\"", 
    "\"catch\"", 
    "\"using\"", 
    "\"_asm\"", 
    "\"__volatile__\"", 
    "SCOPE", 
    "TIMESEQUAL", 
    "DIVIDEEQUAL", 
    "MINUSEQUAL", 
    "PLUSEQUAL", 
    "MODEQUAL", 
    "SHIFTLEFTEQUAL", 
    "SHIFTRIGHTEQUAL", 
    "BITWISEANDEQUAL", 
    "BITWISEXOREQUAL", 
    "BITWISEOREQUAL", 
    "QUESTIONMARK", 
    "OR", 
    "AND", 
    "BITWISEOR", 
    "BITWISEXOR", 
    "NOTEQUAL", 
    "EQUAL", 
    "LESSTHANOREQUALTO", 
    "GREATERTHANOREQUALTO", 
    "SHIFTLEFT", 
    "SHIFTRIGHT", 
    "PLUS", 
    "MINUS", 
    "DIVIDE", 
    "MOD", 
    "DOTMBR", 
    "POINTERTOMBR", 
    "PLUSPLUS", 
    "MINUSMINUS", 
    "\"sizeof\"", 
    "POINTERTO", 
    "\"dynamic_cast\"", 
    "\"static_cast\"", 
    "\"reinterpret_cast\"", 
    "NOT", 
    "\"new\"", 
    "\"_cdecl\"", 
    "\"__cdecl\"", 
    "\"_near\"", 
    "\"__near\"", 
    "\"_far\"", 
    "\"__far\"", 
    "\"__interrupt\"", 
    "\"pascal\"", 
    "\"_pascal\"", 
    "\"__pascal\"", 
    "\"delete\"", 
    "DECIMALINT", 
    "HEXADECIMALINT", 
    "CharLiteral", 
    "FLOATONE", 
    "FLOATTWO", 
    "Whitespace", 
    "Comment", 
    "CPPComment", 
    "a line directive", 
    "ALL_TO_NL", 
    "LineDirective", 
    "Space", 
    "Pragma", 
    "Error", 
    "EndOfLine", 
    "Escape", 
    "Digit", 
    "Decimal", 
    "LongSuffix", 
    "UnsignedSuffix", 
    "FloatSuffix", 
    "Exponent", 
    "Vocabulary", 
    "Number"
]
    

### generate bit set
def mk_tokenSet_0(): 
    ### var1
    data = [ -2305842901840413808L, 2305845622701293567L, 140600049401857L, 0L, 0L, 0L]
    return data
_tokenSet_0 = antlr.BitSet(mk_tokenSet_0())

### generate bit set
def mk_tokenSet_1(): 
    ### var1
    data = [ 2L, 0L, 0L, 0L]
    return data
_tokenSet_1 = antlr.BitSet(mk_tokenSet_1())

### generate bit set
def mk_tokenSet_2(): 
    ### var1
    data = [ -2305842905061649536L, 2305843009347911679L, 1L, 0L, 0L, 0L]
    return data
_tokenSet_2 = antlr.BitSet(mk_tokenSet_2())

### generate bit set
def mk_tokenSet_3(): 
    ### var1
    data = [ -2305842902376497248L, 2613487599615L, 140600049401857L, 0L, 0L, 0L]
    return data
_tokenSet_3 = antlr.BitSet(mk_tokenSet_3())

### generate bit set
def mk_tokenSet_4(): 
    ### var1
    data = [ 23073296L, 2199023255552L, 1L, 0L, 0L, 0L]
    return data
_tokenSet_4 = antlr.BitSet(mk_tokenSet_4())

### generate bit set
def mk_tokenSet_5(): 
    ### var1
    data = [ 23073312L, 2199023255552L, 1L, 0L, 0L, 0L]
    return data
_tokenSet_5 = antlr.BitSet(mk_tokenSet_5())

### generate bit set
def mk_tokenSet_6(): 
    ### var1
    data = [ 39850496L, 0L, 1L, 0L, 0L, 0L]
    return data
_tokenSet_6 = antlr.BitSet(mk_tokenSet_6())

### generate bit set
def mk_tokenSet_7(): 
    ### var1
    data = [ 2187334176L, 0L, 1L, 0L, 0L, 0L]
    return data
_tokenSet_7 = antlr.BitSet(mk_tokenSet_7())

### generate bit set
def mk_tokenSet_8(): 
    ### var1
    data = [ 4608L, 134217728L, 1L, 0L, 0L, 0L]
    return data
_tokenSet_8 = antlr.BitSet(mk_tokenSet_8())

### generate bit set
def mk_tokenSet_9(): 
    ### var1
    data = [ -2305842905061649504L, 268435455L, 1L, 0L, 0L, 0L]
    return data
_tokenSet_9 = antlr.BitSet(mk_tokenSet_9())

### generate bit set
def mk_tokenSet_10(): 
    ### var1
    data = [ -2305842901840424064L, 414464344063L, 140600049401857L, 0L, 0L, 0L]
    return data
_tokenSet_10 = antlr.BitSet(mk_tokenSet_10())

### generate bit set
def mk_tokenSet_11(): 
    ### var1
    data = [ -2305842902376736800L, 3163243413503L, 281443771086847L, 0L, 0L, 0L]
    return data
_tokenSet_11 = antlr.BitSet(mk_tokenSet_11())

### generate bit set
def mk_tokenSet_12(): 
    ### var1
    data = [ -2305842901840397422L, 2305845622701293567L, 140600049401857L, 0L, 0L, 0L]
    return data
_tokenSet_12 = antlr.BitSet(mk_tokenSet_12())

### generate bit set
def mk_tokenSet_13(): 
    ### var1
    data = [ -2305842905061649536L, 134217727L, 1L, 0L, 0L, 0L]
    return data
_tokenSet_13 = antlr.BitSet(mk_tokenSet_13())

### generate bit set
def mk_tokenSet_14(): 
    ### var1
    data = [ -2305842902376767584L, 2613487599615L, 140600049401857L, 0L, 0L, 0L]
    return data
_tokenSet_14 = antlr.BitSet(mk_tokenSet_14())

### generate bit set
def mk_tokenSet_15(): 
    ### var1
    data = [ -2305842901840134254L, 8070413692165947391L, 9007196851404801L, 0L, 0L, 0L]
    return data
_tokenSet_15 = antlr.BitSet(mk_tokenSet_15())

### generate bit set
def mk_tokenSet_16(): 
    ### var1
    data = [ -2305842901840424064L, 2305845622701293567L, 140600049401857L, 0L, 0L, 0L]
    return data
_tokenSet_16 = antlr.BitSet(mk_tokenSet_16())

### generate bit set
def mk_tokenSet_17(): 
    ### var1
    data = [ 7919487584L, 39446992912384L, 140600317837311L, 0L, 0L, 0L]
    return data
_tokenSet_17 = antlr.BitSet(mk_tokenSet_17())

### generate bit set
def mk_tokenSet_18(): 
    ### var1
    data = [ 2048L, 0L, 0L, 0L]
    return data
_tokenSet_18 = antlr.BitSet(mk_tokenSet_18())

### generate bit set
def mk_tokenSet_19(): 
    ### var1
    data = [ 3072L, 0L, 0L, 0L]
    return data
_tokenSet_19 = antlr.BitSet(mk_tokenSet_19())

### generate bit set
def mk_tokenSet_20(): 
    ### var1
    data = [ -2305842901840397422L, 2305845682830835711L, 140600049401857L, 0L, 0L, 0L]
    return data
_tokenSet_20 = antlr.BitSet(mk_tokenSet_20())

### generate bit set
def mk_tokenSet_21(): 
    ### var1
    data = [ 512L, 0L, 1L, 0L, 0L, 0L]
    return data
_tokenSet_21 = antlr.BitSet(mk_tokenSet_21())

### generate bit set
def mk_tokenSet_22(): 
    ### var1
    data = [ 512L, 2338475474944L, 0L, 0L]
    return data
_tokenSet_22 = antlr.BitSet(mk_tokenSet_22())

### generate bit set
def mk_tokenSet_23(): 
    ### var1
    data = [ -2305842905598521216L, 63L, 0L, 0L]
    return data
_tokenSet_23 = antlr.BitSet(mk_tokenSet_23())

### generate bit set
def mk_tokenSet_24(): 
    ### var1
    data = [ 402653696L, 412316860416L, 140600049401857L, 0L, 0L, 0L]
    return data
_tokenSet_24 = antlr.BitSet(mk_tokenSet_24())

### generate bit set
def mk_tokenSet_25(): 
    ### var1
    data = [ 106703094304L, 414330126398L, 140600049401857L, 0L, 0L, 0L]
    return data
_tokenSet_25 = antlr.BitSet(mk_tokenSet_25())

### generate bit set
def mk_tokenSet_26(): 
    ### var1
    data = [ 3221225984L, 2013265920L, 1L, 0L, 0L, 0L]
    return data
_tokenSet_26 = antlr.BitSet(mk_tokenSet_26())

### generate bit set
def mk_tokenSet_27(): 
    ### var1
    data = [ 2148041312L, 3163109195776L, 140843721684991L, 0L, 0L, 0L]
    return data
_tokenSet_27 = antlr.BitSet(mk_tokenSet_27())

### generate bit set
def mk_tokenSet_28(): 
    ### var1
    data = [ -2305842905061646464L, 2305843009347911679L, 1L, 0L, 0L, 0L]
    return data
_tokenSet_28 = antlr.BitSet(mk_tokenSet_28())

### generate bit set
def mk_tokenSet_29(): 
    ### var1
    data = [ -2305842902377293920L, 2613487599615L, 140600049401857L, 0L, 0L, 0L]
    return data
_tokenSet_29 = antlr.BitSet(mk_tokenSet_29())

### generate bit set
def mk_tokenSet_30(): 
    ### var1
    data = [ -2305842905061648512L, 2305843009347911679L, 1L, 0L, 0L, 0L]
    return data
_tokenSet_30 = antlr.BitSet(mk_tokenSet_30())

### generate bit set
def mk_tokenSet_31(): 
    ### var1
    data = [ -2305842901839609952L, 8069287732129562623L, 9007196851404801L, 0L, 0L, 0L]
    return data
_tokenSet_31 = antlr.BitSet(mk_tokenSet_31())

### generate bit set
def mk_tokenSet_32(): 
    ### var1
    data = [ -2305842902913884288L, 8069287732129562623L, 8866596802002945L, 0L, 0L, 0L]
    return data
_tokenSet_32 = antlr.BitSet(mk_tokenSet_32())

### generate bit set
def mk_tokenSet_33(): 
    ### var1
    data = [ 3623879168L, 414330126336L, 140600049401857L, 0L, 0L, 0L]
    return data
_tokenSet_33 = antlr.BitSet(mk_tokenSet_33())

### generate bit set
def mk_tokenSet_34(): 
    ### var1
    data = [ 106703651424L, 3163109195838L, 281443771086847L, 0L, 0L, 0L]
    return data
_tokenSet_34 = antlr.BitSet(mk_tokenSet_34())

### generate bit set
def mk_tokenSet_35(): 
    ### var1
    data = [ -2305842901840396400L, 2305845622701293567L, 140600049401857L, 0L, 0L, 0L]
    return data
_tokenSet_35 = antlr.BitSet(mk_tokenSet_35())

### generate bit set
def mk_tokenSet_36(): 
    ### var1
    data = [ -2305842902913903744L, 2682207076351L, 8866596802002945L, 0L, 0L, 0L]
    return data
_tokenSet_36 = antlr.BitSet(mk_tokenSet_36())

### generate bit set
def mk_tokenSet_37(): 
    ### var1
    data = [ 7919488608L, 70233318490112L, 140600317837311L, 0L, 0L, 0L]
    return data
_tokenSet_37 = antlr.BitSet(mk_tokenSet_37())

### generate bit set
def mk_tokenSet_38(): 
    ### var1
    data = [ -2305842897544544270L, 8070450527952961535L, 9007199254740991L, 0L, 0L, 0L]
    return data
_tokenSet_38 = antlr.BitSet(mk_tokenSet_38())

### generate bit set
def mk_tokenSet_39(): 
    ### var1
    data = [ 23073280L, 2199023255552L, 1L, 0L, 0L, 0L]
    return data
_tokenSet_39 = antlr.BitSet(mk_tokenSet_39())

### generate bit set
def mk_tokenSet_40(): 
    ### var1
    data = [ -2305842905061649536L, 268435455L, 1L, 0L, 0L, 0L]
    return data
_tokenSet_40 = antlr.BitSet(mk_tokenSet_40())

### generate bit set
def mk_tokenSet_41(): 
    ### var1
    data = [ 512L, 2013265920L, 1L, 0L, 0L, 0L]
    return data
_tokenSet_41 = antlr.BitSet(mk_tokenSet_41())

### generate bit set
def mk_tokenSet_42(): 
    ### var1
    data = [ 2148043360L, 3163109195776L, 140843721684991L, 0L, 0L, 0L]
    return data
_tokenSet_42 = antlr.BitSet(mk_tokenSet_42())

### generate bit set
def mk_tokenSet_43(): 
    ### var1
    data = [ -2305842903450968160L, 2613487599615L, 140600049401857L, 0L, 0L, 0L]
    return data
_tokenSet_43 = antlr.BitSet(mk_tokenSet_43())

### generate bit set
def mk_tokenSet_44(): 
    ### var1
    data = [ -2305842901840405616L, 2305845682830835711L, 140600049401857L, 0L, 0L, 0L]
    return data
_tokenSet_44 = antlr.BitSet(mk_tokenSet_44())

### generate bit set
def mk_tokenSet_45(): 
    ### var1
    data = [ 68608L, 0L, 0L, 0L]
    return data
_tokenSet_45 = antlr.BitSet(mk_tokenSet_45())

### generate bit set
def mk_tokenSet_46(): 
    ### var1
    data = [ -2305842901840134254L, 9223335196772794367L, 9007196851404801L, 0L, 0L, 0L]
    return data
_tokenSet_46 = antlr.BitSet(mk_tokenSet_46())

### generate bit set
def mk_tokenSet_47(): 
    ### var1
    data = [ 16843264L, 0L, 1L, 0L, 0L, 0L]
    return data
_tokenSet_47 = antlr.BitSet(mk_tokenSet_47())

### generate bit set
def mk_tokenSet_48(): 
    ### var1
    data = [ -2305842905598520448L, 134217727L, 1L, 0L, 0L, 0L]
    return data
_tokenSet_48 = antlr.BitSet(mk_tokenSet_48())

### generate bit set
def mk_tokenSet_49(): 
    ### var1
    data = [ -2305842897544814624L, 2305983611410579455L, 140600317837311L, 0L, 0L, 0L]
    return data
_tokenSet_49 = antlr.BitSet(mk_tokenSet_49())

### generate bit set
def mk_tokenSet_50(): 
    ### var1
    data = [ 103079215104L, 62L, 0L, 0L]
    return data
_tokenSet_50 = antlr.BitSet(mk_tokenSet_50())

### generate bit set
def mk_tokenSet_51(): 
    ### var1
    data = [ 2147746304L, 3781601263554L, 8866596802002945L, 0L, 0L, 0L]
    return data
_tokenSet_51 = antlr.BitSet(mk_tokenSet_51())

### generate bit set
def mk_tokenSet_52(): 
    ### var1
    data = [ 7919487584L, 70233318490112L, 281443838197759L, 0L, 0L, 0L]
    return data
_tokenSet_52 = antlr.BitSet(mk_tokenSet_52())

### generate bit set
def mk_tokenSet_53(): 
    ### var1
    data = [ 7919487584L, 70233318490112L, 140600317837311L, 0L, 0L, 0L]
    return data
_tokenSet_53 = antlr.BitSet(mk_tokenSet_53())

### generate bit set
def mk_tokenSet_54(): 
    ### var1
    data = [ 4294967296L, 0L, 0L, 0L]
    return data
_tokenSet_54 = antlr.BitSet(mk_tokenSet_54())

### generate bit set
def mk_tokenSet_55(): 
    ### var1
    data = [ 7516833888L, 37248103874560L, 3221225470L, 0L, 0L, 0L]
    return data
_tokenSet_55 = antlr.BitSet(mk_tokenSet_55())

### generate bit set
def mk_tokenSet_56(): 
    ### var1
    data = [ -2305842901840422000L, 2305845682830835711L, 140600049401857L, 0L, 0L, 0L]
    return data
_tokenSet_56 = antlr.BitSet(mk_tokenSet_56())

### generate bit set
def mk_tokenSet_57(): 
    ### var1
    data = [ 7919488608L, 39446992912384L, 140600317837311L, 0L, 0L, 0L]
    return data
_tokenSet_57 = antlr.BitSet(mk_tokenSet_57())

### generate bit set
def mk_tokenSet_58(): 
    ### var1
    data = [ 64L, 0L, 0L, 0L]
    return data
_tokenSet_58 = antlr.BitSet(mk_tokenSet_58())

### generate bit set
def mk_tokenSet_59(): 
    ### var1
    data = [ 1024L, 0L, 0L, 0L]
    return data
_tokenSet_59 = antlr.BitSet(mk_tokenSet_59())

### generate bit set
def mk_tokenSet_60(): 
    ### var1
    data = [ 16384L, 0L, 0L, 0L]
    return data
_tokenSet_60 = antlr.BitSet(mk_tokenSet_60())

### generate bit set
def mk_tokenSet_61(): 
    ### var1
    data = [ 540672L, 0L, 0L, 0L]
    return data
_tokenSet_61 = antlr.BitSet(mk_tokenSet_61())

### generate bit set
def mk_tokenSet_62(): 
    ### var1
    data = [ 641024L, 1099511627776L, 0L, 0L]
    return data
_tokenSet_62 = antlr.BitSet(mk_tokenSet_62())

### generate bit set
def mk_tokenSet_63(): 
    ### var1
    data = [ 7919488608L, 70235465973760L, 140603270627327L, 0L, 0L, 0L]
    return data
_tokenSet_63 = antlr.BitSet(mk_tokenSet_63())

### generate bit set
def mk_tokenSet_64(): 
    ### var1
    data = [ 1074268160L, 0L, 0L, 0L]
    return data
_tokenSet_64 = antlr.BitSet(mk_tokenSet_64())

### generate bit set
def mk_tokenSet_65(): 
    ### var1
    data = [ 105629352480L, 2613353381950L, 140600049401857L, 0L, 0L, 0L]
    return data
_tokenSet_65 = antlr.BitSet(mk_tokenSet_65())

### generate bit set
def mk_tokenSet_66(): 
    ### var1
    data = [ 2147484160L, 2201036521472L, 1L, 0L, 0L, 0L]
    return data
_tokenSet_66 = antlr.BitSet(mk_tokenSet_66())

### generate bit set
def mk_tokenSet_67(): 
    ### var1
    data = [ 7919405664L, 69133806862336L, 281443771086847L, 0L, 0L, 0L]
    return data
_tokenSet_67 = antlr.BitSet(mk_tokenSet_67())

### generate bit set
def mk_tokenSet_68(): 
    ### var1
    data = [ 7516751936L, 65970697666560L, 0L, 0L]
    return data
_tokenSet_68 = antlr.BitSet(mk_tokenSet_68())

### generate bit set
def mk_tokenSet_69(): 
    ### var1
    data = [ -2305842901839863936L, 2305913378092089343L, 1L, 0L, 0L, 0L]
    return data
_tokenSet_69 = antlr.BitSet(mk_tokenSet_69())

### generate bit set
def mk_tokenSet_70(): 
    ### var1
    data = [ 2147746304L, 2682089635778L, 8866596802002945L, 0L, 0L, 0L]
    return data
_tokenSet_70 = antlr.BitSet(mk_tokenSet_70())

### generate bit set
def mk_tokenSet_71(): 
    ### var1
    data = [ -2305842901839586336L, 3234110373887L, 8866599205339135L, 0L, 0L, 0L]
    return data
_tokenSet_71 = antlr.BitSet(mk_tokenSet_71())

### generate bit set
def mk_tokenSet_72(): 
    ### var1
    data = [ 1074284544L, 0L, 0L, 0L]
    return data
_tokenSet_72 = antlr.BitSet(mk_tokenSet_72())

### generate bit set
def mk_tokenSet_73(): 
    ### var1
    data = [ -2305842897544553504L, 39517994090495L, 8866599205339135L, 0L, 0L, 0L]
    return data
_tokenSet_73 = antlr.BitSet(mk_tokenSet_73())

### generate bit set
def mk_tokenSet_74(): 
    ### var1
    data = [ 5369317440L, 36283883716608L, 0L, 0L]
    return data
_tokenSet_74 = antlr.BitSet(mk_tokenSet_74())

### generate bit set
def mk_tokenSet_75(): 
    ### var1
    data = [ 2147747328L, 2684237119426L, 8866596802002945L, 0L, 0L, 0L]
    return data
_tokenSet_75 = antlr.BitSet(mk_tokenSet_75())

### generate bit set
def mk_tokenSet_76(): 
    ### var1
    data = [ 525312L, 0L, 0L, 0L]
    return data
_tokenSet_76 = antlr.BitSet(mk_tokenSet_76())

### generate bit set
def mk_tokenSet_77(): 
    ### var1
    data = [ 2147811840L, 2682089635778L, 8866596802002945L, 0L, 0L, 0L]
    return data
_tokenSet_77 = antlr.BitSet(mk_tokenSet_77())

### generate bit set
def mk_tokenSet_78(): 
    ### var1
    data = [ 2550137344L, 2613353381888L, 140600049401857L, 0L, 0L, 0L]
    return data
_tokenSet_78 = antlr.BitSet(mk_tokenSet_78())

### generate bit set
def mk_tokenSet_79(): 
    ### var1
    data = [ 106703653472L, 3163109195838L, 281443771086847L, 0L, 0L, 0L]
    return data
_tokenSet_79 = antlr.BitSet(mk_tokenSet_79())

### generate bit set
def mk_tokenSet_80(): 
    ### var1
    data = [ 526336L, 0L, 0L, 0L]
    return data
_tokenSet_80 = antlr.BitSet(mk_tokenSet_80())

### generate bit set
def mk_tokenSet_81(): 
    ### var1
    data = [ -2305842897544814624L, 2305913242666401791L, 140600317837311L, 0L, 0L, 0L]
    return data
_tokenSet_81 = antlr.BitSet(mk_tokenSet_81())

### generate bit set
def mk_tokenSet_82(): 
    ### var1
    data = [ 7919487584L, 109815737090048L, 140600317837311L, 0L, 0L, 0L]
    return data
_tokenSet_82 = antlr.BitSet(mk_tokenSet_82())

### generate bit set
def mk_tokenSet_83(): 
    ### var1
    data = [ -2305842905061613696L, 2305913378092089343L, 1L, 0L, 0L, 0L]
    return data
_tokenSet_83 = antlr.BitSet(mk_tokenSet_83())

### generate bit set
def mk_tokenSet_84(): 
    ### var1
    data = [ 66560L, 0L, 0L, 0L]
    return data
_tokenSet_84 = antlr.BitSet(mk_tokenSet_84())

### generate bit set
def mk_tokenSet_85(): 
    ### var1
    data = [ 2147483648L, 0L, 0L, 0L]
    return data
_tokenSet_85 = antlr.BitSet(mk_tokenSet_85())

### generate bit set
def mk_tokenSet_86(): 
    ### var1
    data = [ 512L, 2199023255552L, 1L, 0L, 0L, 0L]
    return data
_tokenSet_86 = antlr.BitSet(mk_tokenSet_86())

### generate bit set
def mk_tokenSet_87(): 
    ### var1
    data = [ 4294967296L, 35184372088832L, 0L, 0L]
    return data
_tokenSet_87 = antlr.BitSet(mk_tokenSet_87())

### generate bit set
def mk_tokenSet_88(): 
    ### var1
    data = [ -2305842899155511328L, 38347615502335L, 140600049401857L, 0L, 0L, 0L]
    return data
_tokenSet_88 = antlr.BitSet(mk_tokenSet_88())

### generate bit set
def mk_tokenSet_89(): 
    ### var1
    data = [ 109924876896L, 38347481284670L, 281443771086847L, 0L, 0L, 0L]
    return data
_tokenSet_89 = antlr.BitSet(mk_tokenSet_89())

### generate bit set
def mk_tokenSet_90(): 
    ### var1
    data = [ 6845661760L, 36146444763136L, 140600049401857L, 0L, 0L, 0L]
    return data
_tokenSet_90 = antlr.BitSet(mk_tokenSet_90())

### generate bit set
def mk_tokenSet_91(): 
    ### var1
    data = [ -2305842897544552480L, 2305983680130056191L, 9007197107257343L, 0L, 0L, 0L]
    return data
_tokenSet_91 = antlr.BitSet(mk_tokenSet_91())

### generate bit set
def mk_tokenSet_92(): 
    ### var1
    data = [ 4295491648L, 35184372088832L, 0L, 0L]
    return data
_tokenSet_92 = antlr.BitSet(mk_tokenSet_92())

### generate bit set
def mk_tokenSet_93(): 
    ### var1
    data = [ 4295524416L, 35184372088832L, 0L, 0L]
    return data
_tokenSet_93 = antlr.BitSet(mk_tokenSet_93())

### generate bit set
def mk_tokenSet_94(): 
    ### var1
    data = [ 4295491648L, 0L, 0L, 0L]
    return data
_tokenSet_94 = antlr.BitSet(mk_tokenSet_94())

### generate bit set
def mk_tokenSet_95(): 
    ### var1
    data = [ 6443008064L, 35734127902720L, 0L, 0L]
    return data
_tokenSet_95 = antlr.BitSet(mk_tokenSet_95())

### generate bit set
def mk_tokenSet_96(): 
    ### var1
    data = [ 524864L, 0L, 0L, 0L]
    return data
_tokenSet_96 = antlr.BitSet(mk_tokenSet_96())

### generate bit set
def mk_tokenSet_97(): 
    ### var1
    data = [ -2305842902914165888L, 37797859688447L, 140600049401857L, 0L, 0L, 0L]
    return data
_tokenSet_97 = antlr.BitSet(mk_tokenSet_97())

### generate bit set
def mk_tokenSet_98(): 
    ### var1
    data = [ -2305842903450478624L, 3163243413503L, 281443771086847L, 0L, 0L, 0L]
    return data
_tokenSet_98 = antlr.BitSet(mk_tokenSet_98())

### generate bit set
def mk_tokenSet_99(): 
    ### var1
    data = [ 524352L, 0L, 0L, 0L]
    return data
_tokenSet_99 = antlr.BitSet(mk_tokenSet_99())

### generate bit set
def mk_tokenSet_100(): 
    ### var1
    data = [ 2550661728L, 962072674304L, 140600049401857L, 0L, 0L, 0L]
    return data
_tokenSet_100 = antlr.BitSet(mk_tokenSet_100())

### generate bit set
def mk_tokenSet_101(): 
    ### var1
    data = [ 512L, 16777152L, 1L, 0L, 0L, 0L]
    return data
_tokenSet_101 = antlr.BitSet(mk_tokenSet_101())

### generate bit set
def mk_tokenSet_102(): 
    ### var1
    data = [ 2550661728L, 962089451456L, 140600049401857L, 0L, 0L, 0L]
    return data
_tokenSet_102 = antlr.BitSet(mk_tokenSet_102())

### generate bit set
def mk_tokenSet_103(): 
    ### var1
    data = [ -2305842903450511392L, 3163243413503L, 140600049401857L, 0L, 0L, 0L]
    return data
_tokenSet_103 = antlr.BitSet(mk_tokenSet_103())

### generate bit set
def mk_tokenSet_104(): 
    ### var1
    data = [ -2305842902913346592L, 3234110373887L, 8866599205337087L, 0L, 0L, 0L]
    return data
_tokenSet_104 = antlr.BitSet(mk_tokenSet_104())

### generate bit set
def mk_tokenSet_105(): 
    ### var1
    data = [ 5369350240L, 36558761623552L, 1048574L, 0L, 0L, 0L]
    return data
_tokenSet_105 = antlr.BitSet(mk_tokenSet_105())

### generate bit set
def mk_tokenSet_106(): 
    ### var1
    data = [ -2305842902913900672L, 8069287732129562623L, 8866596802002945L, 0L, 0L, 0L]
    return data
_tokenSet_106 = antlr.BitSet(mk_tokenSet_106())

### generate bit set
def mk_tokenSet_107(): 
    ### var1
    data = [ -2305842902913344544L, 3234110373887L, 8866599205339135L, 0L, 0L, 0L]
    return data
_tokenSet_107 = antlr.BitSet(mk_tokenSet_107())

### generate bit set
def mk_tokenSet_108(): 
    ### var1
    data = [ -2305842902913884288L, 8070413632036405247L, 8866596802002945L, 0L, 0L, 0L]
    return data
_tokenSet_108 = antlr.BitSet(mk_tokenSet_108())

### generate bit set
def mk_tokenSet_109(): 
    ### var1
    data = [ 4295034880L, 1099511627776L, 0L, 0L]
    return data
_tokenSet_109 = antlr.BitSet(mk_tokenSet_109())

### generate bit set
def mk_tokenSet_110(): 
    ### var1
    data = [ -2305842901839511566L, -36288178683905L, 9007199254740991L, 0L, 0L, 0L]
    return data
_tokenSet_110 = antlr.BitSet(mk_tokenSet_110())

### generate bit set
def mk_tokenSet_111(): 
    ### var1
    data = [ -2305842902913884288L, 9223335136643252223L, 8866596802002945L, 0L, 0L, 0L]
    return data
_tokenSet_111 = antlr.BitSet(mk_tokenSet_111())

### generate bit set
def mk_tokenSet_112(): 
    data = [0L] * 8 ### init list
    data[0] =-16400L
    for x in xrange(1, 3):
        data[x] = -1L
    data[3] =255L
    return data
_tokenSet_112 = antlr.BitSet(mk_tokenSet_112())

### generate bit set
def mk_tokenSet_113(): 
    ### var1
    data = [ 4295032832L, 0L, 1L, 0L, 0L, 0L]
    return data
_tokenSet_113 = antlr.BitSet(mk_tokenSet_113())

### generate bit set
def mk_tokenSet_114(): 
    ### var1
    data = [ 5369350208L, 36283883716608L, 2046L, 0L, 0L, 0L]
    return data
_tokenSet_114 = antlr.BitSet(mk_tokenSet_114())

### generate bit set
def mk_tokenSet_115(): 
    ### var1
    data = [ 5369350208L, 36283883716608L, 4094L, 0L, 0L, 0L]
    return data
_tokenSet_115 = antlr.BitSet(mk_tokenSet_115())

### generate bit set
def mk_tokenSet_116(): 
    ### var1
    data = [ 5369350208L, 36283883716608L, 8190L, 0L, 0L, 0L]
    return data
_tokenSet_116 = antlr.BitSet(mk_tokenSet_116())

### generate bit set
def mk_tokenSet_117(): 
    ### var1
    data = [ 5369350208L, 36283883716608L, 16382L, 0L, 0L, 0L]
    return data
_tokenSet_117 = antlr.BitSet(mk_tokenSet_117())

### generate bit set
def mk_tokenSet_118(): 
    ### var1
    data = [ 5369350208L, 36283883716608L, 32766L, 0L, 0L, 0L]
    return data
_tokenSet_118 = antlr.BitSet(mk_tokenSet_118())

### generate bit set
def mk_tokenSet_119(): 
    ### var1
    data = [ 5369350208L, 36283883716608L, 65534L, 0L, 0L, 0L]
    return data
_tokenSet_119 = antlr.BitSet(mk_tokenSet_119())

### generate bit set
def mk_tokenSet_120(): 
    ### var1
    data = [ 5369350208L, 36558761623552L, 65534L, 0L, 0L, 0L]
    return data
_tokenSet_120 = antlr.BitSet(mk_tokenSet_120())

### generate bit set
def mk_tokenSet_121(): 
    ### var1
    data = [ 96L, 0L, 786432L, 0L, 0L, 0L]
    return data
_tokenSet_121 = antlr.BitSet(mk_tokenSet_121())

### generate bit set
def mk_tokenSet_122(): 
    ### var1
    data = [ 5369350208L, 36558761623552L, 262142L, 0L, 0L, 0L]
    return data
_tokenSet_122 = antlr.BitSet(mk_tokenSet_122())

### generate bit set
def mk_tokenSet_123(): 
    ### var1
    data = [ 5369350240L, 36558761623552L, 4194302L, 0L, 0L, 0L]
    return data
_tokenSet_123 = antlr.BitSet(mk_tokenSet_123())

### generate bit set
def mk_tokenSet_124(): 
    ### var1
    data = [ 0L, 137438953472L, 50331648L, 0L, 0L, 0L]
    return data
_tokenSet_124 = antlr.BitSet(mk_tokenSet_124())

### generate bit set
def mk_tokenSet_125(): 
    ### var1
    data = [ 5369350240L, 36558761623552L, 16777214L, 0L, 0L, 0L]
    return data
_tokenSet_125 = antlr.BitSet(mk_tokenSet_125())

### generate bit set
def mk_tokenSet_126(): 
    ### var1
    data = [ 5369350240L, 36696200577024L, 67108862L, 0L, 0L, 0L]
    return data
_tokenSet_126 = antlr.BitSet(mk_tokenSet_126())

### generate bit set
def mk_tokenSet_127(): 
    ### var1
    data = [ 103079215872L, 134217726L, 1L, 0L, 0L, 0L]
    return data
_tokenSet_127 = antlr.BitSet(mk_tokenSet_127())

### generate bit set
def mk_tokenSet_128(): 
    ### var1
    data = [ 5369350240L, 36696200577024L, 268435454L, 0L, 0L, 0L]
    return data
_tokenSet_128 = antlr.BitSet(mk_tokenSet_128())

### generate bit set
def mk_tokenSet_129(): 
    ### var1
    data = [ 2147746304L, 70749519810L, 8725754342801409L, 0L, 0L, 0L]
    return data
_tokenSet_129 = antlr.BitSet(mk_tokenSet_129())

### generate bit set
def mk_tokenSet_130(): 
    ### var1
    data = [ 7517096544L, 39517876649922L, 8866599205339135L, 0L, 0L, 0L]
    return data
_tokenSet_130 = antlr.BitSet(mk_tokenSet_130())

### generate bit set
def mk_tokenSet_131(): 
    ### var1
    data = [ 0L, 0L, 140806207832065L, 0L, 0L, 0L]
    return data
_tokenSet_131 = antlr.BitSet(mk_tokenSet_131())

### generate bit set
def mk_tokenSet_132(): 
    ### var1
    data = [ -2305842902913903744L, 3231962890239L, 8866596802002945L, 0L, 0L, 0L]
    return data
_tokenSet_132 = antlr.BitSet(mk_tokenSet_132())

### generate bit set
def mk_tokenSet_133(): 
    ### var1
    data = [ 2147484192L, 16777152L, 1L, 0L, 0L, 0L]
    return data
_tokenSet_133 = antlr.BitSet(mk_tokenSet_133())

### generate bit set
def mk_tokenSet_134(): 
    ### var1
    data = [ 2147746304L, 70732742656L, 8725724278030337L, 0L, 0L, 0L]
    return data
_tokenSet_134 = antlr.BitSet(mk_tokenSet_134())

### generate bit set
def mk_tokenSet_135(): 
    ### var1
    data = [ 0L, 2L, 30064771072L, 0L, 0L, 0L]
    return data
_tokenSet_135 = antlr.BitSet(mk_tokenSet_135())

### generate bit set
def mk_tokenSet_136(): 
    ### var1
    data = [ -2305842902914165888L, 134217727L, 1L, 0L, 0L, 0L]
    return data
_tokenSet_136 = antlr.BitSet(mk_tokenSet_136())

### generate bit set
def mk_tokenSet_137(): 
    ### var1
    data = [ -2305842897544814624L, 39447127130111L, 140600317837311L, 0L, 0L, 0L]
    return data
_tokenSet_137 = antlr.BitSet(mk_tokenSet_137())

### generate bit set
def mk_tokenSet_138(): 
    ### var1
    data = [ 402653696L, 962072674304L, 140600049401857L, 0L, 0L, 0L]
    return data
_tokenSet_138 = antlr.BitSet(mk_tokenSet_138())

### generate bit set
def mk_tokenSet_139(): 
    ### var1
    data = [ 110998964832L, 39515729166334L, 9007197107257343L, 0L, 0L, 0L]
    return data
_tokenSet_139 = antlr.BitSet(mk_tokenSet_139())

### generate bit set
def mk_tokenSet_140(): 
    ### var1
    data = [ 7516833888L, 36696200577024L, 268435454L, 0L, 0L, 0L]
    return data
_tokenSet_140 = antlr.BitSet(mk_tokenSet_140())
    
