### $ANTLR 2.7.5 (20050403): "mig.g" -> "MIGParser.py"$
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

class Parser(antlr.LLkParser):
    ### user action >>>
    ### user action <<<
    
    def __init__(self, *args, **kwargs):
        antlr.LLkParser.__init__(self, *args, **kwargs)
        self.tokenNames = _tokenNames
        self.buildTokenTypeASTClassMap()
        self.astFactory = antlr.ASTFactory(self.getTokenTypeToASTClassMap())
        self.astFactory.setASTNodeClass()
        
    def statements(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        statements_AST = None
        try:      ## for error handling
            pass
            while True:
                if (_tokenSet_0.member(self.LA(1))):
                    pass
                    self.statement()
                    self.addASTChild(currentAST, self.returnAST)
                else:
                    break
                
            statements_AST = currentAST.root
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_1)
        
        self.returnAST = statements_AST
    
    def statement(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        statement_AST = None
        try:      ## for error handling
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [LITERAL_subsystem]:
                pass
                self.subsystem()
                self.addASTChild(currentAST, self.returnAST)
                self.match(SEMI)
                statement_AST = currentAST.root
            elif la1 and la1 in [LITERAL_waittime]:
                pass
                self.wait_time()
                self.addASTChild(currentAST, self.returnAST)
                self.match(SEMI)
                statement_AST = currentAST.root
            elif la1 and la1 in [LITERAL_msgoption]:
                pass
                self.msg_option()
                self.addASTChild(currentAST, self.returnAST)
                self.match(SEMI)
                statement_AST = currentAST.root
            elif la1 and la1 in [LITERAL_UserTypeLimit]:
                pass
                self.user_type_limit()
                self.addASTChild(currentAST, self.returnAST)
                self.match(SEMI)
                statement_AST = currentAST.root
            elif la1 and la1 in [LITERAL_OnStackLimit]:
                pass
                self.on_stack_limit()
                self.addASTChild(currentAST, self.returnAST)
                self.match(SEMI)
                statement_AST = currentAST.root
            elif la1 and la1 in [LITERAL_error]:
                pass
                self.error()
                self.addASTChild(currentAST, self.returnAST)
                self.match(SEMI)
                statement_AST = currentAST.root
            elif la1 and la1 in [LITERAL_serverprefix]:
                pass
                self.server_prefix()
                self.addASTChild(currentAST, self.returnAST)
                self.match(SEMI)
                statement_AST = currentAST.root
            elif la1 and la1 in [LITERAL_userprefix]:
                pass
                self.user_prefix()
                self.addASTChild(currentAST, self.returnAST)
                self.match(SEMI)
                statement_AST = currentAST.root
            elif la1 and la1 in [LITERAL_serverdemux]:
                pass
                self.server_demux()
                self.addASTChild(currentAST, self.returnAST)
                self.match(SEMI)
                statement_AST = currentAST.root
            elif la1 and la1 in [LITERAL_type]:
                pass
                self.type_decl()
                self.addASTChild(currentAST, self.returnAST)
                self.match(SEMI)
                statement_AST = currentAST.root
            elif la1 and la1 in [LITERAL_routine,LITERAL_simpleroutine]:
                pass
                self.routine_decl()
                self.addASTChild(currentAST, self.returnAST)
                self.match(SEMI)
                statement_AST = currentAST.root
            elif la1 and la1 in [LITERAL_skip]:
                pass
                tmp12_AST = None
                tmp12_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp12_AST)
                self.match(LITERAL_skip)
                self.match(SEMI)
                statement_AST = currentAST.root
            elif la1 and la1 in [LITERAL_import,LITERAL_uimport,LITERAL_simport,LITERAL_dimport,LITERAL_iimport]:
                pass
                self.import_decl()
                self.addASTChild(currentAST, self.returnAST)
                self.match(SEMI)
                statement_AST = currentAST.root
            elif la1 and la1 in [RCSDECL]:
                pass
                self.rcs_decl()
                self.addASTChild(currentAST, self.returnAST)
                self.match(SEMI)
                statement_AST = currentAST.root
            elif la1 and la1 in [SEMI]:
                pass
                self.match(SEMI)
                statement_AST = currentAST.root
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_2)
        
        self.returnAST = statement_AST
    
    def subsystem(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        subsystem_AST = None
        try:      ## for error handling
            pass
            self.match(LITERAL_subsystem)
            self.subsystem_mods()
            self.addASTChild(currentAST, self.returnAST)
            self.subsystem_name()
            self.addASTChild(currentAST, self.returnAST)
            self.subsystem_base()
            self.addASTChild(currentAST, self.returnAST)
            subsystem_AST = currentAST.root
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_3)
        
        self.returnAST = subsystem_AST
    
    def wait_time(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        wait_time_AST = None
        try:      ## for error handling
            pass
            self.match(LITERAL_waittime)
            tmp19_AST = None
            tmp19_AST = self.astFactory.create(self.LT(1))
            self.addASTChild(currentAST, tmp19_AST)
            self.match(STRING)
            wait_time_AST = currentAST.root
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_3)
        
        self.returnAST = wait_time_AST
    
    def msg_option(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        msg_option_AST = None
        try:      ## for error handling
            pass
            self.match(LITERAL_msgoption)
            tmp21_AST = None
            tmp21_AST = self.astFactory.create(self.LT(1))
            self.addASTChild(currentAST, tmp21_AST)
            self.match(STRING)
            msg_option_AST = currentAST.root
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_3)
        
        self.returnAST = msg_option_AST
    
    def user_type_limit(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        user_type_limit_AST = None
        try:      ## for error handling
            pass
            self.match(LITERAL_UserTypeLimit)
            tmp23_AST = None
            tmp23_AST = self.astFactory.create(self.LT(1))
            self.addASTChild(currentAST, tmp23_AST)
            self.match(NUMBER)
            user_type_limit_AST = currentAST.root
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_3)
        
        self.returnAST = user_type_limit_AST
    
    def on_stack_limit(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        on_stack_limit_AST = None
        try:      ## for error handling
            pass
            self.match(LITERAL_OnStackLimit)
            tmp25_AST = None
            tmp25_AST = self.astFactory.create(self.LT(1))
            self.addASTChild(currentAST, tmp25_AST)
            self.match(NUMBER)
            on_stack_limit_AST = currentAST.root
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_3)
        
        self.returnAST = on_stack_limit_AST
    
    def error(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        error_AST = None
        try:      ## for error handling
            pass
            self.match(LITERAL_error)
            tmp27_AST = None
            tmp27_AST = self.astFactory.create(self.LT(1))
            self.addASTChild(currentAST, tmp27_AST)
            self.match(IDENT)
            error_AST = currentAST.root
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_3)
        
        self.returnAST = error_AST
    
    def server_prefix(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        server_prefix_AST = None
        try:      ## for error handling
            pass
            self.match(LITERAL_serverprefix)
            tmp29_AST = None
            tmp29_AST = self.astFactory.create(self.LT(1))
            self.addASTChild(currentAST, tmp29_AST)
            self.match(IDENT)
            server_prefix_AST = currentAST.root
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_3)
        
        self.returnAST = server_prefix_AST
    
    def user_prefix(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        user_prefix_AST = None
        try:      ## for error handling
            pass
            self.match(LITERAL_userprefix)
            tmp31_AST = None
            tmp31_AST = self.astFactory.create(self.LT(1))
            self.addASTChild(currentAST, tmp31_AST)
            self.match(IDENT)
            user_prefix_AST = currentAST.root
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_3)
        
        self.returnAST = user_prefix_AST
    
    def server_demux(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        server_demux_AST = None
        try:      ## for error handling
            pass
            self.match(LITERAL_serverdemux)
            tmp33_AST = None
            tmp33_AST = self.astFactory.create(self.LT(1))
            self.addASTChild(currentAST, tmp33_AST)
            self.match(IDENT)
            server_demux_AST = currentAST.root
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_3)
        
        self.returnAST = server_demux_AST
    
    def type_decl(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        type_decl_AST = None
        try:      ## for error handling
            pass
            self.match(LITERAL_type)
            self.named_type_spec()
            self.addASTChild(currentAST, self.returnAST)
            type_decl_AST = currentAST.root
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_3)
        
        self.returnAST = type_decl_AST
    
    def routine_decl(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        routine_decl_AST = None
        try:      ## for error handling
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [LITERAL_routine]:
                pass
                self.routine()
                self.addASTChild(currentAST, self.returnAST)
                routine_decl_AST = currentAST.root
            elif la1 and la1 in [LITERAL_simpleroutine]:
                pass
                self.simple_routine()
                self.addASTChild(currentAST, self.returnAST)
                routine_decl_AST = currentAST.root
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_3)
        
        self.returnAST = routine_decl_AST
    
    def import_decl(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        import_decl_AST = None
        try:      ## for error handling
            pass
            self.import_indicant()
            self.addASTChild(currentAST, self.returnAST)
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [QUOTEDSTRING]:
                pass
                tmp35_AST = None
                tmp35_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp35_AST)
                self.match(QUOTEDSTRING)
            elif la1 and la1 in [ANGLEQUOTEDSTRING]:
                pass
                tmp36_AST = None
                tmp36_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp36_AST)
                self.match(ANGLEQUOTEDSTRING)
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
            import_decl_AST = currentAST.root
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_3)
        
        self.returnAST = import_decl_AST
    
    def rcs_decl(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        rcs_decl_AST = None
        try:      ## for error handling
            pass
            tmp37_AST = None
            tmp37_AST = self.astFactory.create(self.LT(1))
            self.addASTChild(currentAST, tmp37_AST)
            self.match(RCSDECL)
            rcs_decl_AST = currentAST.root
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_3)
        
        self.returnAST = rcs_decl_AST
    
    def subsystem_mods(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        subsystem_mods_AST = None
        try:      ## for error handling
            pass
            while True:
                if (self.LA(1)==LITERAL_kerneluser or self.LA(1)==LITERAL_kernelserver):
                    pass
                    self.subsystem_mod()
                    self.addASTChild(currentAST, self.returnAST)
                else:
                    break
                
            subsystem_mods_AST = currentAST.root
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_4)
        
        self.returnAST = subsystem_mods_AST
    
    def subsystem_name(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        subsystem_name_AST = None
        try:      ## for error handling
            pass
            tmp38_AST = None
            tmp38_AST = self.astFactory.create(self.LT(1))
            self.addASTChild(currentAST, tmp38_AST)
            self.match(IDENT)
            subsystem_name_AST = currentAST.root
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_5)
        
        self.returnAST = subsystem_name_AST
    
    def subsystem_base(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        subsystem_base_AST = None
        try:      ## for error handling
            pass
            tmp39_AST = None
            tmp39_AST = self.astFactory.create(self.LT(1))
            self.addASTChild(currentAST, tmp39_AST)
            self.match(NUMBER)
            subsystem_base_AST = currentAST.root
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_3)
        
        self.returnAST = subsystem_base_AST
    
    def subsystem_mod(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        subsystem_mod_AST = None
        try:      ## for error handling
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [LITERAL_kerneluser]:
                pass
                tmp40_AST = None
                tmp40_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp40_AST)
                self.match(LITERAL_kerneluser)
                subsystem_mod_AST = currentAST.root
            elif la1 and la1 in [LITERAL_kernelserver]:
                pass
                tmp41_AST = None
                tmp41_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp41_AST)
                self.match(LITERAL_kernelserver)
                subsystem_mod_AST = currentAST.root
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_6)
        
        self.returnAST = subsystem_mod_AST
    
    def import_indicant(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        import_indicant_AST = None
        try:      ## for error handling
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [LITERAL_import]:
                pass
                tmp42_AST = None
                tmp42_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp42_AST)
                self.match(LITERAL_import)
                import_indicant_AST = currentAST.root
            elif la1 and la1 in [LITERAL_uimport]:
                pass
                tmp43_AST = None
                tmp43_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp43_AST)
                self.match(LITERAL_uimport)
                import_indicant_AST = currentAST.root
            elif la1 and la1 in [LITERAL_simport]:
                pass
                tmp44_AST = None
                tmp44_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp44_AST)
                self.match(LITERAL_simport)
                import_indicant_AST = currentAST.root
            elif la1 and la1 in [LITERAL_dimport]:
                pass
                tmp45_AST = None
                tmp45_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp45_AST)
                self.match(LITERAL_dimport)
                import_indicant_AST = currentAST.root
            elif la1 and la1 in [LITERAL_iimport]:
                pass
                tmp46_AST = None
                tmp46_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp46_AST)
                self.match(LITERAL_iimport)
                import_indicant_AST = currentAST.root
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_7)
        
        self.returnAST = import_indicant_AST
    
    def named_type_spec(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        named_type_spec_AST = None
        try:      ## for error handling
            pass
            tmp47_AST = None
            tmp47_AST = self.astFactory.create(self.LT(1))
            self.addASTChild(currentAST, tmp47_AST)
            self.match(IDENT)
            self.match(EQUAL)
            self.trans_type_spec()
            self.addASTChild(currentAST, self.returnAST)
            named_type_spec_AST = currentAST.root
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_8)
        
        self.returnAST = named_type_spec_AST
    
    def trans_type_spec(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        trans_type_spec_AST = None
        try:      ## for error handling
            pass
            self.type_spec()
            self.addASTChild(currentAST, self.returnAST)
            while True:
                if (_tokenSet_9.member(self.LA(1))):
                    pass
                    self.trans_type_spec_extended()
                    self.addASTChild(currentAST, self.returnAST)
                else:
                    break
                
            trans_type_spec_AST = currentAST.root
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_8)
        
        self.returnAST = trans_type_spec_AST
    
    def type_spec(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        type_spec_AST = None
        try:      ## for error handling
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [NUMBER,LPAREN,LITERAL_polymorphic,LITERAL_MACH_MSG_TYPE_MOVE_RECEIVE,LITERAL_MACH_MSG_TYPE_COPY_SEND,LITERAL_MACH_MSG_TYPE_MAKE_SEND,LITERAL_MACH_MSG_TYPE_MOVE_SEND,LITERAL_MACH_MSG_TYPE_MAKE_SEND_ONCE,LITERAL_MACH_MSG_TYPE_MOVE_SEND_ONCE,LITERAL_MACH_MSG_TYPE_PORT_NAME,LITERAL_MACH_MSG_TYPE_PORT_RECEIVE,LITERAL_MACH_MSG_TYPE_PORT_SEND,LITERAL_MACH_MSG_TYPE_PORT_SEND_ONCE,LITERAL_MACH_MSG_TYPE_POLYMORPHIC,LITERAL_MACH_MSG_TYPE_UNSTRUCTURED,LITERAL_MACH_MSG_TYPE_BIT,LITERAL_MACH_MSG_TYPE_BOOLEAN,59,60,61,62,LITERAL_MACH_MSG_TYPE_REAL,64,65,LITERAL_MACH_MSG_TYPE_CHAR,LITERAL_MACH_MSG_TYPE_BYTE,LITERAL_MACH_MSG_TYPE_STRING,LITERAL_MACH_MSG_TYPE_STRING_C]:
                pass
                self.basic_type_spec()
                self.addASTChild(currentAST, self.returnAST)
                type_spec_AST = currentAST.root
            elif la1 and la1 in [IDENT]:
                pass
                self.prev_type_spec()
                self.addASTChild(currentAST, self.returnAST)
                type_spec_AST = currentAST.root
            elif la1 and la1 in [LITERAL_array]:
                pass
                self.array_spec()
                self.addASTChild(currentAST, self.returnAST)
                type_spec_AST = currentAST.root
            elif la1 and la1 in [CARET]:
                pass
                tmp49_AST = None
                tmp49_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp49_AST)
                self.match(CARET)
                self.type_spec()
                self.addASTChild(currentAST, self.returnAST)
                type_spec_AST = currentAST.root
            elif la1 and la1 in [LITERAL_struct]:
                pass
                self.struct_head()
                self.addASTChild(currentAST, self.returnAST)
                type_spec_AST = currentAST.root
            elif la1 and la1 in [LITERAL_c_string]:
                pass
                self.c_string_spec()
                self.addASTChild(currentAST, self.returnAST)
                type_spec_AST = currentAST.root
            elif la1 and la1 in [LITERAL_PointerTo,LITERAL_PointerToIfNot,LITERAL_ValueOf]:
                pass
                self.native_type_spec()
                self.addASTChild(currentAST, self.returnAST)
                type_spec_AST = currentAST.root
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_10)
        
        self.returnAST = type_spec_AST
    
    def trans_type_spec_extended(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        trans_type_spec_extended_AST = None
        try:      ## for error handling
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [LITERAL_intran]:
                pass
                tmp50_AST = None
                tmp50_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp50_AST)
                self.match(LITERAL_intran)
                self.match(COLON)
                tmp52_AST = None
                tmp52_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp52_AST)
                self.match(IDENT)
                tmp53_AST = None
                tmp53_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp53_AST)
                self.match(IDENT)
                self.match(LPAREN)
                tmp55_AST = None
                tmp55_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp55_AST)
                self.match(IDENT)
                self.match(RPAREN)
                trans_type_spec_extended_AST = currentAST.root
            elif la1 and la1 in [LITERAL_outtran]:
                pass
                tmp57_AST = None
                tmp57_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp57_AST)
                self.match(LITERAL_outtran)
                self.match(COLON)
                tmp59_AST = None
                tmp59_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp59_AST)
                self.match(IDENT)
                tmp60_AST = None
                tmp60_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp60_AST)
                self.match(IDENT)
                self.match(LPAREN)
                tmp62_AST = None
                tmp62_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp62_AST)
                self.match(IDENT)
                self.match(RPAREN)
                trans_type_spec_extended_AST = currentAST.root
            elif la1 and la1 in [LITERAL_destructor]:
                pass
                tmp64_AST = None
                tmp64_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp64_AST)
                self.match(LITERAL_destructor)
                self.match(COLON)
                tmp66_AST = None
                tmp66_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp66_AST)
                self.match(IDENT)
                self.match(LPAREN)
                tmp68_AST = None
                tmp68_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp68_AST)
                self.match(IDENT)
                self.match(RPAREN)
                trans_type_spec_extended_AST = currentAST.root
            elif la1 and la1 in [LITERAL_ctype]:
                pass
                tmp70_AST = None
                tmp70_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp70_AST)
                self.match(LITERAL_ctype)
                self.match(COLON)
                tmp72_AST = None
                tmp72_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp72_AST)
                self.match(IDENT)
                trans_type_spec_extended_AST = currentAST.root
            elif la1 and la1 in [LITERAL_cusertype]:
                pass
                tmp73_AST = None
                tmp73_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp73_AST)
                self.match(LITERAL_cusertype)
                self.match(COLON)
                tmp75_AST = None
                tmp75_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp75_AST)
                self.match(IDENT)
                trans_type_spec_extended_AST = currentAST.root
            elif la1 and la1 in [LITERAL_cservertype]:
                pass
                tmp76_AST = None
                tmp76_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp76_AST)
                self.match(LITERAL_cservertype)
                self.match(COLON)
                tmp78_AST = None
                tmp78_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp78_AST)
                self.match(IDENT)
                trans_type_spec_extended_AST = currentAST.root
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_10)
        
        self.returnAST = trans_type_spec_extended_AST
    
    def basic_type_spec(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        basic_type_spec_AST = None
        try:      ## for error handling
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [NUMBER,LITERAL_polymorphic,LITERAL_MACH_MSG_TYPE_MOVE_RECEIVE,LITERAL_MACH_MSG_TYPE_COPY_SEND,LITERAL_MACH_MSG_TYPE_MAKE_SEND,LITERAL_MACH_MSG_TYPE_MOVE_SEND,LITERAL_MACH_MSG_TYPE_MAKE_SEND_ONCE,LITERAL_MACH_MSG_TYPE_MOVE_SEND_ONCE,LITERAL_MACH_MSG_TYPE_PORT_NAME,LITERAL_MACH_MSG_TYPE_PORT_RECEIVE,LITERAL_MACH_MSG_TYPE_PORT_SEND,LITERAL_MACH_MSG_TYPE_PORT_SEND_ONCE,LITERAL_MACH_MSG_TYPE_POLYMORPHIC,LITERAL_MACH_MSG_TYPE_UNSTRUCTURED,LITERAL_MACH_MSG_TYPE_BIT,LITERAL_MACH_MSG_TYPE_BOOLEAN,59,60,61,62,LITERAL_MACH_MSG_TYPE_REAL,64,65,LITERAL_MACH_MSG_TYPE_CHAR,LITERAL_MACH_MSG_TYPE_BYTE,LITERAL_MACH_MSG_TYPE_STRING,LITERAL_MACH_MSG_TYPE_STRING_C]:
                pass
                self.ipc_type()
                self.addASTChild(currentAST, self.returnAST)
                basic_type_spec_AST = currentAST.root
            elif la1 and la1 in [LPAREN]:
                pass
                self.match(LPAREN)
                self.ipc_type()
                self.addASTChild(currentAST, self.returnAST)
                self.match(COMMA)
                self.int_exp()
                self.addASTChild(currentAST, self.returnAST)
                la1 = self.LA(1)
                if False:
                    pass
                elif la1 and la1 in [COMMA]:
                    pass
                    self.ipc_flags()
                    self.addASTChild(currentAST, self.returnAST)
                elif la1 and la1 in [RPAREN]:
                    pass
                else:
                        raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                    
                self.match(RPAREN)
                basic_type_spec_AST = currentAST.root
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_10)
        
        self.returnAST = basic_type_spec_AST
    
    def prev_type_spec(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        prev_type_spec_AST = None
        try:      ## for error handling
            pass
            tmp82_AST = None
            tmp82_AST = self.astFactory.create(self.LT(1))
            self.addASTChild(currentAST, tmp82_AST)
            self.match(IDENT)
            prev_type_spec_AST = currentAST.root
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_10)
        
        self.returnAST = prev_type_spec_AST
    
    def array_spec(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        array_spec_AST = None
        try:      ## for error handling
            pass
            self.match(LITERAL_array)
            self.match(LBRACK)
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [RBRACK,STAR]:
                pass
                self.var_array_head()
                self.addASTChild(currentAST, self.returnAST)
            elif la1 and la1 in [NUMBER,LPAREN]:
                pass
                self.array_head()
                self.addASTChild(currentAST, self.returnAST)
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
            self.type_spec()
            self.addASTChild(currentAST, self.returnAST)
            array_spec_AST = currentAST.root
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_10)
        
        self.returnAST = array_spec_AST
    
    def struct_head(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        struct_head_AST = None
        try:      ## for error handling
            pass
            self.match(LITERAL_struct)
            self.match(LBRACK)
            self.int_exp()
            self.addASTChild(currentAST, self.returnAST)
            self.match(RBRACK)
            self.match(OF)
            self.type_spec()
            self.addASTChild(currentAST, self.returnAST)
            struct_head_AST = currentAST.root
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_10)
        
        self.returnAST = struct_head_AST
    
    def c_string_spec(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        c_string_spec_AST = None
        try:      ## for error handling
            pass
            self.match(LITERAL_c_string)
            if (self.LA(1)==LBRACK) and (self.LA(2)==NUMBER or self.LA(2)==LPAREN):
                pass
                self.match(LBRACK)
                self.int_exp()
                self.addASTChild(currentAST, self.returnAST)
                self.match(RBRACK)
            elif (self.LA(1)==LBRACK) and (self.LA(2)==STAR):
                pass
                self.match(LBRACK)
                tmp93_AST = None
                tmp93_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp93_AST)
                self.match(STAR)
                tmp94_AST = None
                tmp94_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp94_AST)
                self.match(COLON)
                self.int_exp()
                self.addASTChild(currentAST, self.returnAST)
                self.match(RBRACK)
            else:
                raise antlr.NoViableAltException(self.LT(1), self.getFilename())
            
            c_string_spec_AST = currentAST.root
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_10)
        
        self.returnAST = c_string_spec_AST
    
    def native_type_spec(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        native_type_spec_AST = None
        try:      ## for error handling
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [LITERAL_PointerTo]:
                pass
                tmp96_AST = None
                tmp96_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp96_AST)
                self.match(LITERAL_PointerTo)
                self.match(LPAREN)
                self.type_phrase()
                self.addASTChild(currentAST, self.returnAST)
                self.match(RPAREN)
                native_type_spec_AST = currentAST.root
            elif la1 and la1 in [LITERAL_PointerToIfNot]:
                pass
                tmp99_AST = None
                tmp99_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp99_AST)
                self.match(LITERAL_PointerToIfNot)
                self.match(LPAREN)
                self.type_phrase()
                self.addASTChild(currentAST, self.returnAST)
                self.match(COMMA)
                self.type_phrase()
                self.addASTChild(currentAST, self.returnAST)
                self.match(RPAREN)
                native_type_spec_AST = currentAST.root
            elif la1 and la1 in [LITERAL_ValueOf]:
                pass
                tmp103_AST = None
                tmp103_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp103_AST)
                self.match(LITERAL_ValueOf)
                self.match(LPAREN)
                self.type_phrase()
                self.addASTChild(currentAST, self.returnAST)
                self.match(RPAREN)
                native_type_spec_AST = currentAST.root
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_10)
        
        self.returnAST = native_type_spec_AST
    
    def type_phrase(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        type_phrase_AST = None
        try:      ## for error handling
            pass
            _cnt48= 0
            while True:
                if (self.LA(1)==IDENT):
                    pass
                    tmp106_AST = None
                    tmp106_AST = self.astFactory.create(self.LT(1))
                    self.addASTChild(currentAST, tmp106_AST)
                    self.match(IDENT)
                else:
                    break
                
                _cnt48 += 1
            if _cnt48 < 1:
                raise antlr.NoViableAltException(self.LT(1), self.getFilename())
            type_phrase_AST = currentAST.root
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_11)
        
        self.returnAST = type_phrase_AST
    
    def ipc_type(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        ipc_type_AST = None
        try:      ## for error handling
            pass
            self.prim_ipc_type()
            self.addASTChild(currentAST, self.returnAST)
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [BAR]:
                pass
                self.match(BAR)
                self.prim_ipc_type()
                self.addASTChild(currentAST, self.returnAST)
            elif la1 and la1 in [SEMI,LITERAL_intran,RPAREN,LITERAL_outtran,LITERAL_destructor,LITERAL_ctype,LITERAL_cusertype,LITERAL_cservertype,COMMA]:
                pass
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
            ipc_type_AST = currentAST.root
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_10)
        
        self.returnAST = ipc_type_AST
    
    def int_exp(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        int_exp_AST = None
        try:      ## for error handling
            if (self.LA(1)==NUMBER) and (self.LA(2)==RPAREN or self.LA(2)==COMMA or self.LA(2)==RBRACK):
                pass
                tmp108_AST = None
                tmp108_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp108_AST)
                self.match(NUMBER)
                int_exp_AST = currentAST.root
            elif (self.LA(1)==NUMBER) and (_tokenSet_12.member(self.LA(2))):
                pass
                tmp109_AST = None
                tmp109_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp109_AST)
                self.match(NUMBER)
                la1 = self.LA(1)
                if False:
                    pass
                elif la1 and la1 in [PLUS]:
                    pass
                    tmp110_AST = None
                    tmp110_AST = self.astFactory.create(self.LT(1))
                    self.addASTChild(currentAST, tmp110_AST)
                    self.match(PLUS)
                elif la1 and la1 in [MINUS]:
                    pass
                    tmp111_AST = None
                    tmp111_AST = self.astFactory.create(self.LT(1))
                    self.addASTChild(currentAST, tmp111_AST)
                    self.match(MINUS)
                elif la1 and la1 in [STAR]:
                    pass
                    tmp112_AST = None
                    tmp112_AST = self.astFactory.create(self.LT(1))
                    self.addASTChild(currentAST, tmp112_AST)
                    self.match(STAR)
                elif la1 and la1 in [DIV]:
                    pass
                    tmp113_AST = None
                    tmp113_AST = self.astFactory.create(self.LT(1))
                    self.addASTChild(currentAST, tmp113_AST)
                    self.match(DIV)
                else:
                        raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                    
                self.int_exp()
                self.addASTChild(currentAST, self.returnAST)
                int_exp_AST = currentAST.root
            elif (self.LA(1)==LPAREN):
                pass
                tmp114_AST = None
                tmp114_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp114_AST)
                self.match(LPAREN)
                self.int_exp()
                self.addASTChild(currentAST, self.returnAST)
                tmp115_AST = None
                tmp115_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp115_AST)
                self.match(RPAREN)
                int_exp_AST = currentAST.root
            else:
                raise antlr.NoViableAltException(self.LT(1), self.getFilename())
            
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_13)
        
        self.returnAST = int_exp_AST
    
    def ipc_flags(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        ipc_flags_AST = None
        try:      ## for error handling
            pass
            _cnt70= 0
            while True:
                if (self.LA(1)==COMMA):
                    pass
                    self.match(COMMA)
                    self.ipc_flag()
                    self.addASTChild(currentAST, self.returnAST)
                else:
                    break
                
                _cnt70 += 1
            if _cnt70 < 1:
                raise antlr.NoViableAltException(self.LT(1), self.getFilename())
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [LBRACK]:
                pass
                tmp117_AST = None
                tmp117_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp117_AST)
                self.match(LBRACK)
                tmp118_AST = None
                tmp118_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp118_AST)
                self.match(RBRACK)
            elif la1 and la1 in [SEMI,RPAREN]:
                pass
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
            ipc_flags_AST = currentAST.root
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_14)
        
        self.returnAST = ipc_flags_AST
    
    def prim_ipc_type(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        prim_ipc_type_AST = None
        try:      ## for error handling
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [NUMBER]:
                pass
                tmp119_AST = None
                tmp119_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp119_AST)
                self.match(NUMBER)
                prim_ipc_type_AST = currentAST.root
            elif la1 and la1 in [LITERAL_polymorphic,LITERAL_MACH_MSG_TYPE_MOVE_RECEIVE,LITERAL_MACH_MSG_TYPE_COPY_SEND,LITERAL_MACH_MSG_TYPE_MAKE_SEND,LITERAL_MACH_MSG_TYPE_MOVE_SEND,LITERAL_MACH_MSG_TYPE_MAKE_SEND_ONCE,LITERAL_MACH_MSG_TYPE_MOVE_SEND_ONCE,LITERAL_MACH_MSG_TYPE_PORT_NAME,LITERAL_MACH_MSG_TYPE_PORT_RECEIVE,LITERAL_MACH_MSG_TYPE_PORT_SEND,LITERAL_MACH_MSG_TYPE_PORT_SEND_ONCE,LITERAL_MACH_MSG_TYPE_POLYMORPHIC,LITERAL_MACH_MSG_TYPE_UNSTRUCTURED,LITERAL_MACH_MSG_TYPE_BIT,LITERAL_MACH_MSG_TYPE_BOOLEAN,59,60,61,62,LITERAL_MACH_MSG_TYPE_REAL,64,65,LITERAL_MACH_MSG_TYPE_CHAR,LITERAL_MACH_MSG_TYPE_BYTE,LITERAL_MACH_MSG_TYPE_STRING,LITERAL_MACH_MSG_TYPE_STRING_C]:
                pass
                self.symbolic_type()
                self.addASTChild(currentAST, self.returnAST)
                prim_ipc_type_AST = currentAST.root
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_15)
        
        self.returnAST = prim_ipc_type_AST
    
    def symbolic_type(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        symbolic_type_AST = None
        try:      ## for error handling
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [LITERAL_polymorphic]:
                pass
                tmp120_AST = None
                tmp120_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp120_AST)
                self.match(LITERAL_polymorphic)
                symbolic_type_AST = currentAST.root
            elif la1 and la1 in [LITERAL_MACH_MSG_TYPE_MOVE_RECEIVE]:
                pass
                tmp121_AST = None
                tmp121_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp121_AST)
                self.match(LITERAL_MACH_MSG_TYPE_MOVE_RECEIVE)
                symbolic_type_AST = currentAST.root
            elif la1 and la1 in [LITERAL_MACH_MSG_TYPE_COPY_SEND]:
                pass
                tmp122_AST = None
                tmp122_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp122_AST)
                self.match(LITERAL_MACH_MSG_TYPE_COPY_SEND)
                symbolic_type_AST = currentAST.root
            elif la1 and la1 in [LITERAL_MACH_MSG_TYPE_MAKE_SEND]:
                pass
                tmp123_AST = None
                tmp123_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp123_AST)
                self.match(LITERAL_MACH_MSG_TYPE_MAKE_SEND)
                symbolic_type_AST = currentAST.root
            elif la1 and la1 in [LITERAL_MACH_MSG_TYPE_MOVE_SEND]:
                pass
                tmp124_AST = None
                tmp124_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp124_AST)
                self.match(LITERAL_MACH_MSG_TYPE_MOVE_SEND)
                symbolic_type_AST = currentAST.root
            elif la1 and la1 in [LITERAL_MACH_MSG_TYPE_MAKE_SEND_ONCE]:
                pass
                tmp125_AST = None
                tmp125_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp125_AST)
                self.match(LITERAL_MACH_MSG_TYPE_MAKE_SEND_ONCE)
                symbolic_type_AST = currentAST.root
            elif la1 and la1 in [LITERAL_MACH_MSG_TYPE_MOVE_SEND_ONCE]:
                pass
                tmp126_AST = None
                tmp126_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp126_AST)
                self.match(LITERAL_MACH_MSG_TYPE_MOVE_SEND_ONCE)
                symbolic_type_AST = currentAST.root
            elif la1 and la1 in [LITERAL_MACH_MSG_TYPE_PORT_NAME]:
                pass
                tmp127_AST = None
                tmp127_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp127_AST)
                self.match(LITERAL_MACH_MSG_TYPE_PORT_NAME)
                symbolic_type_AST = currentAST.root
            elif la1 and la1 in [LITERAL_MACH_MSG_TYPE_PORT_RECEIVE]:
                pass
                tmp128_AST = None
                tmp128_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp128_AST)
                self.match(LITERAL_MACH_MSG_TYPE_PORT_RECEIVE)
                symbolic_type_AST = currentAST.root
            elif la1 and la1 in [LITERAL_MACH_MSG_TYPE_PORT_SEND]:
                pass
                tmp129_AST = None
                tmp129_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp129_AST)
                self.match(LITERAL_MACH_MSG_TYPE_PORT_SEND)
                symbolic_type_AST = currentAST.root
            elif la1 and la1 in [LITERAL_MACH_MSG_TYPE_PORT_SEND_ONCE]:
                pass
                tmp130_AST = None
                tmp130_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp130_AST)
                self.match(LITERAL_MACH_MSG_TYPE_PORT_SEND_ONCE)
                symbolic_type_AST = currentAST.root
            elif la1 and la1 in [LITERAL_MACH_MSG_TYPE_POLYMORPHIC]:
                pass
                tmp131_AST = None
                tmp131_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp131_AST)
                self.match(LITERAL_MACH_MSG_TYPE_POLYMORPHIC)
                symbolic_type_AST = currentAST.root
            elif la1 and la1 in [LITERAL_MACH_MSG_TYPE_UNSTRUCTURED]:
                pass
                tmp132_AST = None
                tmp132_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp132_AST)
                self.match(LITERAL_MACH_MSG_TYPE_UNSTRUCTURED)
                symbolic_type_AST = currentAST.root
            elif la1 and la1 in [LITERAL_MACH_MSG_TYPE_BIT]:
                pass
                tmp133_AST = None
                tmp133_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp133_AST)
                self.match(LITERAL_MACH_MSG_TYPE_BIT)
                symbolic_type_AST = currentAST.root
            elif la1 and la1 in [LITERAL_MACH_MSG_TYPE_BOOLEAN]:
                pass
                tmp134_AST = None
                tmp134_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp134_AST)
                self.match(LITERAL_MACH_MSG_TYPE_BOOLEAN)
                symbolic_type_AST = currentAST.root
            elif la1 and la1 in [59]:
                pass
                tmp135_AST = None
                tmp135_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp135_AST)
                self.match(59)
                symbolic_type_AST = currentAST.root
            elif la1 and la1 in [60]:
                pass
                tmp136_AST = None
                tmp136_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp136_AST)
                self.match(60)
                symbolic_type_AST = currentAST.root
            elif la1 and la1 in [61]:
                pass
                tmp137_AST = None
                tmp137_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp137_AST)
                self.match(61)
                symbolic_type_AST = currentAST.root
            elif la1 and la1 in [62]:
                pass
                tmp138_AST = None
                tmp138_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp138_AST)
                self.match(62)
                symbolic_type_AST = currentAST.root
            elif la1 and la1 in [LITERAL_MACH_MSG_TYPE_REAL]:
                pass
                tmp139_AST = None
                tmp139_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp139_AST)
                self.match(LITERAL_MACH_MSG_TYPE_REAL)
                symbolic_type_AST = currentAST.root
            elif la1 and la1 in [64]:
                pass
                tmp140_AST = None
                tmp140_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp140_AST)
                self.match(64)
                symbolic_type_AST = currentAST.root
            elif la1 and la1 in [65]:
                pass
                tmp141_AST = None
                tmp141_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp141_AST)
                self.match(65)
                symbolic_type_AST = currentAST.root
            elif la1 and la1 in [LITERAL_MACH_MSG_TYPE_CHAR]:
                pass
                tmp142_AST = None
                tmp142_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp142_AST)
                self.match(LITERAL_MACH_MSG_TYPE_CHAR)
                symbolic_type_AST = currentAST.root
            elif la1 and la1 in [LITERAL_MACH_MSG_TYPE_BYTE]:
                pass
                tmp143_AST = None
                tmp143_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp143_AST)
                self.match(LITERAL_MACH_MSG_TYPE_BYTE)
                symbolic_type_AST = currentAST.root
            elif la1 and la1 in [LITERAL_MACH_MSG_TYPE_STRING]:
                pass
                tmp144_AST = None
                tmp144_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp144_AST)
                self.match(LITERAL_MACH_MSG_TYPE_STRING)
                symbolic_type_AST = currentAST.root
            elif la1 and la1 in [LITERAL_MACH_MSG_TYPE_STRING_C]:
                pass
                tmp145_AST = None
                tmp145_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp145_AST)
                self.match(LITERAL_MACH_MSG_TYPE_STRING_C)
                symbolic_type_AST = currentAST.root
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_15)
        
        self.returnAST = symbolic_type_AST
    
    def var_array_head(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        var_array_head_AST = None
        try:      ## for error handling
            if (self.LA(1)==RBRACK):
                pass
                self.match(RBRACK)
                self.match(OF)
                var_array_head_AST = currentAST.root
            elif (self.LA(1)==STAR) and (self.LA(2)==RBRACK):
                pass
                tmp148_AST = None
                tmp148_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp148_AST)
                self.match(STAR)
                self.match(RBRACK)
                self.match(OF)
                var_array_head_AST = currentAST.root
            elif (self.LA(1)==STAR) and (self.LA(2)==COLON):
                pass
                tmp151_AST = None
                tmp151_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp151_AST)
                self.match(STAR)
                tmp152_AST = None
                tmp152_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp152_AST)
                self.match(COLON)
                self.int_exp()
                self.addASTChild(currentAST, self.returnAST)
                self.match(RBRACK)
                self.match(OF)
                var_array_head_AST = currentAST.root
            else:
                raise antlr.NoViableAltException(self.LT(1), self.getFilename())
            
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_16)
        
        self.returnAST = var_array_head_AST
    
    def array_head(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        array_head_AST = None
        try:      ## for error handling
            pass
            self.int_exp()
            self.addASTChild(currentAST, self.returnAST)
            self.match(RBRACK)
            self.match(OF)
            array_head_AST = currentAST.root
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_16)
        
        self.returnAST = array_head_AST
    
    def routine(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        routine_AST = None
        try:      ## for error handling
            pass
            self.match(LITERAL_routine)
            self.identifier()
            self.addASTChild(currentAST, self.returnAST)
            self.arguments()
            self.addASTChild(currentAST, self.returnAST)
            routine_AST = currentAST.root
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_3)
        
        self.returnAST = routine_AST
    
    def simple_routine(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        simple_routine_AST = None
        try:      ## for error handling
            pass
            self.match(LITERAL_simpleroutine)
            self.identifier()
            self.addASTChild(currentAST, self.returnAST)
            self.arguments()
            self.addASTChild(currentAST, self.returnAST)
            simple_routine_AST = currentAST.root
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_3)
        
        self.returnAST = simple_routine_AST
    
    def identifier(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        identifier_AST = None
        try:      ## for error handling
            pass
            tmp159_AST = None
            tmp159_AST = self.astFactory.create(self.LT(1))
            self.addASTChild(currentAST, tmp159_AST)
            self.match(IDENT)
            identifier_AST = currentAST.root
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_17)
        
        self.returnAST = identifier_AST
    
    def arguments(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        arguments_AST = None
        try:      ## for error handling
            pass
            self.match(LPAREN)
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [IDENT,LITERAL_waittime,LITERAL_msgoption,LITERAL_in,LITERAL_out,LITERAL_inout,LITERAL_requestport,LITERAL_replyport,LITERAL_sreplyport,LITERAL_ureplyport,LITERAL_sendtime,LITERAL_sectoken,LITERAL_serversectoken,LITERAL_usersectoken,LITERAL_audittoken,LITERAL_serveraudittoken,LITERAL_useraudittoken,LITERAL_msgseqno,LITERAL_serverimpl,LITERAL_userimpl]:
                pass
                self.argument_list()
                self.addASTChild(currentAST, self.returnAST)
            elif la1 and la1 in [RPAREN]:
                pass
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
            self.match(RPAREN)
            arguments_AST = currentAST.root
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_3)
        
        self.returnAST = arguments_AST
    
    def argument_list(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        argument_list_AST = None
        try:      ## for error handling
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [IDENT,LITERAL_waittime,LITERAL_msgoption,LITERAL_in,LITERAL_out,LITERAL_inout,LITERAL_requestport,LITERAL_replyport,LITERAL_sreplyport,LITERAL_ureplyport,LITERAL_sendtime,LITERAL_sectoken,LITERAL_serversectoken,LITERAL_usersectoken,LITERAL_audittoken,LITERAL_serveraudittoken,LITERAL_useraudittoken,LITERAL_msgseqno]:
                pass
                self.argument()
                self.addASTChild(currentAST, self.returnAST)
                la1 = self.LA(1)
                if False:
                    pass
                elif la1 and la1 in [SEMI]:
                    pass
                    self.match(SEMI)
                    self.argument_list()
                    self.addASTChild(currentAST, self.returnAST)
                elif la1 and la1 in [RPAREN]:
                    pass
                else:
                        raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                    
                argument_list_AST = currentAST.root
            elif la1 and la1 in [LITERAL_serverimpl,LITERAL_userimpl]:
                pass
                self.trailer()
                self.addASTChild(currentAST, self.returnAST)
                la1 = self.LA(1)
                if False:
                    pass
                elif la1 and la1 in [SEMI]:
                    pass
                    self.match(SEMI)
                    self.argument_list()
                    self.addASTChild(currentAST, self.returnAST)
                elif la1 and la1 in [RPAREN]:
                    pass
                else:
                        raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                    
                argument_list_AST = currentAST.root
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_18)
        
        self.returnAST = argument_list_AST
    
    def argument(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        argument_AST = None
        try:      ## for error handling
            pass
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [LITERAL_waittime,LITERAL_msgoption,LITERAL_in,LITERAL_out,LITERAL_inout,LITERAL_requestport,LITERAL_replyport,LITERAL_sreplyport,LITERAL_ureplyport,LITERAL_sendtime,LITERAL_sectoken,LITERAL_serversectoken,LITERAL_usersectoken,LITERAL_audittoken,LITERAL_serveraudittoken,LITERAL_useraudittoken,LITERAL_msgseqno]:
                pass
                self.direction()
                self.addASTChild(currentAST, self.returnAST)
            elif la1 and la1 in [IDENT]:
                pass
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
            self.identifier()
            self.addASTChild(currentAST, self.returnAST)
            self.argument_type()
            self.addASTChild(currentAST, self.returnAST)
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [COMMA]:
                pass
                self.ipc_flags()
                self.addASTChild(currentAST, self.returnAST)
            elif la1 and la1 in [SEMI,RPAREN]:
                pass
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
            argument_AST = currentAST.root
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_14)
        
        self.returnAST = argument_AST
    
    def trailer(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        trailer_AST = None
        try:      ## for error handling
            pass
            self.tr_impl_keyword()
            self.addASTChild(currentAST, self.returnAST)
            tmp164_AST = None
            tmp164_AST = self.astFactory.create(self.LT(1))
            self.addASTChild(currentAST, tmp164_AST)
            self.match(IDENT)
            self.argument_type()
            self.addASTChild(currentAST, self.returnAST)
            trailer_AST = currentAST.root
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_14)
        
        self.returnAST = trailer_AST
    
    def direction(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        direction_AST = None
        try:      ## for error handling
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [LITERAL_in]:
                pass
                tmp165_AST = None
                tmp165_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp165_AST)
                self.match(LITERAL_in)
                direction_AST = currentAST.root
            elif la1 and la1 in [LITERAL_out]:
                pass
                tmp166_AST = None
                tmp166_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp166_AST)
                self.match(LITERAL_out)
                direction_AST = currentAST.root
            elif la1 and la1 in [LITERAL_inout]:
                pass
                tmp167_AST = None
                tmp167_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp167_AST)
                self.match(LITERAL_inout)
                direction_AST = currentAST.root
            elif la1 and la1 in [LITERAL_requestport]:
                pass
                tmp168_AST = None
                tmp168_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp168_AST)
                self.match(LITERAL_requestport)
                direction_AST = currentAST.root
            elif la1 and la1 in [LITERAL_replyport]:
                pass
                tmp169_AST = None
                tmp169_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp169_AST)
                self.match(LITERAL_replyport)
                direction_AST = currentAST.root
            elif la1 and la1 in [LITERAL_sreplyport]:
                pass
                tmp170_AST = None
                tmp170_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp170_AST)
                self.match(LITERAL_sreplyport)
                direction_AST = currentAST.root
            elif la1 and la1 in [LITERAL_ureplyport]:
                pass
                tmp171_AST = None
                tmp171_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp171_AST)
                self.match(LITERAL_ureplyport)
                direction_AST = currentAST.root
            elif la1 and la1 in [LITERAL_waittime]:
                pass
                tmp172_AST = None
                tmp172_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp172_AST)
                self.match(LITERAL_waittime)
                direction_AST = currentAST.root
            elif la1 and la1 in [LITERAL_sendtime]:
                pass
                tmp173_AST = None
                tmp173_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp173_AST)
                self.match(LITERAL_sendtime)
                direction_AST = currentAST.root
            elif la1 and la1 in [LITERAL_msgoption]:
                pass
                tmp174_AST = None
                tmp174_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp174_AST)
                self.match(LITERAL_msgoption)
                direction_AST = currentAST.root
            elif la1 and la1 in [LITERAL_sectoken]:
                pass
                tmp175_AST = None
                tmp175_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp175_AST)
                self.match(LITERAL_sectoken)
                direction_AST = currentAST.root
            elif la1 and la1 in [LITERAL_serversectoken]:
                pass
                tmp176_AST = None
                tmp176_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp176_AST)
                self.match(LITERAL_serversectoken)
                direction_AST = currentAST.root
            elif la1 and la1 in [LITERAL_usersectoken]:
                pass
                tmp177_AST = None
                tmp177_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp177_AST)
                self.match(LITERAL_usersectoken)
                direction_AST = currentAST.root
            elif la1 and la1 in [LITERAL_audittoken]:
                pass
                tmp178_AST = None
                tmp178_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp178_AST)
                self.match(LITERAL_audittoken)
                direction_AST = currentAST.root
            elif la1 and la1 in [LITERAL_serveraudittoken]:
                pass
                tmp179_AST = None
                tmp179_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp179_AST)
                self.match(LITERAL_serveraudittoken)
                direction_AST = currentAST.root
            elif la1 and la1 in [LITERAL_useraudittoken]:
                pass
                tmp180_AST = None
                tmp180_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp180_AST)
                self.match(LITERAL_useraudittoken)
                direction_AST = currentAST.root
            elif la1 and la1 in [LITERAL_msgseqno]:
                pass
                tmp181_AST = None
                tmp181_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp181_AST)
                self.match(LITERAL_msgseqno)
                direction_AST = currentAST.root
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_4)
        
        self.returnAST = direction_AST
    
    def argument_type(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        argument_type_AST = None
        try:      ## for error handling
            pass
            self.match(COLON)
            if (self.LA(1)==IDENT) and (self.LA(2)==SEMI or self.LA(2)==RPAREN or self.LA(2)==COMMA):
                pass
                self.identifier()
                self.addASTChild(currentAST, self.returnAST)
            elif (self.LA(1)==IDENT) and (self.LA(2)==EQUAL):
                pass
                self.named_type_spec()
                self.addASTChild(currentAST, self.returnAST)
            elif (self.LA(1)==LITERAL_PointerTo or self.LA(1)==LITERAL_PointerToIfNot or self.LA(1)==LITERAL_ValueOf):
                pass
                self.native_type_spec()
                self.addASTChild(currentAST, self.returnAST)
            else:
                raise antlr.NoViableAltException(self.LT(1), self.getFilename())
            
            argument_type_AST = currentAST.root
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_8)
        
        self.returnAST = argument_type_AST
    
    def tr_impl_keyword(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        tr_impl_keyword_AST = None
        try:      ## for error handling
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [LITERAL_serverimpl]:
                pass
                tmp183_AST = None
                tmp183_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp183_AST)
                self.match(LITERAL_serverimpl)
                tr_impl_keyword_AST = currentAST.root
            elif la1 and la1 in [LITERAL_userimpl]:
                pass
                tmp184_AST = None
                tmp184_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp184_AST)
                self.match(LITERAL_userimpl)
                tr_impl_keyword_AST = currentAST.root
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_4)
        
        self.returnAST = tr_impl_keyword_AST
    
    def ipc_flag(self):    
        
        self.returnAST = None
        currentAST = antlr.ASTPair()
        ipc_flag_AST = None
        try:      ## for error handling
            la1 = self.LA(1)
            if False:
                pass
            elif la1 and la1 in [LITERAL_samecount]:
                pass
                tmp185_AST = None
                tmp185_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp185_AST)
                self.match(LITERAL_samecount)
                ipc_flag_AST = currentAST.root
            elif la1 and la1 in [LITERAL_retcode]:
                pass
                tmp186_AST = None
                tmp186_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp186_AST)
                self.match(LITERAL_retcode)
                ipc_flag_AST = currentAST.root
            elif la1 and la1 in [LITERAL_physicalcopy]:
                pass
                tmp187_AST = None
                tmp187_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp187_AST)
                self.match(LITERAL_physicalcopy)
                ipc_flag_AST = currentAST.root
            elif la1 and la1 in [LITERAL_servercopy]:
                pass
                tmp188_AST = None
                tmp188_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp188_AST)
                self.match(LITERAL_servercopy)
                ipc_flag_AST = currentAST.root
            elif la1 and la1 in [LITERAL_overwrite]:
                pass
                tmp189_AST = None
                tmp189_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp189_AST)
                self.match(LITERAL_overwrite)
                ipc_flag_AST = currentAST.root
            elif la1 and la1 in [LITERAL_dealloc]:
                pass
                tmp190_AST = None
                tmp190_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp190_AST)
                self.match(LITERAL_dealloc)
                ipc_flag_AST = currentAST.root
            elif la1 and la1 in [LITERAL_notdealloc]:
                pass
                tmp191_AST = None
                tmp191_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp191_AST)
                self.match(LITERAL_notdealloc)
                ipc_flag_AST = currentAST.root
            elif la1 and la1 in [LITERAL_CountInOut]:
                pass
                tmp192_AST = None
                tmp192_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp192_AST)
                self.match(LITERAL_CountInOut)
                ipc_flag_AST = currentAST.root
            elif la1 and la1 in [LITERAL_auto]:
                pass
                tmp193_AST = None
                tmp193_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp193_AST)
                self.match(LITERAL_auto)
                ipc_flag_AST = currentAST.root
            elif la1 and la1 in [LITERAL_const]:
                pass
                tmp194_AST = None
                tmp194_AST = self.astFactory.create(self.LT(1))
                self.addASTChild(currentAST, tmp194_AST)
                self.match(LITERAL_const)
                ipc_flag_AST = currentAST.root
            else:
                    raise antlr.NoViableAltException(self.LT(1), self.getFilename())
                
        
        except antlr.RecognitionException, ex:
            self.reportError(ex)
            self.consume()
            self.consumeUntil(_tokenSet_19)
        
        self.returnAST = ipc_flag_AST
    
    
    def buildTokenTypeASTClassMap(self):
        self.tokenTypeToASTClassMap = None

