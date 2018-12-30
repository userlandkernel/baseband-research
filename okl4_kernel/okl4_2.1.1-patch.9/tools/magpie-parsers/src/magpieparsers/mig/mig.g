/* MiG grammar from CMU's parser.y revision 2.8 converted to Antlr by nfd */

options {
	language = "Python";
}

class MIGParser extends Parser;

options {
	k = 4;
	buildAST = true;
}

/* FIXME: case-insensitive keywords */

statements : (statement)*
	;

statement : subsystem SEMI!
		| wait_time SEMI!
		| msg_option SEMI!
		| user_type_limit SEMI!
		| on_stack_limit SEMI!
		| error SEMI!
		| server_prefix SEMI!
		| user_prefix SEMI!
		| server_demux SEMI!
		| type_decl SEMI!
		| routine_decl SEMI!
		| "skip" SEMI!
		| import_decl SEMI!
		| rcs_decl SEMI!
		| SEMI!
	;

subsystem : "subsystem"! subsystem_mods subsystem_name subsystem_base ;

subsystem_mods : (subsystem_mod)* ;
	
subsystem_mod : "kerneluser" | "kernelserver" ;
	
subsystem_name : IDENT ;
	
subsystem_base : NUMBER ;

wait_time : "waittime"! STRING ;

msg_option : "msgoption"! STRING ;

user_type_limit : "UserTypeLimit"! NUMBER ;

on_stack_limit : "OnStackLimit"! NUMBER ;

error : "error"! IDENT ;

server_prefix : "serverprefix"! IDENT ; 

user_prefix : "userprefix"! IDENT ;

server_demux : "serverdemux"! IDENT ;

import_decl : import_indicant (QUOTEDSTRING | ANGLEQUOTEDSTRING) ;

rcs_decl : RCSDECL;

import_indicant : "import" 
	| "uimport"
	| "simport"
	| "dimport"
	| "iimport"
	;

type_decl : "type"! named_type_spec ; 

named_type_spec : IDENT EQUAL! trans_type_spec ;

trans_type_spec :  type_spec (trans_type_spec_extended)*
	;
	
trans_type_spec_extended : "intran" COLON! IDENT IDENT LPAREN! IDENT RPAREN!
	| "outtran" COLON! IDENT IDENT LPAREN! IDENT RPAREN!
	| "destructor" COLON! IDENT LPAREN! IDENT RPAREN!
	| "ctype" COLON! IDENT
	| "cusertype" COLON! IDENT
	| "cservertype" COLON! IDENT
	; 

type_spec : basic_type_spec
	| prev_type_spec
	| array_spec
	| CARET type_spec
	| struct_head
	| c_string_spec
	| native_type_spec
	;
	
native_type_spec : "PointerTo" LPAREN! type_phrase RPAREN!
	| "PointerToIfNot" LPAREN! type_phrase COMMA! type_phrase RPAREN!
	| "ValueOf" LPAREN! type_phrase RPAREN!
	;

basic_type_spec : ipc_type
	| LPAREN! ipc_type COMMA! int_exp (ipc_flags)? RPAREN!
	;
	
prim_ipc_type : NUMBER
	| symbolic_type
	;
	
symbolic_type : "polymorphic"
	| "MACH_MSG_TYPE_MOVE_RECEIVE"
	| "MACH_MSG_TYPE_COPY_SEND"
	| "MACH_MSG_TYPE_MAKE_SEND"
	| "MACH_MSG_TYPE_MOVE_SEND"
	| "MACH_MSG_TYPE_MAKE_SEND_ONCE"
	| "MACH_MSG_TYPE_MOVE_SEND_ONCE"
	| "MACH_MSG_TYPE_PORT_NAME"
	| "MACH_MSG_TYPE_PORT_RECEIVE"
	| "MACH_MSG_TYPE_PORT_SEND"
	| "MACH_MSG_TYPE_PORT_SEND_ONCE"
	| "MACH_MSG_TYPE_POLYMORPHIC"
	| "MACH_MSG_TYPE_UNSTRUCTURED"
	| "MACH_MSG_TYPE_BIT"	
	| "MACH_MSG_TYPE_BOOLEAN"
	| "MACH_MSG_TYPE_INTEGER_8"
	| "MACH_MSG_TYPE_INTEGER_16"
	| "MACH_MSG_TYPE_INTEGER_32"
	| "MACH_MSG_TYPE_INTEGER_64"
	| "MACH_MSG_TYPE_REAL"
	| "MACH_MSG_TYPE_REAL_32"
	| "MACH_MSG_TYPE_REAL_64"
	| "MACH_MSG_TYPE_CHAR"
	| "MACH_MSG_TYPE_BYTE"
	| "MACH_MSG_TYPE_STRING"
	| "MACH_MSG_TYPE_STRING_C"
	;