_tokenNames = [
    "<0>", 
    "EOF", 
    "<2>", 
    "NULL_TREE_LOOKAHEAD", 
    "SEMI", 
    "\"skip\"", 
    "\"subsystem\"", 
    "\"kerneluser\"", 
    "\"kernelserver\"", 
    "IDENT", 
    "NUMBER", 
    "\"waittime\"", 
    "STRING", 
    "\"msgoption\"", 
    "\"UserTypeLimit\"", 
    "\"OnStackLimit\"", 
    "\"error\"", 
    "\"serverprefix\"", 
    "\"userprefix\"", 
    "\"serverdemux\"", 
    "QUOTEDSTRING", 
    "ANGLEQUOTEDSTRING", 
    "RCSDECL", 
    "\"import\"", 
    "\"uimport\"", 
    "\"simport\"", 
    "\"dimport\"", 
    "\"iimport\"", 
    "\"type\"", 
    "EQUAL", 
    "\"intran\"", 
    "COLON", 
    "LPAREN", 
    "RPAREN", 
    "\"outtran\"", 
    "\"destructor\"", 
    "\"ctype\"", 
    "\"cusertype\"", 
    "\"cservertype\"", 
    "CARET", 
    "\"PointerTo\"", 
    "\"PointerToIfNot\"", 
    "COMMA", 
    "\"ValueOf\"", 
    "\"polymorphic\"", 
    "\"MACH_MSG_TYPE_MOVE_RECEIVE\"", 
    "\"MACH_MSG_TYPE_COPY_SEND\"", 
    "\"MACH_MSG_TYPE_MAKE_SEND\"", 
    "\"MACH_MSG_TYPE_MOVE_SEND\"", 
    "\"MACH_MSG_TYPE_MAKE_SEND_ONCE\"", 
    "\"MACH_MSG_TYPE_MOVE_SEND_ONCE\"", 
    "\"MACH_MSG_TYPE_PORT_NAME\"", 
    "\"MACH_MSG_TYPE_PORT_RECEIVE\"", 
    "\"MACH_MSG_TYPE_PORT_SEND\"", 
    "\"MACH_MSG_TYPE_PORT_SEND_ONCE\"", 
    "\"MACH_MSG_TYPE_POLYMORPHIC\"", 
    "\"MACH_MSG_TYPE_UNSTRUCTURED\"", 
    "\"MACH_MSG_TYPE_BIT\"", 
    "\"MACH_MSG_TYPE_BOOLEAN\"", 
    "\"MACH_MSG_TYPE_INTEGER_8\"", 
    "\"MACH_MSG_TYPE_INTEGER_16\"", 
    "\"MACH_MSG_TYPE_INTEGER_32\"", 
    "\"MACH_MSG_TYPE_INTEGER_64\"", 
    "\"MACH_MSG_TYPE_REAL\"", 
    "\"MACH_MSG_TYPE_REAL_32\"", 
    "\"MACH_MSG_TYPE_REAL_64\"", 
    "\"MACH_MSG_TYPE_CHAR\"", 
    "\"MACH_MSG_TYPE_BYTE\"", 
    "\"MACH_MSG_TYPE_STRING\"", 
    "\"MACH_MSG_TYPE_STRING_C\"", 
    "BAR", 
    "\"array\"", 
    "LBRACK", 
    "RBRACK", 
    "OF", 
    "STAR", 
    "\"struct\"", 
    "\"c_string\"", 
    "PLUS", 
    "MINUS", 
    "DIV", 
    "\"routine\"", 
    "\"simpleroutine\"", 
    "\"in\"", 
    "\"out\"", 
    "\"inout\"", 
    "\"requestport\"", 
    "\"replyport\"", 
    "\"sreplyport\"", 
    "\"ureplyport\"", 
    "\"sendtime\"", 
    "\"sectoken\"", 
    "\"serversectoken\"", 
    "\"usersectoken\"", 
    "\"audittoken\"", 
    "\"serveraudittoken\"", 
    "\"useraudittoken\"", 
    "\"msgseqno\"", 
    "\"serverimpl\"", 
    "\"userimpl\"", 
    "\"samecount\"", 
    "\"retcode\"", 
    "\"physicalcopy\"", 
    "\"servercopy\"", 
    "\"overwrite\"", 
    "\"dealloc\"", 
    "\"notdealloc\"", 
    "\"CountInOut\"", 
    "\"auto\"", 
    "\"const\"", 
    "TILDE", 
    "LANGLE", 
    "RANGLE", 
    "LETTER", 
    "DIGIT", 
    "WS", 
    "COMMENT"
]
    

### generate bit set
def mk_tokenSet_0(): 
    ### var1
    data = [ 533719152L, 393216L, 0L, 0L]
    return data
_tokenSet_0 = antlr.BitSet(mk_tokenSet_0())

### generate bit set
def mk_tokenSet_1(): 
    ### var1
    data = [ 2L, 0L]
    return data
_tokenSet_1 = antlr.BitSet(mk_tokenSet_1())

### generate bit set
def mk_tokenSet_2(): 
    ### var1
    data = [ 533719154L, 393216L, 0L, 0L]
    return data
_tokenSet_2 = antlr.BitSet(mk_tokenSet_2())

### generate bit set
def mk_tokenSet_3(): 
    ### var1
    data = [ 16L, 0L]
    return data
_tokenSet_3 = antlr.BitSet(mk_tokenSet_3())

### generate bit set
def mk_tokenSet_4(): 
    ### var1
    data = [ 512L, 0L]
    return data
_tokenSet_4 = antlr.BitSet(mk_tokenSet_4())

### generate bit set
def mk_tokenSet_5(): 
    ### var1
    data = [ 1024L, 0L]
    return data
_tokenSet_5 = antlr.BitSet(mk_tokenSet_5())

### generate bit set
def mk_tokenSet_6(): 
    ### var1
    data = [ 896L, 0L]
    return data