ipc_type : prim_ipc_type (BAR! prim_ipc_type)? ;

prev_type_spec : IDENT ; 

array_spec : "array"! LBRACK! (var_array_head | array_head) type_spec
	;

var_array_head: RBRACK! OF!
	| STAR RBRACK! OF!
	| STAR COLON int_exp RBRACK! OF!
	;
		
array_head : int_exp RBRACK! OF! ;

struct_head : "struct"! LBRACK! int_exp RBRACK! OF! type_spec ;

c_string_spec : "c_string"!
		(LBRACK! int_exp RBRACK! | LBRACK! STAR COLON int_exp RBRACK!)
	;
	
type_phrase : (IDENT)+ ;

int_exp : NUMBER
	| NUMBER (PLUS | MINUS | STAR | DIV) int_exp
	| LPAREN int_exp RPAREN
	;

routine_decl : routine
	| simple_routine
	;

routine: "routine"! identifier arguments ;	

simple_routine : "simpleroutine"!  identifier arguments ;

arguments: LPAREN! (argument_list)? RPAREN! ;

argument_list : argument (SEMI! argument_list)?
	| trailer (SEMI! argument_list)?
	;

argument : (direction)? identifier argument_type (ipc_flags)? ;

identifier: IDENT;

trailer : tr_impl_keyword IDENT argument_type ;

direction : "in"
	| "out"
	| "inout"
	| "requestport"
	| "replyport"
	| "sreplyport"
	| "ureplyport"
	| "waittime"
	| "sendtime"
	| "msgoption"
	| "sectoken"
	| "serversectoken"
	| "usersectoken"
	| "audittoken"
	| "serveraudittoken"
	| "useraudittoken"
	| "msgseqno"
	;

tr_impl_keyword : "serverimpl" | "userimpl" ;

argument_type : COLON! (identifier | named_type_spec | native_type_spec) ;

ipc_flags : (COMMA! ipc_flag)+ (LBRACK RBRACK)?;

ipc_flag : "samecount"
	| "retcode"
	| "physicalcopy"
	| "servercopy"
	| "overwrite"
	| "dealloc"
	| "notdealloc"
	| "CountInOut"
	| "auto"
	| "const"
	;
	
class MIGLexer extends Lexer;

options {
	k = 2;
}

COLON : ':' ;
SEMI : ';' ;
COMMA : ',' ;
PLUS : '+' ;
MINUS : '-' ;
STAR : '*' ;
DIV : '/' ;
LPAREN : '(' ;
RPAREN : ')' ;
EQUAL : '=' ;
CARET : '^' ;
TILDE : '~' ;
LANGLE : '<' ;
RANGLE : '>' ;
LBRACK : '[' ;
RBRACK : ']' ;
BAR : '|' ;
OF: "of"; 

IDENT : (LETTER | '_')  (LETTER | DIGIT | '_')* ;
NUMBER : (DIGIT)+ ;
protected LETTER : ('A'..'Z') | ('a' .. 'z') ;
protected DIGIT : '0'..'9' ;
QUOTEDSTRING : ('"' (LETTER | '-' | DIGIT | '.' | '_' | '/')+ '"') ;
ANGLEQUOTEDSTRING : ('<' (LETTER | '-' | DIGIT |  '.' | '_' | '/')+ '>') ;
WS    : ( ' ' | '\r' '\n' | '\n' | '\t')
        {$setType(Token.SKIP);}
      ;
COMMENT : "/*" (options {greedy=false;} :.)* "*/" 
		{ $setType(Token.SKIP); }
	;