_tokenSet_6 = antlr.BitSet(mk_tokenSet_6())

### generate bit set
def mk_tokenSet_7(): 
    ### var1
    data = [ 3145728L, 0L]
    return data
_tokenSet_7 = antlr.BitSet(mk_tokenSet_7())

### generate bit set
def mk_tokenSet_8(): 
    ### var1
    data = [ 4406636445712L, 0L]
    return data
_tokenSet_8 = antlr.BitSet(mk_tokenSet_8())

### generate bit set
def mk_tokenSet_9(): 
    ### var1
    data = [ 533649686528L, 0L]
    return data
_tokenSet_9 = antlr.BitSet(mk_tokenSet_9())

### generate bit set
def mk_tokenSet_10(): 
    ### var1
    data = [ 4940286132240L, 0L]
    return data
_tokenSet_10 = antlr.BitSet(mk_tokenSet_10())

### generate bit set
def mk_tokenSet_11(): 
    ### var1
    data = [ 4406636445696L, 0L]
    return data
_tokenSet_11 = antlr.BitSet(mk_tokenSet_11())

### generate bit set
def mk_tokenSet_12(): 
    ### var1
    data = [ 0L, 116736L, 0L, 0L]
    return data
_tokenSet_12 = antlr.BitSet(mk_tokenSet_12())

### generate bit set
def mk_tokenSet_13(): 
    ### var1
    data = [ 4406636445696L, 512L, 0L, 0L]
    return data
_tokenSet_13 = antlr.BitSet(mk_tokenSet_13())

### generate bit set
def mk_tokenSet_14(): 
    ### var1
    data = [ 8589934608L, 0L]
    return data
_tokenSet_14 = antlr.BitSet(mk_tokenSet_14())

### generate bit set
def mk_tokenSet_15(): 
    ### var1
    data = [ 4940286132240L, 64L, 0L, 0L]
    return data
_tokenSet_15 = antlr.BitSet(mk_tokenSet_15())

### generate bit set
def mk_tokenSet_16(): 
    ### var1
    data = [ -4943507356160L, 12479L, 0L, 0L]
    return data
_tokenSet_16 = antlr.BitSet(mk_tokenSet_16())

### generate bit set
def mk_tokenSet_17(): 
    ### var1
    data = [ 4413078896656L, 0L]
    return data
_tokenSet_17 = antlr.BitSet(mk_tokenSet_17())

### generate bit set
def mk_tokenSet_18(): 
    ### var1
    data = [ 8589934592L, 0L]
    return data
_tokenSet_18 = antlr.BitSet(mk_tokenSet_18())

### generate bit set
def mk_tokenSet_19(): 
    ### var1
    data = [ 4406636445712L, 256L, 0L, 0L]
    return data
_tokenSet_19 = antlr.BitSet(mk_tokenSet_19())
    
