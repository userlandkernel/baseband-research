/* $ANTLR 2.7.6 (20060903): "CPP_parser_cpp.g" -> "CPPParser.cpp"$ */
#include "CPPParser.hpp"
#include <antlr/NoViableAltException.hpp>
#include <antlr/SemanticException.hpp>
#include <antlr/ASTFactory.hpp>
#line 96 "CPP_parser_cpp.g"

//	The statements in this block appear only in CPPParser.cpp and not in CPPLexer.cpp

// DO NOT USE THESE CONSTANTS - THEY WILL BE DELETED IN THE NEXT VERSION
int id_type = ID_VAR_NAME;	// variable type: ID_FUN_NAME, ID_VAR_NAME, ID_INPUT_PARAMETER,
							// ID_CLASS_DEF, ID_SYSTEM_FUNCTION, ID_CONST_DECL, 
							// ID_TYPEDEF_VAR

int statementTrace = 0;	// Used to control selected (level) tracing (see support.cpp)
						// 1 Shows which external and member statements selected
						// 2 Shows above plus all declarations/definitions
						// 3 reserved for future use
						// 4 and above available for user


void CPPParser::init()
{
	//antlrTrace(false);	// This is a dynamic trace facility for use with -traceParser etc.
						// It requires modification in LLkParser.cpp and LLkParser.hpp
						//  otherwise it should be commented out (see MyReadMe.txt)
						// true shows antlr trace (can be set and reset during parsing)
						// false stops showing antlr trace 
						// Provided the parser is always generated with -traceParser this
						//  facility allows trace output to be turned on or off by changing
						//  the setting here from false to true or vice versa and then
						//  recompiling and linking CPPParser only thus avoiding the need
						//  to use antlr.Tool to re-generate the lexer and parser again. 
					
	// Creates a dictionary to hold symbols with 4001 buckets, 200 scopes and 800,000 characters
	// These can be changed to suit the size of program(s) being parsed
	symbols = new CPPDictionary(4001, 200, 800000);

	// Set template parameter and external scopes
	templateParameterScope = symbols->getCurrentScopeIndex();	// Set template parameter scope to 0
	symbols->saveScope();	// Advance currentScope from 0 to 1
	externalScope = symbols->getCurrentScopeIndex();	// Set external scope to 1

	// Declare predefined scope "std" in external scope
	CPPSymbol *a = new CPPSymbol("std", CPPSymbol::otTypedef);
	symbols->define("std", a);

	// Global flags to allow for nested declarations
	_td = false;		// For typedef
	_fd = false;		// For friend
	_sc = scInvalid;	// For StorageClass
	_tq = tqInvalid;	// For TypeQualifier
	_ts = tsInvalid;	// For TypeSpecifier
	_ds = dsInvalid;	// For DeclSpecifier

	functionDefinition = 0;
	qualifierPrefix[0] = '\0';
	enclosingClass = "";
	assign_stmt_RHS_found = 0;
	in_parameter_list = false;
	K_and_R = false;	// used to distinguish old K & R parameter definitions
	in_return = false;
	is_address = false;
	is_pointer = false;

	temp_nodes = NULL;
	temp_nodes_next = temp_nodes_end = 0;
	}

void CPPParser::uninit()
{
	delete symbols;
	free_temp_nodes();
}


#line 78 "CPPParser.cpp"
CPPParser::CPPParser(ANTLR_USE_NAMESPACE(antlr)TokenBuffer& tokenBuf, int k)
: ANTLR_USE_NAMESPACE(antlr)LLkParser(tokenBuf,k)
{
}

CPPParser::CPPParser(ANTLR_USE_NAMESPACE(antlr)TokenBuffer& tokenBuf)
: ANTLR_USE_NAMESPACE(antlr)LLkParser(tokenBuf,2)
{
}

CPPParser::CPPParser(ANTLR_USE_NAMESPACE(antlr)TokenStream& lexer, int k)
: ANTLR_USE_NAMESPACE(antlr)LLkParser(lexer,k)
{
}

CPPParser::CPPParser(ANTLR_USE_NAMESPACE(antlr)TokenStream& lexer)
: ANTLR_USE_NAMESPACE(antlr)LLkParser(lexer,2)
{
}

CPPParser::CPPParser(const ANTLR_USE_NAMESPACE(antlr)ParserSharedInputState& state)
: ANTLR_USE_NAMESPACE(antlr)LLkParser(state,2)
{
}

ASTNode * CPPParser::translation_unit() {
#line 355 "CPP_parser_cpp.g"
	ASTNode *node;
#line 107 "CPPParser.cpp"
#line 355 "CPP_parser_cpp.g"
	ASTNode *ed;
#line 110 "CPPParser.cpp"
	
	try {      // for error handling
		if ( inputState->guessing==0 ) {
#line 357 "CPP_parser_cpp.g"
			enterExternalScope();
#line 116 "CPPParser.cpp"
		}
		if ( inputState->guessing==0 ) {
#line 357 "CPP_parser_cpp.g"
			node = mk_node("translation_unit");
#line 121 "CPPParser.cpp"
		}
		{ // ( ... )+
		int _cnt3=0;
		for (;;) {
			if ((_tokenSet_0.member(LA(1)))) {
				ed=external_declaration();
				if ( inputState->guessing==0 ) {
#line 358 "CPP_parser_cpp.g"
					node->add_child(ed);
#line 131 "CPPParser.cpp"
				}
			}
			else {
				if ( _cnt3>=1 ) { goto _loop3; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());}
			}
			
			_cnt3++;
		}
		_loop3:;
		}  // ( ... )+
		match(ANTLR_USE_NAMESPACE(antlr)Token::EOF_TYPE);
		if ( inputState->guessing==0 ) {
#line 360 "CPP_parser_cpp.g"
			exitExternalScope();
#line 146 "CPPParser.cpp"
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_1);
		} else {
			throw;
		}
	}
	return node;
}

ASTNode * CPPParser::external_declaration() {
#line 363 "CPP_parser_cpp.g"
	ASTNode *node;
#line 163 "CPPParser.cpp"
#line 363 "CPP_parser_cpp.g"
	
		ASTNode *d, *th, *es, *dh, *db, *cd, *fd, *dn, *idl, *cds, *cfdd, *ds;
		const char *s; K_and_R = false;
		
#line 169 "CPPParser.cpp"
	
	try {      // for error handling
		{
		switch ( LA(1)) {
		case LITERAL_namespace:
		{
			dn=decl_namespace();
			if ( inputState->guessing==0 ) {
#line 465 "CPP_parser_cpp.g"
				node = mk_node("unfinished_namespace_decl");
#line 180 "CPPParser.cpp"
			}
			break;
		}
		case SEMICOLON:
		{
			match(SEMICOLON);
			if ( inputState->guessing==0 ) {
#line 470 "CPP_parser_cpp.g"
				end_of_stmt();
#line 190 "CPPParser.cpp"
			}
			break;
		}
		default:
			bool synPredMatched7 = false;
			if (((LA(1) == LITERAL_template) && (LA(2) == LESSTHAN))) {
				int _m7 = mark();
				synPredMatched7 = true;
				inputState->guessing++;
				try {
					{
					match(LITERAL_template);
					match(LESSTHAN);
					match(GREATERTHAN);
					}
				}
				catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
					synPredMatched7 = false;
				}
				rewind(_m7);
				inputState->guessing--;
			}
			if ( synPredMatched7 ) {
				match(LITERAL_template);
				match(LESSTHAN);
				match(GREATERTHAN);
				d=declaration();
				if ( inputState->guessing==0 ) {
#line 374 "CPP_parser_cpp.g"
					node = mk_node("template_specialisation"); node->add_child(d);
#line 221 "CPPParser.cpp"
				}
			}
			else {
				bool synPredMatched10 = false;
				if (((_tokenSet_2.member(LA(1))) && (_tokenSet_3.member(LA(2))))) {
					int _m10 = mark();
					synPredMatched10 = true;
					inputState->guessing++;
					try {
						{
						{
						switch ( LA(1)) {
						case LITERAL_typedef:
						{
							match(LITERAL_typedef);
							break;
						}
						case LITERAL_class:
						case LITERAL_struct:
						case LITERAL_union:
						{
							break;
						}
						default:
						{
							throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
						}
						}
						}
						class_head();
						}
					}
					catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
						synPredMatched10 = false;
					}
					rewind(_m10);
					inputState->guessing--;
				}
				if ( synPredMatched10 ) {
					d=declaration();
					if ( inputState->guessing==0 ) {
#line 386 "CPP_parser_cpp.g"
						node = d;
#line 265 "CPPParser.cpp"
					}
				}
				else {
					bool synPredMatched12 = false;
					if (((LA(1) == LITERAL_template) && (LA(2) == LESSTHAN))) {
						int _m12 = mark();
						synPredMatched12 = true;
						inputState->guessing++;
						try {
							{
							template_head();
							class_head();
							}
						}
						catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
							synPredMatched12 = false;
						}
						rewind(_m12);
						inputState->guessing--;
					}
					if ( synPredMatched12 ) {
						th=template_head();
						d=declaration();
						if ( inputState->guessing==0 ) {
#line 391 "CPP_parser_cpp.g"
							node = mk_node("template"); node->add_child(th); node->add_child(d);
#line 292 "CPPParser.cpp"
						}
					}
					else {
						bool synPredMatched15 = false;
						if (((LA(1) == LITERAL_enum) && (LA(2) == ID || LA(2) == LCURLY))) {
							int _m15 = mark();
							synPredMatched15 = true;
							inputState->guessing++;
							try {
								{
								match(LITERAL_enum);
								{
								switch ( LA(1)) {
								case ID:
								{
									match(ID);
									break;
								}
								case LCURLY:
								{
									break;
								}
								default:
								{
									throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
								}
								}
								}
								match(LCURLY);
								}
							}
							catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
								synPredMatched15 = false;
							}
							rewind(_m15);
							inputState->guessing--;
						}
						if ( synPredMatched15 ) {
							if ( inputState->guessing==0 ) {
#line 395 "CPP_parser_cpp.g"
								node = mk_node("enum");
#line 334 "CPPParser.cpp"
							}
							es=enum_specifier();
							if ( inputState->guessing==0 ) {
#line 396 "CPP_parser_cpp.g"
								node->add_child(es);
#line 340 "CPPParser.cpp"
							}
							{
							switch ( LA(1)) {
							case ID:
							case LITERAL__stdcall:
							case LITERAL___stdcall:
							case LPAREN:
							case OPERATOR:
							case LITERAL_this:
							case LITERAL_true:
							case LITERAL_false:
							case STAR:
							case AMPERSAND:
							case TILDE:
							case SCOPE:
							case LITERAL__cdecl:
							case LITERAL___cdecl:
							case LITERAL__near:
							case LITERAL___near:
							case LITERAL__far:
							case LITERAL___far:
							case LITERAL___interrupt:
							case LITERAL_pascal:
							case LITERAL__pascal:
							case LITERAL___pascal:
							{
								idl=init_declarator_list();
								if ( inputState->guessing==0 ) {
#line 397 "CPP_parser_cpp.g"
									node->add_child(idl);
#line 371 "CPPParser.cpp"
								}
								break;
							}
							case SEMICOLON:
							{
								break;
							}
							default:
							{
								throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
							}
							}
							}
							match(SEMICOLON);
							if ( inputState->guessing==0 ) {
#line 398 "CPP_parser_cpp.g"
								end_of_stmt();
#line 389 "CPPParser.cpp"
							}
						}
						else {
							bool synPredMatched19 = false;
							if (((_tokenSet_4.member(LA(1))) && (_tokenSet_5.member(LA(2))))) {
								int _m19 = mark();
								synPredMatched19 = true;
								inputState->guessing++;
								try {
									{
									{
									switch ( LA(1)) {
									case LITERAL_template:
									{
										template_head();
										break;
									}
									case ID:
									case LITERAL_inline:
									case LITERAL__inline:
									case LITERAL___inline:
									case LITERAL_virtual:
									case TILDE:
									case SCOPE:
									{
										break;
									}
									default:
									{
										throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
									}
									}
									}
									dtor_head(1);
									match(LCURLY);
									}
								}
								catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
									synPredMatched19 = false;
								}
								rewind(_m19);
								inputState->guessing--;
							}
							if ( synPredMatched19 ) {
								{
								switch ( LA(1)) {
								case LITERAL_template:
								{
									th=template_head();
									break;
								}
								case ID:
								case LITERAL_inline:
								case LITERAL__inline:
								case LITERAL___inline:
								case LITERAL_virtual:
								case TILDE:
								case SCOPE:
								{
									break;
								}
								default:
								{
									throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
								}
								}
								}
								dh=dtor_head(1);
								db=dtor_body();
								if ( inputState->guessing==0 ) {
#line 403 "CPP_parser_cpp.g"
									node = mk_node("unfinished_destructor");
#line 462 "CPPParser.cpp"
								}
							}
							else {
								bool synPredMatched23 = false;
								if (((_tokenSet_6.member(LA(1))) && (_tokenSet_7.member(LA(2))))) {
									int _m23 = mark();
									synPredMatched23 = true;
									inputState->guessing++;
									try {
										{
										{
										if ((true) && (true)) {
											cds=ctor_decl_spec();
										}
										else {
										}
										
										}
										if (!(qualifiedItemIsOneOf(qiCtor)))
											throw ANTLR_USE_NAMESPACE(antlr)SemanticException("qualifiedItemIsOneOf(qiCtor)");
										}
									}
									catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
										synPredMatched23 = false;
									}
									rewind(_m23);
									inputState->guessing--;
								}
								if ( synPredMatched23 ) {
									cd=ctor_definition();
									if ( inputState->guessing==0 ) {
#line 414 "CPP_parser_cpp.g"
										node = mk_node("unfinished_non-templated constructor");
#line 496 "CPPParser.cpp"
									}
								}
								else {
									bool synPredMatched26 = false;
									if (((_tokenSet_8.member(LA(1))) && (_tokenSet_9.member(LA(2))))) {
										int _m26 = mark();
										synPredMatched26 = true;
										inputState->guessing++;
										try {
											{
											{
											switch ( LA(1)) {
											case LITERAL_inline:
											{
												match(LITERAL_inline);
												break;
											}
											case ID:
											case OPERATOR:
											case SCOPE:
											{
												break;
											}
											default:
											{
												throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
											}
											}
											}
											scope_override();
											conversion_function_decl_or_def();
											}
										}
										catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
											synPredMatched26 = false;
										}
										rewind(_m26);
										inputState->guessing--;
									}
									if ( synPredMatched26 ) {
										{
										switch ( LA(1)) {
										case LITERAL_inline:
										{
											match(LITERAL_inline);
											break;
										}
										case ID:
										case OPERATOR:
										case SCOPE:
										{
											break;
										}
										default:
										{
											throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
										}
										}
										}
										s=scope_override();
										cfdd=conversion_function_decl_or_def();
										if ( inputState->guessing==0 ) {
#line 419 "CPP_parser_cpp.g"
											node = mk_node("unfinished_typecast");
#line 561 "CPPParser.cpp"
										}
									}
									else {
										bool synPredMatched29 = false;
										if (((_tokenSet_2.member(LA(1))) && (_tokenSet_3.member(LA(2))))) {
											int _m29 = mark();
											synPredMatched29 = true;
											inputState->guessing++;
											try {
												{
												declaration_specifiers();
												function_declarator(0);
												match(SEMICOLON);
												}
											}
											catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
												synPredMatched29 = false;
											}
											rewind(_m29);
											inputState->guessing--;
										}
										if ( synPredMatched29 ) {
											d=declaration();
											if ( inputState->guessing==0 ) {
#line 424 "CPP_parser_cpp.g"
												node = d;
#line 588 "CPPParser.cpp"
											}
										}
										else {
											bool synPredMatched31 = false;
											if (((_tokenSet_10.member(LA(1))) && (_tokenSet_11.member(LA(2))))) {
												int _m31 = mark();
												synPredMatched31 = true;
												inputState->guessing++;
												try {
													{
													declaration_specifiers();
													function_declarator(1);
													match(LCURLY);
													}
												}
												catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
													synPredMatched31 = false;
												}
												rewind(_m31);
												inputState->guessing--;
											}
											if ( synPredMatched31 ) {
												fd=function_definition();
												if ( inputState->guessing==0 ) {
#line 428 "CPP_parser_cpp.g"
													node = fd;
#line 615 "CPPParser.cpp"
												}
											}
											else {
												bool synPredMatched33 = false;
												if (((_tokenSet_10.member(LA(1))) && (_tokenSet_11.member(LA(2))))) {
													int _m33 = mark();
													synPredMatched33 = true;
													inputState->guessing++;
													try {
														{
														declaration_specifiers();
														function_declarator(1);
														declaration();
														}
													}
													catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
														synPredMatched33 = false;
													}
													rewind(_m33);
													inputState->guessing--;
												}
												if ( synPredMatched33 ) {
													if ( inputState->guessing==0 ) {
#line 432 "CPP_parser_cpp.g"
														K_and_R = true;
#line 641 "CPPParser.cpp"
													}
													fd=function_definition();
													if ( inputState->guessing==0 ) {
#line 433 "CPP_parser_cpp.g"
														node = fd;
#line 647 "CPPParser.cpp"
													}
												}
												else {
													bool synPredMatched36 = false;
													if (((LA(1) == LITERAL_template) && (LA(2) == LESSTHAN))) {
														int _m36 = mark();
														synPredMatched36 = true;
														inputState->guessing++;
														try {
															{
															template_head();
															declaration_specifiers();
															{
															switch ( LA(1)) {
															case ID:
															case LITERAL__stdcall:
															case LITERAL___stdcall:
															case LPAREN:
															case OPERATOR:
															case LITERAL_this:
															case LITERAL_true:
															case LITERAL_false:
															case STAR:
															case AMPERSAND:
															case TILDE:
															case SCOPE:
															case LITERAL__cdecl:
															case LITERAL___cdecl:
															case LITERAL__near:
															case LITERAL___near:
															case LITERAL__far:
															case LITERAL___far:
															case LITERAL___interrupt:
															case LITERAL_pascal:
															case LITERAL__pascal:
															case LITERAL___pascal:
															{
																init_declarator_list();
																break;
															}
															case SEMICOLON:
															{
																break;
															}
															default:
															{
																throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
															}
															}
															}
															match(SEMICOLON);
															if ( inputState->guessing==0 ) {
#line 436 "CPP_parser_cpp.g"
																end_of_stmt();
#line 702 "CPPParser.cpp"
															}
															}
														}
														catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
															synPredMatched36 = false;
														}
														rewind(_m36);
														inputState->guessing--;
													}
													if ( synPredMatched36 ) {
														if ( inputState->guessing==0 ) {
#line 437 "CPP_parser_cpp.g"
															beginTemplateDeclaration();
#line 716 "CPPParser.cpp"
														}
														th=template_head();
														ds=declaration_specifiers();
														{
														switch ( LA(1)) {
														case ID:
														case LITERAL__stdcall:
														case LITERAL___stdcall:
														case LPAREN:
														case OPERATOR:
														case LITERAL_this:
														case LITERAL_true:
														case LITERAL_false:
														case STAR:
														case AMPERSAND:
														case TILDE:
														case SCOPE:
														case LITERAL__cdecl:
														case LITERAL___cdecl:
														case LITERAL__near:
														case LITERAL___near:
														case LITERAL__far:
														case LITERAL___far:
														case LITERAL___interrupt:
														case LITERAL_pascal:
														case LITERAL__pascal:
														case LITERAL___pascal:
														{
															idl=init_declarator_list();
															break;
														}
														case SEMICOLON:
														{
															break;
														}
														default:
														{
															throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
														}
														}
														}
														match(SEMICOLON);
														if ( inputState->guessing==0 ) {
#line 438 "CPP_parser_cpp.g"
															end_of_stmt();
#line 762 "CPPParser.cpp"
														}
														if ( inputState->guessing==0 ) {
#line 439 "CPP_parser_cpp.g"
															endTemplateDeclaration();
#line 767 "CPPParser.cpp"
														}
														if ( inputState->guessing==0 ) {
#line 440 "CPP_parser_cpp.g"
															node = mk_node("unfinished_templated_forward_class_decl");
#line 772 "CPPParser.cpp"
														}
													}
													else if ((LA(1) == LITERAL_template) && (LA(2) == LESSTHAN)) {
														if ( inputState->guessing==0 ) {
#line 443 "CPP_parser_cpp.g"
															beginTemplateDefinition();
#line 779 "CPPParser.cpp"
														}
														th=template_head();
														if ( inputState->guessing==0 ) {
#line 445 "CPP_parser_cpp.g"
															node = mk_node("templated_function");
#line 785 "CPPParser.cpp"
														}
														{
														bool synPredMatched40 = false;
														if (((_tokenSet_6.member(LA(1))) && (_tokenSet_7.member(LA(2))))) {
															int _m40 = mark();
															synPredMatched40 = true;
															inputState->guessing++;
															try {
																{
																ctor_decl_spec();
																if (!(qualifiedItemIsOneOf(qiCtor)))
																	throw ANTLR_USE_NAMESPACE(antlr)SemanticException("qualifiedItemIsOneOf(qiCtor)");
																}
															}
															catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
																synPredMatched40 = false;
															}
															rewind(_m40);
															inputState->guessing--;
														}
														if ( synPredMatched40 ) {
															cd=ctor_definition();
															if ( inputState->guessing==0 ) {
#line 451 "CPP_parser_cpp.g"
																node->add_child(cd);
#line 811 "CPPParser.cpp"
															}
														}
														else {
															bool synPredMatched42 = false;
															if (((_tokenSet_2.member(LA(1))) && (_tokenSet_3.member(LA(2))))) {
																int _m42 = mark();
																synPredMatched42 = true;
																inputState->guessing++;
																try {
																	{
																	declaration_specifiers();
																	function_declarator(0);
																	match(SEMICOLON);
																	}
																}
																catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
																	synPredMatched42 = false;
																}
																rewind(_m42);
																inputState->guessing--;
															}
															if ( synPredMatched42 ) {
																d=declaration();
																if ( inputState->guessing==0 ) {
#line 456 "CPP_parser_cpp.g"
																	node->add_child(d);
#line 838 "CPPParser.cpp"
																}
															}
															else {
																bool synPredMatched44 = false;
																if (((_tokenSet_10.member(LA(1))) && (_tokenSet_11.member(LA(2))))) {
																	int _m44 = mark();
																	synPredMatched44 = true;
																	inputState->guessing++;
																	try {
																		{
																		declaration_specifiers();
																		function_declarator(1);
																		match(LCURLY);
																		}
																	}
																	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
																		synPredMatched44 = false;
																	}
																	rewind(_m44);
																	inputState->guessing--;
																}
																if ( synPredMatched44 ) {
																	fd=function_definition();
																	if ( inputState->guessing==0 ) {
#line 460 "CPP_parser_cpp.g"
																		node->add_child(fd);
#line 865 "CPPParser.cpp"
																	}
																}
														else {
															throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
														}
														}}
														}
														if ( inputState->guessing==0 ) {
#line 462 "CPP_parser_cpp.g"
															endTemplateDefinition();
#line 876 "CPPParser.cpp"
														}
													}
													else if ((_tokenSet_2.member(LA(1))) && (_tokenSet_3.member(LA(2)))) {
														d=declaration();
														if ( inputState->guessing==0 ) {
#line 468 "CPP_parser_cpp.g"
															node = d;
#line 884 "CPPParser.cpp"
														}
													}
		else {
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}}}}}}}}}}}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_12);
		} else {
			throw;
		}
	}
	return node;
}

ASTNode * CPPParser::declaration() {
#line 679 "CPP_parser_cpp.g"
	ASTNode *node;
#line 907 "CPPParser.cpp"
#line 679 "CPP_parser_cpp.g"
	
		ASTNode *a, *idl, *ud, *ds, *ls;
		
#line 912 "CPPParser.cpp"
	
	try {      // for error handling
		bool synPredMatched104 = false;
		if (((LA(1) == LITERAL_extern) && (LA(2) == StringLiteral))) {
			int _m104 = mark();
			synPredMatched104 = true;
			inputState->guessing++;
			try {
				{
				match(LITERAL_extern);
				match(StringLiteral);
				}
			}
			catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
				synPredMatched104 = false;
			}
			rewind(_m104);
			inputState->guessing--;
		}
		if ( synPredMatched104 ) {
			ls=linkage_specification();
			if ( inputState->guessing==0 ) {
#line 685 "CPP_parser_cpp.g"
				node = ls;
#line 937 "CPPParser.cpp"
			}
		}
		else if ((_tokenSet_13.member(LA(1))) && (_tokenSet_14.member(LA(2)))) {
			if ( inputState->guessing==0 ) {
#line 687 "CPP_parser_cpp.g"
				beginDeclaration(); node = mk_node("declaration");
#line 944 "CPPParser.cpp"
			}
			ds=declaration_specifiers();
			if ( inputState->guessing==0 ) {
#line 689 "CPP_parser_cpp.g"
				node->add_child(ds);
#line 950 "CPPParser.cpp"
			}
			{
			switch ( LA(1)) {
			case ID:
			case COMMA:
			case LITERAL__stdcall:
			case LITERAL___stdcall:
			case LITERAL___attribute__:
			case LPAREN:
			case OPERATOR:
			case LITERAL_this:
			case LITERAL_true:
			case LITERAL_false:
			case STAR:
			case AMPERSAND:
			case TILDE:
			case SCOPE:
			case LITERAL__cdecl:
			case LITERAL___cdecl:
			case LITERAL__near:
			case LITERAL___near:
			case LITERAL__far:
			case LITERAL___far:
			case LITERAL___interrupt:
			case LITERAL_pascal:
			case LITERAL__pascal:
			case LITERAL___pascal:
			{
				{
				switch ( LA(1)) {
				case COMMA:
				{
					match(COMMA);
					break;
				}
				case ID:
				case LITERAL__stdcall:
				case LITERAL___stdcall:
				case LITERAL___attribute__:
				case LPAREN:
				case OPERATOR:
				case LITERAL_this:
				case LITERAL_true:
				case LITERAL_false:
				case STAR:
				case AMPERSAND:
				case TILDE:
				case SCOPE:
				case LITERAL__cdecl:
				case LITERAL___cdecl:
				case LITERAL__near:
				case LITERAL___near:
				case LITERAL__far:
				case LITERAL___far:
				case LITERAL___interrupt:
				case LITERAL_pascal:
				case LITERAL__pascal:
				case LITERAL___pascal:
				{
					break;
				}
				default:
				{
					throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
				}
				}
				}
				{
				switch ( LA(1)) {
				case LITERAL___attribute__:
				{
					a=attribute();
					if ( inputState->guessing==0 ) {
#line 691 "CPP_parser_cpp.g"
						node->add_child(a);
#line 1026 "CPPParser.cpp"
					}
					break;
				}
				case ID:
				case LITERAL__stdcall:
				case LITERAL___stdcall:
				case LPAREN:
				case OPERATOR:
				case LITERAL_this:
				case LITERAL_true:
				case LITERAL_false:
				case STAR:
				case AMPERSAND:
				case TILDE:
				case SCOPE:
				case LITERAL__cdecl:
				case LITERAL___cdecl:
				case LITERAL__near:
				case LITERAL___near:
				case LITERAL__far:
				case LITERAL___far:
				case LITERAL___interrupt:
				case LITERAL_pascal:
				case LITERAL__pascal:
				case LITERAL___pascal:
				{
					break;
				}
				default:
				{
					throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
				}
				}
				}
				idl=init_declarator_list();
				if ( inputState->guessing==0 ) {
#line 692 "CPP_parser_cpp.g"
					node->add_child(idl);
#line 1065 "CPPParser.cpp"
				}
				break;
			}
			case SEMICOLON:
			{
				break;
			}
			default:
			{
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
			}
			}
			}
			match(SEMICOLON);
			if ( inputState->guessing==0 ) {
#line 693 "CPP_parser_cpp.g"
				end_of_stmt();
#line 1083 "CPPParser.cpp"
			}
			if ( inputState->guessing==0 ) {
#line 694 "CPP_parser_cpp.g"
				endDeclaration();
#line 1088 "CPPParser.cpp"
			}
		}
		else if ((LA(1) == LITERAL_using)) {
			ud=using_declaration();
			if ( inputState->guessing==0 ) {
#line 697 "CPP_parser_cpp.g"
				node = ud;
#line 1096 "CPPParser.cpp"
			}
		}
		else {
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_15);
		} else {
			throw;
		}
	}
	return node;
}

ASTNode * CPPParser::class_head() {
#line 1056 "CPP_parser_cpp.g"
	ASTNode *node;
#line 1118 "CPPParser.cpp"
	ANTLR_USE_NAMESPACE(antlr)RefToken  id = ANTLR_USE_NAMESPACE(antlr)nullToken;
#line 1056 "CPP_parser_cpp.g"
	
		ASTNode *tal, *bc;
		node = mk_node("class_head") ;
		
#line 1125 "CPPParser.cpp"
	
	try {      // for error handling
		{
		switch ( LA(1)) {
		case LITERAL_struct:
		{
			match(LITERAL_struct);
			if ( inputState->guessing==0 ) {
#line 1062 "CPP_parser_cpp.g"
				node->add_child(mk_node("struct"));
#line 1136 "CPPParser.cpp"
			}
			break;
		}
		case LITERAL_union:
		{
			match(LITERAL_union);
			if ( inputState->guessing==0 ) {
#line 1063 "CPP_parser_cpp.g"
				node->add_child(mk_node("union"));
#line 1146 "CPPParser.cpp"
			}
			break;
		}
		case LITERAL_class:
		{
			match(LITERAL_class);
			if ( inputState->guessing==0 ) {
#line 1064 "CPP_parser_cpp.g"
				node->add_child(mk_node("class"));
#line 1156 "CPPParser.cpp"
			}
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		{
		switch ( LA(1)) {
		case ID:
		{
			id = LT(1);
			match(ID);
			if ( inputState->guessing==0 ) {
#line 1066 "CPP_parser_cpp.g"
				node->set_leaf(id->getText().data());
#line 1175 "CPPParser.cpp"
			}
			{
			switch ( LA(1)) {
			case LESSTHAN:
			{
				match(LESSTHAN);
				tal=template_argument_list();
				if ( inputState->guessing==0 ) {
#line 1067 "CPP_parser_cpp.g"
					node->add_child(tal);
#line 1186 "CPPParser.cpp"
				}
				match(GREATERTHAN);
				break;
			}
			case LCURLY:
			case COLON:
			{
				break;
			}
			default:
			{
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
			}
			}
			}
			{
			switch ( LA(1)) {
			case COLON:
			{
				bc=base_clause();
				if ( inputState->guessing==0 ) {
#line 1068 "CPP_parser_cpp.g"
					node->add_child(bc);
#line 1210 "CPPParser.cpp"
				}
				break;
			}
			case LCURLY:
			{
				break;
			}
			default:
			{
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
			}
			}
			}
			break;
		}
		case LCURLY:
		{
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		match(LCURLY);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_1);
		} else {
			throw;
		}
	}
	return node;
}

ASTNode * CPPParser::template_head() {
#line 1574 "CPP_parser_cpp.g"
	ASTNode *node;
#line 1252 "CPPParser.cpp"
#line 1574 "CPP_parser_cpp.g"
	
		ASTNode *tpl;
		node = mk_node("template_head");
		
#line 1258 "CPPParser.cpp"
	
	try {      // for error handling
		match(LITERAL_template);
		match(LESSTHAN);
		tpl=template_parameter_list();
		if ( inputState->guessing==0 ) {
#line 1581 "CPP_parser_cpp.g"
			node->add_child(tpl);
#line 1267 "CPPParser.cpp"
		}
		match(GREATERTHAN);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_16);
		} else {
			throw;
		}
	}
	return node;
}

ASTNode * CPPParser::enum_specifier() {
#line 923 "CPP_parser_cpp.g"
	ASTNode *node;
#line 1285 "CPPParser.cpp"
	ANTLR_USE_NAMESPACE(antlr)RefToken  id = ANTLR_USE_NAMESPACE(antlr)nullToken;
#line 923 "CPP_parser_cpp.g"
	
		ASTNode *el;
		node = mk_node("enum_specifier");
		
#line 1292 "CPPParser.cpp"
	
	try {      // for error handling
		match(LITERAL_enum);
		{
		switch ( LA(1)) {
		case LCURLY:
		{
			match(LCURLY);
			el=enumerator_list();
			if ( inputState->guessing==0 ) {
#line 929 "CPP_parser_cpp.g"
				node->add_child(el);
#line 1305 "CPPParser.cpp"
			}
			match(RCURLY);
			break;
		}
		case ID:
		{
			id = LT(1);
			match(ID);
			if ( inputState->guessing==0 ) {
#line 930 "CPP_parser_cpp.g"
				node->set_leaf(id->getText().data());
#line 1317 "CPPParser.cpp"
			}
			if ( inputState->guessing==0 ) {
#line 932 "CPP_parser_cpp.g"
				beginEnumDefinition((id->getText()).data());
#line 1322 "CPPParser.cpp"
			}
			{
			switch ( LA(1)) {
			case LCURLY:
			{
				match(LCURLY);
				el=enumerator_list();
				if ( inputState->guessing==0 ) {
#line 933 "CPP_parser_cpp.g"
					node->add_child(el);
#line 1333 "CPPParser.cpp"
				}
				match(RCURLY);
				break;
			}
			case LESSTHAN:
			case GREATERTHAN:
			case ID:
			case SEMICOLON:
			case RCURLY:
			case ASSIGNEQUAL:
			case COLON:
			case COMMA:
			case LITERAL__stdcall:
			case LITERAL___stdcall:
			case LITERAL___attribute__:
			case LPAREN:
			case RPAREN:
			case OPERATOR:
			case LITERAL_this:
			case LITERAL_true:
			case LITERAL_false:
			case STAR:
			case AMPERSAND:
			case LSQUARE:
			case RSQUARE:
			case TILDE:
			case ELLIPSIS:
			case SCOPE:
			case TIMESEQUAL:
			case DIVIDEEQUAL:
			case MINUSEQUAL:
			case PLUSEQUAL:
			case MODEQUAL:
			case SHIFTLEFTEQUAL:
			case SHIFTRIGHTEQUAL:
			case BITWISEANDEQUAL:
			case BITWISEXOREQUAL:
			case BITWISEOREQUAL:
			case QUESTIONMARK:
			case OR:
			case AND:
			case BITWISEOR:
			case BITWISEXOR:
			case NOTEQUAL:
			case EQUAL:
			case LESSTHANOREQUALTO:
			case GREATERTHANOREQUALTO:
			case SHIFTLEFT:
			case SHIFTRIGHT:
			case PLUS:
			case MINUS:
			case DIVIDE:
			case MOD:
			case DOTMBR:
			case POINTERTOMBR:
			case LITERAL__cdecl:
			case LITERAL___cdecl:
			case LITERAL__near:
			case LITERAL___near:
			case LITERAL__far:
			case LITERAL___far:
			case LITERAL___interrupt:
			case LITERAL_pascal:
			case LITERAL__pascal:
			case LITERAL___pascal:
			{
				break;
			}
			default:
			{
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
			}
			}
			}
			if ( inputState->guessing==0 ) {
#line 934 "CPP_parser_cpp.g"
				endEnumDefinition();
#line 1411 "CPPParser.cpp"
			}
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_17);
		} else {
			throw;
		}
	}
	return node;
}

ASTNode * CPPParser::init_declarator_list() {
#line 998 "CPP_parser_cpp.g"
	ASTNode *node;
#line 1436 "CPPParser.cpp"
#line 998 "CPP_parser_cpp.g"
	
		ASTNode *id, *a;
		node = mk_node("init_declarator_list");
		
#line 1442 "CPPParser.cpp"
	
	try {      // for error handling
		id=init_declarator();
		if ( inputState->guessing==0 ) {
#line 1003 "CPP_parser_cpp.g"
			node->add_child(id);
#line 1449 "CPPParser.cpp"
		}
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == COMMA)) {
				match(COMMA);
				id=init_declarator();
				if ( inputState->guessing==0 ) {
#line 1004 "CPP_parser_cpp.g"
					node->add_child(id);
#line 1459 "CPPParser.cpp"
				}
			}
			else {
				goto _loop165;
			}
			
		}
		_loop165:;
		} // ( ... )*
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == LITERAL___attribute__)) {
				a=attribute();
				if ( inputState->guessing==0 ) {
#line 1005 "CPP_parser_cpp.g"
					node->add_child(a);
#line 1476 "CPPParser.cpp"
				}
			}
			else {
				goto _loop167;
			}
			
		}
		_loop167:;
		} // ( ... )*
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_18);
		} else {
			throw;
		}
	}
	return node;
}

ASTNode * CPPParser::dtor_head(
	int definition
) {
#line 1418 "CPP_parser_cpp.g"
	ASTNode *node;
#line 1503 "CPPParser.cpp"
#line 1418 "CPP_parser_cpp.g"
	
		ASTNode *dds, *dd;
		node = mk_node("dtor_head");
		
#line 1509 "CPPParser.cpp"
	
	try {      // for error handling
		dds=dtor_decl_spec();
		if ( inputState->guessing==0 ) {
#line 1424 "CPP_parser_cpp.g"
			node->add_child(dds);
#line 1516 "CPPParser.cpp"
		}
		dd=dtor_declarator(definition);
		if ( inputState->guessing==0 ) {
#line 1425 "CPP_parser_cpp.g"
			node->add_child(dd);
#line 1522 "CPPParser.cpp"
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_19);
		} else {
			throw;
		}
	}
	return node;
}

ASTNode * CPPParser::dtor_body() {
#line 1452 "CPP_parser_cpp.g"
	ASTNode *node;
#line 1539 "CPPParser.cpp"
#line 1452 "CPP_parser_cpp.g"
	ASTNode *cs;
		node = mk_node("dtor_body");
		
#line 1544 "CPPParser.cpp"
	
	try {      // for error handling
		cs=compound_statement();
		if ( inputState->guessing==0 ) {
#line 1457 "CPP_parser_cpp.g"
			node->add_child(cs);
#line 1551 "CPPParser.cpp"
		}
		if ( inputState->guessing==0 ) {
#line 1458 "CPP_parser_cpp.g"
			endDestructorDefinition();
#line 1556 "CPPParser.cpp"
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_20);
		} else {
			throw;
		}
	}
	return node;
}

ASTNode * CPPParser::ctor_decl_spec() {
#line 1354 "CPP_parser_cpp.g"
	ASTNode *node;
#line 1573 "CPPParser.cpp"
#line 1354 "CPP_parser_cpp.g"
	
		node = mk_node("ctor_decl_spec");
		
#line 1578 "CPPParser.cpp"
	
	try {      // for error handling
		{ // ( ... )*
		for (;;) {
			switch ( LA(1)) {
			case LITERAL_inline:
			case LITERAL__inline:
			case LITERAL___inline:
			{
				{
				switch ( LA(1)) {
				case LITERAL_inline:
				{
					match(LITERAL_inline);
					break;
				}
				case LITERAL__inline:
				{
					match(LITERAL__inline);
					break;
				}
				case LITERAL___inline:
				{
					match(LITERAL___inline);
					break;
				}
				default:
				{
					throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
				}
				}
				}
				if ( inputState->guessing==0 ) {
#line 1359 "CPP_parser_cpp.g"
					node->add_child(mk_node("inline"));
#line 1614 "CPPParser.cpp"
				}
				break;
			}
			case LITERAL_explicit:
			{
				match(LITERAL_explicit);
				if ( inputState->guessing==0 ) {
#line 1360 "CPP_parser_cpp.g"
					node->add_child(mk_node("explicit"));
#line 1624 "CPPParser.cpp"
				}
				break;
			}
			default:
			{
				goto _loop259;
			}
			}
		}
		_loop259:;
		} // ( ... )*
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_21);
		} else {
			throw;
		}
	}
	return node;
}

ASTNode * CPPParser::ctor_definition() {
#line 1334 "CPP_parser_cpp.g"
	ASTNode *node;
#line 1651 "CPPParser.cpp"
#line 1334 "CPP_parser_cpp.g"
	ASTNode *ch, *cb;
		node = mk_node("ctor_definition");
		
#line 1656 "CPPParser.cpp"
	
	try {      // for error handling
		ch=ctor_head();
		if ( inputState->guessing==0 ) {
#line 1339 "CPP_parser_cpp.g"
			node->add_child(ch);
#line 1663 "CPPParser.cpp"
		}
		cb=ctor_body();
		if ( inputState->guessing==0 ) {
#line 1340 "CPP_parser_cpp.g"
			node->add_child(cb);
#line 1669 "CPPParser.cpp"
		}
		if ( inputState->guessing==0 ) {
#line 1341 "CPP_parser_cpp.g"
			endConstructorDefinition();
#line 1674 "CPPParser.cpp"
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_20);
		} else {
			throw;
		}
	}
	return node;
}

char * CPPParser::scope_override() {
#line 2375 "CPP_parser_cpp.g"
	char *s;
#line 1691 "CPPParser.cpp"
	ANTLR_USE_NAMESPACE(antlr)RefToken  id = ANTLR_USE_NAMESPACE(antlr)nullToken;
#line 2375 "CPP_parser_cpp.g"
	
		ASTNode *tal;
		static char sitem[CPPParser_MaxQualifiedItemSize+1];
		sitem[0]='\0';
		
#line 1699 "CPPParser.cpp"
	
	try {      // for error handling
		{
		switch ( LA(1)) {
		case SCOPE:
		{
			match(SCOPE);
			if ( inputState->guessing==0 ) {
#line 2383 "CPP_parser_cpp.g"
				strcat(sitem,"::");
#line 1710 "CPPParser.cpp"
			}
			break;
		}
		case ID:
		case OPERATOR:
		case LITERAL_this:
		case LITERAL_true:
		case LITERAL_false:
		case STAR:
		case TILDE:
		{
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		{ // ( ... )*
		for (;;) {
			if (((LA(1) == ID) && (LA(2) == LESSTHAN || LA(2) == SCOPE))&&(scopedItem())) {
				id = LT(1);
				match(ID);
				{
				switch ( LA(1)) {
				case LESSTHAN:
				{
					match(LESSTHAN);
					tal=template_argument_list();
					match(GREATERTHAN);
					break;
				}
				case SCOPE:
				{
					break;
				}
				default:
				{
					throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
				}
				}
				}
				match(SCOPE);
				if ( inputState->guessing==0 ) {
#line 2387 "CPP_parser_cpp.g"
					
								//printf("scope_override entered\n");
								strcat(sitem,(id->getText()).data());
								strcat(sitem,"::");
								
#line 1762 "CPPParser.cpp"
				}
			}
			else {
				goto _loop495;
			}
			
		}
		_loop495:;
		} // ( ... )*
		if ( inputState->guessing==0 ) {
#line 2393 "CPP_parser_cpp.g"
			s = sitem;
#line 1775 "CPPParser.cpp"
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_22);
		} else {
			throw;
		}
	}
	return s;
}

ASTNode * CPPParser::conversion_function_decl_or_def() {
#line 1125 "CPP_parser_cpp.g"
	ASTNode *node;
#line 1792 "CPPParser.cpp"
#line 1125 "CPP_parser_cpp.g"
	
			char *tq;
			ASTNode *ds, *tpl, *pl, *es, *cs;
			node = mk_node("unfinished_conversion_function_decl_or_def");
			
#line 1799 "CPPParser.cpp"
	
	try {      // for error handling
		match(OPERATOR);
		ds=declaration_specifiers();
		{
		switch ( LA(1)) {
		case STAR:
		{
			match(STAR);
			break;
		}
		case AMPERSAND:
		{
			match(AMPERSAND);
			break;
		}
		case LESSTHAN:
		case LPAREN:
		{
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		{
		switch ( LA(1)) {
		case LESSTHAN:
		{
			match(LESSTHAN);
			tpl=template_parameter_list();
			match(GREATERTHAN);
			break;
		}
		case LPAREN:
		{
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		match(LPAREN);
		{
		switch ( LA(1)) {
		case LITERAL_typedef:
		case LITERAL_enum:
		case ID:
		case LITERAL_inline:
		case LITERAL_extern:
		case LITERAL___extension__:
		case LITERAL__inline:
		case LITERAL___inline:
		case LITERAL___inline__:
		case LITERAL_virtual:
		case LITERAL_explicit:
		case LITERAL_friend:
		case LITERAL__stdcall:
		case LITERAL___stdcall:
		case LITERAL_typename:
		case LPAREN:
		case LITERAL_const:
		case LITERAL___const:
		case LITERAL_auto:
		case LITERAL_register:
		case LITERAL_static:
		case LITERAL_mutable:
		case LITERAL_const_cast:
		case LITERAL_volatile:
		case LITERAL___volatile:
		case LITERAL___restrict:
		case LITERAL___restrict__:
		case LITERAL_char:
		case LITERAL_bool:
		case LITERAL_short:
		case LITERAL_int:
		case 73:
		case 74:
		case 75:
		case LITERAL_long:
		case LITERAL_signed:
		case LITERAL___signed:
		case LITERAL___signed__:
		case LITERAL_unsigned:
		case LITERAL_float:
		case LITERAL_double:
		case LITERAL_void:
		case LITERAL__declspec:
		case LITERAL___declspec:
		case LITERAL___builtin_va_list:
		case LITERAL_class:
		case LITERAL_struct:
		case LITERAL_union:
		case OPERATOR:
		case LITERAL_this:
		case LITERAL_true:
		case LITERAL_false:
		case STAR:
		case AMPERSAND:
		case TILDE:
		case ELLIPSIS:
		case SCOPE:
		case LITERAL__cdecl:
		case LITERAL___cdecl:
		case LITERAL__near:
		case LITERAL___near:
		case LITERAL__far:
		case LITERAL___far:
		case LITERAL___interrupt:
		case LITERAL_pascal:
		case LITERAL__pascal:
		case LITERAL___pascal:
		{
			pl=parameter_list();
			break;
		}
		case RPAREN:
		{
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		match(RPAREN);
		{
		switch ( LA(1)) {
		case LITERAL_const:
		case LITERAL___const:
		case LITERAL_const_cast:
		case LITERAL_volatile:
		case LITERAL___volatile:
		case LITERAL___restrict:
		case LITERAL___restrict__:
		{
			tq=type_qualifier();
			break;
		}
		case LCURLY:
		case SEMICOLON:
		case LITERAL_throw:
		{
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		{
		switch ( LA(1)) {
		case LITERAL_throw:
		{
			es=exception_specification();
			break;
		}
		case LCURLY:
		case SEMICOLON:
		{
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		{
		switch ( LA(1)) {
		case LCURLY:
		{
			cs=compound_statement();
			break;
		}
		case SEMICOLON:
		{
			match(SEMICOLON);
			if ( inputState->guessing==0 ) {
#line 1137 "CPP_parser_cpp.g"
				end_of_stmt();
#line 1987 "CPPParser.cpp"
			}
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_20);
		} else {
			throw;
		}
	}
	return node;
}

ASTNode * CPPParser::declaration_specifiers() {
#line 712 "CPP_parser_cpp.g"
	ASTNode *node;
#line 2012 "CPPParser.cpp"
#line 712 "CPP_parser_cpp.g"
	
		char *specname;
		ASTNode *dd;
		
		// Global flags to allow for nested declarations
		_td = false;		// For typedef
		_fd = false;		// For friend
		_tq = tqInvalid;	// For TypeQualifier
		_ts = tsInvalid;	// For TypeSpecifier
		_ds = dsInvalid;	// For DeclSpecifier
	
		 // Locals
		bool td = false;	// For typedef
		bool fd = false;	// For friend
		char *sc = NULL;	// auto,register,static,extern,mutable
		char *tq = NULL;	// const,const_cast,volatile,cdecl
		ASTNode *ts = NULL;	// char,int,double, etc., class,struct,union
		// inline,virtual,explicit:
		DeclSpecifier ds = dsInvalid; 	
		
#line 2034 "CPPParser.cpp"
	
	try {      // for error handling
		{
		switch ( LA(1)) {
		case LITERAL_typedef:
		case LITERAL_enum:
		case ID:
		case LITERAL_inline:
		case LITERAL_extern:
		case LITERAL___extension__:
		case LITERAL__inline:
		case LITERAL___inline:
		case LITERAL___inline__:
		case LITERAL_virtual:
		case LITERAL_explicit:
		case LITERAL_friend:
		case LITERAL__stdcall:
		case LITERAL___stdcall:
		case LITERAL_const:
		case LITERAL___const:
		case LITERAL_auto:
		case LITERAL_register:
		case LITERAL_static:
		case LITERAL_mutable:
		case LITERAL_const_cast:
		case LITERAL_volatile:
		case LITERAL___volatile:
		case LITERAL___restrict:
		case LITERAL___restrict__:
		case LITERAL_char:
		case LITERAL_bool:
		case LITERAL_short:
		case LITERAL_int:
		case 73:
		case 74:
		case 75:
		case LITERAL_long:
		case LITERAL_signed:
		case LITERAL___signed:
		case LITERAL___signed__:
		case LITERAL_unsigned:
		case LITERAL_float:
		case LITERAL_double:
		case LITERAL_void:
		case LITERAL__declspec:
		case LITERAL___declspec:
		case LITERAL___builtin_va_list:
		case LITERAL_class:
		case LITERAL_struct:
		case LITERAL_union:
		case SCOPE:
		{
			if ( inputState->guessing==0 ) {
#line 735 "CPP_parser_cpp.g"
				node = mk_node("declaration_specifiers");
#line 2090 "CPPParser.cpp"
			}
			{ // ( ... )*
			for (;;) {
				if ((_tokenSet_23.member(LA(1)))) {
					{
					switch ( LA(1)) {
					case LITERAL_extern:
					case LITERAL_auto:
					case LITERAL_register:
					case LITERAL_static:
					case LITERAL_mutable:
					{
						sc=storage_class_specifier();
						if ( inputState->guessing==0 ) {
#line 738 "CPP_parser_cpp.g"
							specname = sc;
#line 2107 "CPPParser.cpp"
						}
						break;
					}
					case LITERAL_const:
					case LITERAL___const:
					case LITERAL_const_cast:
					case LITERAL_volatile:
					case LITERAL___volatile:
					case LITERAL___restrict:
					case LITERAL___restrict__:
					{
						tq=type_qualifier();
						if ( inputState->guessing==0 ) {
#line 739 "CPP_parser_cpp.g"
							specname = tq;
#line 2123 "CPPParser.cpp"
						}
						break;
					}
					case LITERAL___extension__:
					{
						match(LITERAL___extension__);
						if ( inputState->guessing==0 ) {
#line 740 "CPP_parser_cpp.g"
							specname = "__extension__";
#line 2133 "CPPParser.cpp"
						}
						break;
					}
					case LITERAL_inline:
					case LITERAL__inline:
					case LITERAL___inline:
					case LITERAL___inline__:
					{
						{
						switch ( LA(1)) {
						case LITERAL_inline:
						{
							match(LITERAL_inline);
							break;
						}
						case LITERAL__inline:
						{
							match(LITERAL__inline);
							break;
						}
						case LITERAL___inline:
						{
							match(LITERAL___inline);
							break;
						}
						case LITERAL___inline__:
						{
							match(LITERAL___inline__);
							break;
						}
						default:
						{
							throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
						}
						}
						}
						if ( inputState->guessing==0 ) {
#line 742 "CPP_parser_cpp.g"
							ds = dsINLINE; specname = "inline";
#line 2173 "CPPParser.cpp"
						}
						break;
					}
					case LITERAL_virtual:
					{
						match(LITERAL_virtual);
						if ( inputState->guessing==0 ) {
#line 743 "CPP_parser_cpp.g"
							ds = dsVIRTUAL; specname = "virtual";
#line 2183 "CPPParser.cpp"
						}
						break;
					}
					case LITERAL_explicit:
					{
						match(LITERAL_explicit);
						if ( inputState->guessing==0 ) {
#line 744 "CPP_parser_cpp.g"
							ds = dsEXPLICIT; specname = "explicit";
#line 2193 "CPPParser.cpp"
						}
						break;
					}
					case LITERAL_typedef:
					{
						match(LITERAL_typedef);
						if ( inputState->guessing==0 ) {
#line 745 "CPP_parser_cpp.g"
							td=true;
#line 2203 "CPPParser.cpp"
						}
						if ( inputState->guessing==0 ) {
#line 745 "CPP_parser_cpp.g"
							specname = "typedef";
#line 2208 "CPPParser.cpp"
						}
						break;
					}
					case LITERAL_friend:
					{
						match(LITERAL_friend);
						if ( inputState->guessing==0 ) {
#line 746 "CPP_parser_cpp.g"
							fd=true;
#line 2218 "CPPParser.cpp"
						}
						if ( inputState->guessing==0 ) {
#line 746 "CPP_parser_cpp.g"
							specname = "friend";
#line 2223 "CPPParser.cpp"
						}
						break;
					}
					case LITERAL__stdcall:
					case LITERAL___stdcall:
					{
						{
						switch ( LA(1)) {
						case LITERAL__stdcall:
						{
							match(LITERAL__stdcall);
							break;
						}
						case LITERAL___stdcall:
						{
							match(LITERAL___stdcall);
							break;
						}
						default:
						{
							throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
						}
						}
						}
						if ( inputState->guessing==0 ) {
#line 747 "CPP_parser_cpp.g"
							specname = "stdcall";
#line 2251 "CPPParser.cpp"
						}
						break;
					}
					default:
					{
						throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
					}
					}
					}
					if ( inputState->guessing==0 ) {
#line 749 "CPP_parser_cpp.g"
						node->add_child(mk_node(specname));
#line 2264 "CPPParser.cpp"
					}
				}
				else {
					goto _loop118;
				}
				
			}
			_loop118:;
			} // ( ... )*
			ts=type_specifier(ds);
			if ( inputState->guessing==0 ) {
#line 752 "CPP_parser_cpp.g"
				node->add_child(ts);
#line 2278 "CPPParser.cpp"
			}
			break;
		}
		case LITERAL_typename:
		{
			match(LITERAL_typename);
			if ( inputState->guessing==0 ) {
#line 754 "CPP_parser_cpp.g"
				td=true;
#line 2288 "CPPParser.cpp"
			}
			if ( inputState->guessing==0 ) {
#line 755 "CPP_parser_cpp.g"
				node = mk_node("declaration_specifiers");
#line 2293 "CPPParser.cpp"
			}
			dd=direct_declarator();
			if ( inputState->guessing==0 ) {
#line 756 "CPP_parser_cpp.g"
				node->add_child(dd); node->set_leaf("typename");
#line 2299 "CPPParser.cpp"
			}
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		if ( inputState->guessing==0 ) {
#line 758 "CPP_parser_cpp.g"
			declarationSpecifier(td,fd,sc,tq,ts,ds);
#line 2312 "CPPParser.cpp"
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_17);
		} else {
			throw;
		}
	}
	return node;
}

ASTNode * CPPParser::function_declarator(
	int definition
) {
#line 1259 "CPP_parser_cpp.g"
	ASTNode *node;
#line 2331 "CPPParser.cpp"
#line 1259 "CPP_parser_cpp.g"
	
		char *po;
		ASTNode *fd, *fdd;
		node = mk_node("function_declarator");
		
#line 2338 "CPPParser.cpp"
	
	try {      // for error handling
		bool synPredMatched239 = false;
		if (((_tokenSet_24.member(LA(1))) && (_tokenSet_25.member(LA(2))))) {
			int _m239 = mark();
			synPredMatched239 = true;
			inputState->guessing++;
			try {
				{
				ptr_operator();
				}
			}
			catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
				synPredMatched239 = false;
			}
			rewind(_m239);
			inputState->guessing--;
		}
		if ( synPredMatched239 ) {
			po=ptr_operator();
			if ( inputState->guessing==0 ) {
#line 1267 "CPP_parser_cpp.g"
				node->set_leaf(po);
#line 2362 "CPPParser.cpp"
			}
			fd=function_declarator(definition);
			if ( inputState->guessing==0 ) {
#line 1268 "CPP_parser_cpp.g"
				node->add_child(fd);
#line 2368 "CPPParser.cpp"
			}
		}
		else if ((_tokenSet_26.member(LA(1))) && (_tokenSet_27.member(LA(2)))) {
			fdd=function_direct_declarator(definition);
			if ( inputState->guessing==0 ) {
#line 1270 "CPP_parser_cpp.g"
				node->add_child(fdd);
#line 2376 "CPPParser.cpp"
			}
		}
		else {
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_28);
		} else {
			throw;
		}
	}
	return node;
}

ASTNode * CPPParser::function_definition() {
#line 646 "CPP_parser_cpp.g"
	ASTNode *node;
#line 2398 "CPPParser.cpp"
#line 646 "CPP_parser_cpp.g"
	
		ASTNode *ds, *fd, *decl_node, *d, *cs;
		
#line 2403 "CPPParser.cpp"
	
	try {      // for error handling
		if ( inputState->guessing==0 ) {
#line 651 "CPP_parser_cpp.g"
			beginFunctionDefinition(); node = mk_node("function_definition");
#line 2409 "CPPParser.cpp"
		}
		{
		if (((_tokenSet_13.member(LA(1))) && (_tokenSet_29.member(LA(2))))&&(( !(LA(1)==SCOPE||LA(1)==ID) || qualifiedItemIsOneOf(qiType|qiCtor) ))) {
			ds=declaration_specifiers();
			if ( inputState->guessing==0 ) {
#line 655 "CPP_parser_cpp.g"
				node->add_child(ds);
#line 2417 "CPPParser.cpp"
			}
			fd=function_declarator(1);
			if ( inputState->guessing==0 ) {
#line 656 "CPP_parser_cpp.g"
				node->add_child(fd);
#line 2423 "CPPParser.cpp"
			}
			{
			if ((_tokenSet_30.member(LA(1))) && (_tokenSet_31.member(LA(2)))) {
				if ( inputState->guessing==0 ) {
#line 658 "CPP_parser_cpp.g"
					decl_node = mk_node("declaration_list");
#line 2430 "CPPParser.cpp"
				}
				{ // ( ... )*
				for (;;) {
					if ((_tokenSet_2.member(LA(1)))) {
						d=declaration();
						if ( inputState->guessing==0 ) {
#line 659 "CPP_parser_cpp.g"
							decl_node->add_child(d);
#line 2439 "CPPParser.cpp"
						}
					}
					else {
						goto _loop98;
					}
					
				}
				_loop98:;
				} // ( ... )*
				if ( inputState->guessing==0 ) {
#line 660 "CPP_parser_cpp.g"
					in_parameter_list = false;
#line 2452 "CPPParser.cpp"
				}
				if ( inputState->guessing==0 ) {
#line 661 "CPP_parser_cpp.g"
					node->add_child(decl_node);
#line 2457 "CPPParser.cpp"
				}
			}
			else if ((LA(1) == LCURLY) && (_tokenSet_32.member(LA(2)))) {
			}
			else {
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
			}
			
			}
			cs=compound_statement();
			if ( inputState->guessing==0 ) {
#line 663 "CPP_parser_cpp.g"
				node->add_child(cs);
#line 2471 "CPPParser.cpp"
			}
		}
		else if ((_tokenSet_33.member(LA(1))) && (_tokenSet_34.member(LA(2)))) {
			fd=function_declarator(1);
			if ( inputState->guessing==0 ) {
#line 667 "CPP_parser_cpp.g"
				node->add_child(fd);
#line 2479 "CPPParser.cpp"
			}
			{
			if ((_tokenSet_30.member(LA(1))) && (_tokenSet_31.member(LA(2)))) {
				if ( inputState->guessing==0 ) {
#line 669 "CPP_parser_cpp.g"
					decl_node = mk_node("declaration_list");
#line 2486 "CPPParser.cpp"
				}
				{ // ( ... )*
				for (;;) {
					if ((_tokenSet_2.member(LA(1)))) {
						d=declaration();
					}
					else {
						goto _loop101;
					}
					
				}
				_loop101:;
				} // ( ... )*
				if ( inputState->guessing==0 ) {
#line 670 "CPP_parser_cpp.g"
					decl_node->add_child(d);
#line 2503 "CPPParser.cpp"
				}
				if ( inputState->guessing==0 ) {
#line 671 "CPP_parser_cpp.g"
					in_parameter_list = false;
#line 2508 "CPPParser.cpp"
				}
				if ( inputState->guessing==0 ) {
#line 672 "CPP_parser_cpp.g"
					node->add_child(decl_node);
#line 2513 "CPPParser.cpp"
				}
			}
			else if ((LA(1) == LCURLY) && (_tokenSet_32.member(LA(2)))) {
			}
			else {
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
			}
			
			}
			cs=compound_statement();
			if ( inputState->guessing==0 ) {
#line 674 "CPP_parser_cpp.g"
				node->add_child(cs);
#line 2527 "CPPParser.cpp"
			}
		}
		else {
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		
		}
		if ( inputState->guessing==0 ) {
#line 676 "CPP_parser_cpp.g"
			endFunctionDefinition();
#line 2538 "CPPParser.cpp"
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_20);
		} else {
			throw;
		}
	}
	return node;
}

ASTNode * CPPParser::decl_namespace() {
#line 474 "CPP_parser_cpp.g"
	ASTNode *node;
#line 2555 "CPPParser.cpp"
	ANTLR_USE_NAMESPACE(antlr)RefToken  ns = ANTLR_USE_NAMESPACE(antlr)nullToken;
	ANTLR_USE_NAMESPACE(antlr)RefToken  ns2 = ANTLR_USE_NAMESPACE(antlr)nullToken;
#line 474 "CPP_parser_cpp.g"
	const char *qid; ASTNode *ed;
#line 2560 "CPPParser.cpp"
	
	try {      // for error handling
		match(LITERAL_namespace);
		{
		if ((LA(1) == ID || LA(1) == LCURLY) && (_tokenSet_35.member(LA(2)))) {
			if ( inputState->guessing==0 ) {
#line 479 "CPP_parser_cpp.g"
				node = mk_node("namespace");
#line 2569 "CPPParser.cpp"
			}
			{
			switch ( LA(1)) {
			case ID:
			{
				ns = LT(1);
				match(ID);
				if ( inputState->guessing==0 ) {
#line 480 "CPP_parser_cpp.g"
					node->set_leaf(ns->getText().data());
#line 2580 "CPPParser.cpp"
				}
				if ( inputState->guessing==0 ) {
#line 481 "CPP_parser_cpp.g"
					_td = true; declaratorID(ns->getText().data(), qiType);
								
#line 2586 "CPPParser.cpp"
				}
				break;
			}
			case LCURLY:
			{
				break;
			}
			default:
			{
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
			}
			}
			}
			match(LCURLY);
			if ( inputState->guessing==0 ) {
#line 487 "CPP_parser_cpp.g"
				enterNewLocalScope();
#line 2604 "CPPParser.cpp"
			}
			{ // ( ... )*
			for (;;) {
				if ((_tokenSet_0.member(LA(1)))) {
					ed=external_declaration();
					if ( inputState->guessing==0 ) {
#line 488 "CPP_parser_cpp.g"
						node->add_child(ed);
#line 2613 "CPPParser.cpp"
					}
				}
				else {
					goto _loop49;
				}
				
			}
			_loop49:;
			} // ( ... )*
			if ( inputState->guessing==0 ) {
#line 489 "CPP_parser_cpp.g"
				exitLocalScope();
#line 2626 "CPPParser.cpp"
			}
			match(RCURLY);
		}
		else if ((LA(1) == ID) && (LA(2) == ASSIGNEQUAL)) {
			if ( inputState->guessing==0 ) {
#line 494 "CPP_parser_cpp.g"
				node = mk_node("unfinished_namespace_assignment");
#line 2634 "CPPParser.cpp"
			}
			ns2 = LT(1);
			match(ID);
			if ( inputState->guessing==0 ) {
#line 496 "CPP_parser_cpp.g"
				_td = true; declaratorID(ns2->getText().data(), qiType);
#line 2641 "CPPParser.cpp"
			}
			match(ASSIGNEQUAL);
			qid=qualified_id();
			match(SEMICOLON);
			if ( inputState->guessing==0 ) {
#line 497 "CPP_parser_cpp.g"
				end_of_stmt();
#line 2649 "CPPParser.cpp"
			}
		}
		else {
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_12);
		} else {
			throw;
		}
	}
	return node;
}

const char * CPPParser::qualified_id() {
#line 969 "CPP_parser_cpp.g"
	const char *q;
#line 2672 "CPPParser.cpp"
	ANTLR_USE_NAMESPACE(antlr)RefToken  id = ANTLR_USE_NAMESPACE(antlr)nullToken;
#line 969 "CPP_parser_cpp.g"
	
		ASTNode *tal;
		const char *so, *o;
		static char qitem[CPPParser_MaxQualifiedItemSize+1];
		
#line 2680 "CPPParser.cpp"
	
	try {      // for error handling
		so=scope_override();
		if ( inputState->guessing==0 ) {
#line 977 "CPP_parser_cpp.g"
			strcpy(qitem, so);
#line 2687 "CPPParser.cpp"
		}
		{
		switch ( LA(1)) {
		case ID:
		{
			id = LT(1);
			match(ID);
			{
			if ((LA(1) == LESSTHAN) && (_tokenSet_36.member(LA(2)))) {
				match(LESSTHAN);
				tal=template_argument_list();
				match(GREATERTHAN);
			}
			else if ((_tokenSet_37.member(LA(1))) && (_tokenSet_38.member(LA(2)))) {
			}
			else {
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
			}
			
			}
			if ( inputState->guessing==0 ) {
#line 981 "CPP_parser_cpp.g"
				strcat(qitem,(id->getText()).data());
#line 2711 "CPPParser.cpp"
			}
			break;
		}
		case OPERATOR:
		{
			match(OPERATOR);
			o=optor();
			if ( inputState->guessing==0 ) {
#line 984 "CPP_parser_cpp.g"
				strcat(qitem,"operator"); strcat(qitem, o);
#line 2722 "CPPParser.cpp"
			}
			break;
		}
		case LITERAL_this:
		{
			match(LITERAL_this);
			break;
		}
		case LITERAL_true:
		case LITERAL_false:
		{
			{
			switch ( LA(1)) {
			case LITERAL_true:
			{
				match(LITERAL_true);
				break;
			}
			case LITERAL_false:
			{
				match(LITERAL_false);
				break;
			}
			default:
			{
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
			}
			}
			}
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		if ( inputState->guessing==0 ) {
#line 990 "CPP_parser_cpp.g"
			q = qitem;
#line 2763 "CPPParser.cpp"
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_37);
		} else {
			throw;
		}
	}
	return q;
}

ASTNode * CPPParser::member_declaration() {
#line 501 "CPP_parser_cpp.g"
	ASTNode *node;
#line 2780 "CPPParser.cpp"
#line 501 "CPP_parser_cpp.g"
	
		ASTNode *as, *d, *es, *mdl, *cds, *cd, *dh, *db, *fd, *cfdd, *ds, *cs, *md,
				*idl, *th;
		const char *q;
		
#line 2787 "CPPParser.cpp"
	
	try {      // for error handling
		{
		switch ( LA(1)) {
		case LITERAL_public:
		case LITERAL_protected:
		case LITERAL_private:
		{
			as=access_specifier();
			match(COLON);
			if ( inputState->guessing==0 ) {
#line 639 "CPP_parser_cpp.g"
				node = as;
#line 2801 "CPPParser.cpp"
			}
			break;
		}
		case SEMICOLON:
		{
			match(SEMICOLON);
			if ( inputState->guessing==0 ) {
#line 641 "CPP_parser_cpp.g"
				end_of_stmt();
#line 2811 "CPPParser.cpp"
			}
			if ( inputState->guessing==0 ) {
#line 642 "CPP_parser_cpp.g"
				node = mk_node("unfinished_member-decl-524");
#line 2816 "CPPParser.cpp"
			}
			break;
		}
		default:
			bool synPredMatched54 = false;
			if (((_tokenSet_2.member(LA(1))) && (_tokenSet_3.member(LA(2))))) {
				int _m54 = mark();
				synPredMatched54 = true;
				inputState->guessing++;
				try {
					{
					{
					switch ( LA(1)) {
					case LITERAL_typedef:
					{
						match(LITERAL_typedef);
						break;
					}
					case LITERAL_class:
					case LITERAL_struct:
					case LITERAL_union:
					{
						break;
					}
					default:
					{
						throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
					}
					}
					}
					class_head();
					}
				}
				catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
					synPredMatched54 = false;
				}
				rewind(_m54);
				inputState->guessing--;
			}
			if ( synPredMatched54 ) {
				d=declaration();
				if ( inputState->guessing==0 ) {
#line 515 "CPP_parser_cpp.g"
					node = d;
#line 2861 "CPPParser.cpp"
				}
			}
			else {
				bool synPredMatched57 = false;
				if (((LA(1) == LITERAL_enum) && (LA(2) == ID || LA(2) == LCURLY))) {
					int _m57 = mark();
					synPredMatched57 = true;
					inputState->guessing++;
					try {
						{
						match(LITERAL_enum);
						{
						switch ( LA(1)) {
						case ID:
						{
							match(ID);
							break;
						}
						case LCURLY:
						{
							break;
						}
						default:
						{
							throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
						}
						}
						}
						match(LCURLY);
						}
					}
					catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
						synPredMatched57 = false;
					}
					rewind(_m57);
					inputState->guessing--;
				}
				if ( synPredMatched57 ) {
					if ( inputState->guessing==0 ) {
#line 519 "CPP_parser_cpp.g"
						node = mk_node("member_declaration");
#line 2903 "CPPParser.cpp"
					}
					es=enum_specifier();
					if ( inputState->guessing==0 ) {
#line 520 "CPP_parser_cpp.g"
						node->add_child(es);
#line 2909 "CPPParser.cpp"
					}
					{
					switch ( LA(1)) {
					case ID:
					case COLON:
					case LITERAL__stdcall:
					case LITERAL___stdcall:
					case LPAREN:
					case OPERATOR:
					case LITERAL_this:
					case LITERAL_true:
					case LITERAL_false:
					case STAR:
					case AMPERSAND:
					case TILDE:
					case SCOPE:
					case LITERAL__cdecl:
					case LITERAL___cdecl:
					case LITERAL__near:
					case LITERAL___near:
					case LITERAL__far:
					case LITERAL___far:
					case LITERAL___interrupt:
					case LITERAL_pascal:
					case LITERAL__pascal:
					case LITERAL___pascal:
					{
						mdl=member_declarator_list();
						if ( inputState->guessing==0 ) {
#line 521 "CPP_parser_cpp.g"
							node->add_child(mdl);
#line 2941 "CPPParser.cpp"
						}
						break;
					}
					case SEMICOLON:
					{
						break;
					}
					default:
					{
						throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
					}
					}
					}
					match(SEMICOLON);
					if ( inputState->guessing==0 ) {
#line 521 "CPP_parser_cpp.g"
						end_of_stmt();
#line 2959 "CPPParser.cpp"
					}
				}
				else {
					bool synPredMatched60 = false;
					if (((_tokenSet_6.member(LA(1))) && (_tokenSet_7.member(LA(2))))) {
						int _m60 = mark();
						synPredMatched60 = true;
						inputState->guessing++;
						try {
							{
							ctor_decl_spec();
							if (!(qualifiedItemIsOneOf(qiCtor)))
								throw ANTLR_USE_NAMESPACE(antlr)SemanticException("qualifiedItemIsOneOf(qiCtor)");
							ctor_declarator(0);
							match(SEMICOLON);
							}
						}
						catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
							synPredMatched60 = false;
						}
						rewind(_m60);
						inputState->guessing--;
					}
					if ( synPredMatched60 ) {
						if ( inputState->guessing==0 ) {
#line 528 "CPP_parser_cpp.g"
							node = mk_node("member_declaration");
#line 2987 "CPPParser.cpp"
						}
						cds=ctor_decl_spec();
						if ( inputState->guessing==0 ) {
#line 529 "CPP_parser_cpp.g"
							node->add_child(cds);
#line 2993 "CPPParser.cpp"
						}
						cd=ctor_declarator(0);
						if ( inputState->guessing==0 ) {
#line 530 "CPP_parser_cpp.g"
							node->add_child(cd);
#line 2999 "CPPParser.cpp"
						}
						match(SEMICOLON);
						if ( inputState->guessing==0 ) {
#line 531 "CPP_parser_cpp.g"
							end_of_stmt();
#line 3005 "CPPParser.cpp"
						}
					}
					else {
						bool synPredMatched63 = false;
						if (((_tokenSet_6.member(LA(1))) && (_tokenSet_7.member(LA(2))))) {
							int _m63 = mark();
							synPredMatched63 = true;
							inputState->guessing++;
							try {
								{
								ctor_decl_spec();
								if (!(qualifiedItemIsOneOf(qiCtor)))
									throw ANTLR_USE_NAMESPACE(antlr)SemanticException("qualifiedItemIsOneOf(qiCtor)");
								ctor_declarator(1);
								{
								switch ( LA(1)) {
								case COLON:
								{
									match(COLON);
									break;
								}
								case LCURLY:
								{
									match(LCURLY);
									break;
								}
								default:
								{
									throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
								}
								}
								}
								}
							}
							catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
								synPredMatched63 = false;
							}
							rewind(_m63);
							inputState->guessing--;
						}
						if ( synPredMatched63 ) {
							if ( inputState->guessing==0 ) {
#line 544 "CPP_parser_cpp.g"
								node = mk_node("member_declaration");
#line 3050 "CPPParser.cpp"
							}
							cd=ctor_definition();
							if ( inputState->guessing==0 ) {
#line 545 "CPP_parser_cpp.g"
								node->add_child(cd);
#line 3056 "CPPParser.cpp"
							}
						}
						else {
							bool synPredMatched65 = false;
							if (((_tokenSet_39.member(LA(1))) && (_tokenSet_5.member(LA(2))))) {
								int _m65 = mark();
								synPredMatched65 = true;
								inputState->guessing++;
								try {
									{
									dtor_head(0);
									match(SEMICOLON);
									}
								}
								catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
									synPredMatched65 = false;
								}
								rewind(_m65);
								inputState->guessing--;
							}
							if ( synPredMatched65 ) {
								if ( inputState->guessing==0 ) {
#line 550 "CPP_parser_cpp.g"
									node = mk_node("member_declaration");
#line 3081 "CPPParser.cpp"
								}
								dh=dtor_head(0);
								if ( inputState->guessing==0 ) {
#line 551 "CPP_parser_cpp.g"
									node->add_child(dh);
#line 3087 "CPPParser.cpp"
								}
								match(SEMICOLON);
								if ( inputState->guessing==0 ) {
#line 552 "CPP_parser_cpp.g"
									end_of_stmt();
#line 3093 "CPPParser.cpp"
								}
							}
							else {
								bool synPredMatched67 = false;
								if (((_tokenSet_39.member(LA(1))) && (_tokenSet_5.member(LA(2))))) {
									int _m67 = mark();
									synPredMatched67 = true;
									inputState->guessing++;
									try {
										{
										dtor_head(1);
										match(LCURLY);
										}
									}
									catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
										synPredMatched67 = false;
									}
									rewind(_m67);
									inputState->guessing--;
								}
								if ( synPredMatched67 ) {
									if ( inputState->guessing==0 ) {
#line 557 "CPP_parser_cpp.g"
										node = mk_node("member_declaration");
#line 3118 "CPPParser.cpp"
									}
									dh=dtor_head(1);
									if ( inputState->guessing==0 ) {
#line 558 "CPP_parser_cpp.g"
										node->add_child(dh);
#line 3124 "CPPParser.cpp"
									}
									db=dtor_body();
									if ( inputState->guessing==0 ) {
#line 559 "CPP_parser_cpp.g"
										node->add_child(db);
#line 3130 "CPPParser.cpp"
									}
								}
								else {
									bool synPredMatched69 = false;
									if (((_tokenSet_2.member(LA(1))) && (_tokenSet_3.member(LA(2))))) {
										int _m69 = mark();
										synPredMatched69 = true;
										inputState->guessing++;
										try {
											{
											declaration_specifiers();
											function_declarator(0);
											match(SEMICOLON);
											}
										}
										catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
											synPredMatched69 = false;
										}
										rewind(_m69);
										inputState->guessing--;
									}
									if ( synPredMatched69 ) {
										d=declaration();
										if ( inputState->guessing==0 ) {
#line 564 "CPP_parser_cpp.g"
											node = d;
#line 3157 "CPPParser.cpp"
										}
									}
									else {
										bool synPredMatched71 = false;
										if (((_tokenSet_10.member(LA(1))) && (_tokenSet_11.member(LA(2))))) {
											int _m71 = mark();
											synPredMatched71 = true;
											inputState->guessing++;
											try {
												{
												declaration_specifiers();
												function_declarator(1);
												match(LCURLY);
												}
											}
											catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
												synPredMatched71 = false;
											}
											rewind(_m71);
											inputState->guessing--;
										}
										if ( synPredMatched71 ) {
											if ( inputState->guessing==0 ) {
#line 568 "CPP_parser_cpp.g"
												beginFieldDeclaration();
#line 3183 "CPPParser.cpp"
											}
											fd=function_definition();
											if ( inputState->guessing==0 ) {
#line 570 "CPP_parser_cpp.g"
												node = fd;
#line 3189 "CPPParser.cpp"
											}
										}
										else {
											bool synPredMatched74 = false;
											if (((LA(1) == LITERAL_inline || LA(1) == OPERATOR) && (_tokenSet_40.member(LA(2))))) {
												int _m74 = mark();
												synPredMatched74 = true;
												inputState->guessing++;
												try {
													{
													{
													switch ( LA(1)) {
													case LITERAL_inline:
													{
														match(LITERAL_inline);
														break;
													}
													case OPERATOR:
													{
														break;
													}
													default:
													{
														throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
													}
													}
													}
													conversion_function_decl_or_def();
													}
												}
												catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
													synPredMatched74 = false;
												}
												rewind(_m74);
												inputState->guessing--;
											}
											if ( synPredMatched74 ) {
												{
												switch ( LA(1)) {
												case LITERAL_inline:
												{
													match(LITERAL_inline);
													break;
												}
												case OPERATOR:
												{
													break;
												}
												default:
												{
													throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
												}
												}
												}
												cfdd=conversion_function_decl_or_def();
												if ( inputState->guessing==0 ) {
#line 575 "CPP_parser_cpp.g"
													node = mk_node("unfinished_udtc-439");
#line 3248 "CPPParser.cpp"
												}
											}
											else {
												bool synPredMatched77 = false;
												if (((_tokenSet_41.member(LA(1))) && (_tokenSet_42.member(LA(2))))) {
													int _m77 = mark();
													synPredMatched77 = true;
													inputState->guessing++;
													try {
														{
														qualified_id();
														match(SEMICOLON);
														}
													}
													catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
														synPredMatched77 = false;
													}
													rewind(_m77);
													inputState->guessing--;
												}
												if ( synPredMatched77 ) {
													q=qualified_id();
													match(SEMICOLON);
													if ( inputState->guessing==0 ) {
#line 580 "CPP_parser_cpp.g"
														end_of_stmt();
#line 3275 "CPPParser.cpp"
													}
													if ( inputState->guessing==0 ) {
#line 581 "CPP_parser_cpp.g"
														node = mk_node("unfinished_declhack-446");
#line 3280 "CPPParser.cpp"
													}
												}
												else {
													bool synPredMatched79 = false;
													if (((_tokenSet_13.member(LA(1))) && (_tokenSet_43.member(LA(2))))) {
														int _m79 = mark();
														synPredMatched79 = true;
														inputState->guessing++;
														try {
															{
															declaration_specifiers();
															}
														}
														catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
															synPredMatched79 = false;
														}
														rewind(_m79);
														inputState->guessing--;
													}
													if ( synPredMatched79 ) {
														if ( inputState->guessing==0 ) {
#line 586 "CPP_parser_cpp.g"
															beginFieldDeclaration();
#line 3304 "CPPParser.cpp"
														}
														if ( inputState->guessing==0 ) {
#line 587 "CPP_parser_cpp.g"
															node = mk_node("member_declaration");
#line 3309 "CPPParser.cpp"
														}
														ds=declaration_specifiers();
														if ( inputState->guessing==0 ) {
#line 588 "CPP_parser_cpp.g"
															node->add_child(ds);
#line 3315 "CPPParser.cpp"
														}
														{
														switch ( LA(1)) {
														case ID:
														case COLON:
														case LITERAL__stdcall:
														case LITERAL___stdcall:
														case LPAREN:
														case OPERATOR:
														case LITERAL_this:
														case LITERAL_true:
														case LITERAL_false:
														case STAR:
														case AMPERSAND:
														case TILDE:
														case SCOPE:
														case LITERAL__cdecl:
														case LITERAL___cdecl:
														case LITERAL__near:
														case LITERAL___near:
														case LITERAL__far:
														case LITERAL___far:
														case LITERAL___interrupt:
														case LITERAL_pascal:
														case LITERAL__pascal:
														case LITERAL___pascal:
														{
															md=member_declarator_list();
															if ( inputState->guessing==0 ) {
#line 589 "CPP_parser_cpp.g"
																node->add_child(md);
#line 3347 "CPPParser.cpp"
															}
															break;
														}
														case SEMICOLON:
														{
															break;
														}
														default:
														{
															throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
														}
														}
														}
														match(SEMICOLON);
														if ( inputState->guessing==0 ) {
#line 590 "CPP_parser_cpp.g"
															end_of_stmt();
#line 3365 "CPPParser.cpp"
														}
													}
													else {
														bool synPredMatched82 = false;
														if (((_tokenSet_33.member(LA(1))) && (_tokenSet_34.member(LA(2))))) {
															int _m82 = mark();
															synPredMatched82 = true;
															inputState->guessing++;
															try {
																{
																function_declarator(0);
																match(SEMICOLON);
																}
															}
															catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
																synPredMatched82 = false;
															}
															rewind(_m82);
															inputState->guessing--;
														}
														if ( synPredMatched82 ) {
															if ( inputState->guessing==0 ) {
#line 594 "CPP_parser_cpp.g"
																beginFieldDeclaration();
#line 3390 "CPPParser.cpp"
															}
															fd=function_declarator(0);
															match(SEMICOLON);
															if ( inputState->guessing==0 ) {
#line 595 "CPP_parser_cpp.g"
																end_of_stmt();
#line 3397 "CPPParser.cpp"
															}
															if ( inputState->guessing==0 ) {
#line 596 "CPP_parser_cpp.g"
																node = fd;
#line 3402 "CPPParser.cpp"
															}
														}
														else if ((_tokenSet_33.member(LA(1))) && (_tokenSet_34.member(LA(2)))) {
															fd=function_declarator(1);
															cs=compound_statement();
															if ( inputState->guessing==0 ) {
#line 599 "CPP_parser_cpp.g"
																endFunctionDefinition();
#line 3411 "CPPParser.cpp"
															}
															if ( inputState->guessing==0 ) {
#line 600 "CPP_parser_cpp.g"
																node = mk_node("member_declaration"); node->add_child(fd); node->add_child(cs);
#line 3416 "CPPParser.cpp"
															}
														}
														else {
															bool synPredMatched85 = false;
															if (((LA(1) == LITERAL_template) && (LA(2) == LESSTHAN))) {
																int _m85 = mark();
																synPredMatched85 = true;
																inputState->guessing++;
																try {
																	{
																	template_head();
																	ds=declaration_specifiers();
																	{
																	switch ( LA(1)) {
																	case ID:
																	case LITERAL__stdcall:
																	case LITERAL___stdcall:
																	case LPAREN:
																	case OPERATOR:
																	case LITERAL_this:
																	case LITERAL_true:
																	case LITERAL_false:
																	case STAR:
																	case AMPERSAND:
																	case TILDE:
																	case SCOPE:
																	case LITERAL__cdecl:
																	case LITERAL___cdecl:
																	case LITERAL__near:
																	case LITERAL___near:
																	case LITERAL__far:
																	case LITERAL___far:
																	case LITERAL___interrupt:
																	case LITERAL_pascal:
																	case LITERAL__pascal:
																	case LITERAL___pascal:
																	{
																		init_declarator_list();
																		break;
																	}
																	case SEMICOLON:
																	{
																		break;
																	}
																	default:
																	{
																		throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
																	}
																	}
																	}
																	match(SEMICOLON);
																	}
																}
																catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
																	synPredMatched85 = false;
																}
																rewind(_m85);
																inputState->guessing--;
															}
															if ( synPredMatched85 ) {
																if ( inputState->guessing==0 ) {
#line 605 "CPP_parser_cpp.g"
																	beginTemplateDeclaration(); node = mk_node("member_declaration");
#line 3480 "CPPParser.cpp"
																}
																th=template_head();
																if ( inputState->guessing==0 ) {
#line 606 "CPP_parser_cpp.g"
																	node->add_child(th);
#line 3486 "CPPParser.cpp"
																}
																ds=declaration_specifiers();
																if ( inputState->guessing==0 ) {
#line 607 "CPP_parser_cpp.g"
																	node->add_child(ds);
#line 3492 "CPPParser.cpp"
																}
																{
																switch ( LA(1)) {
																case ID:
																case LITERAL__stdcall:
																case LITERAL___stdcall:
																case LPAREN:
																case OPERATOR:
																case LITERAL_this:
																case LITERAL_true:
																case LITERAL_false:
																case STAR:
																case AMPERSAND:
																case TILDE:
																case SCOPE:
																case LITERAL__cdecl:
																case LITERAL___cdecl:
																case LITERAL__near:
																case LITERAL___near:
																case LITERAL__far:
																case LITERAL___far:
																case LITERAL___interrupt:
																case LITERAL_pascal:
																case LITERAL__pascal:
																case LITERAL___pascal:
																{
																	idl=init_declarator_list();
																	if ( inputState->guessing==0 ) {
#line 608 "CPP_parser_cpp.g"
																		node->add_child(idl);
#line 3523 "CPPParser.cpp"
																	}
																	break;
																}
																case SEMICOLON:
																{
																	break;
																}
																default:
																{
																	throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
																}
																}
																}
																match(SEMICOLON);
																if ( inputState->guessing==0 ) {
#line 609 "CPP_parser_cpp.g"
																	end_of_stmt();
#line 3541 "CPPParser.cpp"
																}
																if ( inputState->guessing==0 ) {
#line 610 "CPP_parser_cpp.g"
																	endTemplateDeclaration();
#line 3546 "CPPParser.cpp"
																}
															}
															else if ((LA(1) == LITERAL_template) && (LA(2) == LESSTHAN)) {
																if ( inputState->guessing==0 ) {
#line 614 "CPP_parser_cpp.g"
																	beginTemplateDefinition();
#line 3553 "CPPParser.cpp"
																}
																th=template_head();
																if ( inputState->guessing==0 ) {
#line 615 "CPP_parser_cpp.g"
																	node = mk_node("templated_function"); node->add_child(th);
#line 3559 "CPPParser.cpp"
																}
																{
																bool synPredMatched89 = false;
																if (((_tokenSet_6.member(LA(1))) && (_tokenSet_7.member(LA(2))))) {
																	int _m89 = mark();
																	synPredMatched89 = true;
																	inputState->guessing++;
																	try {
																		{
																		ctor_decl_spec();
																		if (!(qualifiedItemIsOneOf(qiCtor)))
																			throw ANTLR_USE_NAMESPACE(antlr)SemanticException("qualifiedItemIsOneOf(qiCtor)");
																		}
																	}
																	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
																		synPredMatched89 = false;
																	}
																	rewind(_m89);
																	inputState->guessing--;
																}
																if ( synPredMatched89 ) {
																	cd=ctor_definition();
																	if ( inputState->guessing==0 ) {
#line 623 "CPP_parser_cpp.g"
																		node->add_child(cd);
#line 3585 "CPPParser.cpp"
																	}
																}
																else {
																	bool synPredMatched91 = false;
																	if (((_tokenSet_2.member(LA(1))) && (_tokenSet_3.member(LA(2))))) {
																		int _m91 = mark();
																		synPredMatched91 = true;
																		inputState->guessing++;
																		try {
																			{
																			declaration_specifiers();
																			function_declarator(0);
																			match(SEMICOLON);
																			}
																		}
																		catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
																			synPredMatched91 = false;
																		}
																		rewind(_m91);
																		inputState->guessing--;
																	}
																	if ( synPredMatched91 ) {
																		d=declaration();
																		if ( inputState->guessing==0 ) {
#line 627 "CPP_parser_cpp.g"
																			node->add_child(d);
#line 3612 "CPPParser.cpp"
																		}
																	}
																	else {
																		bool synPredMatched93 = false;
																		if (((_tokenSet_10.member(LA(1))) && (_tokenSet_11.member(LA(2))))) {
																			int _m93 = mark();
																			synPredMatched93 = true;
																			inputState->guessing++;
																			try {
																				{
																				declaration_specifiers();
																				function_declarator(1);
																				match(LCURLY);
																				}
																			}
																			catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
																				synPredMatched93 = false;
																			}
																			rewind(_m93);
																			inputState->guessing--;
																		}
																		if ( synPredMatched93 ) {
																			fd=function_definition();
																			if ( inputState->guessing==0 ) {
#line 632 "CPP_parser_cpp.g"
																				node->add_child(fd);
#line 3639 "CPPParser.cpp"
																			}
																		}
																		else if ((LA(1) == OPERATOR) && (_tokenSet_13.member(LA(2)))) {
																			cfdd=conversion_function_decl_or_def();
																			if ( inputState->guessing==0 ) {
#line 634 "CPP_parser_cpp.g"
																				node->add_child(cfdd);
#line 3647 "CPPParser.cpp"
																			}
																		}
																else {
																	throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
																}
																}}
																}
																if ( inputState->guessing==0 ) {
#line 636 "CPP_parser_cpp.g"
																	endTemplateDefinition();
#line 3658 "CPPParser.cpp"
																}
															}
		else {
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}}}}}}}}}}}}}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_44);
		} else {
			throw;
		}
	}
	return node;
}

ASTNode * CPPParser::member_declarator_list() {
#line 1104 "CPP_parser_cpp.g"
	ASTNode *node;
#line 3681 "CPPParser.cpp"
#line 1104 "CPP_parser_cpp.g"
	
		ASTNode *md;
		node = mk_node("member_declarator_list");
		
#line 3687 "CPPParser.cpp"
	
	try {      // for error handling
		md=member_declarator();
		if ( inputState->guessing==0 ) {
#line 1109 "CPP_parser_cpp.g"
			node->add_child(md);
#line 3694 "CPPParser.cpp"
		}
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == COMMA)) {
				match(COMMA);
				md=member_declarator();
				if ( inputState->guessing==0 ) {
#line 1110 "CPP_parser_cpp.g"
					node->add_child(md);
#line 3704 "CPPParser.cpp"
				}
			}
			else {
				goto _loop191;
			}
			
		}
		_loop191:;
		} // ( ... )*
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_18);
		} else {
			throw;
		}
	}
	return node;
}

ASTNode * CPPParser::ctor_declarator(
	int definition
) {
#line 1363 "CPP_parser_cpp.g"
	ASTNode *node;
#line 3731 "CPPParser.cpp"
#line 1363 "CPP_parser_cpp.g"
	
		char *q;
		ASTNode *pl, *es;
		node = mk_node("ctor_declarator");
		
#line 3738 "CPPParser.cpp"
	
	try {      // for error handling
		q=qualified_ctor_id();
		if ( inputState->guessing==0 ) {
#line 1371 "CPP_parser_cpp.g"
			node->set_leaf(q);
#line 3745 "CPPParser.cpp"
		}
		if ( inputState->guessing==0 ) {
#line 1372 "CPP_parser_cpp.g"
			declaratorParameterList(definition);
#line 3750 "CPPParser.cpp"
		}
		match(LPAREN);
		{
		switch ( LA(1)) {
		case LITERAL_typedef:
		case LITERAL_enum:
		case ID:
		case LITERAL_inline:
		case LITERAL_extern:
		case LITERAL___extension__:
		case LITERAL__inline:
		case LITERAL___inline:
		case LITERAL___inline__:
		case LITERAL_virtual:
		case LITERAL_explicit:
		case LITERAL_friend:
		case LITERAL__stdcall:
		case LITERAL___stdcall:
		case LITERAL_typename:
		case LPAREN:
		case LITERAL_const:
		case LITERAL___const:
		case LITERAL_auto:
		case LITERAL_register:
		case LITERAL_static:
		case LITERAL_mutable:
		case LITERAL_const_cast:
		case LITERAL_volatile:
		case LITERAL___volatile:
		case LITERAL___restrict:
		case LITERAL___restrict__:
		case LITERAL_char:
		case LITERAL_bool:
		case LITERAL_short:
		case LITERAL_int:
		case 73:
		case 74:
		case 75:
		case LITERAL_long:
		case LITERAL_signed:
		case LITERAL___signed:
		case LITERAL___signed__:
		case LITERAL_unsigned:
		case LITERAL_float:
		case LITERAL_double:
		case LITERAL_void:
		case LITERAL__declspec:
		case LITERAL___declspec:
		case LITERAL___builtin_va_list:
		case LITERAL_class:
		case LITERAL_struct:
		case LITERAL_union:
		case OPERATOR:
		case LITERAL_this:
		case LITERAL_true:
		case LITERAL_false:
		case STAR:
		case AMPERSAND:
		case TILDE:
		case ELLIPSIS:
		case SCOPE:
		case LITERAL__cdecl:
		case LITERAL___cdecl:
		case LITERAL__near:
		case LITERAL___near:
		case LITERAL__far:
		case LITERAL___far:
		case LITERAL___interrupt:
		case LITERAL_pascal:
		case LITERAL__pascal:
		case LITERAL___pascal:
		{
			pl=parameter_list();
			if ( inputState->guessing==0 ) {
#line 1373 "CPP_parser_cpp.g"
				node->add_child(pl);
#line 3827 "CPPParser.cpp"
			}
			break;
		}
		case RPAREN:
		{
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		match(RPAREN);
		if ( inputState->guessing==0 ) {
#line 1374 "CPP_parser_cpp.g"
			declaratorEndParameterList(definition);
#line 3845 "CPPParser.cpp"
		}
		{
		switch ( LA(1)) {
		case LITERAL_throw:
		{
			es=exception_specification();
			if ( inputState->guessing==0 ) {
#line 1375 "CPP_parser_cpp.g"
				node->add_child(es);
#line 3855 "CPPParser.cpp"
			}
			break;
		}
		case LCURLY:
		case SEMICOLON:
		case COLON:
		{
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_45);
		} else {
			throw;
		}
	}
	return node;
}

ASTNode * CPPParser::compound_statement() {
#line 1732 "CPP_parser_cpp.g"
	ASTNode *node;
#line 3886 "CPPParser.cpp"
#line 1732 "CPP_parser_cpp.g"
	ASTNode *sl;
		node = mk_node("compound_statement");
		
#line 3891 "CPPParser.cpp"
	
	try {      // for error handling
		match(LCURLY);
		if ( inputState->guessing==0 ) {
#line 1737 "CPP_parser_cpp.g"
			end_of_stmt(); enterNewLocalScope();
#line 3898 "CPPParser.cpp"
		}
		{
		switch ( LA(1)) {
		case LITERAL_typedef:
		case LITERAL_enum:
		case ID:
		case LCURLY:
		case SEMICOLON:
		case LITERAL_inline:
		case LITERAL_extern:
		case StringLiteral:
		case LITERAL___extension__:
		case LITERAL__inline:
		case LITERAL___inline:
		case LITERAL___inline__:
		case LITERAL_virtual:
		case LITERAL_explicit:
		case LITERAL_friend:
		case LITERAL__stdcall:
		case LITERAL___stdcall:
		case LITERAL_typename:
		case LPAREN:
		case LITERAL_const:
		case LITERAL___const:
		case LITERAL_auto:
		case LITERAL_register:
		case LITERAL_static:
		case LITERAL_mutable:
		case LITERAL_const_cast:
		case LITERAL_volatile:
		case LITERAL___volatile:
		case LITERAL___restrict:
		case LITERAL___restrict__:
		case LITERAL_char:
		case LITERAL_bool:
		case LITERAL_short:
		case LITERAL_int:
		case 73:
		case 74:
		case 75:
		case LITERAL_long:
		case LITERAL_signed:
		case LITERAL___signed:
		case LITERAL___signed__:
		case LITERAL_unsigned:
		case LITERAL_float:
		case LITERAL_double:
		case LITERAL_void:
		case LITERAL__declspec:
		case LITERAL___declspec:
		case LITERAL___builtin_va_list:
		case LITERAL_class:
		case LITERAL_struct:
		case LITERAL_union:
		case OPERATOR:
		case LITERAL_this:
		case LITERAL_true:
		case LITERAL_false:
		case OCTALINT:
		case STAR:
		case AMPERSAND:
		case TILDE:
		case LITERAL_asm:
		case LITERAL___asm:
		case LITERAL___asm__:
		case LITERAL_throw:
		case LITERAL_case:
		case LITERAL_default:
		case LITERAL_if:
		case LITERAL_switch:
		case LITERAL_while:
		case LITERAL_do:
		case LITERAL_for:
		case LITERAL_goto:
		case LITERAL_continue:
		case LITERAL_break:
		case LITERAL_return:
		case LITERAL_try:
		case LITERAL_using:
		case LITERAL__asm:
		case SCOPE:
		case PLUS:
		case MINUS:
		case PLUSPLUS:
		case MINUSMINUS:
		case LITERAL_sizeof:
		case LITERAL_dynamic_cast:
		case LITERAL_static_cast:
		case LITERAL_reinterpret_cast:
		case NOT:
		case LITERAL_new:
		case LITERAL_delete:
		case DECIMALINT:
		case HEXADECIMALINT:
		case CharLiteral:
		case FLOATONE:
		case FLOATTWO:
		{
			sl=statement_list();
			if ( inputState->guessing==0 ) {
#line 1738 "CPP_parser_cpp.g"
				node->add_child(sl);
#line 4001 "CPPParser.cpp"
			}
			break;
		}
		case RCURLY:
		{
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		match(RCURLY);
		if ( inputState->guessing==0 ) {
#line 1739 "CPP_parser_cpp.g"
			exitLocalScope();
#line 4019 "CPPParser.cpp"
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_46);
		} else {
			throw;
		}
	}
	return node;
}

ASTNode * CPPParser::access_specifier() {
#line 1098 "CPP_parser_cpp.g"
	ASTNode *node;
#line 4036 "CPPParser.cpp"
	
	try {      // for error handling
		switch ( LA(1)) {
		case LITERAL_public:
		{
			match(LITERAL_public);
			if ( inputState->guessing==0 ) {
#line 1099 "CPP_parser_cpp.g"
				node = mk_node("public");
#line 4046 "CPPParser.cpp"
			}
			break;
		}
		case LITERAL_protected:
		{
			match(LITERAL_protected);
			if ( inputState->guessing==0 ) {
#line 1100 "CPP_parser_cpp.g"
				node = mk_node("protected");
#line 4056 "CPPParser.cpp"
			}
			break;
		}
		case LITERAL_private:
		{
			match(LITERAL_private);
			if ( inputState->guessing==0 ) {
#line 1101 "CPP_parser_cpp.g"
				node = mk_node("private");
#line 4066 "CPPParser.cpp"
			}
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_47);
		} else {
			throw;
		}
	}
	return node;
}

ASTNode * CPPParser::linkage_specification() {
#line 700 "CPP_parser_cpp.g"
	ASTNode *node;
#line 4090 "CPPParser.cpp"
#line 700 "CPP_parser_cpp.g"
	
		ASTNode *ed, *d;
		node = mk_node("linkage_specification");
		
#line 4096 "CPPParser.cpp"
	
	try {      // for error handling
		match(LITERAL_extern);
		match(StringLiteral);
		{
		switch ( LA(1)) {
		case LCURLY:
		{
			match(LCURLY);
			{ // ( ... )*
			for (;;) {
				if ((_tokenSet_0.member(LA(1)))) {
					ed=external_declaration();
					if ( inputState->guessing==0 ) {
#line 707 "CPP_parser_cpp.g"
						node->add_child(ed);
#line 4113 "CPPParser.cpp"
					}
				}
				else {
					goto _loop111;
				}
				
			}
			_loop111:;
			} // ( ... )*
			match(RCURLY);
			break;
		}
		case LITERAL_typedef:
		case LITERAL_enum:
		case ID:
		case LITERAL_inline:
		case LITERAL_extern:
		case LITERAL___extension__:
		case LITERAL__inline:
		case LITERAL___inline:
		case LITERAL___inline__:
		case LITERAL_virtual:
		case LITERAL_explicit:
		case LITERAL_friend:
		case LITERAL__stdcall:
		case LITERAL___stdcall:
		case LITERAL_typename:
		case LITERAL_const:
		case LITERAL___const:
		case LITERAL_auto:
		case LITERAL_register:
		case LITERAL_static:
		case LITERAL_mutable:
		case LITERAL_const_cast:
		case LITERAL_volatile:
		case LITERAL___volatile:
		case LITERAL___restrict:
		case LITERAL___restrict__:
		case LITERAL_char:
		case LITERAL_bool:
		case LITERAL_short:
		case LITERAL_int:
		case 73:
		case 74:
		case 75:
		case LITERAL_long:
		case LITERAL_signed:
		case LITERAL___signed:
		case LITERAL___signed__:
		case LITERAL_unsigned:
		case LITERAL_float:
		case LITERAL_double:
		case LITERAL_void:
		case LITERAL__declspec:
		case LITERAL___declspec:
		case LITERAL___builtin_va_list:
		case LITERAL_class:
		case LITERAL_struct:
		case LITERAL_union:
		case LITERAL_using:
		case SCOPE:
		{
			d=declaration();
			if ( inputState->guessing==0 ) {
#line 708 "CPP_parser_cpp.g"
				node->add_child(d);
#line 4180 "CPPParser.cpp"
			}
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_15);
		} else {
			throw;
		}
	}
	return node;
}

ASTNode * CPPParser::attribute() {
#line 761 "CPP_parser_cpp.g"
	ASTNode *a;
#line 4205 "CPPParser.cpp"
#line 761 "CPP_parser_cpp.g"
	
		ASTNode *adl;
		a = mk_node("attribute");
		
#line 4211 "CPPParser.cpp"
	
	try {      // for error handling
		match(LITERAL___attribute__);
		match(LPAREN);
		match(LPAREN);
		adl=attribute_decl_list();
		if ( inputState->guessing==0 ) {
#line 766 "CPP_parser_cpp.g"
			a->add_child(adl);
#line 4221 "CPPParser.cpp"
		}
		match(RPAREN);
		match(RPAREN);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_48);
		} else {
			throw;
		}
	}
	return a;
}

ASTNode * CPPParser::using_declaration() {
#line 1857 "CPP_parser_cpp.g"
	ASTNode *node;
#line 4240 "CPPParser.cpp"
#line 1857 "CPP_parser_cpp.g"
	
		const char *qid;
		node = mk_node("unfinished_using_declaration");
		
#line 4246 "CPPParser.cpp"
	
	try {      // for error handling
		match(LITERAL_using);
		{
		switch ( LA(1)) {
		case LITERAL_namespace:
		{
			match(LITERAL_namespace);
			qid=qualified_id();
			break;
		}
		case ID:
		case OPERATOR:
		case LITERAL_this:
		case LITERAL_true:
		case LITERAL_false:
		case SCOPE:
		{
			qid=qualified_id();
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		match(SEMICOLON);
		if ( inputState->guessing==0 ) {
#line 1866 "CPP_parser_cpp.g"
			end_of_stmt();
#line 4278 "CPPParser.cpp"
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_15);
		} else {
			throw;
		}
	}
	return node;
}

char * CPPParser::storage_class_specifier() {
#line 806 "CPP_parser_cpp.g"
	char *s;
#line 4295 "CPPParser.cpp"
	
	try {      // for error handling
		switch ( LA(1)) {
		case LITERAL_auto:
		{
			match(LITERAL_auto);
			if ( inputState->guessing==0 ) {
#line 807 "CPP_parser_cpp.g"
				s = "auto";
#line 4305 "CPPParser.cpp"
			}
			break;
		}
		case LITERAL_register:
		{
			match(LITERAL_register);
			if ( inputState->guessing==0 ) {
#line 808 "CPP_parser_cpp.g"
				s = "register";
#line 4315 "CPPParser.cpp"
			}
			break;
		}
		case LITERAL_static:
		{
			match(LITERAL_static);
			if ( inputState->guessing==0 ) {
#line 809 "CPP_parser_cpp.g"
				s = "static";
#line 4325 "CPPParser.cpp"
			}
			break;
		}
		case LITERAL_extern:
		{
			match(LITERAL_extern);
			if ( inputState->guessing==0 ) {
#line 810 "CPP_parser_cpp.g"
				s = "extern";
#line 4335 "CPPParser.cpp"
			}
			break;
		}
		case LITERAL_mutable:
		{
			match(LITERAL_mutable);
			if ( inputState->guessing==0 ) {
#line 811 "CPP_parser_cpp.g"
				s = "mutable";
#line 4345 "CPPParser.cpp"
			}
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_49);
		} else {
			throw;
		}
	}
	return s;
}

char * CPPParser::type_qualifier() {
#line 814 "CPP_parser_cpp.g"
	char *s;
#line 4369 "CPPParser.cpp"
	
	try {      // for error handling
		switch ( LA(1)) {
		case LITERAL_const:
		case LITERAL___const:
		case LITERAL_const_cast:
		{
			{
			switch ( LA(1)) {
			case LITERAL_const:
			{
				match(LITERAL_const);
				break;
			}
			case LITERAL_const_cast:
			{
				match(LITERAL_const_cast);
				break;
			}
			case LITERAL___const:
			{
				match(LITERAL___const);
				break;
			}
			default:
			{
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
			}
			}
			}
			if ( inputState->guessing==0 ) {
#line 815 "CPP_parser_cpp.g"
				s = "const";
#line 4403 "CPPParser.cpp"
			}
			break;
		}
		case LITERAL_volatile:
		case LITERAL___volatile:
		{
			{
			switch ( LA(1)) {
			case LITERAL_volatile:
			{
				match(LITERAL_volatile);
				break;
			}
			case LITERAL___volatile:
			{
				match(LITERAL___volatile);
				break;
			}
			default:
			{
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
			}
			}
			}
			if ( inputState->guessing==0 ) {
#line 816 "CPP_parser_cpp.g"
				s = "volatile";;
#line 4431 "CPPParser.cpp"
			}
			break;
		}
		case LITERAL___restrict:
		case LITERAL___restrict__:
		{
			{
			switch ( LA(1)) {
			case LITERAL___restrict:
			{
				match(LITERAL___restrict);
				break;
			}
			case LITERAL___restrict__:
			{
				match(LITERAL___restrict__);
				break;
			}
			default:
			{
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
			}
			}
			}
			if ( inputState->guessing==0 ) {
#line 817 "CPP_parser_cpp.g"
				s = "__restrict";
#line 4459 "CPPParser.cpp"
			}
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_50);
		} else {
			throw;
		}
	}
	return s;
}

ASTNode * CPPParser::type_specifier(
	DeclSpecifier ds
) {
#line 820 "CPP_parser_cpp.g"
	ASTNode *node;
#line 4485 "CPPParser.cpp"
	
	try {      // for error handling
		switch ( LA(1)) {
		case ID:
		case LITERAL_char:
		case LITERAL_bool:
		case LITERAL_short:
		case LITERAL_int:
		case 73:
		case 74:
		case 75:
		case LITERAL_long:
		case LITERAL_signed:
		case LITERAL___signed:
		case LITERAL___signed__:
		case LITERAL_unsigned:
		case LITERAL_float:
		case LITERAL_double:
		case LITERAL_void:
		case LITERAL__declspec:
		case LITERAL___declspec:
		case LITERAL___builtin_va_list:
		case SCOPE:
		{
			node=simple_type_specifier();
			break;
		}
		case LITERAL_class:
		case LITERAL_struct:
		case LITERAL_union:
		{
			node=class_specifier(ds);
			break;
		}
		case LITERAL_enum:
		{
			node=enum_specifier();
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_17);
		} else {
			throw;
		}
	}
	return node;
}

ASTNode * CPPParser::direct_declarator() {
#line 1169 "CPP_parser_cpp.g"
	ASTNode *node;
#line 4545 "CPPParser.cpp"
	ANTLR_USE_NAMESPACE(antlr)RefToken  dtor = ANTLR_USE_NAMESPACE(antlr)nullToken;
#line 1169 "CPP_parser_cpp.g"
	
		const char *id, *tq;
		ASTNode *d, *ds, *pl, *es, *el, *ce;
		
#line 4552 "CPPParser.cpp"
	
	try {      // for error handling
		switch ( LA(1)) {
		case TILDE:
		{
			if ( inputState->guessing==0 ) {
#line 1218 "CPP_parser_cpp.g"
				node = mk_node("direct_declarator");
#line 4561 "CPPParser.cpp"
			}
			match(TILDE);
			dtor = LT(1);
			match(ID);
			if ( inputState->guessing==0 ) {
#line 1221 "CPP_parser_cpp.g"
				declaratorID(dtor->getText().data(),qiDtor);
#line 4569 "CPPParser.cpp"
			}
			if ( inputState->guessing==0 ) {
#line 1222 "CPP_parser_cpp.g"
				printf("%d warning direct_declarator5 entered unexpectedly with %s\n",
								LT(1)->getLine(),(dtor->getText()).data());
#line 4575 "CPPParser.cpp"
			}
			match(LPAREN);
			if ( inputState->guessing==0 ) {
#line 1224 "CPP_parser_cpp.g"
				declaratorParameterList(0);
#line 4581 "CPPParser.cpp"
			}
			{
			switch ( LA(1)) {
			case LITERAL_typedef:
			case LITERAL_enum:
			case ID:
			case LITERAL_inline:
			case LITERAL_extern:
			case LITERAL___extension__:
			case LITERAL__inline:
			case LITERAL___inline:
			case LITERAL___inline__:
			case LITERAL_virtual:
			case LITERAL_explicit:
			case LITERAL_friend:
			case LITERAL__stdcall:
			case LITERAL___stdcall:
			case LITERAL_typename:
			case LPAREN:
			case LITERAL_const:
			case LITERAL___const:
			case LITERAL_auto:
			case LITERAL_register:
			case LITERAL_static:
			case LITERAL_mutable:
			case LITERAL_const_cast:
			case LITERAL_volatile:
			case LITERAL___volatile:
			case LITERAL___restrict:
			case LITERAL___restrict__:
			case LITERAL_char:
			case LITERAL_bool:
			case LITERAL_short:
			case LITERAL_int:
			case 73:
			case 74:
			case 75:
			case LITERAL_long:
			case LITERAL_signed:
			case LITERAL___signed:
			case LITERAL___signed__:
			case LITERAL_unsigned:
			case LITERAL_float:
			case LITERAL_double:
			case LITERAL_void:
			case LITERAL__declspec:
			case LITERAL___declspec:
			case LITERAL___builtin_va_list:
			case LITERAL_class:
			case LITERAL_struct:
			case LITERAL_union:
			case OPERATOR:
			case LITERAL_this:
			case LITERAL_true:
			case LITERAL_false:
			case STAR:
			case AMPERSAND:
			case TILDE:
			case ELLIPSIS:
			case SCOPE:
			case LITERAL__cdecl:
			case LITERAL___cdecl:
			case LITERAL__near:
			case LITERAL___near:
			case LITERAL__far:
			case LITERAL___far:
			case LITERAL___interrupt:
			case LITERAL_pascal:
			case LITERAL__pascal:
			case LITERAL___pascal:
			{
				pl=parameter_list();
				break;
			}
			case RPAREN:
			{
				break;
			}
			default:
			{
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
			}
			}
			}
			match(RPAREN);
			if ( inputState->guessing==0 ) {
#line 1226 "CPP_parser_cpp.g"
				declaratorEndParameterList(0);
#line 4670 "CPPParser.cpp"
			}
			if ( inputState->guessing==0 ) {
#line 1227 "CPP_parser_cpp.g"
				node->add_child(mk_node("unfinished_direct_declarator"));
#line 4675 "CPPParser.cpp"
			}
			break;
		}
		case LPAREN:
		{
			match(LPAREN);
			d=declarator();
			if ( inputState->guessing==0 ) {
#line 1229 "CPP_parser_cpp.g"
				node = d;
#line 4686 "CPPParser.cpp"
			}
			match(RPAREN);
			ds=declarator_suffixes();
			if ( inputState->guessing==0 ) {
#line 1229 "CPP_parser_cpp.g"
				node->add_child(ds);
#line 4693 "CPPParser.cpp"
			}
			break;
		}
		default:
			bool synPredMatched215 = false;
			if (((_tokenSet_41.member(LA(1))) && (_tokenSet_27.member(LA(2))))) {
				int _m215 = mark();
				synPredMatched215 = true;
				inputState->guessing++;
				try {
					{
					qualified_id();
					match(LPAREN);
					{
					switch ( LA(1)) {
					case RPAREN:
					{
						match(RPAREN);
						break;
					}
					case LITERAL_typedef:
					case LITERAL_enum:
					case ID:
					case LITERAL_inline:
					case LITERAL_extern:
					case LITERAL___extension__:
					case LITERAL__inline:
					case LITERAL___inline:
					case LITERAL___inline__:
					case LITERAL_virtual:
					case LITERAL_explicit:
					case LITERAL_friend:
					case LITERAL__stdcall:
					case LITERAL___stdcall:
					case LITERAL_typename:
					case LITERAL_const:
					case LITERAL___const:
					case LITERAL_auto:
					case LITERAL_register:
					case LITERAL_static:
					case LITERAL_mutable:
					case LITERAL_const_cast:
					case LITERAL_volatile:
					case LITERAL___volatile:
					case LITERAL___restrict:
					case LITERAL___restrict__:
					case LITERAL_char:
					case LITERAL_bool:
					case LITERAL_short:
					case LITERAL_int:
					case 73:
					case 74:
					case 75:
					case LITERAL_long:
					case LITERAL_signed:
					case LITERAL___signed:
					case LITERAL___signed__:
					case LITERAL_unsigned:
					case LITERAL_float:
					case LITERAL_double:
					case LITERAL_void:
					case LITERAL__declspec:
					case LITERAL___declspec:
					case LITERAL___builtin_va_list:
					case LITERAL_class:
					case LITERAL_struct:
					case LITERAL_union:
					case SCOPE:
					{
						declaration_specifiers();
						break;
					}
					default:
					{
						throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
					}
					}
					}
					}
				}
				catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
					synPredMatched215 = false;
				}
				rewind(_m215);
				inputState->guessing--;
			}
			if ( synPredMatched215 ) {
				if ( inputState->guessing==0 ) {
#line 1175 "CPP_parser_cpp.g"
					node = mk_node("direct_declarator");
#line 4784 "CPPParser.cpp"
				}
				id=qualified_id();
				if ( inputState->guessing==0 ) {
#line 1177 "CPP_parser_cpp.g"
					node->set_leaf(id);
#line 4790 "CPPParser.cpp"
				}
				if ( inputState->guessing==0 ) {
#line 1178 "CPP_parser_cpp.g"
					declaratorID(id, qiFun);
#line 4795 "CPPParser.cpp"
				}
				match(LPAREN);
				if ( inputState->guessing==0 ) {
#line 1179 "CPP_parser_cpp.g"
					declaratorParameterList(0);
#line 4801 "CPPParser.cpp"
				}
				{
				switch ( LA(1)) {
				case LITERAL_typedef:
				case LITERAL_enum:
				case ID:
				case LITERAL_inline:
				case LITERAL_extern:
				case LITERAL___extension__:
				case LITERAL__inline:
				case LITERAL___inline:
				case LITERAL___inline__:
				case LITERAL_virtual:
				case LITERAL_explicit:
				case LITERAL_friend:
				case LITERAL__stdcall:
				case LITERAL___stdcall:
				case LITERAL_typename:
				case LPAREN:
				case LITERAL_const:
				case LITERAL___const:
				case LITERAL_auto:
				case LITERAL_register:
				case LITERAL_static:
				case LITERAL_mutable:
				case LITERAL_const_cast:
				case LITERAL_volatile:
				case LITERAL___volatile:
				case LITERAL___restrict:
				case LITERAL___restrict__:
				case LITERAL_char:
				case LITERAL_bool:
				case LITERAL_short:
				case LITERAL_int:
				case 73:
				case 74:
				case 75:
				case LITERAL_long:
				case LITERAL_signed:
				case LITERAL___signed:
				case LITERAL___signed__:
				case LITERAL_unsigned:
				case LITERAL_float:
				case LITERAL_double:
				case LITERAL_void:
				case LITERAL__declspec:
				case LITERAL___declspec:
				case LITERAL___builtin_va_list:
				case LITERAL_class:
				case LITERAL_struct:
				case LITERAL_union:
				case OPERATOR:
				case LITERAL_this:
				case LITERAL_true:
				case LITERAL_false:
				case STAR:
				case AMPERSAND:
				case TILDE:
				case ELLIPSIS:
				case SCOPE:
				case LITERAL__cdecl:
				case LITERAL___cdecl:
				case LITERAL__near:
				case LITERAL___near:
				case LITERAL__far:
				case LITERAL___far:
				case LITERAL___interrupt:
				case LITERAL_pascal:
				case LITERAL__pascal:
				case LITERAL___pascal:
				{
					pl=parameter_list();
					if ( inputState->guessing==0 ) {
#line 1180 "CPP_parser_cpp.g"
						node->add_child(pl);
#line 4877 "CPPParser.cpp"
					}
					break;
				}
				case RPAREN:
				{
					break;
				}
				default:
				{
					throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
				}
				}
				}
				match(RPAREN);
				if ( inputState->guessing==0 ) {
#line 1181 "CPP_parser_cpp.g"
					declaratorEndParameterList(0);
#line 4895 "CPPParser.cpp"
				}
				{ // ( ... )*
				for (;;) {
					if ((_tokenSet_51.member(LA(1)))) {
						tq=type_qualifier();
						if ( inputState->guessing==0 ) {
#line 1182 "CPP_parser_cpp.g"
							node->add_child(mk_node(tq));
#line 4904 "CPPParser.cpp"
						}
					}
					else {
						goto _loop218;
					}
					
				}
				_loop218:;
				} // ( ... )*
				{
				switch ( LA(1)) {
				case LITERAL_throw:
				{
					es=exception_specification();
					if ( inputState->guessing==0 ) {
#line 1183 "CPP_parser_cpp.g"
						node->add_child(es);
#line 4922 "CPPParser.cpp"
					}
					break;
				}
				case LESSTHAN:
				case GREATERTHAN:
				case ID:
				case SEMICOLON:
				case RCURLY:
				case ASSIGNEQUAL:
				case COLON:
				case COMMA:
				case LITERAL__stdcall:
				case LITERAL___stdcall:
				case LITERAL___attribute__:
				case LPAREN:
				case RPAREN:
				case OPERATOR:
				case LITERAL_this:
				case LITERAL_true:
				case LITERAL_false:
				case STAR:
				case AMPERSAND:
				case LSQUARE:
				case RSQUARE:
				case TILDE:
				case LITERAL_asm:
				case LITERAL___asm:
				case LITERAL___asm__:
				case ELLIPSIS:
				case SCOPE:
				case TIMESEQUAL:
				case DIVIDEEQUAL:
				case MINUSEQUAL:
				case PLUSEQUAL:
				case MODEQUAL:
				case SHIFTLEFTEQUAL:
				case SHIFTRIGHTEQUAL:
				case BITWISEANDEQUAL:
				case BITWISEXOREQUAL:
				case BITWISEOREQUAL:
				case QUESTIONMARK:
				case OR:
				case AND:
				case BITWISEOR:
				case BITWISEXOR:
				case NOTEQUAL:
				case EQUAL:
				case LESSTHANOREQUALTO:
				case GREATERTHANOREQUALTO:
				case SHIFTLEFT:
				case SHIFTRIGHT:
				case PLUS:
				case MINUS:
				case DIVIDE:
				case MOD:
				case DOTMBR:
				case POINTERTOMBR:
				case LITERAL__cdecl:
				case LITERAL___cdecl:
				case LITERAL__near:
				case LITERAL___near:
				case LITERAL__far:
				case LITERAL___far:
				case LITERAL___interrupt:
				case LITERAL_pascal:
				case LITERAL__pascal:
				case LITERAL___pascal:
				{
					break;
				}
				default:
				{
					throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
				}
				}
				}
			}
			else {
				bool synPredMatched221 = false;
				if (((_tokenSet_41.member(LA(1))) && (_tokenSet_27.member(LA(2))))) {
					int _m221 = mark();
					synPredMatched221 = true;
					inputState->guessing++;
					try {
						{
						qualified_id();
						match(LPAREN);
						qualified_id();
						}
					}
					catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
						synPredMatched221 = false;
					}
					rewind(_m221);
					inputState->guessing--;
				}
				if ( synPredMatched221 ) {
					if ( inputState->guessing==0 ) {
#line 1185 "CPP_parser_cpp.g"
						node = mk_node("direct_declarator");
#line 5023 "CPPParser.cpp"
					}
					id=qualified_id();
					if ( inputState->guessing==0 ) {
#line 1186 "CPP_parser_cpp.g"
						node->set_leaf(id);
#line 5029 "CPPParser.cpp"
					}
					if ( inputState->guessing==0 ) {
#line 1187 "CPP_parser_cpp.g"
						declaratorID(id,qiVar);
#line 5034 "CPPParser.cpp"
					}
					match(LPAREN);
					el=expression_list();
					if ( inputState->guessing==0 ) {
#line 1189 "CPP_parser_cpp.g"
						node->add_child(el);
#line 5041 "CPPParser.cpp"
					}
					match(RPAREN);
				}
				else {
					bool synPredMatched223 = false;
					if (((_tokenSet_41.member(LA(1))) && (_tokenSet_27.member(LA(2))))) {
						int _m223 = mark();
						synPredMatched223 = true;
						inputState->guessing++;
						try {
							{
							qualified_id();
							match(LSQUARE);
							}
						}
						catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
							synPredMatched223 = false;
						}
						rewind(_m223);
						inputState->guessing--;
					}
					if ( synPredMatched223 ) {
						if ( inputState->guessing==0 ) {
#line 1193 "CPP_parser_cpp.g"
							node = mk_node("direct_declarator");
#line 5067 "CPPParser.cpp"
						}
						id=qualified_id();
						if ( inputState->guessing==0 ) {
#line 1194 "CPP_parser_cpp.g"
							node->set_leaf(id);
#line 5073 "CPPParser.cpp"
						}
						if ( inputState->guessing==0 ) {
#line 1195 "CPP_parser_cpp.g"
							if (_td == true)
										declaratorID(id, qiType);
									else
										declaratorID(id, qiVar);
									is_address = false;
									is_pointer = false;
									
#line 5084 "CPPParser.cpp"
						}
						{ // ( ... )+
						int _cnt226=0;
						for (;;) {
							if ((LA(1) == LSQUARE) && (_tokenSet_52.member(LA(2)))) {
								match(LSQUARE);
								{
								switch ( LA(1)) {
								case ID:
								case StringLiteral:
								case LPAREN:
								case LITERAL_const_cast:
								case LITERAL_char:
								case LITERAL_bool:
								case LITERAL_short:
								case LITERAL_int:
								case 73:
								case 74:
								case 75:
								case LITERAL_long:
								case LITERAL_signed:
								case LITERAL___signed:
								case LITERAL___signed__:
								case LITERAL_unsigned:
								case LITERAL_float:
								case LITERAL_double:
								case LITERAL_void:
								case LITERAL__declspec:
								case LITERAL___declspec:
								case LITERAL___builtin_va_list:
								case OPERATOR:
								case LITERAL_this:
								case LITERAL_true:
								case LITERAL_false:
								case OCTALINT:
								case STAR:
								case AMPERSAND:
								case TILDE:
								case SCOPE:
								case PLUS:
								case MINUS:
								case PLUSPLUS:
								case MINUSMINUS:
								case LITERAL_sizeof:
								case LITERAL_dynamic_cast:
								case LITERAL_static_cast:
								case LITERAL_reinterpret_cast:
								case NOT:
								case LITERAL_new:
								case LITERAL_delete:
								case DECIMALINT:
								case HEXADECIMALINT:
								case CharLiteral:
								case FLOATONE:
								case FLOATTWO:
								{
									ce=constant_expression();
									if ( inputState->guessing==0 ) {
#line 1203 "CPP_parser_cpp.g"
										node->add_child(ce);
#line 5145 "CPPParser.cpp"
									}
									break;
								}
								case RSQUARE:
								{
									break;
								}
								default:
								{
									throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
								}
								}
								}
								match(RSQUARE);
							}
							else {
								if ( _cnt226>=1 ) { goto _loop226; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());}
							}
							
							_cnt226++;
						}
						_loop226:;
						}  // ( ... )+
						if ( inputState->guessing==0 ) {
#line 1205 "CPP_parser_cpp.g"
							declaratorArray();
#line 5172 "CPPParser.cpp"
						}
					}
					else if ((_tokenSet_41.member(LA(1))) && (_tokenSet_53.member(LA(2)))) {
						if ( inputState->guessing==0 ) {
#line 1208 "CPP_parser_cpp.g"
							node = mk_node("direct_declarator");
#line 5179 "CPPParser.cpp"
						}
						id=qualified_id();
						if ( inputState->guessing==0 ) {
#line 1209 "CPP_parser_cpp.g"
							node->set_leaf(id);
#line 5185 "CPPParser.cpp"
						}
						if ( inputState->guessing==0 ) {
#line 1210 "CPP_parser_cpp.g"
							if (_td == true)
										declaratorID(id,qiType);
									else
										declaratorID(id,qiVar);
									is_address = false;
									is_pointer = false;
									
#line 5196 "CPPParser.cpp"
						}
					}
		else {
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}}}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_54);
		} else {
			throw;
		}
	}
	return node;
}

ASTNode * CPPParser::attribute_decl_list() {
#line 769 "CPP_parser_cpp.g"
	ASTNode *node;
#line 5218 "CPPParser.cpp"
#line 769 "CPP_parser_cpp.g"
	
		node = mk_node("attribute_decl_list");
	
		ASTNode *c1, *c2;
		
#line 5225 "CPPParser.cpp"
	
	try {      // for error handling
		switch ( LA(1)) {
		case LITERAL___mode__:
		{
			match(LITERAL___mode__);
			match(LPAREN);
			match(ID);
			match(RPAREN);
			if ( inputState->guessing==0 ) {
#line 775 "CPP_parser_cpp.g"
				node->set_name("mode");
#line 5238 "CPPParser.cpp"
			}
			break;
		}
		case LITERAL_const:
		{
			match(LITERAL_const);
			if ( inputState->guessing==0 ) {
#line 776 "CPP_parser_cpp.g"
				node->set_name("const");
#line 5248 "CPPParser.cpp"
			}
			break;
		}
		case LITERAL___const:
		{
			match(LITERAL___const);
			if ( inputState->guessing==0 ) {
#line 777 "CPP_parser_cpp.g"
				node->set_name("const");
#line 5258 "CPPParser.cpp"
			}
			break;
		}
		case LITERAL___noreturn__:
		{
			match(LITERAL___noreturn__);
			if ( inputState->guessing==0 ) {
#line 778 "CPP_parser_cpp.g"
				node->set_name("noreturn");
#line 5268 "CPPParser.cpp"
			}
			break;
		}
		case LITERAL___returns_twice__:
		{
			match(LITERAL___returns_twice__);
			if ( inputState->guessing==0 ) {
#line 779 "CPP_parser_cpp.g"
				node->set_name("returns_twice");
#line 5278 "CPPParser.cpp"
			}
			break;
		}
		case LITERAL___noinline__:
		{
			match(LITERAL___noinline__);
			if ( inputState->guessing==0 ) {
#line 780 "CPP_parser_cpp.g"
				node->set_name("noinline");
#line 5288 "CPPParser.cpp"
			}
			break;
		}
		case LITERAL___always_inline__:
		{
			match(LITERAL___always_inline__);
			if ( inputState->guessing==0 ) {
#line 781 "CPP_parser_cpp.g"
				node->set_name("always_inline");
#line 5298 "CPPParser.cpp"
			}
			break;
		}
		case LITERAL___flatten__:
		{
			match(LITERAL___flatten__);
			if ( inputState->guessing==0 ) {
#line 782 "CPP_parser_cpp.g"
				node->set_name("flatten");
#line 5308 "CPPParser.cpp"
			}
			break;
		}
		case LITERAL___pure__:
		{
			match(LITERAL___pure__);
			if ( inputState->guessing==0 ) {
#line 783 "CPP_parser_cpp.g"
				node->set_name("pure");
#line 5318 "CPPParser.cpp"
			}
			break;
		}
		case LITERAL___const__:
		{
			match(LITERAL___const__);
			if ( inputState->guessing==0 ) {
#line 784 "CPP_parser_cpp.g"
				node->set_name("const");
#line 5328 "CPPParser.cpp"
			}
			break;
		}
		case LITERAL___nothrow__:
		{
			match(LITERAL___nothrow__);
			if ( inputState->guessing==0 ) {
#line 785 "CPP_parser_cpp.g"
				node->set_name("nothrow");
#line 5338 "CPPParser.cpp"
			}
			break;
		}
		case LITERAL___sentinel__:
		{
			match(LITERAL___sentinel__);
			if ( inputState->guessing==0 ) {
#line 786 "CPP_parser_cpp.g"
				node->set_name("sentinel");
#line 5348 "CPPParser.cpp"
			}
			break;
		}
		case LITERAL___format__:
		{
			match(LITERAL___format__);
			if ( inputState->guessing==0 ) {
#line 787 "CPP_parser_cpp.g"
				node->set_name("format");
#line 5358 "CPPParser.cpp"
			}
			match(LPAREN);
			match(ID);
			match(COMMA);
			c1=constant();
			if ( inputState->guessing==0 ) {
#line 787 "CPP_parser_cpp.g"
				node->add_child(c1);
#line 5367 "CPPParser.cpp"
			}
			match(COMMA);
			c2=constant();
			if ( inputState->guessing==0 ) {
#line 788 "CPP_parser_cpp.g"
				node->add_child(c2);
#line 5374 "CPPParser.cpp"
			}
			match(RPAREN);
			break;
		}
		case LITERAL___format_arg__:
		{
			match(LITERAL___format_arg__);
			if ( inputState->guessing==0 ) {
#line 789 "CPP_parser_cpp.g"
				node->set_name("format_arg");
#line 5385 "CPPParser.cpp"
			}
			break;
		}
		case LITERAL___no_instrument_function__:
		{
			match(LITERAL___no_instrument_function__);
			if ( inputState->guessing==0 ) {
#line 790 "CPP_parser_cpp.g"
				node->set_name("no_instrument_function");
#line 5395 "CPPParser.cpp"
			}
			break;
		}
		case LITERAL___section__:
		{
			match(LITERAL___section__);
			if ( inputState->guessing==0 ) {
#line 791 "CPP_parser_cpp.g"
				node->set_name("section");
#line 5405 "CPPParser.cpp"
			}
			break;
		}
		case LITERAL___constructor__:
		{
			match(LITERAL___constructor__);
			if ( inputState->guessing==0 ) {
#line 792 "CPP_parser_cpp.g"
				node->set_name("constructor");
#line 5415 "CPPParser.cpp"
			}
			break;
		}
		case LITERAL___destructor__:
		{
			match(LITERAL___destructor__);
			if ( inputState->guessing==0 ) {
#line 793 "CPP_parser_cpp.g"
				node->set_name("destructor");
#line 5425 "CPPParser.cpp"
			}
			break;
		}
		case LITERAL___used__:
		{
			match(LITERAL___used__);
			if ( inputState->guessing==0 ) {
#line 794 "CPP_parser_cpp.g"
				node->set_name("used");
#line 5435 "CPPParser.cpp"
			}
			break;
		}
		case LITERAL___unused__:
		{
			match(LITERAL___unused__);
			if ( inputState->guessing==0 ) {
#line 795 "CPP_parser_cpp.g"
				node->set_name("unused");
#line 5445 "CPPParser.cpp"
			}
			break;
		}
		case LITERAL___deprecated__:
		{
			match(LITERAL___deprecated__);
			if ( inputState->guessing==0 ) {
#line 796 "CPP_parser_cpp.g"
				node->set_name("deprecated");
#line 5455 "CPPParser.cpp"
			}
			break;
		}
		case LITERAL___weak__:
		{
			match(LITERAL___weak__);
			if ( inputState->guessing==0 ) {
#line 797 "CPP_parser_cpp.g"
				node->set_name("weak");
#line 5465 "CPPParser.cpp"
			}
			break;
		}
		case LITERAL___malloc__:
		{
			match(LITERAL___malloc__);
			if ( inputState->guessing==0 ) {
#line 798 "CPP_parser_cpp.g"
				node->set_name("malloc");
#line 5475 "CPPParser.cpp"
			}
			break;
		}
		case LITERAL___alias__:
		{
			match(LITERAL___alias__);
			if ( inputState->guessing==0 ) {
#line 799 "CPP_parser_cpp.g"
				node->set_name("alias");
#line 5485 "CPPParser.cpp"
			}
			break;
		}
		case LITERAL___warn_unused_result__:
		{
			match(LITERAL___warn_unused_result__);
			if ( inputState->guessing==0 ) {
#line 800 "CPP_parser_cpp.g"
				node->set_name("warn_unused_result");
#line 5495 "CPPParser.cpp"
			}
			break;
		}
		case LITERAL___nonnull__:
		{
			match(LITERAL___nonnull__);
			if ( inputState->guessing==0 ) {
#line 801 "CPP_parser_cpp.g"
				node->set_name("nonnull");
#line 5505 "CPPParser.cpp"
			}
			match(LPAREN);
			c1=constant();
			if ( inputState->guessing==0 ) {
#line 801 "CPP_parser_cpp.g"
				node->add_child(c1);
#line 5512 "CPPParser.cpp"
			}
			{ // ( ... )*
			for (;;) {
				if ((LA(1) == COMMA)) {
					match(COMMA);
					c2=constant();
					if ( inputState->guessing==0 ) {
#line 802 "CPP_parser_cpp.g"
						node->add_child(c2);
#line 5522 "CPPParser.cpp"
					}
				}
				else {
					goto _loop122;
				}
				
			}
			_loop122:;
			} // ( ... )*
			match(RPAREN);
			break;
		}
		case LITERAL___externally_visible__:
		{
			match(LITERAL___externally_visible__);
			if ( inputState->guessing==0 ) {
#line 803 "CPP_parser_cpp.g"
				node->set_name("externally_visible");
#line 5541 "CPPParser.cpp"
			}
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_55);
		} else {
			throw;
		}
	}
	return node;
}

ASTNode * CPPParser::constant() {
#line 2434 "CPP_parser_cpp.g"
	ASTNode *node;
#line 5565 "CPPParser.cpp"
	ANTLR_USE_NAMESPACE(antlr)RefToken  oi = ANTLR_USE_NAMESPACE(antlr)nullToken;
	ANTLR_USE_NAMESPACE(antlr)RefToken  di = ANTLR_USE_NAMESPACE(antlr)nullToken;
	ANTLR_USE_NAMESPACE(antlr)RefToken  hi = ANTLR_USE_NAMESPACE(antlr)nullToken;
	ANTLR_USE_NAMESPACE(antlr)RefToken  cl = ANTLR_USE_NAMESPACE(antlr)nullToken;
	ANTLR_USE_NAMESPACE(antlr)RefToken  sl = ANTLR_USE_NAMESPACE(antlr)nullToken;
	ANTLR_USE_NAMESPACE(antlr)RefToken  f1 = ANTLR_USE_NAMESPACE(antlr)nullToken;
	ANTLR_USE_NAMESPACE(antlr)RefToken  f2 = ANTLR_USE_NAMESPACE(antlr)nullToken;
#line 2434 "CPP_parser_cpp.g"
	
		ASTNode *litnode;
		node = mk_node("expression"); node->set_leaf("literal");
		litnode = mk_node("literal"); node->add_child(litnode);
		
#line 5579 "CPPParser.cpp"
	
	try {      // for error handling
		{
		switch ( LA(1)) {
		case OCTALINT:
		{
			oi = LT(1);
			match(OCTALINT);
			if ( inputState->guessing==0 ) {
#line 2441 "CPP_parser_cpp.g"
				litnode->set_leaf(oi->getText().data()); litnode->set_name("int");
#line 5591 "CPPParser.cpp"
			}
			break;
		}
		case DECIMALINT:
		{
			di = LT(1);
			match(DECIMALINT);
			if ( inputState->guessing==0 ) {
#line 2442 "CPP_parser_cpp.g"
				litnode->set_leaf(di->getText().data()); litnode->set_name("int");
#line 5602 "CPPParser.cpp"
			}
			break;
		}
		case HEXADECIMALINT:
		{
			hi = LT(1);
			match(HEXADECIMALINT);
			if ( inputState->guessing==0 ) {
#line 2443 "CPP_parser_cpp.g"
				litnode->set_leaf(hi->getText().data()); litnode->set_name("int");
#line 5613 "CPPParser.cpp"
			}
			break;
		}
		case CharLiteral:
		{
			cl = LT(1);
			match(CharLiteral);
			if ( inputState->guessing==0 ) {
#line 2444 "CPP_parser_cpp.g"
				litnode->set_leaf(cl->getText().data()); litnode->set_name("char");
#line 5624 "CPPParser.cpp"
			}
			break;
		}
		case StringLiteral:
		{
			{ // ( ... )+
			int _cnt507=0;
			for (;;) {
				if ((LA(1) == StringLiteral)) {
					sl = LT(1);
					match(StringLiteral);
					if ( inputState->guessing==0 ) {
#line 2445 "CPP_parser_cpp.g"
						litnode->extend_leaf(sl->getText().data());
#line 5639 "CPPParser.cpp"
					}
				}
				else {
					if ( _cnt507>=1 ) { goto _loop507; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());}
				}
				
				_cnt507++;
			}
			_loop507:;
			}  // ( ... )+
			if ( inputState->guessing==0 ) {
#line 2445 "CPP_parser_cpp.g"
				litnode->set_name("string");
#line 5653 "CPPParser.cpp"
			}
			break;
		}
		case FLOATONE:
		{
			f1 = LT(1);
			match(FLOATONE);
			if ( inputState->guessing==0 ) {
#line 2446 "CPP_parser_cpp.g"
				litnode->set_leaf(f1->getText().data()); litnode->set_name("float");
#line 5664 "CPPParser.cpp"
			}
			break;
		}
		case FLOATTWO:
		{
			f2 = LT(1);
			match(FLOATTWO);
			if ( inputState->guessing==0 ) {
#line 2447 "CPP_parser_cpp.g"
				litnode->set_leaf(f2->getText().data()); litnode->set_name("float");
#line 5675 "CPPParser.cpp"
			}
			break;
		}
		case LITERAL_true:
		{
			match(LITERAL_true);
			if ( inputState->guessing==0 ) {
#line 2448 "CPP_parser_cpp.g"
				litnode->set_leaf("true"); litnode->set_name("bool");
#line 5685 "CPPParser.cpp"
			}
			break;
		}
		case LITERAL_false:
		{
			match(LITERAL_false);
			if ( inputState->guessing==0 ) {
#line 2449 "CPP_parser_cpp.g"
				litnode->set_leaf("false"); litnode->set_name("bool");
#line 5695 "CPPParser.cpp"
			}
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_56);
		} else {
			throw;
		}
	}
	return node;
}

ASTNode * CPPParser::simple_type_specifier() {
#line 826 "CPP_parser_cpp.g"
	ASTNode *node;
#line 5720 "CPPParser.cpp"
#line 826 "CPP_parser_cpp.g"
	
		char *s;
		node = mk_node("simple_type_specifier");
		
#line 5726 "CPPParser.cpp"
	
	try {      // for error handling
		{
		if (((LA(1) == ID || LA(1) == SCOPE) && (_tokenSet_17.member(LA(2))))&&(qualifiedItemIsOneOf(qiType|qiCtor))) {
			s=qualified_type();
			if ( inputState->guessing==0 ) {
#line 832 "CPP_parser_cpp.g"
				node->add_child(mk_node(s));
#line 5735 "CPPParser.cpp"
			}
		}
		else if (((LA(1) >= LITERAL_char && LA(1) <= LITERAL___builtin_va_list))) {
			{ // ( ... )+
			int _cnt135=0;
			for (;;) {
				switch ( LA(1)) {
				case LITERAL_char:
				{
					match(LITERAL_char);
					if ( inputState->guessing==0 ) {
#line 834 "CPP_parser_cpp.g"
						node->add_child(mk_node("char"));
#line 5749 "CPPParser.cpp"
					}
					break;
				}
				case LITERAL_bool:
				{
					match(LITERAL_bool);
					if ( inputState->guessing==0 ) {
#line 836 "CPP_parser_cpp.g"
						node->add_child(mk_node("bool"));
#line 5759 "CPPParser.cpp"
					}
					break;
				}
				case LITERAL_short:
				{
					match(LITERAL_short);
					if ( inputState->guessing==0 ) {
#line 837 "CPP_parser_cpp.g"
						node->add_child(mk_node("short"));
#line 5769 "CPPParser.cpp"
					}
					break;
				}
				case LITERAL_int:
				{
					match(LITERAL_int);
					if ( inputState->guessing==0 ) {
#line 838 "CPP_parser_cpp.g"
						node->add_child(mk_node("int"));
#line 5779 "CPPParser.cpp"
					}
					break;
				}
				case 73:
				case 74:
				{
					{
					switch ( LA(1)) {
					case 73:
					{
						match(73);
						break;
					}
					case 74:
					{
						match(74);
						break;
					}
					default:
					{
						throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
					}
					}
					}
					if ( inputState->guessing==0 ) {
#line 839 "CPP_parser_cpp.g"
						node->add_child(mk_node("__int64"));
#line 5807 "CPPParser.cpp"
					}
					break;
				}
				case 75:
				{
					match(75);
					if ( inputState->guessing==0 ) {
#line 840 "CPP_parser_cpp.g"
						node->add_child(mk_node("__w64"));
#line 5817 "CPPParser.cpp"
					}
					break;
				}
				case LITERAL_long:
				{
					match(LITERAL_long);
					if ( inputState->guessing==0 ) {
#line 841 "CPP_parser_cpp.g"
						node->add_child(mk_node("long"));
#line 5827 "CPPParser.cpp"
					}
					break;
				}
				case LITERAL_signed:
				case LITERAL___signed:
				case LITERAL___signed__:
				{
					{
					switch ( LA(1)) {
					case LITERAL_signed:
					{
						match(LITERAL_signed);
						break;
					}
					case LITERAL___signed:
					{
						match(LITERAL___signed);
						break;
					}
					case LITERAL___signed__:
					{
						match(LITERAL___signed__);
						break;
					}
					default:
					{
						throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
					}
					}
					}
					if ( inputState->guessing==0 ) {
#line 843 "CPP_parser_cpp.g"
						node->add_child(mk_node("signed"));
#line 5861 "CPPParser.cpp"
					}
					break;
				}
				case LITERAL_unsigned:
				{
					match(LITERAL_unsigned);
					if ( inputState->guessing==0 ) {
#line 844 "CPP_parser_cpp.g"
						node->add_child(mk_node("unsigned"));
#line 5871 "CPPParser.cpp"
					}
					break;
				}
				case LITERAL_float:
				{
					match(LITERAL_float);
					if ( inputState->guessing==0 ) {
#line 845 "CPP_parser_cpp.g"
						node->add_child(mk_node("float"));
#line 5881 "CPPParser.cpp"
					}
					break;
				}
				case LITERAL_double:
				{
					match(LITERAL_double);
					if ( inputState->guessing==0 ) {
#line 846 "CPP_parser_cpp.g"
						node->add_child(mk_node("double"));
#line 5891 "CPPParser.cpp"
					}
					break;
				}
				case LITERAL_void:
				{
					match(LITERAL_void);
					if ( inputState->guessing==0 ) {
#line 847 "CPP_parser_cpp.g"
						node->add_child(mk_node("void"));
#line 5901 "CPPParser.cpp"
					}
					break;
				}
				case LITERAL__declspec:
				case LITERAL___declspec:
				{
					{
					switch ( LA(1)) {
					case LITERAL__declspec:
					{
						match(LITERAL__declspec);
						break;
					}
					case LITERAL___declspec:
					{
						match(LITERAL___declspec);
						break;
					}
					default:
					{
						throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
					}
					}
					}
					match(LPAREN);
					match(ID);
					match(RPAREN);
					break;
				}
				case LITERAL___builtin_va_list:
				{
					match(LITERAL___builtin_va_list);
					if ( inputState->guessing==0 ) {
#line 849 "CPP_parser_cpp.g"
						node->add_child(mk_node("__builtin_va_list"));
#line 5937 "CPPParser.cpp"
					}
					break;
				}
				default:
				{
					if ( _cnt135>=1 ) { goto _loop135; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());}
				}
				}
				_cnt135++;
			}
			_loop135:;
			}  // ( ... )+
		}
		else {
			bool synPredMatched137 = false;
			if (((LA(1) == ID || LA(1) == SCOPE) && (_tokenSet_17.member(LA(2))))) {
				int _m137 = mark();
				synPredMatched137 = true;
				inputState->guessing++;
				try {
					{
					qualified_type();
					qualified_id();
					}
				}
				catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
					synPredMatched137 = false;
				}
				rewind(_m137);
				inputState->guessing--;
			}
			if ( synPredMatched137 ) {
				s=qualified_type();
				if ( inputState->guessing==0 ) {
#line 853 "CPP_parser_cpp.g"
					node->add_child(mk_node(s));
#line 5974 "CPPParser.cpp"
				}
				if ( inputState->guessing==0 ) {
#line 854 "CPP_parser_cpp.g"
					printf("simple_type_specifier third option entered\n");
#line 5979 "CPPParser.cpp"
				}
			}
		else {
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_17);
		} else {
			throw;
		}
	}
	return node;
}

ASTNode * CPPParser::class_specifier(
	DeclSpecifier ds
) {
#line 883 "CPP_parser_cpp.g"
	ASTNode *node;
#line 6004 "CPPParser.cpp"
#line 883 "CPP_parser_cpp.g"
	
		ASTNode *bases, *members, *md;
		char *saveClass;
		const char *id;
		TypeSpecifier ts;
		
#line 6012 "CPPParser.cpp"
	
	try {      // for error handling
		{
		switch ( LA(1)) {
		case LITERAL_class:
		{
			match(LITERAL_class);
			if ( inputState->guessing==0 ) {
#line 890 "CPP_parser_cpp.g"
				ts = tsSTRUCT; node = mk_node("class");
#line 6023 "CPPParser.cpp"
			}
			break;
		}
		case LITERAL_struct:
		{
			match(LITERAL_struct);
			if ( inputState->guessing==0 ) {
#line 891 "CPP_parser_cpp.g"
				ts = tsUNION; node = mk_node("struct");
#line 6033 "CPPParser.cpp"
			}
			break;
		}
		case LITERAL_union:
		{
			match(LITERAL_union);
			if ( inputState->guessing==0 ) {
#line 892 "CPP_parser_cpp.g"
				ts = tsCLASS; node = mk_node("union");
#line 6043 "CPPParser.cpp"
			}
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		{
		switch ( LA(1)) {
		case ID:
		case OPERATOR:
		case LITERAL_this:
		case LITERAL_true:
		case LITERAL_false:
		case SCOPE:
		{
			id=qualified_id();
			if ( inputState->guessing==0 ) {
#line 894 "CPP_parser_cpp.g"
				node->set_leaf(id);
#line 6066 "CPPParser.cpp"
			}
			{
			if ((LA(1) == LCURLY || LA(1) == COLON) && (_tokenSet_44.member(LA(2)))) {
				if ( inputState->guessing==0 ) {
#line 896 "CPP_parser_cpp.g"
					saveClass = enclosingClass;
									enclosingClass = symbols->strdup(id);
									
#line 6075 "CPPParser.cpp"
				}
				{
				switch ( LA(1)) {
				case COLON:
				{
					bases=base_clause();
					if ( inputState->guessing==0 ) {
#line 899 "CPP_parser_cpp.g"
						node->add_child(bases);
#line 6085 "CPPParser.cpp"
					}
					break;
				}
				case LCURLY:
				{
					break;
				}
				default:
				{
					throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
				}
				}
				}
				match(LCURLY);
				if ( inputState->guessing==0 ) {
#line 901 "CPP_parser_cpp.g"
					beginClassDefinition(ts, id);
#line 6103 "CPPParser.cpp"
				}
				if ( inputState->guessing==0 ) {
#line 902 "CPP_parser_cpp.g"
					members = mk_node("members");
#line 6108 "CPPParser.cpp"
				}
				{ // ( ... )*
				for (;;) {
					if ((_tokenSet_57.member(LA(1)))) {
						md=member_declaration();
						if ( inputState->guessing==0 ) {
#line 903 "CPP_parser_cpp.g"
							members->add_child(md);
#line 6117 "CPPParser.cpp"
						}
					}
					else {
						goto _loop146;
					}
					
				}
				_loop146:;
				} // ( ... )*
				if ( inputState->guessing==0 ) {
#line 904 "CPP_parser_cpp.g"
					endClassDefinition();
#line 6130 "CPPParser.cpp"
				}
				match(RCURLY);
				if ( inputState->guessing==0 ) {
#line 906 "CPP_parser_cpp.g"
					node->add_child(members); enclosingClass = saveClass;
#line 6136 "CPPParser.cpp"
				}
			}
			else if ((_tokenSet_17.member(LA(1))) && (_tokenSet_38.member(LA(2)))) {
				if ( inputState->guessing==0 ) {
#line 908 "CPP_parser_cpp.g"
					classForwardDeclaration(ts, ds, id);
#line 6143 "CPPParser.cpp"
				}
			}
			else {
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
			}
			
			}
			break;
		}
		case LCURLY:
		{
			match(LCURLY);
			if ( inputState->guessing==0 ) {
#line 912 "CPP_parser_cpp.g"
				saveClass = enclosingClass; enclosingClass = "__anonymous";
#line 6159 "CPPParser.cpp"
			}
			if ( inputState->guessing==0 ) {
#line 913 "CPP_parser_cpp.g"
				beginClassDefinition(ts, "anonymous");
#line 6164 "CPPParser.cpp"
			}
			if ( inputState->guessing==0 ) {
#line 914 "CPP_parser_cpp.g"
				members = mk_node("members");
#line 6169 "CPPParser.cpp"
			}
			{ // ( ... )*
			for (;;) {
				if ((_tokenSet_57.member(LA(1)))) {
					md=member_declaration();
					if ( inputState->guessing==0 ) {
#line 915 "CPP_parser_cpp.g"
						members->add_child(md);
#line 6178 "CPPParser.cpp"
					}
				}
				else {
					goto _loop148;
				}
				
			}
			_loop148:;
			} // ( ... )*
			if ( inputState->guessing==0 ) {
#line 916 "CPP_parser_cpp.g"
				endClassDefinition();
#line 6191 "CPPParser.cpp"
			}
			if ( inputState->guessing==0 ) {
#line 917 "CPP_parser_cpp.g"
				node->add_child(members);
#line 6196 "CPPParser.cpp"
			}
			match(RCURLY);
			if ( inputState->guessing==0 ) {
#line 919 "CPP_parser_cpp.g"
				enclosingClass = saveClass;
#line 6202 "CPPParser.cpp"
			}
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_17);
		} else {
			throw;
		}
	}
	return node;
}

char * CPPParser::qualified_type() {
#line 858 "CPP_parser_cpp.g"
	char *q;
#line 6227 "CPPParser.cpp"
	ANTLR_USE_NAMESPACE(antlr)RefToken  id = ANTLR_USE_NAMESPACE(antlr)nullToken;
#line 858 "CPP_parser_cpp.g"
	
		ASTNode *tal;
		char *s; static char qitem[CPPParser_MaxQualifiedItemSize+1];
		
#line 6234 "CPPParser.cpp"
	
	try {      // for error handling
		s=scope_override();
		id = LT(1);
		match(ID);
		{
		if ((LA(1) == LESSTHAN) && (_tokenSet_36.member(LA(2)))) {
			match(LESSTHAN);
			tal=template_argument_list();
			match(GREATERTHAN);
		}
		else if ((_tokenSet_58.member(LA(1))) && (_tokenSet_38.member(LA(2)))) {
		}
		else {
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		
		}
		if ( inputState->guessing==0 ) {
#line 875 "CPP_parser_cpp.g"
			
					//printf("qualified_type entered\n");
					strcpy(qitem, s);
					strcat(qitem, (id->getText()).data());
					q = qitem;
					
#line 6261 "CPPParser.cpp"
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_58);
		} else {
			throw;
		}
	}
	return q;
}

ASTNode * CPPParser::template_argument_list() {
#line 1644 "CPP_parser_cpp.g"
	ASTNode *node;
#line 6278 "CPPParser.cpp"
#line 1644 "CPP_parser_cpp.g"
	
		ASTNode *ta;
		node = mk_node("template_argument_list");
		
#line 6284 "CPPParser.cpp"
	
	try {      // for error handling
		ta=template_argument();
		if ( inputState->guessing==0 ) {
#line 1649 "CPP_parser_cpp.g"
			node->add_child(ta);
#line 6291 "CPPParser.cpp"
		}
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == COMMA)) {
				match(COMMA);
				ta=template_argument();
				if ( inputState->guessing==0 ) {
#line 1650 "CPP_parser_cpp.g"
					node->add_child(ta);
#line 6301 "CPPParser.cpp"
				}
			}
			else {
				goto _loop322;
			}
			
		}
		_loop322:;
		} // ( ... )*
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_59);
		} else {
			throw;
		}
	}
	return node;
}

ASTNode * CPPParser::base_clause() {
#line 1072 "CPP_parser_cpp.g"
	ASTNode *node;
#line 6326 "CPPParser.cpp"
#line 1072 "CPP_parser_cpp.g"
	
		ASTNode *bs;
		node = mk_node("base_clause");
		
#line 6332 "CPPParser.cpp"
	
	try {      // for error handling
		match(COLON);
		bs=base_specifier();
		if ( inputState->guessing==0 ) {
#line 1077 "CPP_parser_cpp.g"
			node->add_child(bs);
#line 6340 "CPPParser.cpp"
		}
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == COMMA)) {
				match(COMMA);
				bs=base_specifier();
				if ( inputState->guessing==0 ) {
#line 1078 "CPP_parser_cpp.g"
					node->add_child(bs);
#line 6350 "CPPParser.cpp"
				}
			}
			else {
				goto _loop183;
			}
			
		}
		_loop183:;
		} // ( ... )*
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_60);
		} else {
			throw;
		}
	}
	return node;
}

ASTNode * CPPParser::enumerator_list() {
#line 939 "CPP_parser_cpp.g"
	ASTNode *node;
#line 6375 "CPPParser.cpp"
#line 939 "CPP_parser_cpp.g"
	
		ASTNode *e;
		node = mk_node("enumerator_list");
		
#line 6381 "CPPParser.cpp"
	
	try {      // for error handling
		e=enumerator();
		if ( inputState->guessing==0 ) {
#line 944 "CPP_parser_cpp.g"
			node->add_child(e);
#line 6388 "CPPParser.cpp"
		}
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == COMMA) && (LA(2) == ID)) {
				match(COMMA);
				e=enumerator();
				if ( inputState->guessing==0 ) {
#line 948 "CPP_parser_cpp.g"
					node->add_child(e);
#line 6398 "CPPParser.cpp"
				}
			}
			else {
				goto _loop154;
			}
			
		}
		_loop154:;
		} // ( ... )*
		{
		switch ( LA(1)) {
		case COMMA:
		{
			match(COMMA);
			break;
		}
		case RCURLY:
		{
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_61);
		} else {
			throw;
		}
	}
	return node;
}

ASTNode * CPPParser::enumerator() {
#line 952 "CPP_parser_cpp.g"
	ASTNode *node;
#line 6440 "CPPParser.cpp"
	ANTLR_USE_NAMESPACE(antlr)RefToken  id = ANTLR_USE_NAMESPACE(antlr)nullToken;
#line 952 "CPP_parser_cpp.g"
	
		node = mk_node("enumerator");
		ASTNode *cx;
		
#line 6447 "CPPParser.cpp"
	
	try {      // for error handling
		id = LT(1);
		match(ID);
		if ( inputState->guessing==0 ) {
#line 957 "CPP_parser_cpp.g"
			node->set_leaf(id->getText().data());
#line 6455 "CPPParser.cpp"
		}
		{
		switch ( LA(1)) {
		case ASSIGNEQUAL:
		{
			match(ASSIGNEQUAL);
			cx=constant_expression();
			if ( inputState->guessing==0 ) {
#line 958 "CPP_parser_cpp.g"
				node->add_child(cx);
#line 6466 "CPPParser.cpp"
			}
			break;
		}
		case RCURLY:
		case COMMA:
		{
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		if ( inputState->guessing==0 ) {
#line 959 "CPP_parser_cpp.g"
			enumElement(id->getText().data());
#line 6484 "CPPParser.cpp"
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_62);
		} else {
			throw;
		}
	}
	return node;
}

ASTNode * CPPParser::constant_expression() {
#line 1971 "CPP_parser_cpp.g"
	ASTNode *node;
#line 6501 "CPPParser.cpp"
#line 1971 "CPP_parser_cpp.g"
	
		ASTNode *cs;
		
#line 6506 "CPPParser.cpp"
	
	try {      // for error handling
		cs=conditional_expression();
		if ( inputState->guessing==0 ) {
#line 1976 "CPP_parser_cpp.g"
			node = cs;
#line 6513 "CPPParser.cpp"
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_63);
		} else {
			throw;
		}
	}
	return node;
}

const char * CPPParser::optor() {
#line 2453 "CPP_parser_cpp.g"
	const char *out;
#line 6530 "CPPParser.cpp"
#line 2453 "CPP_parser_cpp.g"
	
		const char *x;
		
#line 6535 "CPPParser.cpp"
	
	try {      // for error handling
		switch ( LA(1)) {
		case LITERAL_new:
		{
			match(LITERAL_new);
			if ( inputState->guessing==0 ) {
#line 2458 "CPP_parser_cpp.g"
				out = "new";
#line 6545 "CPPParser.cpp"
			}
			{
			if ((LA(1) == LSQUARE) && (LA(2) == RSQUARE)) {
				match(LSQUARE);
				match(RSQUARE);
			}
			else if ((_tokenSet_64.member(LA(1))) && (_tokenSet_38.member(LA(2)))) {
			}
			else {
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
			}
			
			}
			break;
		}
		case LITERAL_delete:
		{
			match(LITERAL_delete);
			if ( inputState->guessing==0 ) {
#line 2462 "CPP_parser_cpp.g"
				out = "delete";
#line 6567 "CPPParser.cpp"
			}
			{
			if ((LA(1) == LSQUARE) && (LA(2) == RSQUARE)) {
				match(LSQUARE);
				match(RSQUARE);
			}
			else if ((_tokenSet_64.member(LA(1))) && (_tokenSet_38.member(LA(2)))) {
			}
			else {
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
			}
			
			}
			break;
		}
		case LPAREN:
		{
			match(LPAREN);
			match(RPAREN);
			if ( inputState->guessing==0 ) {
#line 2465 "CPP_parser_cpp.g"
				out = "()";
#line 6590 "CPPParser.cpp"
			}
			break;
		}
		case LSQUARE:
		{
			match(LSQUARE);
			match(RSQUARE);
			if ( inputState->guessing==0 ) {
#line 2466 "CPP_parser_cpp.g"
				out = "arrayindex";
#line 6601 "CPPParser.cpp"
			}
			break;
		}
		case LESSTHAN:
		case GREATERTHAN:
		case ASSIGNEQUAL:
		case COMMA:
		case STAR:
		case AMPERSAND:
		case TILDE:
		case TIMESEQUAL:
		case DIVIDEEQUAL:
		case MINUSEQUAL:
		case PLUSEQUAL:
		case MODEQUAL:
		case SHIFTLEFTEQUAL:
		case SHIFTRIGHTEQUAL:
		case BITWISEANDEQUAL:
		case BITWISEXOREQUAL:
		case BITWISEOREQUAL:
		case OR:
		case AND:
		case BITWISEOR:
		case BITWISEXOR:
		case NOTEQUAL:
		case EQUAL:
		case LESSTHANOREQUALTO:
		case GREATERTHANOREQUALTO:
		case SHIFTLEFT:
		case SHIFTRIGHT:
		case PLUS:
		case MINUS:
		case DIVIDE:
		case MOD:
		case POINTERTOMBR:
		case PLUSPLUS:
		case MINUSMINUS:
		case POINTERTO:
		case NOT:
		{
			x=optor_simple_tokclass();
			if ( inputState->guessing==0 ) {
#line 2467 "CPP_parser_cpp.g"
				out = x;
#line 6646 "CPPParser.cpp"
			}
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_64);
		} else {
			throw;
		}
	}
	return out;
}

void CPPParser::typeID() {
	
	try {      // for error handling
		if (!(isTypeName((LT(1)->getText()).data())))
			throw ANTLR_USE_NAMESPACE(antlr)SemanticException("isTypeName((LT(1)->getText()).data())");
		match(ID);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_1);
		} else {
			throw;
		}
	}
}

ASTNode * CPPParser::init_declarator() {
#line 1008 "CPP_parser_cpp.g"
	ASTNode *node;
#line 6687 "CPPParser.cpp"
#line 1008 "CPP_parser_cpp.g"
	
		ASTNode *d, *i, *el, *an;
		node = mk_node("init_declarator");
		
#line 6693 "CPPParser.cpp"
	
	try {      // for error handling
		d=declarator();
		if ( inputState->guessing==0 ) {
#line 1013 "CPP_parser_cpp.g"
			node->add_child(d);
#line 6700 "CPPParser.cpp"
		}
		{
		switch ( LA(1)) {
		case LITERAL_asm:
		case LITERAL___asm:
		case LITERAL___asm__:
		{
			an=func_asm_name();
			if ( inputState->guessing==0 ) {
#line 1014 "CPP_parser_cpp.g"
				node->add_child(an);
#line 6712 "CPPParser.cpp"
			}
			break;
		}
		case SEMICOLON:
		case ASSIGNEQUAL:
		case COMMA:
		case LITERAL___attribute__:
		case LPAREN:
		{
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		{
		switch ( LA(1)) {
		case ASSIGNEQUAL:
		{
			match(ASSIGNEQUAL);
			i=initializer();
			if ( inputState->guessing==0 ) {
#line 1018 "CPP_parser_cpp.g"
				node->add_child(i);
#line 6739 "CPPParser.cpp"
			}
			break;
		}
		case LPAREN:
		{
			match(LPAREN);
			el=expression_list();
			match(RPAREN);
			if ( inputState->guessing==0 ) {
#line 1021 "CPP_parser_cpp.g"
				node->add_child(el);
#line 6751 "CPPParser.cpp"
			}
			break;
		}
		case SEMICOLON:
		case COMMA:
		case LITERAL___attribute__:
		{
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_65);
		} else {
			throw;
		}
	}
	return node;
}

ASTNode * CPPParser::declarator() {
#line 1153 "CPP_parser_cpp.g"
	ASTNode *node;
#line 6782 "CPPParser.cpp"
#line 1153 "CPP_parser_cpp.g"
	
		ASTNode *d;
		char *p;
		
#line 6788 "CPPParser.cpp"
	
	try {      // for error handling
		bool synPredMatched211 = false;
		if (((_tokenSet_24.member(LA(1))) && (_tokenSet_66.member(LA(2))))) {
			int _m211 = mark();
			synPredMatched211 = true;
			inputState->guessing++;
			try {
				{
				ptr_operator();
				}
			}
			catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
				synPredMatched211 = false;
			}
			rewind(_m211);
			inputState->guessing--;
		}
		if ( synPredMatched211 ) {
			if ( inputState->guessing==0 ) {
#line 1159 "CPP_parser_cpp.g"
				node = mk_node("declarator");
#line 6811 "CPPParser.cpp"
			}
			p=ptr_operator();
			d=declarator();
			if ( inputState->guessing==0 ) {
#line 1163 "CPP_parser_cpp.g"
				node->set_leaf(p); node->add_child(d);
#line 6818 "CPPParser.cpp"
			}
		}
		else if ((_tokenSet_67.member(LA(1))) && (_tokenSet_68.member(LA(2)))) {
			if ( inputState->guessing==0 ) {
#line 1165 "CPP_parser_cpp.g"
				node = mk_node("declarator");
#line 6825 "CPPParser.cpp"
			}
			d=direct_declarator();
			if ( inputState->guessing==0 ) {
#line 1166 "CPP_parser_cpp.g"
				node->add_child(d);
#line 6831 "CPPParser.cpp"
			}
		}
		else {
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_69);
		} else {
			throw;
		}
	}
	return node;
}

ASTNode * CPPParser::func_asm_name() {
#line 1325 "CPP_parser_cpp.g"
	ASTNode *node;
#line 6853 "CPPParser.cpp"
	ANTLR_USE_NAMESPACE(antlr)RefToken  sl = ANTLR_USE_NAMESPACE(antlr)nullToken;
	
	try {      // for error handling
		if ( inputState->guessing==0 ) {
#line 1327 "CPP_parser_cpp.g"
			node = mk_node("func_asm_name");
#line 6860 "CPPParser.cpp"
		}
		{
		switch ( LA(1)) {
		case LITERAL_asm:
		{
			match(LITERAL_asm);
			break;
		}
		case LITERAL___asm:
		{
			match(LITERAL___asm);
			break;
		}
		case LITERAL___asm__:
		{
			match(LITERAL___asm__);
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		match(LPAREN);
		{ // ( ... )+
		int _cnt253=0;
		for (;;) {
			if ((LA(1) == StringLiteral)) {
				sl = LT(1);
				match(StringLiteral);
				if ( inputState->guessing==0 ) {
#line 1330 "CPP_parser_cpp.g"
					node->extend_leaf(sl->getText().data());
#line 6895 "CPPParser.cpp"
				}
			}
			else {
				if ( _cnt253>=1 ) { goto _loop253; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());}
			}
			
			_cnt253++;
		}
		_loop253:;
		}  // ( ... )+
		match(RPAREN);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_70);
		} else {
			throw;
		}
	}
	return node;
}

ASTNode * CPPParser::initializer() {
#line 1025 "CPP_parser_cpp.g"
	ASTNode *node;
#line 6922 "CPPParser.cpp"
#line 1025 "CPP_parser_cpp.g"
	
	ASTNode *rx, *i1, *i2, *d1, *i3, *d2, *i4;
	node = mk_node("initializer");
	
#line 6928 "CPPParser.cpp"
	
	try {      // for error handling
		switch ( LA(1)) {
		case LCURLY:
		{
			match(LCURLY);
			i1=initializer();
			if ( inputState->guessing==0 ) {
#line 1031 "CPP_parser_cpp.g"
				node->add_child(i1);
#line 6939 "CPPParser.cpp"
			}
			{ // ( ... )*
			for (;;) {
				if ((LA(1) == COMMA)) {
					match(COMMA);
					i2=initializer();
					if ( inputState->guessing==0 ) {
#line 1032 "CPP_parser_cpp.g"
						node->add_child(i2);
#line 6949 "CPPParser.cpp"
					}
				}
				else {
					goto _loop173;
				}
				
			}
			_loop173:;
			} // ( ... )*
			match(RCURLY);
			break;
		}
		case DOT:
		{
			d2=c99_designator();
			if ( inputState->guessing==0 ) {
#line 1035 "CPP_parser_cpp.g"
				node->add_child(d2);
#line 6968 "CPPParser.cpp"
			}
			i4=initializer();
			if ( inputState->guessing==0 ) {
#line 1036 "CPP_parser_cpp.g"
				node->add_child(i4);
#line 6974 "CPPParser.cpp"
			}
			break;
		}
		default:
			if ((_tokenSet_71.member(LA(1))) && (_tokenSet_72.member(LA(2)))) {
				rx=remainder_expression();
				if ( inputState->guessing==0 ) {
#line 1030 "CPP_parser_cpp.g"
					node->add_child(rx);
#line 6984 "CPPParser.cpp"
				}
			}
			else if ((LA(1) == ID) && (LA(2) == COLON)) {
				d1=gcc_designator();
				if ( inputState->guessing==0 ) {
#line 1033 "CPP_parser_cpp.g"
					node->add_child(d1);
#line 6992 "CPPParser.cpp"
				}
				i3=initializer();
				if ( inputState->guessing==0 ) {
#line 1034 "CPP_parser_cpp.g"
					node->add_child(i3);
#line 6998 "CPPParser.cpp"
				}
			}
		else {
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_73);
		} else {
			throw;
		}
	}
	return node;
}

ASTNode * CPPParser::expression_list() {
#line 2424 "CPP_parser_cpp.g"
	ASTNode *node;
#line 7020 "CPPParser.cpp"
#line 2424 "CPP_parser_cpp.g"
	
		ASTNode *x;
		node = mk_node("expression");
		
#line 7026 "CPPParser.cpp"
	
	try {      // for error handling
		x=assignment_expression();
		if ( inputState->guessing==0 ) {
#line 2430 "CPP_parser_cpp.g"
			node->add_child(x);
#line 7033 "CPPParser.cpp"
		}
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == COMMA)) {
				match(COMMA);
				x=assignment_expression();
				if ( inputState->guessing==0 ) {
#line 2431 "CPP_parser_cpp.g"
					node = exprnode_acc(node, x, "comma");
#line 7043 "CPPParser.cpp"
				}
			}
			else {
				goto _loop503;
			}
			
		}
		_loop503:;
		} // ( ... )*
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_55);
		} else {
			throw;
		}
	}
	return node;
}

ASTNode * CPPParser::remainder_expression() {
#line 1933 "CPP_parser_cpp.g"
	ASTNode *node;
#line 7068 "CPPParser.cpp"
#line 1933 "CPP_parser_cpp.g"
	
		ASTNode *x;
		
#line 7073 "CPPParser.cpp"
	
	try {      // for error handling
		{
		bool synPredMatched388 = false;
		if (((_tokenSet_71.member(LA(1))) && (_tokenSet_74.member(LA(2))))) {
			int _m388 = mark();
			synPredMatched388 = true;
			inputState->guessing++;
			try {
				{
				conditional_expression();
				{
				switch ( LA(1)) {
				case COMMA:
				{
					match(COMMA);
					break;
				}
				case SEMICOLON:
				{
					match(SEMICOLON);
					break;
				}
				case RPAREN:
				{
					match(RPAREN);
					break;
				}
				default:
				{
					throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
				}
				}
				}
				}
			}
			catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
				synPredMatched388 = false;
			}
			rewind(_m388);
			inputState->guessing--;
		}
		if ( synPredMatched388 ) {
			if ( inputState->guessing==0 ) {
#line 1940 "CPP_parser_cpp.g"
				assign_stmt_RHS_found += 1;
#line 7120 "CPPParser.cpp"
			}
			x=assignment_expression();
			if ( inputState->guessing==0 ) {
#line 1941 "CPP_parser_cpp.g"
				node = x;
#line 7126 "CPPParser.cpp"
			}
			if ( inputState->guessing==0 ) {
#line 1942 "CPP_parser_cpp.g"
				
							if (assign_stmt_RHS_found > 0)
								assign_stmt_RHS_found -= 1;
							else
								{
								printf("%d warning Error in assign_stmt_RHS_found = %d\n",
									LT(1)->getLine(),assign_stmt_RHS_found);
								printf("Press return to continue\n");
								getchar();
								}
							
#line 7141 "CPPParser.cpp"
			}
		}
		else if ((_tokenSet_71.member(LA(1))) && (_tokenSet_74.member(LA(2)))) {
			x=assignment_expression();
			if ( inputState->guessing==0 ) {
#line 1954 "CPP_parser_cpp.g"
				node = x;
#line 7149 "CPPParser.cpp"
			}
		}
		else {
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_75);
		} else {
			throw;
		}
	}
	return node;
}

ASTNode * CPPParser::gcc_designator() {
#line 1043 "CPP_parser_cpp.g"
	ASTNode *node;
#line 7172 "CPPParser.cpp"
	ANTLR_USE_NAMESPACE(antlr)RefToken  id = ANTLR_USE_NAMESPACE(antlr)nullToken;
#line 1043 "CPP_parser_cpp.g"
	node = mk_node("designator");
#line 7176 "CPPParser.cpp"
	
	try {      // for error handling
		id = LT(1);
		match(ID);
		match(COLON);
		if ( inputState->guessing==0 ) {
#line 1045 "CPP_parser_cpp.g"
			node->set_leaf(id->getText().data());
#line 7185 "CPPParser.cpp"
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_76);
		} else {
			throw;
		}
	}
	return node;
}

ASTNode * CPPParser::c99_designator() {
#line 1051 "CPP_parser_cpp.g"
	ASTNode *node;
#line 7202 "CPPParser.cpp"
	ANTLR_USE_NAMESPACE(antlr)RefToken  id = ANTLR_USE_NAMESPACE(antlr)nullToken;
#line 1051 "CPP_parser_cpp.g"
	node = mk_node("designator");
#line 7206 "CPPParser.cpp"
	
	try {      // for error handling
		match(DOT);
		id = LT(1);
		match(ID);
		match(EQUALS);
		if ( inputState->guessing==0 ) {
#line 1053 "CPP_parser_cpp.g"
			node->set_leaf(id->getText().data());
#line 7216 "CPPParser.cpp"
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_76);
		} else {
			throw;
		}
	}
	return node;
}

ASTNode * CPPParser::base_specifier() {
#line 1081 "CPP_parser_cpp.g"
	ASTNode *node;
#line 7233 "CPPParser.cpp"
#line 1081 "CPP_parser_cpp.g"
	
		ASTNode *as;
		char *qt;
		
#line 7239 "CPPParser.cpp"
	
	try {      // for error handling
		if ( inputState->guessing==0 ) {
#line 1087 "CPP_parser_cpp.g"
			node = mk_node("base_specifier");
#line 7245 "CPPParser.cpp"
		}
		{
		switch ( LA(1)) {
		case LITERAL_virtual:
		{
			match(LITERAL_virtual);
			if ( inputState->guessing==0 ) {
#line 1088 "CPP_parser_cpp.g"
				node->add_child(mk_node("virtual"));
#line 7255 "CPPParser.cpp"
			}
			{
			switch ( LA(1)) {
			case LITERAL_public:
			case LITERAL_protected:
			case LITERAL_private:
			{
				as=access_specifier();
				if ( inputState->guessing==0 ) {
#line 1089 "CPP_parser_cpp.g"
					node->add_child(as);
#line 7267 "CPPParser.cpp"
				}
				break;
			}
			case ID:
			case SCOPE:
			{
				break;
			}
			default:
			{
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
			}
			}
			}
			qt=qualified_type();
			if ( inputState->guessing==0 ) {
#line 1090 "CPP_parser_cpp.g"
				node->add_child(mk_node(qt));
#line 7286 "CPPParser.cpp"
			}
			break;
		}
		case LITERAL_public:
		case LITERAL_protected:
		case LITERAL_private:
		{
			as=access_specifier();
			if ( inputState->guessing==0 ) {
#line 1091 "CPP_parser_cpp.g"
				node->add_child(as);
#line 7298 "CPPParser.cpp"
			}
			{
			switch ( LA(1)) {
			case LITERAL_virtual:
			{
				match(LITERAL_virtual);
				if ( inputState->guessing==0 ) {
#line 1092 "CPP_parser_cpp.g"
					node->add_child(mk_node("virtual"));
#line 7308 "CPPParser.cpp"
				}
				break;
			}
			case ID:
			case SCOPE:
			{
				break;
			}
			default:
			{
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
			}
			}
			}
			qt=qualified_type();
			if ( inputState->guessing==0 ) {
#line 1093 "CPP_parser_cpp.g"
				node->add_child(mk_node(qt));
#line 7327 "CPPParser.cpp"
			}
			break;
		}
		case ID:
		case SCOPE:
		{
			qt=qualified_type();
			if ( inputState->guessing==0 ) {
#line 1094 "CPP_parser_cpp.g"
				node->add_child(mk_node(qt));
#line 7338 "CPPParser.cpp"
			}
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_77);
		} else {
			throw;
		}
	}
	return node;
}

ASTNode * CPPParser::member_declarator() {
#line 1113 "CPP_parser_cpp.g"
	ASTNode *node;
#line 7363 "CPPParser.cpp"
	ANTLR_USE_NAMESPACE(antlr)RefToken  id = ANTLR_USE_NAMESPACE(antlr)nullToken;
#line 1113 "CPP_parser_cpp.g"
	ASTNode *c, *d;
#line 7367 "CPPParser.cpp"
	
	try {      // for error handling
		bool synPredMatched195 = false;
		if (((LA(1) == ID || LA(1) == COLON) && (_tokenSet_78.member(LA(2))))) {
			int _m195 = mark();
			synPredMatched195 = true;
			inputState->guessing++;
			try {
				{
				{
				switch ( LA(1)) {
				case ID:
				{
					match(ID);
					break;
				}
				case COLON:
				{
					break;
				}
				default:
				{
					throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
				}
				}
				}
				match(COLON);
				constant_expression();
				}
			}
			catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
				synPredMatched195 = false;
			}
			rewind(_m195);
			inputState->guessing--;
		}
		if ( synPredMatched195 ) {
			if ( inputState->guessing==0 ) {
#line 1116 "CPP_parser_cpp.g"
				node = mk_node("member_declarator_bitfield");
#line 7408 "CPPParser.cpp"
			}
			{
			switch ( LA(1)) {
			case ID:
			{
				id = LT(1);
				match(ID);
				if ( inputState->guessing==0 ) {
#line 1117 "CPP_parser_cpp.g"
					node->set_leaf(id->getText().data());
#line 7419 "CPPParser.cpp"
				}
				break;
			}
			case COLON:
			{
				break;
			}
			default:
			{
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
			}
			}
			}
			match(COLON);
			c=constant_expression();
			if ( inputState->guessing==0 ) {
#line 1118 "CPP_parser_cpp.g"
				node->add_child(c);
#line 7438 "CPPParser.cpp"
			}
			{
			switch ( LA(1)) {
			case ASSIGNEQUAL:
			{
				match(ASSIGNEQUAL);
				match(OCTALINT);
				if ( inputState->guessing==0 ) {
#line 1119 "CPP_parser_cpp.g"
					node->add_child(mk_node("purevirtual"));
#line 7449 "CPPParser.cpp"
				}
				break;
			}
			case SEMICOLON:
			case COMMA:
			{
				break;
			}
			default:
			{
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
			}
			}
			}
		}
		else if ((_tokenSet_79.member(LA(1))) && (_tokenSet_80.member(LA(2)))) {
			d=declarator();
			if ( inputState->guessing==0 ) {
#line 1121 "CPP_parser_cpp.g"
				node = mk_node("member_declarator"); node->add_child(d);
#line 7470 "CPPParser.cpp"
			}
			{
			switch ( LA(1)) {
			case ASSIGNEQUAL:
			{
				match(ASSIGNEQUAL);
				match(OCTALINT);
				if ( inputState->guessing==0 ) {
#line 1122 "CPP_parser_cpp.g"
					node->add_child(mk_node("purevirtual"));
#line 7481 "CPPParser.cpp"
				}
				break;
			}
			case SEMICOLON:
			case COMMA:
			{
				break;
			}
			default:
			{
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
			}
			}
			}
		}
		else {
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_81);
		} else {
			throw;
		}
	}
	return node;
}

ASTNode * CPPParser::template_parameter_list() {
#line 1584 "CPP_parser_cpp.g"
	ASTNode *node;
#line 7516 "CPPParser.cpp"
#line 1584 "CPP_parser_cpp.g"
	
		ASTNode *tp;
		node = mk_node("template_parameter_list");
		
#line 7522 "CPPParser.cpp"
	
	try {      // for error handling
		if ( inputState->guessing==0 ) {
#line 1590 "CPP_parser_cpp.g"
			beginTemplateParameterList();
#line 7528 "CPPParser.cpp"
		}
		tp=template_parameter();
		if ( inputState->guessing==0 ) {
#line 1591 "CPP_parser_cpp.g"
			node->add_child(tp);
#line 7534 "CPPParser.cpp"
		}
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == COMMA)) {
				match(COMMA);
				tp=template_parameter();
				if ( inputState->guessing==0 ) {
#line 1592 "CPP_parser_cpp.g"
					node->add_child(tp);
#line 7544 "CPPParser.cpp"
				}
			}
			else {
				goto _loop311;
			}
			
		}
		_loop311:;
		} // ( ... )*
		if ( inputState->guessing==0 ) {
#line 1593 "CPP_parser_cpp.g"
			endTemplateParameterList();
#line 7557 "CPPParser.cpp"
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_59);
		} else {
			throw;
		}
	}
	return node;
}

ASTNode * CPPParser::parameter_list() {
#line 1461 "CPP_parser_cpp.g"
	ASTNode *node;
#line 7574 "CPPParser.cpp"
#line 1461 "CPP_parser_cpp.g"
	
		ASTNode *pdl;
		node = mk_node("parameter_list");
		
#line 7580 "CPPParser.cpp"
	
	try {      // for error handling
		pdl=parameter_declaration_list();
		if ( inputState->guessing==0 ) {
#line 1466 "CPP_parser_cpp.g"
			node->add_child(pdl);
#line 7587 "CPPParser.cpp"
		}
		{
		switch ( LA(1)) {
		case ELLIPSIS:
		{
			match(ELLIPSIS);
			if ( inputState->guessing==0 ) {
#line 1466 "CPP_parser_cpp.g"
				node->set_leaf("...");
#line 7597 "CPPParser.cpp"
			}
			break;
		}
		case RPAREN:
		{
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_55);
		} else {
			throw;
		}
	}
	return node;
}

ASTNode * CPPParser::exception_specification() {
#line 1562 "CPP_parser_cpp.g"
	ASTNode *node;
#line 7626 "CPPParser.cpp"
#line 1562 "CPP_parser_cpp.g"
	
		char *so; node = mk_node("unfinished_exception_specification");
		
#line 7631 "CPPParser.cpp"
	
	try {      // for error handling
		match(LITERAL_throw);
		match(LPAREN);
		{
		switch ( LA(1)) {
		case ID:
		case RPAREN:
		case SCOPE:
		{
			{
			switch ( LA(1)) {
			case ID:
			case SCOPE:
			{
				so=scope_override();
				match(ID);
				{
				switch ( LA(1)) {
				case COMMA:
				{
					match(COMMA);
					so=scope_override();
					match(ID);
					break;
				}
				case RPAREN:
				{
					break;
				}
				default:
				{
					throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
				}
				}
				}
				break;
			}
			case RPAREN:
			{
				break;
			}
			default:
			{
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
			}
			}
			}
			break;
		}
		case ELLIPSIS:
		{
			match(ELLIPSIS);
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		match(RPAREN);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_82);
		} else {
			throw;
		}
	}
	return node;
}

char * CPPParser::cv_qualifier_seq() {
#line 1142 "CPP_parser_cpp.g"
	char *s;
#line 7709 "CPPParser.cpp"
#line 1142 "CPP_parser_cpp.g"
	
		char *tq;
		static char seq[CPPParser_MaxQualifiedItemSize+1];
		seq[0] = '\0';
		
#line 7716 "CPPParser.cpp"
	
	try {      // for error handling
		{ // ( ... )*
		for (;;) {
			if ((_tokenSet_51.member(LA(1)))) {
				tq=type_qualifier();
				if ( inputState->guessing==0 ) {
#line 1149 "CPP_parser_cpp.g"
					strcat(seq, tq);
#line 7726 "CPPParser.cpp"
				}
			}
			else {
				goto _loop208;
			}
			
		}
		_loop208:;
		} // ( ... )*
		if ( inputState->guessing==0 ) {
#line 1150 "CPP_parser_cpp.g"
			s = seq;
#line 7739 "CPPParser.cpp"
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_83);
		} else {
			throw;
		}
	}
	return s;
}

char * CPPParser::ptr_operator() {
#line 2345 "CPP_parser_cpp.g"
	char *oper;
#line 7756 "CPPParser.cpp"
#line 2345 "CPP_parser_cpp.g"
	
		char *ptm;
		oper = "UNKNOWN";
		
#line 7762 "CPPParser.cpp"
	
	try {      // for error handling
		{
		switch ( LA(1)) {
		case AMPERSAND:
		{
			match(AMPERSAND);
			if ( inputState->guessing==0 ) {
#line 2350 "CPP_parser_cpp.g"
				oper = "&";
#line 7773 "CPPParser.cpp"
			}
			if ( inputState->guessing==0 ) {
#line 2350 "CPP_parser_cpp.g"
				is_address = true;
#line 7778 "CPPParser.cpp"
			}
			break;
		}
		case LITERAL__cdecl:
		case LITERAL___cdecl:
		{
			{
			switch ( LA(1)) {
			case LITERAL__cdecl:
			{
				match(LITERAL__cdecl);
				break;
			}
			case LITERAL___cdecl:
			{
				match(LITERAL___cdecl);
				break;
			}
			default:
			{
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
			}
			}
			}
			if ( inputState->guessing==0 ) {
#line 2351 "CPP_parser_cpp.g"
				oper = "_cdecl";
#line 7806 "CPPParser.cpp"
			}
			break;
		}
		case LITERAL__near:
		case LITERAL___near:
		{
			{
			switch ( LA(1)) {
			case LITERAL__near:
			{
				match(LITERAL__near);
				break;
			}
			case LITERAL___near:
			{
				match(LITERAL___near);
				break;
			}
			default:
			{
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
			}
			}
			}
			if ( inputState->guessing==0 ) {
#line 2352 "CPP_parser_cpp.g"
				oper = "_near";
#line 7834 "CPPParser.cpp"
			}
			break;
		}
		case LITERAL__far:
		case LITERAL___far:
		{
			{
			switch ( LA(1)) {
			case LITERAL__far:
			{
				match(LITERAL__far);
				break;
			}
			case LITERAL___far:
			{
				match(LITERAL___far);
				break;
			}
			default:
			{
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
			}
			}
			}
			if ( inputState->guessing==0 ) {
#line 2353 "CPP_parser_cpp.g"
				oper = "_far";
#line 7862 "CPPParser.cpp"
			}
			break;
		}
		case LITERAL___interrupt:
		{
			match(LITERAL___interrupt);
			if ( inputState->guessing==0 ) {
#line 2354 "CPP_parser_cpp.g"
				oper = "__interrupt";
#line 7872 "CPPParser.cpp"
			}
			break;
		}
		case LITERAL_pascal:
		case LITERAL__pascal:
		case LITERAL___pascal:
		{
			{
			switch ( LA(1)) {
			case LITERAL_pascal:
			{
				match(LITERAL_pascal);
				break;
			}
			case LITERAL__pascal:
			{
				match(LITERAL__pascal);
				break;
			}
			case LITERAL___pascal:
			{
				match(LITERAL___pascal);
				break;
			}
			default:
			{
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
			}
			}
			}
			if ( inputState->guessing==0 ) {
#line 2355 "CPP_parser_cpp.g"
				oper = "pascal";
#line 7906 "CPPParser.cpp"
			}
			break;
		}
		case LITERAL__stdcall:
		case LITERAL___stdcall:
		{
			{
			switch ( LA(1)) {
			case LITERAL__stdcall:
			{
				match(LITERAL__stdcall);
				break;
			}
			case LITERAL___stdcall:
			{
				match(LITERAL___stdcall);
				break;
			}
			default:
			{
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
			}
			}
			}
			if ( inputState->guessing==0 ) {
#line 2356 "CPP_parser_cpp.g"
				oper = "_stdcall";
#line 7934 "CPPParser.cpp"
			}
			break;
		}
		case ID:
		case STAR:
		case SCOPE:
		{
			ptm=ptr_to_member();
			if ( inputState->guessing==0 ) {
#line 2357 "CPP_parser_cpp.g"
				oper = ptm;
#line 7946 "CPPParser.cpp"
			}
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_17);
		} else {
			throw;
		}
	}
	return oper;
}

ASTNode * CPPParser::declarator_suffixes() {
#line 1232 "CPP_parser_cpp.g"
	ASTNode *node;
#line 7971 "CPPParser.cpp"
#line 1232 "CPP_parser_cpp.g"
	
		ASTNode *cx, *pl, *es;
		char *tq;
		node = mk_node("declarator_suffixes");
		
#line 7978 "CPPParser.cpp"
	
	try {      // for error handling
		{
		if ((LA(1) == LSQUARE)) {
			{ // ( ... )+
			int _cnt232=0;
			for (;;) {
				if ((LA(1) == LSQUARE) && (_tokenSet_52.member(LA(2)))) {
					match(LSQUARE);
					{
					switch ( LA(1)) {
					case ID:
					case StringLiteral:
					case LPAREN:
					case LITERAL_const_cast:
					case LITERAL_char:
					case LITERAL_bool:
					case LITERAL_short:
					case LITERAL_int:
					case 73:
					case 74:
					case 75:
					case LITERAL_long:
					case LITERAL_signed:
					case LITERAL___signed:
					case LITERAL___signed__:
					case LITERAL_unsigned:
					case LITERAL_float:
					case LITERAL_double:
					case LITERAL_void:
					case LITERAL__declspec:
					case LITERAL___declspec:
					case LITERAL___builtin_va_list:
					case OPERATOR:
					case LITERAL_this:
					case LITERAL_true:
					case LITERAL_false:
					case OCTALINT:
					case STAR:
					case AMPERSAND:
					case TILDE:
					case SCOPE:
					case PLUS:
					case MINUS:
					case PLUSPLUS:
					case MINUSMINUS:
					case LITERAL_sizeof:
					case LITERAL_dynamic_cast:
					case LITERAL_static_cast:
					case LITERAL_reinterpret_cast:
					case NOT:
					case LITERAL_new:
					case LITERAL_delete:
					case DECIMALINT:
					case HEXADECIMALINT:
					case CharLiteral:
					case FLOATONE:
					case FLOATTWO:
					{
						cx=constant_expression();
						if ( inputState->guessing==0 ) {
#line 1241 "CPP_parser_cpp.g"
							node->add_child(cx);
#line 8042 "CPPParser.cpp"
						}
						break;
					}
					case RSQUARE:
					{
						break;
					}
					default:
					{
						throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
					}
					}
					}
					match(RSQUARE);
				}
				else {
					if ( _cnt232>=1 ) { goto _loop232; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());}
				}
				
				_cnt232++;
			}
			_loop232:;
			}  // ( ... )+
			if ( inputState->guessing==0 ) {
#line 1242 "CPP_parser_cpp.g"
				declaratorArray();
#line 8069 "CPPParser.cpp"
			}
		}
		else if (((LA(1) == LPAREN))&&((!((LA(1)==LPAREN)&&(LA(2)==ID))||(qualifiedItemIsOneOf(qiType|qiCtor,1))))) {
			match(LPAREN);
			if ( inputState->guessing==0 ) {
#line 1244 "CPP_parser_cpp.g"
				declaratorParameterList(0);
#line 8077 "CPPParser.cpp"
			}
			{
			switch ( LA(1)) {
			case LITERAL_typedef:
			case LITERAL_enum:
			case ID:
			case LITERAL_inline:
			case LITERAL_extern:
			case LITERAL___extension__:
			case LITERAL__inline:
			case LITERAL___inline:
			case LITERAL___inline__:
			case LITERAL_virtual:
			case LITERAL_explicit:
			case LITERAL_friend:
			case LITERAL__stdcall:
			case LITERAL___stdcall:
			case LITERAL_typename:
			case LPAREN:
			case LITERAL_const:
			case LITERAL___const:
			case LITERAL_auto:
			case LITERAL_register:
			case LITERAL_static:
			case LITERAL_mutable:
			case LITERAL_const_cast:
			case LITERAL_volatile:
			case LITERAL___volatile:
			case LITERAL___restrict:
			case LITERAL___restrict__:
			case LITERAL_char:
			case LITERAL_bool:
			case LITERAL_short:
			case LITERAL_int:
			case 73:
			case 74:
			case 75:
			case LITERAL_long:
			case LITERAL_signed:
			case LITERAL___signed:
			case LITERAL___signed__:
			case LITERAL_unsigned:
			case LITERAL_float:
			case LITERAL_double:
			case LITERAL_void:
			case LITERAL__declspec:
			case LITERAL___declspec:
			case LITERAL___builtin_va_list:
			case LITERAL_class:
			case LITERAL_struct:
			case LITERAL_union:
			case OPERATOR:
			case LITERAL_this:
			case LITERAL_true:
			case LITERAL_false:
			case STAR:
			case AMPERSAND:
			case TILDE:
			case ELLIPSIS:
			case SCOPE:
			case LITERAL__cdecl:
			case LITERAL___cdecl:
			case LITERAL__near:
			case LITERAL___near:
			case LITERAL__far:
			case LITERAL___far:
			case LITERAL___interrupt:
			case LITERAL_pascal:
			case LITERAL__pascal:
			case LITERAL___pascal:
			{
				pl=parameter_list();
				if ( inputState->guessing==0 ) {
#line 1245 "CPP_parser_cpp.g"
					node->add_child(pl);
#line 8153 "CPPParser.cpp"
				}
				break;
			}
			case RPAREN:
			{
				break;
			}
			default:
			{
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
			}
			}
			}
			match(RPAREN);
			if ( inputState->guessing==0 ) {
#line 1246 "CPP_parser_cpp.g"
				declaratorEndParameterList(0);
#line 8171 "CPPParser.cpp"
			}
			{ // ( ... )*
			for (;;) {
				if ((_tokenSet_51.member(LA(1)))) {
					tq=type_qualifier();
					if ( inputState->guessing==0 ) {
#line 1247 "CPP_parser_cpp.g"
						node->add_child(mk_node(tq));
#line 8180 "CPPParser.cpp"
					}
				}
				else {
					goto _loop235;
				}
				
			}
			_loop235:;
			} // ( ... )*
			{
			switch ( LA(1)) {
			case LITERAL_throw:
			{
				es=exception_specification();
				if ( inputState->guessing==0 ) {
#line 1248 "CPP_parser_cpp.g"
					node->add_child(es);
#line 8198 "CPPParser.cpp"
				}
				break;
			}
			case LESSTHAN:
			case GREATERTHAN:
			case ID:
			case SEMICOLON:
			case RCURLY:
			case ASSIGNEQUAL:
			case COLON:
			case COMMA:
			case LITERAL__stdcall:
			case LITERAL___stdcall:
			case LITERAL___attribute__:
			case LPAREN:
			case RPAREN:
			case OPERATOR:
			case LITERAL_this:
			case LITERAL_true:
			case LITERAL_false:
			case STAR:
			case AMPERSAND:
			case LSQUARE:
			case RSQUARE:
			case TILDE:
			case LITERAL_asm:
			case LITERAL___asm:
			case LITERAL___asm__:
			case ELLIPSIS:
			case SCOPE:
			case TIMESEQUAL:
			case DIVIDEEQUAL:
			case MINUSEQUAL:
			case PLUSEQUAL:
			case MODEQUAL:
			case SHIFTLEFTEQUAL:
			case SHIFTRIGHTEQUAL:
			case BITWISEANDEQUAL:
			case BITWISEXOREQUAL:
			case BITWISEOREQUAL:
			case QUESTIONMARK:
			case OR:
			case AND:
			case BITWISEOR:
			case BITWISEXOR:
			case NOTEQUAL:
			case EQUAL:
			case LESSTHANOREQUALTO:
			case GREATERTHANOREQUALTO:
			case SHIFTLEFT:
			case SHIFTRIGHT:
			case PLUS:
			case MINUS:
			case DIVIDE:
			case MOD:
			case DOTMBR:
			case POINTERTOMBR:
			case LITERAL__cdecl:
			case LITERAL___cdecl:
			case LITERAL__near:
			case LITERAL___near:
			case LITERAL__far:
			case LITERAL___far:
			case LITERAL___interrupt:
			case LITERAL_pascal:
			case LITERAL__pascal:
			case LITERAL___pascal:
			{
				break;
			}
			default:
			{
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
			}
			}
			}
		}
		else {
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_54);
		} else {
			throw;
		}
	}
	return node;
}

ASTNode * CPPParser::function_direct_declarator(
	int definition
) {
#line 1273 "CPP_parser_cpp.g"
	ASTNode *node;
#line 8298 "CPPParser.cpp"
#line 1273 "CPP_parser_cpp.g"
	
		ASTNode *a, *pl, *es, *an;
		const char *q, *tq;
		node = mk_node("function_direct_declarator");
		
#line 8305 "CPPParser.cpp"
	
	try {      // for error handling
		{
		switch ( LA(1)) {
		case LPAREN:
		{
			match(LPAREN);
			q=qualified_id();
			if ( inputState->guessing==0 ) {
#line 1291 "CPP_parser_cpp.g"
				node->set_leaf(q);
#line 8317 "CPPParser.cpp"
			}
			if ( inputState->guessing==0 ) {
#line 1292 "CPP_parser_cpp.g"
				declaratorID(q,qiFun);
#line 8322 "CPPParser.cpp"
			}
			match(RPAREN);
			break;
		}
		case ID:
		case LITERAL___attribute__:
		case OPERATOR:
		case LITERAL_this:
		case LITERAL_true:
		case LITERAL_false:
		case SCOPE:
		{
			{ // ( ... )*
			for (;;) {
				if ((LA(1) == LITERAL___attribute__)) {
					a=attribute();
					if ( inputState->guessing==0 ) {
#line 1295 "CPP_parser_cpp.g"
						node->add_child(a);
#line 8342 "CPPParser.cpp"
					}
				}
				else {
					goto _loop243;
				}
				
			}
			_loop243:;
			} // ( ... )*
			q=qualified_id();
			if ( inputState->guessing==0 ) {
#line 1296 "CPP_parser_cpp.g"
				node->set_leaf(q);
#line 8356 "CPPParser.cpp"
			}
			if ( inputState->guessing==0 ) {
#line 1297 "CPP_parser_cpp.g"
				declaratorID(q,qiFun);
#line 8361 "CPPParser.cpp"
			}
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		match(LPAREN);
		if ( inputState->guessing==0 ) {
#line 1300 "CPP_parser_cpp.g"
			
					functionParameterList();
					if (K_and_R == false)
						in_parameter_list = true;
					
#line 8379 "CPPParser.cpp"
		}
		{
		switch ( LA(1)) {
		case LITERAL_typedef:
		case LITERAL_enum:
		case ID:
		case LITERAL_inline:
		case LITERAL_extern:
		case LITERAL___extension__:
		case LITERAL__inline:
		case LITERAL___inline:
		case LITERAL___inline__:
		case LITERAL_virtual:
		case LITERAL_explicit:
		case LITERAL_friend:
		case LITERAL__stdcall:
		case LITERAL___stdcall:
		case LITERAL_typename:
		case LPAREN:
		case LITERAL_const:
		case LITERAL___const:
		case LITERAL_auto:
		case LITERAL_register:
		case LITERAL_static:
		case LITERAL_mutable:
		case LITERAL_const_cast:
		case LITERAL_volatile:
		case LITERAL___volatile:
		case LITERAL___restrict:
		case LITERAL___restrict__:
		case LITERAL_char:
		case LITERAL_bool:
		case LITERAL_short:
		case LITERAL_int:
		case 73:
		case 74:
		case 75:
		case LITERAL_long:
		case LITERAL_signed:
		case LITERAL___signed:
		case LITERAL___signed__:
		case LITERAL_unsigned:
		case LITERAL_float:
		case LITERAL_double:
		case LITERAL_void:
		case LITERAL__declspec:
		case LITERAL___declspec:
		case LITERAL___builtin_va_list:
		case LITERAL_class:
		case LITERAL_struct:
		case LITERAL_union:
		case OPERATOR:
		case LITERAL_this:
		case LITERAL_true:
		case LITERAL_false:
		case STAR:
		case AMPERSAND:
		case TILDE:
		case ELLIPSIS:
		case SCOPE:
		case LITERAL__cdecl:
		case LITERAL___cdecl:
		case LITERAL__near:
		case LITERAL___near:
		case LITERAL__far:
		case LITERAL___far:
		case LITERAL___interrupt:
		case LITERAL_pascal:
		case LITERAL__pascal:
		case LITERAL___pascal:
		{
			pl=parameter_list();
			if ( inputState->guessing==0 ) {
#line 1305 "CPP_parser_cpp.g"
				node->add_child(pl);
#line 8455 "CPPParser.cpp"
			}
			break;
		}
		case RPAREN:
		{
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		if ( inputState->guessing==0 ) {
#line 1306 "CPP_parser_cpp.g"
			
					if (K_and_R == false)
				  		in_parameter_list = false;
					else
						in_parameter_list = true;
					
#line 8477 "CPPParser.cpp"
		}
		match(RPAREN);
		{
		switch ( LA(1)) {
		case LITERAL_asm:
		case LITERAL___asm:
		case LITERAL___asm__:
		{
			an=func_asm_name();
			if ( inputState->guessing==0 ) {
#line 1313 "CPP_parser_cpp.g"
				node->add_child(an);
#line 8490 "CPPParser.cpp"
			}
			break;
		}
		case LITERAL_typedef:
		case LITERAL_enum:
		case ID:
		case LCURLY:
		case SEMICOLON:
		case LITERAL_inline:
		case ASSIGNEQUAL:
		case LITERAL_extern:
		case LITERAL___extension__:
		case LITERAL__inline:
		case LITERAL___inline:
		case LITERAL___inline__:
		case LITERAL_virtual:
		case LITERAL_explicit:
		case LITERAL_friend:
		case LITERAL__stdcall:
		case LITERAL___stdcall:
		case LITERAL_typename:
		case LITERAL_const:
		case LITERAL___const:
		case LITERAL_auto:
		case LITERAL_register:
		case LITERAL_static:
		case LITERAL_mutable:
		case LITERAL_const_cast:
		case LITERAL_volatile:
		case LITERAL___volatile:
		case LITERAL___restrict:
		case LITERAL___restrict__:
		case LITERAL_char:
		case LITERAL_bool:
		case LITERAL_short:
		case LITERAL_int:
		case 73:
		case 74:
		case 75:
		case LITERAL_long:
		case LITERAL_signed:
		case LITERAL___signed:
		case LITERAL___signed__:
		case LITERAL_unsigned:
		case LITERAL_float:
		case LITERAL_double:
		case LITERAL_void:
		case LITERAL__declspec:
		case LITERAL___declspec:
		case LITERAL___builtin_va_list:
		case LITERAL_class:
		case LITERAL_struct:
		case LITERAL_union:
		case LITERAL_throw:
		case LITERAL_using:
		case SCOPE:
		{
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		{ // ( ... )*
		for (;;) {
			if ((_tokenSet_51.member(LA(1))) && (_tokenSet_84.member(LA(2)))) {
				tq=type_qualifier();
				if ( inputState->guessing==0 ) {
#line 1315 "CPP_parser_cpp.g"
					node->add_child(mk_node(tq));
#line 8563 "CPPParser.cpp"
				}
			}
			else {
				goto _loop247;
			}
			
		}
		_loop247:;
		} // ( ... )*
		{
		switch ( LA(1)) {
		case ASSIGNEQUAL:
		{
			match(ASSIGNEQUAL);
			match(OCTALINT);
			if ( inputState->guessing==0 ) {
#line 1316 "CPP_parser_cpp.g"
				node->add_child(mk_node("purevirtual"));
#line 8582 "CPPParser.cpp"
			}
			break;
		}
		case LITERAL_typedef:
		case LITERAL_enum:
		case ID:
		case LCURLY:
		case SEMICOLON:
		case LITERAL_inline:
		case LITERAL_extern:
		case LITERAL___extension__:
		case LITERAL__inline:
		case LITERAL___inline:
		case LITERAL___inline__:
		case LITERAL_virtual:
		case LITERAL_explicit:
		case LITERAL_friend:
		case LITERAL__stdcall:
		case LITERAL___stdcall:
		case LITERAL_typename:
		case LITERAL_const:
		case LITERAL___const:
		case LITERAL_auto:
		case LITERAL_register:
		case LITERAL_static:
		case LITERAL_mutable:
		case LITERAL_const_cast:
		case LITERAL_volatile:
		case LITERAL___volatile:
		case LITERAL___restrict:
		case LITERAL___restrict__:
		case LITERAL_char:
		case LITERAL_bool:
		case LITERAL_short:
		case LITERAL_int:
		case 73:
		case 74:
		case 75:
		case LITERAL_long:
		case LITERAL_signed:
		case LITERAL___signed:
		case LITERAL___signed__:
		case LITERAL_unsigned:
		case LITERAL_float:
		case LITERAL_double:
		case LITERAL_void:
		case LITERAL__declspec:
		case LITERAL___declspec:
		case LITERAL___builtin_va_list:
		case LITERAL_class:
		case LITERAL_struct:
		case LITERAL_union:
		case LITERAL_throw:
		case LITERAL_using:
		case SCOPE:
		{
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		if ( inputState->guessing==0 ) {
#line 1317 "CPP_parser_cpp.g"
			functionEndParameterList(definition);
#line 8650 "CPPParser.cpp"
		}
		{
		switch ( LA(1)) {
		case LITERAL_throw:
		{
			es=exception_specification();
			if ( inputState->guessing==0 ) {
#line 1318 "CPP_parser_cpp.g"
				node->add_child(es);
#line 8660 "CPPParser.cpp"
			}
			break;
		}
		case LITERAL_typedef:
		case LITERAL_enum:
		case ID:
		case LCURLY:
		case SEMICOLON:
		case LITERAL_inline:
		case LITERAL_extern:
		case LITERAL___extension__:
		case LITERAL__inline:
		case LITERAL___inline:
		case LITERAL___inline__:
		case LITERAL_virtual:
		case LITERAL_explicit:
		case LITERAL_friend:
		case LITERAL__stdcall:
		case LITERAL___stdcall:
		case LITERAL_typename:
		case LITERAL_const:
		case LITERAL___const:
		case LITERAL_auto:
		case LITERAL_register:
		case LITERAL_static:
		case LITERAL_mutable:
		case LITERAL_const_cast:
		case LITERAL_volatile:
		case LITERAL___volatile:
		case LITERAL___restrict:
		case LITERAL___restrict__:
		case LITERAL_char:
		case LITERAL_bool:
		case LITERAL_short:
		case LITERAL_int:
		case 73:
		case 74:
		case 75:
		case LITERAL_long:
		case LITERAL_signed:
		case LITERAL___signed:
		case LITERAL___signed__:
		case LITERAL_unsigned:
		case LITERAL_float:
		case LITERAL_double:
		case LITERAL_void:
		case LITERAL__declspec:
		case LITERAL___declspec:
		case LITERAL___builtin_va_list:
		case LITERAL_class:
		case LITERAL_struct:
		case LITERAL_union:
		case LITERAL_using:
		case SCOPE:
		{
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_28);
		} else {
			throw;
		}
	}
	return node;
}

ASTNode * CPPParser::ctor_head() {
#line 1344 "CPP_parser_cpp.g"
	ASTNode *node;
#line 8739 "CPPParser.cpp"
#line 1344 "CPP_parser_cpp.g"
	
		ASTNode *cds, *cd;
		node = mk_node("ctor_head");
		
#line 8745 "CPPParser.cpp"
	
	try {      // for error handling
		cds=ctor_decl_spec();
		if ( inputState->guessing==0 ) {
#line 1350 "CPP_parser_cpp.g"
			node->add_child(cds);
#line 8752 "CPPParser.cpp"
		}
		cd=ctor_declarator(1);
		if ( inputState->guessing==0 ) {
#line 1351 "CPP_parser_cpp.g"
			node->add_child(cd);
#line 8758 "CPPParser.cpp"
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_85);
		} else {
			throw;
		}
	}
	return node;
}

ASTNode * CPPParser::ctor_body() {
#line 1393 "CPP_parser_cpp.g"
	ASTNode *node;
#line 8775 "CPPParser.cpp"
#line 1393 "CPP_parser_cpp.g"
	
		ASTNode *ci, *cs;
		node = mk_node("ctor_body");
		
#line 8781 "CPPParser.cpp"
	
	try {      // for error handling
		{
		switch ( LA(1)) {
		case COLON:
		{
			ci=ctor_initializer();
			if ( inputState->guessing==0 ) {
#line 1399 "CPP_parser_cpp.g"
				node->add_child(ci);
#line 8792 "CPPParser.cpp"
			}
			break;
		}
		case LCURLY:
		{
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		cs=compound_statement();
		if ( inputState->guessing==0 ) {
#line 1400 "CPP_parser_cpp.g"
			node->add_child(cs);
#line 8810 "CPPParser.cpp"
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_20);
		} else {
			throw;
		}
	}
	return node;
}

char * CPPParser::qualified_ctor_id() {
#line 1380 "CPP_parser_cpp.g"
	char *q;
#line 8827 "CPPParser.cpp"
	ANTLR_USE_NAMESPACE(antlr)RefToken  id = ANTLR_USE_NAMESPACE(antlr)nullToken;
#line 1380 "CPP_parser_cpp.g"
	
		char *so;
		static char qitem[CPPParser_MaxQualifiedItemSize+1];
		
#line 8834 "CPPParser.cpp"
	
	try {      // for error handling
		so=scope_override();
		if ( inputState->guessing==0 ) {
#line 1387 "CPP_parser_cpp.g"
			strcpy(qitem, so);
#line 8841 "CPPParser.cpp"
		}
		id = LT(1);
		match(ID);
		if ( inputState->guessing==0 ) {
#line 1389 "CPP_parser_cpp.g"
			strcat(qitem,(id->getText()).data());
				 q = qitem;
#line 8849 "CPPParser.cpp"
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_86);
		} else {
			throw;
		}
	}
	return q;
}

ASTNode * CPPParser::ctor_initializer() {
#line 1403 "CPP_parser_cpp.g"
	ASTNode *node;
#line 8866 "CPPParser.cpp"
#line 1403 "CPP_parser_cpp.g"
	node = mk_node("unfinished_ctor_body");
#line 8869 "CPPParser.cpp"
	
	try {      // for error handling
		match(COLON);
		superclass_init();
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == COMMA)) {
				match(COMMA);
				superclass_init();
			}
			else {
				goto _loop268;
			}
			
		}
		_loop268:;
		} // ( ... )*
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_60);
		} else {
			throw;
		}
	}
	return node;
}

void CPPParser::superclass_init() {
#line 1409 "CPP_parser_cpp.g"
	
		ASTNode *el;
		const char *q;
		
#line 8905 "CPPParser.cpp"
	
	try {      // for error handling
		q=qualified_id();
		match(LPAREN);
		{
		switch ( LA(1)) {
		case ID:
		case StringLiteral:
		case LPAREN:
		case LITERAL_const_cast:
		case LITERAL_char:
		case LITERAL_bool:
		case LITERAL_short:
		case LITERAL_int:
		case 73:
		case 74:
		case 75:
		case LITERAL_long:
		case LITERAL_signed:
		case LITERAL___signed:
		case LITERAL___signed__:
		case LITERAL_unsigned:
		case LITERAL_float:
		case LITERAL_double:
		case LITERAL_void:
		case LITERAL__declspec:
		case LITERAL___declspec:
		case LITERAL___builtin_va_list:
		case OPERATOR:
		case LITERAL_this:
		case LITERAL_true:
		case LITERAL_false:
		case OCTALINT:
		case STAR:
		case AMPERSAND:
		case TILDE:
		case SCOPE:
		case PLUS:
		case MINUS:
		case PLUSPLUS:
		case MINUSMINUS:
		case LITERAL_sizeof:
		case LITERAL_dynamic_cast:
		case LITERAL_static_cast:
		case LITERAL_reinterpret_cast:
		case NOT:
		case LITERAL_new:
		case LITERAL_delete:
		case DECIMALINT:
		case HEXADECIMALINT:
		case CharLiteral:
		case FLOATONE:
		case FLOATTWO:
		{
			el=expression_list();
			break;
		}
		case RPAREN:
		{
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		match(RPAREN);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_77);
		} else {
			throw;
		}
	}
}

ASTNode * CPPParser::dtor_decl_spec() {
#line 1428 "CPP_parser_cpp.g"
	ASTNode *node;
#line 8988 "CPPParser.cpp"
#line 1428 "CPP_parser_cpp.g"
	
		node = mk_node("dtor_decl_spec");
		
#line 8993 "CPPParser.cpp"
	
	try {      // for error handling
		{ // ( ... )*
		for (;;) {
			switch ( LA(1)) {
			case LITERAL_inline:
			case LITERAL__inline:
			case LITERAL___inline:
			{
				{
				switch ( LA(1)) {
				case LITERAL_inline:
				{
					match(LITERAL_inline);
					break;
				}
				case LITERAL__inline:
				{
					match(LITERAL__inline);
					break;
				}
				case LITERAL___inline:
				{
					match(LITERAL___inline);
					break;
				}
				default:
				{
					throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
				}
				}
				}
				if ( inputState->guessing==0 ) {
#line 1433 "CPP_parser_cpp.g"
					node->add_child(mk_node("inline"));
#line 9029 "CPPParser.cpp"
				}
				break;
			}
			case LITERAL_virtual:
			{
				match(LITERAL_virtual);
				if ( inputState->guessing==0 ) {
#line 1434 "CPP_parser_cpp.g"
					node->add_child(mk_node("virtual"));
#line 9039 "CPPParser.cpp"
				}
				break;
			}
			default:
			{
				goto _loop275;
			}
			}
		}
		_loop275:;
		} // ( ... )*
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_87);
		} else {
			throw;
		}
	}
	return node;
}

ASTNode * CPPParser::dtor_declarator(
	int definition
) {
#line 1437 "CPP_parser_cpp.g"
	ASTNode *node;
#line 9068 "CPPParser.cpp"
#line 1437 "CPP_parser_cpp.g"
	
		ASTNode *es;
		char *s;
		node = mk_node("unfinished_dtor_declaration");
		
#line 9075 "CPPParser.cpp"
	
	try {      // for error handling
		s=scope_override();
		match(TILDE);
		match(ID);
		if ( inputState->guessing==0 ) {
#line 1446 "CPP_parser_cpp.g"
			declaratorParameterList(definition);
#line 9084 "CPPParser.cpp"
		}
		match(LPAREN);
		match(RPAREN);
		if ( inputState->guessing==0 ) {
#line 1448 "CPP_parser_cpp.g"
			declaratorEndParameterList(definition);
#line 9091 "CPPParser.cpp"
		}
		{
		switch ( LA(1)) {
		case LITERAL_throw:
		{
			es=exception_specification();
			break;
		}
		case LCURLY:
		case SEMICOLON:
		{
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_19);
		} else {
			throw;
		}
	}
	return node;
}

ASTNode * CPPParser::parameter_declaration_list() {
#line 1469 "CPP_parser_cpp.g"
	ASTNode *node;
#line 9126 "CPPParser.cpp"
#line 1469 "CPP_parser_cpp.g"
	
		ASTNode *p;
		
#line 9131 "CPPParser.cpp"
	
	try {      // for error handling
		if ( inputState->guessing==0 ) {
#line 1473 "CPP_parser_cpp.g"
			node = mk_node("parameter_declaration_list");
#line 9137 "CPPParser.cpp"
		}
		{
		p=parameter_declaration();
		if ( inputState->guessing==0 ) {
#line 1474 "CPP_parser_cpp.g"
			node->add_child(p);
#line 9144 "CPPParser.cpp"
		}
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == COMMA)) {
				match(COMMA);
				p=parameter_declaration();
				if ( inputState->guessing==0 ) {
#line 1476 "CPP_parser_cpp.g"
					node->add_child(p);
#line 9154 "CPPParser.cpp"
				}
			}
			else {
				goto _loop284;
			}
			
		}
		_loop284:;
		} // ( ... )*
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_88);
		} else {
			throw;
		}
	}
	return node;
}

ASTNode * CPPParser::parameter_declaration() {
#line 1481 "CPP_parser_cpp.g"
	ASTNode *node;
#line 9180 "CPPParser.cpp"
#line 1481 "CPP_parser_cpp.g"
	
		ASTNode *ds, *d, *ad, *re;
		
#line 9185 "CPPParser.cpp"
	
	try {      // for error handling
		if ( inputState->guessing==0 ) {
#line 1485 "CPP_parser_cpp.g"
			node = mk_node("parameter_declaration"); beginParameterDeclaration();
#line 9191 "CPPParser.cpp"
		}
		{
		if (((_tokenSet_13.member(LA(1))) && (_tokenSet_89.member(LA(2))))&&(!((LA(1)==SCOPE) && (LA(2)==STAR||LA(2)==OPERATOR))&&( !(LA(1)==SCOPE||LA(1)==ID) || qualifiedItemIsOneOf(qiType|qiCtor) ))) {
			ds=declaration_specifiers();
			if ( inputState->guessing==0 ) {
#line 1488 "CPP_parser_cpp.g"
				node->add_child(ds);
#line 9199 "CPPParser.cpp"
			}
			{
			bool synPredMatched289 = false;
			if (((_tokenSet_79.member(LA(1))) && (_tokenSet_90.member(LA(2))))) {
				int _m289 = mark();
				synPredMatched289 = true;
				inputState->guessing++;
				try {
					{
					declarator();
					}
				}
				catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
					synPredMatched289 = false;
				}
				rewind(_m289);
				inputState->guessing--;
			}
			if ( synPredMatched289 ) {
				d=declarator();
				if ( inputState->guessing==0 ) {
#line 1490 "CPP_parser_cpp.g"
					node->add_child(d);
#line 9223 "CPPParser.cpp"
				}
			}
			else if ((_tokenSet_91.member(LA(1))) && (_tokenSet_92.member(LA(2)))) {
				ad=abstract_declarator();
				if ( inputState->guessing==0 ) {
#line 1492 "CPP_parser_cpp.g"
					node->add_child(ad);
#line 9231 "CPPParser.cpp"
				}
			}
			else {
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
			}
			
			}
		}
		else {
			bool synPredMatched291 = false;
			if (((_tokenSet_79.member(LA(1))) && (_tokenSet_90.member(LA(2))))) {
				int _m291 = mark();
				synPredMatched291 = true;
				inputState->guessing++;
				try {
					{
					declarator();
					}
				}
				catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
					synPredMatched291 = false;
				}
				rewind(_m291);
				inputState->guessing--;
			}
			if ( synPredMatched291 ) {
				d=declarator();
				if ( inputState->guessing==0 ) {
#line 1495 "CPP_parser_cpp.g"
					node->add_child(d);
#line 9262 "CPPParser.cpp"
				}
			}
			else if ((LA(1) == ELLIPSIS)) {
				match(ELLIPSIS);
				if ( inputState->guessing==0 ) {
#line 1497 "CPP_parser_cpp.g"
					node->set_leaf("ellipsis");
#line 9270 "CPPParser.cpp"
				}
			}
		else {
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		{
		switch ( LA(1)) {
		case ASSIGNEQUAL:
		{
			match(ASSIGNEQUAL);
			re=remainder_expression();
			if ( inputState->guessing==0 ) {
#line 1500 "CPP_parser_cpp.g"
				node->add_child(re);
#line 9287 "CPPParser.cpp"
			}
			break;
		}
		case GREATERTHAN:
		case COMMA:
		case RPAREN:
		case ELLIPSIS:
		{
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_93);
		} else {
			throw;
		}
	}
	return node;
}

ASTNode * CPPParser::abstract_declarator() {
#line 1523 "CPP_parser_cpp.g"
	ASTNode *node;
#line 9319 "CPPParser.cpp"
#line 1523 "CPP_parser_cpp.g"
	
		char *po;
		ASTNode *ad, *ad2, *ads, *cx;
		node = mk_node("abstract_declarator");
		
#line 9326 "CPPParser.cpp"
	
	try {      // for error handling
		switch ( LA(1)) {
		case ID:
		case LITERAL__stdcall:
		case LITERAL___stdcall:
		case STAR:
		case AMPERSAND:
		case SCOPE:
		case LITERAL__cdecl:
		case LITERAL___cdecl:
		case LITERAL__near:
		case LITERAL___near:
		case LITERAL__far:
		case LITERAL___far:
		case LITERAL___interrupt:
		case LITERAL_pascal:
		case LITERAL__pascal:
		case LITERAL___pascal:
		{
			po=ptr_operator();
			if ( inputState->guessing==0 ) {
#line 1530 "CPP_parser_cpp.g"
				node->set_leaf(po);
#line 9351 "CPPParser.cpp"
			}
			ad=abstract_declarator();
			if ( inputState->guessing==0 ) {
#line 1531 "CPP_parser_cpp.g"
				node->add_child(ad);
#line 9357 "CPPParser.cpp"
			}
			break;
		}
		case LPAREN:
		{
			match(LPAREN);
			ad2=abstract_declarator();
			match(RPAREN);
			if ( inputState->guessing==0 ) {
#line 1534 "CPP_parser_cpp.g"
				node->add_child(ad2);
#line 9369 "CPPParser.cpp"
			}
			{ // ( ... )+
			int _cnt296=0;
			for (;;) {
				if ((LA(1) == LPAREN || LA(1) == LSQUARE)) {
					ads=abstract_declarator_suffix();
					if ( inputState->guessing==0 ) {
#line 1535 "CPP_parser_cpp.g"
						node->add_child(ads);
#line 9379 "CPPParser.cpp"
					}
				}
				else {
					if ( _cnt296>=1 ) { goto _loop296; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());}
				}
				
				_cnt296++;
			}
			_loop296:;
			}  // ( ... )+
			break;
		}
		case LSQUARE:
		{
			{ // ( ... )+
			int _cnt299=0;
			for (;;) {
				if ((LA(1) == LSQUARE)) {
					match(LSQUARE);
					{
					switch ( LA(1)) {
					case ID:
					case StringLiteral:
					case LPAREN:
					case LITERAL_const_cast:
					case LITERAL_char:
					case LITERAL_bool:
					case LITERAL_short:
					case LITERAL_int:
					case 73:
					case 74:
					case 75:
					case LITERAL_long:
					case LITERAL_signed:
					case LITERAL___signed:
					case LITERAL___signed__:
					case LITERAL_unsigned:
					case LITERAL_float:
					case LITERAL_double:
					case LITERAL_void:
					case LITERAL__declspec:
					case LITERAL___declspec:
					case LITERAL___builtin_va_list:
					case OPERATOR:
					case LITERAL_this:
					case LITERAL_true:
					case LITERAL_false:
					case OCTALINT:
					case STAR:
					case AMPERSAND:
					case TILDE:
					case SCOPE:
					case PLUS:
					case MINUS:
					case PLUSPLUS:
					case MINUSMINUS:
					case LITERAL_sizeof:
					case LITERAL_dynamic_cast:
					case LITERAL_static_cast:
					case LITERAL_reinterpret_cast:
					case NOT:
					case LITERAL_new:
					case LITERAL_delete:
					case DECIMALINT:
					case HEXADECIMALINT:
					case CharLiteral:
					case FLOATONE:
					case FLOATTWO:
					{
						cx=constant_expression();
						if ( inputState->guessing==0 ) {
#line 1537 "CPP_parser_cpp.g"
							node->add_child(cx);
#line 9453 "CPPParser.cpp"
						}
						break;
					}
					case RSQUARE:
					{
						break;
					}
					default:
					{
						throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
					}
					}
					}
					match(RSQUARE);
					if ( inputState->guessing==0 ) {
#line 1537 "CPP_parser_cpp.g"
						declaratorArray();
#line 9471 "CPPParser.cpp"
					}
				}
				else {
					if ( _cnt299>=1 ) { goto _loop299; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());}
				}
				
				_cnt299++;
			}
			_loop299:;
			}  // ( ... )+
			break;
		}
		case GREATERTHAN:
		case ASSIGNEQUAL:
		case COMMA:
		case RPAREN:
		case ELLIPSIS:
		{
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_94);
		} else {
			throw;
		}
	}
	return node;
}

ASTNode * CPPParser::type_name() {
#line 1504 "CPP_parser_cpp.g"
	ASTNode *node;
#line 9512 "CPPParser.cpp"
#line 1504 "CPP_parser_cpp.g"
	
		ASTNode *ds, *ad;
		node = mk_node("type_name");
		
#line 9518 "CPPParser.cpp"
	
	try {      // for error handling
		ds=declaration_specifiers();
		if ( inputState->guessing==0 ) {
#line 1510 "CPP_parser_cpp.g"
			node->add_child(ds);
#line 9525 "CPPParser.cpp"
		}
		ad=abstract_declarator();
		if ( inputState->guessing==0 ) {
#line 1511 "CPP_parser_cpp.g"
			node->add_child(ad);
#line 9531 "CPPParser.cpp"
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_95);
		} else {
			throw;
		}
	}
	return node;
}

ASTNode * CPPParser::abstract_declarator_suffix() {
#line 1543 "CPP_parser_cpp.g"
	ASTNode *node;
#line 9548 "CPPParser.cpp"
#line 1543 "CPP_parser_cpp.g"
	
		char *cvs;
		ASTNode *cx, *pl, *es;
		node = mk_node("abstract_declarator");
		
#line 9555 "CPPParser.cpp"
	
	try {      // for error handling
		switch ( LA(1)) {
		case LSQUARE:
		{
			match(LSQUARE);
			{
			switch ( LA(1)) {
			case ID:
			case StringLiteral:
			case LPAREN:
			case LITERAL_const_cast:
			case LITERAL_char:
			case LITERAL_bool:
			case LITERAL_short:
			case LITERAL_int:
			case 73:
			case 74:
			case 75:
			case LITERAL_long:
			case LITERAL_signed:
			case LITERAL___signed:
			case LITERAL___signed__:
			case LITERAL_unsigned:
			case LITERAL_float:
			case LITERAL_double:
			case LITERAL_void:
			case LITERAL__declspec:
			case LITERAL___declspec:
			case LITERAL___builtin_va_list:
			case OPERATOR:
			case LITERAL_this:
			case LITERAL_true:
			case LITERAL_false:
			case OCTALINT:
			case STAR:
			case AMPERSAND:
			case TILDE:
			case SCOPE:
			case PLUS:
			case MINUS:
			case PLUSPLUS:
			case MINUSMINUS:
			case LITERAL_sizeof:
			case LITERAL_dynamic_cast:
			case LITERAL_static_cast:
			case LITERAL_reinterpret_cast:
			case NOT:
			case LITERAL_new:
			case LITERAL_delete:
			case DECIMALINT:
			case HEXADECIMALINT:
			case CharLiteral:
			case FLOATONE:
			case FLOATTWO:
			{
				cx=constant_expression();
				if ( inputState->guessing==0 ) {
#line 1550 "CPP_parser_cpp.g"
					node->add_child(cx);
#line 9616 "CPPParser.cpp"
				}
				break;
			}
			case RSQUARE:
			{
				break;
			}
			default:
			{
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
			}
			}
			}
			match(RSQUARE);
			if ( inputState->guessing==0 ) {
#line 1551 "CPP_parser_cpp.g"
				declaratorArray();
#line 9634 "CPPParser.cpp"
			}
			break;
		}
		case LPAREN:
		{
			match(LPAREN);
			if ( inputState->guessing==0 ) {
#line 1554 "CPP_parser_cpp.g"
				declaratorParameterList(0);
#line 9644 "CPPParser.cpp"
			}
			{
			switch ( LA(1)) {
			case LITERAL_typedef:
			case LITERAL_enum:
			case ID:
			case LITERAL_inline:
			case LITERAL_extern:
			case LITERAL___extension__:
			case LITERAL__inline:
			case LITERAL___inline:
			case LITERAL___inline__:
			case LITERAL_virtual:
			case LITERAL_explicit:
			case LITERAL_friend:
			case LITERAL__stdcall:
			case LITERAL___stdcall:
			case LITERAL_typename:
			case LPAREN:
			case LITERAL_const:
			case LITERAL___const:
			case LITERAL_auto:
			case LITERAL_register:
			case LITERAL_static:
			case LITERAL_mutable:
			case LITERAL_const_cast:
			case LITERAL_volatile:
			case LITERAL___volatile:
			case LITERAL___restrict:
			case LITERAL___restrict__:
			case LITERAL_char:
			case LITERAL_bool:
			case LITERAL_short:
			case LITERAL_int:
			case 73:
			case 74:
			case 75:
			case LITERAL_long:
			case LITERAL_signed:
			case LITERAL___signed:
			case LITERAL___signed__:
			case LITERAL_unsigned:
			case LITERAL_float:
			case LITERAL_double:
			case LITERAL_void:
			case LITERAL__declspec:
			case LITERAL___declspec:
			case LITERAL___builtin_va_list:
			case LITERAL_class:
			case LITERAL_struct:
			case LITERAL_union:
			case OPERATOR:
			case LITERAL_this:
			case LITERAL_true:
			case LITERAL_false:
			case STAR:
			case AMPERSAND:
			case TILDE:
			case ELLIPSIS:
			case SCOPE:
			case LITERAL__cdecl:
			case LITERAL___cdecl:
			case LITERAL__near:
			case LITERAL___near:
			case LITERAL__far:
			case LITERAL___far:
			case LITERAL___interrupt:
			case LITERAL_pascal:
			case LITERAL__pascal:
			case LITERAL___pascal:
			{
				pl=parameter_list();
				if ( inputState->guessing==0 ) {
#line 1555 "CPP_parser_cpp.g"
					node->add_child(pl);
#line 9720 "CPPParser.cpp"
				}
				break;
			}
			case RPAREN:
			{
				break;
			}
			default:
			{
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
			}
			}
			}
			match(RPAREN);
			cvs=cv_qualifier_seq();
			if ( inputState->guessing==0 ) {
#line 1557 "CPP_parser_cpp.g"
				node->add_child(mk_node(cvs));
#line 9739 "CPPParser.cpp"
			}
			if ( inputState->guessing==0 ) {
#line 1558 "CPP_parser_cpp.g"
				declaratorEndParameterList(0);
#line 9744 "CPPParser.cpp"
			}
			{
			switch ( LA(1)) {
			case LITERAL_throw:
			{
				es=exception_specification();
				if ( inputState->guessing==0 ) {
#line 1559 "CPP_parser_cpp.g"
					node->add_child(es);
#line 9754 "CPPParser.cpp"
				}
				break;
			}
			case GREATERTHAN:
			case ASSIGNEQUAL:
			case COMMA:
			case LPAREN:
			case RPAREN:
			case LSQUARE:
			case ELLIPSIS:
			{
				break;
			}
			default:
			{
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
			}
			}
			}
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_96);
		} else {
			throw;
		}
	}
	return node;
}

ASTNode * CPPParser::template_parameter() {
#line 1604 "CPP_parser_cpp.g"
	ASTNode *node;
#line 9796 "CPPParser.cpp"
	ANTLR_USE_NAMESPACE(antlr)RefToken  id = ANTLR_USE_NAMESPACE(antlr)nullToken;
#line 1604 "CPP_parser_cpp.g"
	
		ASTNode *p;
		node = mk_node("unfinished_template_parameter");
		
#line 9803 "CPPParser.cpp"
	
	try {      // for error handling
		{
		if ((LA(1) == LITERAL_typename || LA(1) == LITERAL_class) && (_tokenSet_97.member(LA(2)))) {
			{
			switch ( LA(1)) {
			case LITERAL_class:
			{
				match(LITERAL_class);
				break;
			}
			case LITERAL_typename:
			{
				match(LITERAL_typename);
				break;
			}
			default:
			{
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
			}
			}
			}
			{
			switch ( LA(1)) {
			case ID:
			{
				id = LT(1);
				match(ID);
				{
				switch ( LA(1)) {
				case ASSIGNEQUAL:
				{
					match(ASSIGNEQUAL);
					assigned_type_name();
					break;
				}
				case GREATERTHAN:
				case COMMA:
				{
					break;
				}
				default:
				{
					throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
				}
				}
				}
				break;
			}
			case GREATERTHAN:
			case COMMA:
			{
				break;
			}
			default:
			{
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
			}
			}
			}
			if ( inputState->guessing==0 ) {
#line 1613 "CPP_parser_cpp.g"
				templateTypeParameter(id->getText().data());
#line 9867 "CPPParser.cpp"
			}
		}
		else if ((_tokenSet_98.member(LA(1))) && (_tokenSet_99.member(LA(2)))) {
			p=parameter_declaration();
		}
		else {
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_100);
		} else {
			throw;
		}
	}
	return node;
}

void CPPParser::assigned_type_name() {
#line 1623 "CPP_parser_cpp.g"
	
		char *s;
		ASTNode *ad, *ts;
		
#line 9896 "CPPParser.cpp"
	
	try {      // for error handling
		{
		if ((LA(1) == ID || LA(1) == SCOPE) && (_tokenSet_101.member(LA(2)))) {
			s=qualified_type();
			ad=abstract_declarator();
		}
		else if ((_tokenSet_102.member(LA(1))) && (_tokenSet_103.member(LA(2)))) {
			ts=simple_type_specifier();
			ad=abstract_declarator();
		}
		else {
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_100);
		} else {
			throw;
		}
	}
}

void CPPParser::template_id() {
#line 1637 "CPP_parser_cpp.g"
	
		ASTNode *tal;
		
#line 9929 "CPPParser.cpp"
	
	try {      // for error handling
		match(ID);
		match(LESSTHAN);
		tal=template_argument_list();
		match(GREATERTHAN);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_1);
		} else {
			throw;
		}
	}
}

ASTNode * CPPParser::template_argument() {
#line 1658 "CPP_parser_cpp.g"
	ASTNode *node;
#line 9950 "CPPParser.cpp"
#line 1658 "CPP_parser_cpp.g"
	
		ASTNode *tn, *sx;
		node = mk_node("template_argument");
		
#line 9956 "CPPParser.cpp"
	
	try {      // for error handling
		if (((_tokenSet_13.member(LA(1))) && (_tokenSet_104.member(LA(2))))&&(( !(LA(1)==SCOPE||LA(1)==ID) || qualifiedItemIsOneOf(qiType|qiCtor) ))) {
			tn=type_name();
			if ( inputState->guessing==0 ) {
#line 1665 "CPP_parser_cpp.g"
				node->add_child(tn);
#line 9964 "CPPParser.cpp"
			}
		}
		else if ((_tokenSet_71.member(LA(1))) && (_tokenSet_105.member(LA(2)))) {
			sx=shift_expression();
			if ( inputState->guessing==0 ) {
#line 1666 "CPP_parser_cpp.g"
				node->add_child(sx);
#line 9972 "CPPParser.cpp"
			}
		}
		else {
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_100);
		} else {
			throw;
		}
	}
	return node;
}

ASTNode * CPPParser::shift_expression() {
#line 2051 "CPP_parser_cpp.g"
	ASTNode *node;
#line 9994 "CPPParser.cpp"
#line 2051 "CPP_parser_cpp.g"
	
		ASTNode *x;
		char *leaf;
		
#line 10000 "CPPParser.cpp"
	
	try {      // for error handling
		x=additive_expression();
		if ( inputState->guessing==0 ) {
#line 2056 "CPP_parser_cpp.g"
			node = x;
#line 10007 "CPPParser.cpp"
		}
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == SHIFTLEFT || LA(1) == SHIFTRIGHT)) {
				{
				switch ( LA(1)) {
				case SHIFTLEFT:
				{
					match(SHIFTLEFT);
					if ( inputState->guessing==0 ) {
#line 2057 "CPP_parser_cpp.g"
						leaf = "shiftleft";
#line 10020 "CPPParser.cpp"
					}
					break;
				}
				case SHIFTRIGHT:
				{
					match(SHIFTRIGHT);
					if ( inputState->guessing==0 ) {
#line 2057 "CPP_parser_cpp.g"
						leaf = "shiftright";
#line 10030 "CPPParser.cpp"
					}
					break;
				}
				default:
				{
					throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
				}
				}
				}
				x=additive_expression();
				if ( inputState->guessing==0 ) {
#line 2058 "CPP_parser_cpp.g"
					node = exprnode_acc(node, x, leaf);
#line 10044 "CPPParser.cpp"
				}
			}
			else {
				goto _loop418;
			}
			
		}
		_loop418:;
		} // ( ... )*
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_106);
		} else {
			throw;
		}
	}
	return node;
}

ASTNode * CPPParser::statement_list() {
#line 1676 "CPP_parser_cpp.g"
	ASTNode *node;
#line 10069 "CPPParser.cpp"
#line 1676 "CPP_parser_cpp.g"
	
		ASTNode *s;
		node = mk_node("statement_list");
		
#line 10075 "CPPParser.cpp"
	
	try {      // for error handling
		{ // ( ... )+
		int _cnt326=0;
		for (;;) {
			if ((_tokenSet_107.member(LA(1)))) {
				s=statement();
				if ( inputState->guessing==0 ) {
#line 1681 "CPP_parser_cpp.g"
					node->add_child(s);
#line 10086 "CPPParser.cpp"
				}
			}
			else {
				if ( _cnt326>=1 ) { goto _loop326; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());}
			}
			
			_cnt326++;
		}
		_loop326:;
		}  // ( ... )+
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_61);
		} else {
			throw;
		}
	}
	return node;
}

ASTNode * CPPParser::statement() {
#line 1685 "CPP_parser_cpp.g"
	ASTNode *node;
#line 10112 "CPPParser.cpp"
#line 1685 "CPP_parser_cpp.g"
	
		ASTNode *d, *s, *ds, *e, *compstat, *sstat, *iterstat, *jumpstat, *asmnode;
		node = mk_node("statement");
		
#line 10118 "CPPParser.cpp"
	
	try {      // for error handling
		{
		switch ( LA(1)) {
		case LITERAL_case:
		{
			s=case_statement();
			if ( inputState->guessing==0 ) {
#line 1693 "CPP_parser_cpp.g"
				node->add_child(s);
#line 10129 "CPPParser.cpp"
			}
			break;
		}
		case LITERAL_default:
		{
			ds=default_statement();
			if ( inputState->guessing==0 ) {
#line 1694 "CPP_parser_cpp.g"
				node->add_child(ds);
#line 10139 "CPPParser.cpp"
			}
			break;
		}
		case LCURLY:
		{
			compstat=compound_statement();
			if ( inputState->guessing==0 ) {
#line 1696 "CPP_parser_cpp.g"
				node->add_child(compstat);
#line 10149 "CPPParser.cpp"
			}
			break;
		}
		case LITERAL_if:
		case LITERAL_switch:
		{
			sstat=selection_statement();
			if ( inputState->guessing==0 ) {
#line 1697 "CPP_parser_cpp.g"
				node->add_child(sstat);
#line 10160 "CPPParser.cpp"
			}
			break;
		}
		case LITERAL_while:
		case LITERAL_do:
		case LITERAL_for:
		{
			iterstat=iteration_statement();
			if ( inputState->guessing==0 ) {
#line 1698 "CPP_parser_cpp.g"
				node->add_child(iterstat);
#line 10172 "CPPParser.cpp"
			}
			break;
		}
		case LITERAL_goto:
		case LITERAL_continue:
		case LITERAL_break:
		case LITERAL_return:
		{
			jumpstat=jump_statement();
			if ( inputState->guessing==0 ) {
#line 1699 "CPP_parser_cpp.g"
				node->add_child(jumpstat);
#line 10185 "CPPParser.cpp"
			}
			break;
		}
		case SEMICOLON:
		{
			match(SEMICOLON);
			if ( inputState->guessing==0 ) {
#line 1700 "CPP_parser_cpp.g"
				end_of_stmt();
#line 10195 "CPPParser.cpp"
			}
			break;
		}
		case LITERAL_try:
		{
			try_block();
			if ( inputState->guessing==0 ) {
#line 1701 "CPP_parser_cpp.g"
				node->add_child(mk_node("unfinished_try_block"));
#line 10205 "CPPParser.cpp"
			}
			break;
		}
		case LITERAL_throw:
		{
			throw_statement();
			if ( inputState->guessing==0 ) {
#line 1702 "CPP_parser_cpp.g"
				node->add_child(mk_node("unfinished_throw_stmt"));
#line 10215 "CPPParser.cpp"
			}
			break;
		}
		case LITERAL_asm:
		case LITERAL___asm:
		case LITERAL___asm__:
		case LITERAL__asm:
		{
			asmnode=asm_block();
			if ( inputState->guessing==0 ) {
#line 1703 "CPP_parser_cpp.g"
				node->add_child(asmnode);
#line 10228 "CPPParser.cpp"
			}
			break;
		}
		default:
			bool synPredMatched330 = false;
			if (((_tokenSet_2.member(LA(1))) && (_tokenSet_3.member(LA(2))))) {
				int _m330 = mark();
				synPredMatched330 = true;
				inputState->guessing++;
				try {
					{
					declaration();
					}
				}
				catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
					synPredMatched330 = false;
				}
				rewind(_m330);
				inputState->guessing--;
			}
			if ( synPredMatched330 ) {
				d=declaration();
				if ( inputState->guessing==0 ) {
#line 1691 "CPP_parser_cpp.g"
					node->add_child(d);
#line 10254 "CPPParser.cpp"
				}
			}
			else if ((LA(1) == ID) && (LA(2) == COLON)) {
				labeled_statement();
				if ( inputState->guessing==0 ) {
#line 1692 "CPP_parser_cpp.g"
					node->add_child(mk_node("unfinished_labeled_statement"));
#line 10262 "CPPParser.cpp"
				}
			}
			else if ((_tokenSet_71.member(LA(1))) && (_tokenSet_108.member(LA(2)))) {
				e=expression();
				if ( inputState->guessing==0 ) {
#line 1695 "CPP_parser_cpp.g"
					node->add_child(e);
#line 10270 "CPPParser.cpp"
				}
				match(SEMICOLON);
				if ( inputState->guessing==0 ) {
#line 1695 "CPP_parser_cpp.g"
					end_of_stmt();
#line 10276 "CPPParser.cpp"
				}
			}
		else {
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_109);
		} else {
			throw;
		}
	}
	return node;
}

void CPPParser::labeled_statement() {
#line 1707 "CPP_parser_cpp.g"
	
		ASTNode *s;
		
#line 10301 "CPPParser.cpp"
	
	try {      // for error handling
		match(ID);
		match(COLON);
		s=statement();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_109);
		} else {
			throw;
		}
	}
}

ASTNode * CPPParser::case_statement() {
#line 1714 "CPP_parser_cpp.g"
	ASTNode *node;
#line 10321 "CPPParser.cpp"
#line 1714 "CPP_parser_cpp.g"
	
		ASTNode *cx, *s;
		node = mk_node("case_statement");
		
#line 10327 "CPPParser.cpp"
	
	try {      // for error handling
		match(LITERAL_case);
		cx=constant_expression();
		if ( inputState->guessing==0 ) {
#line 1720 "CPP_parser_cpp.g"
			node->add_child(cx);
#line 10335 "CPPParser.cpp"
		}
		match(COLON);
		s=statement();
		if ( inputState->guessing==0 ) {
#line 1721 "CPP_parser_cpp.g"
			node->add_child(s);
#line 10342 "CPPParser.cpp"
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_109);
		} else {
			throw;
		}
	}
	return node;
}

ASTNode * CPPParser::default_statement() {
#line 1724 "CPP_parser_cpp.g"
	ASTNode *node;
#line 10359 "CPPParser.cpp"
#line 1724 "CPP_parser_cpp.g"
	
		ASTNode *s;
		node = mk_node("default_statement");
		
#line 10365 "CPPParser.cpp"
	
	try {      // for error handling
		match(LITERAL_default);
		match(COLON);
		s=statement();
		if ( inputState->guessing==0 ) {
#line 1729 "CPP_parser_cpp.g"
			node->add_child(s);
#line 10374 "CPPParser.cpp"
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_109);
		} else {
			throw;
		}
	}
	return node;
}

ASTNode * CPPParser::expression() {
#line 1907 "CPP_parser_cpp.g"
	ASTNode *node;
#line 10391 "CPPParser.cpp"
#line 1907 "CPP_parser_cpp.g"
	ASTNode *x;
#line 10394 "CPPParser.cpp"
	
	try {      // for error handling
		x=assignment_expression();
		if ( inputState->guessing==0 ) {
#line 1909 "CPP_parser_cpp.g"
			node = x;
#line 10401 "CPPParser.cpp"
		}
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == COMMA)) {
				match(COMMA);
				x=assignment_expression();
				if ( inputState->guessing==0 ) {
#line 1910 "CPP_parser_cpp.g"
					node = exprnode_acc(node, x, "comma");
#line 10411 "CPPParser.cpp"
				}
			}
			else {
				goto _loop380;
			}
			
		}
		_loop380:;
		} // ( ... )*
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_110);
		} else {
			throw;
		}
	}
	return node;
}

ASTNode * CPPParser::selection_statement() {
#line 1745 "CPP_parser_cpp.g"
	ASTNode *node;
#line 10436 "CPPParser.cpp"
#line 1745 "CPP_parser_cpp.g"
	
		ASTNode *x1, *s1, *s2, *x3, *s3;
		node = mk_node("selection_statement");
		
#line 10442 "CPPParser.cpp"
	
	try {      // for error handling
		switch ( LA(1)) {
		case LITERAL_if:
		{
			match(LITERAL_if);
			match(LPAREN);
			x1=expression();
			if ( inputState->guessing==0 ) {
#line 1752 "CPP_parser_cpp.g"
				node->add_child(x1);
#line 10454 "CPPParser.cpp"
			}
			match(RPAREN);
			s1=statement();
			if ( inputState->guessing==0 ) {
#line 1753 "CPP_parser_cpp.g"
				node->add_child(s1);
#line 10461 "CPPParser.cpp"
			}
			{
			if ((LA(1) == LITERAL_else) && (_tokenSet_107.member(LA(2)))) {
				match(LITERAL_else);
				s2=statement();
				if ( inputState->guessing==0 ) {
#line 1755 "CPP_parser_cpp.g"
					ASTNode *elsenode = mk_node("else"); elsenode->add_child(s2);
							 	node->add_child(elsenode);
#line 10471 "CPPParser.cpp"
				}
			}
			else if ((_tokenSet_109.member(LA(1))) && (_tokenSet_111.member(LA(2)))) {
			}
			else {
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
			}
			
			}
			break;
		}
		case LITERAL_switch:
		{
			match(LITERAL_switch);
			match(LPAREN);
			x3=expression();
			if ( inputState->guessing==0 ) {
#line 1758 "CPP_parser_cpp.g"
				node->add_child(x3);
#line 10491 "CPPParser.cpp"
			}
			match(RPAREN);
			s3=statement();
			if ( inputState->guessing==0 ) {
#line 1759 "CPP_parser_cpp.g"
				node->add_child(s3);
#line 10498 "CPPParser.cpp"
			}
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_109);
		} else {
			throw;
		}
	}
	return node;
}

ASTNode * CPPParser::iteration_statement() {
#line 1762 "CPP_parser_cpp.g"
	ASTNode *node;
#line 10522 "CPPParser.cpp"
#line 1762 "CPP_parser_cpp.g"
	
		ASTNode *x1, *x2, *s1, *s2, *x3, *x4, *x5, *s3, *d1;
		int taken_x4 = 0, taken_x5 = 0;
		node = mk_node("iteration_statement");
		
#line 10529 "CPPParser.cpp"
	
	try {      // for error handling
		switch ( LA(1)) {
		case LITERAL_while:
		{
			match(LITERAL_while);
			match(LPAREN);
			x1=expression();
			match(RPAREN);
			if ( inputState->guessing==0 ) {
#line 1770 "CPP_parser_cpp.g"
				node->set_leaf("while"); node->add_child(x1);
#line 10542 "CPPParser.cpp"
			}
			s1=statement();
			if ( inputState->guessing==0 ) {
#line 1772 "CPP_parser_cpp.g"
				node->add_child(s1);
#line 10548 "CPPParser.cpp"
			}
			break;
		}
		case LITERAL_do:
		{
			match(LITERAL_do);
			s2=statement();
			match(LITERAL_while);
			if ( inputState->guessing==0 ) {
#line 1774 "CPP_parser_cpp.g"
				node->set_leaf("do"); node->add_child(s2);
#line 10560 "CPPParser.cpp"
			}
			match(LPAREN);
			x2=expression();
			match(RPAREN);
			if ( inputState->guessing==0 ) {
#line 1775 "CPP_parser_cpp.g"
				node->add_child(x2);
#line 10568 "CPPParser.cpp"
			}
			match(SEMICOLON);
			if ( inputState->guessing==0 ) {
#line 1776 "CPP_parser_cpp.g"
				end_of_stmt();
#line 10574 "CPPParser.cpp"
			}
			break;
		}
		case LITERAL_for:
		{
			match(LITERAL_for);
			match(LPAREN);
			if ( inputState->guessing==0 ) {
#line 1778 "CPP_parser_cpp.g"
				node->set_leaf("for");
#line 10585 "CPPParser.cpp"
			}
			{
			bool synPredMatched341 = false;
			if (((_tokenSet_2.member(LA(1))) && (_tokenSet_3.member(LA(2))))) {
				int _m341 = mark();
				synPredMatched341 = true;
				inputState->guessing++;
				try {
					{
					declaration();
					}
				}
				catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
					synPredMatched341 = false;
				}
				rewind(_m341);
				inputState->guessing--;
			}
			if ( synPredMatched341 ) {
				d1=declaration();
				if ( inputState->guessing==0 ) {
#line 1779 "CPP_parser_cpp.g"
					node->add_child(d1);
#line 10609 "CPPParser.cpp"
				}
			}
			else if ((_tokenSet_71.member(LA(1))) && (_tokenSet_108.member(LA(2)))) {
				x3=expression();
				match(SEMICOLON);
				if ( inputState->guessing==0 ) {
#line 1780 "CPP_parser_cpp.g"
					end_of_stmt();
#line 10618 "CPPParser.cpp"
				}
				if ( inputState->guessing==0 ) {
#line 1780 "CPP_parser_cpp.g"
					node->add_child(x3);
#line 10623 "CPPParser.cpp"
				}
			}
			else if ((LA(1) == SEMICOLON)) {
				match(SEMICOLON);
				if ( inputState->guessing==0 ) {
#line 1781 "CPP_parser_cpp.g"
					end_of_stmt(); node->add_child(mk_node("blank"));
#line 10631 "CPPParser.cpp"
				}
			}
			else {
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
			}
			
			}
			{
			switch ( LA(1)) {
			case ID:
			case StringLiteral:
			case LPAREN:
			case LITERAL_const_cast:
			case LITERAL_char:
			case LITERAL_bool:
			case LITERAL_short:
			case LITERAL_int:
			case 73:
			case 74:
			case 75:
			case LITERAL_long:
			case LITERAL_signed:
			case LITERAL___signed:
			case LITERAL___signed__:
			case LITERAL_unsigned:
			case LITERAL_float:
			case LITERAL_double:
			case LITERAL_void:
			case LITERAL__declspec:
			case LITERAL___declspec:
			case LITERAL___builtin_va_list:
			case OPERATOR:
			case LITERAL_this:
			case LITERAL_true:
			case LITERAL_false:
			case OCTALINT:
			case STAR:
			case AMPERSAND:
			case TILDE:
			case SCOPE:
			case PLUS:
			case MINUS:
			case PLUSPLUS:
			case MINUSMINUS:
			case LITERAL_sizeof:
			case LITERAL_dynamic_cast:
			case LITERAL_static_cast:
			case LITERAL_reinterpret_cast:
			case NOT:
			case LITERAL_new:
			case LITERAL_delete:
			case DECIMALINT:
			case HEXADECIMALINT:
			case CharLiteral:
			case FLOATONE:
			case FLOATTWO:
			{
				x4=expression();
				if ( inputState->guessing==0 ) {
#line 1783 "CPP_parser_cpp.g"
					taken_x4 = 1; node->add_child(x4);
#line 10693 "CPPParser.cpp"
				}
				break;
			}
			case SEMICOLON:
			{
				break;
			}
			default:
			{
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
			}
			}
			}
			match(SEMICOLON);
			if ( inputState->guessing==0 ) {
#line 1783 "CPP_parser_cpp.g"
				end_of_stmt();
#line 10711 "CPPParser.cpp"
			}
			if ( inputState->guessing==0 ) {
#line 1784 "CPP_parser_cpp.g"
				if(taken_x4 == 0) node->add_child(mk_node("blank"));
#line 10716 "CPPParser.cpp"
			}
			{
			switch ( LA(1)) {
			case ID:
			case StringLiteral:
			case LPAREN:
			case LITERAL_const_cast:
			case LITERAL_char:
			case LITERAL_bool:
			case LITERAL_short:
			case LITERAL_int:
			case 73:
			case 74:
			case 75:
			case LITERAL_long:
			case LITERAL_signed:
			case LITERAL___signed:
			case LITERAL___signed__:
			case LITERAL_unsigned:
			case LITERAL_float:
			case LITERAL_double:
			case LITERAL_void:
			case LITERAL__declspec:
			case LITERAL___declspec:
			case LITERAL___builtin_va_list:
			case OPERATOR:
			case LITERAL_this:
			case LITERAL_true:
			case LITERAL_false:
			case OCTALINT:
			case STAR:
			case AMPERSAND:
			case TILDE:
			case SCOPE:
			case PLUS:
			case MINUS:
			case PLUSPLUS:
			case MINUSMINUS:
			case LITERAL_sizeof:
			case LITERAL_dynamic_cast:
			case LITERAL_static_cast:
			case LITERAL_reinterpret_cast:
			case NOT:
			case LITERAL_new:
			case LITERAL_delete:
			case DECIMALINT:
			case HEXADECIMALINT:
			case CharLiteral:
			case FLOATONE:
			case FLOATTWO:
			{
				x5=expression();
				if ( inputState->guessing==0 ) {
#line 1786 "CPP_parser_cpp.g"
					taken_x5 = 1; node->add_child(x5);
#line 10772 "CPPParser.cpp"
				}
				break;
			}
			case RPAREN:
			{
				break;
			}
			default:
			{
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
			}
			}
			}
			if ( inputState->guessing==0 ) {
#line 1787 "CPP_parser_cpp.g"
				if(taken_x5 == 0) node->add_child(mk_node("blank"));
#line 10789 "CPPParser.cpp"
			}
			match(RPAREN);
			s3=statement();
			if ( inputState->guessing==0 ) {
#line 1788 "CPP_parser_cpp.g"
				node->add_child(s3);
#line 10796 "CPPParser.cpp"
			}
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_109);
		} else {
			throw;
		}
	}
	return node;
}

ASTNode * CPPParser::jump_statement() {
#line 1791 "CPP_parser_cpp.g"
	ASTNode *node;
#line 10820 "CPPParser.cpp"
	ANTLR_USE_NAMESPACE(antlr)RefToken  gotoid = ANTLR_USE_NAMESPACE(antlr)nullToken;
#line 1791 "CPP_parser_cpp.g"
	
		ASTNode *x;
		
#line 10826 "CPPParser.cpp"
	
	try {      // for error handling
		{
		switch ( LA(1)) {
		case LITERAL_goto:
		{
			match(LITERAL_goto);
			gotoid = LT(1);
			match(ID);
			match(SEMICOLON);
			if ( inputState->guessing==0 ) {
#line 1796 "CPP_parser_cpp.g"
				end_of_stmt();
#line 10840 "CPPParser.cpp"
			}
			if ( inputState->guessing==0 ) {
#line 1797 "CPP_parser_cpp.g"
				node = mk_node("jump_goto"); node->set_leaf(gotoid->getText().data());
#line 10845 "CPPParser.cpp"
			}
			break;
		}
		case LITERAL_continue:
		{
			match(LITERAL_continue);
			match(SEMICOLON);
			if ( inputState->guessing==0 ) {
#line 1798 "CPP_parser_cpp.g"
				end_of_stmt();
#line 10856 "CPPParser.cpp"
			}
			if ( inputState->guessing==0 ) {
#line 1799 "CPP_parser_cpp.g"
				node = mk_node("jump_statement"); node->set_leaf("continue");
#line 10861 "CPPParser.cpp"
			}
			break;
		}
		case LITERAL_break:
		{
			match(LITERAL_break);
			match(SEMICOLON);
			if ( inputState->guessing==0 ) {
#line 1800 "CPP_parser_cpp.g"
				end_of_stmt();
#line 10872 "CPPParser.cpp"
			}
			if ( inputState->guessing==0 ) {
#line 1801 "CPP_parser_cpp.g"
				node = mk_node("jump_statement"); node->set_leaf("break");
#line 10877 "CPPParser.cpp"
			}
			break;
		}
		case LITERAL_return:
		{
			match(LITERAL_return);
			if ( inputState->guessing==0 ) {
#line 1803 "CPP_parser_cpp.g"
				in_return = true;
#line 10887 "CPPParser.cpp"
			}
			if ( inputState->guessing==0 ) {
#line 1812 "CPP_parser_cpp.g"
				node = mk_node("jump_statement"); node->set_leaf("return");
#line 10892 "CPPParser.cpp"
			}
			{
			switch ( LA(1)) {
			case ID:
			case StringLiteral:
			case LPAREN:
			case LITERAL_const_cast:
			case LITERAL_char:
			case LITERAL_bool:
			case LITERAL_short:
			case LITERAL_int:
			case 73:
			case 74:
			case 75:
			case LITERAL_long:
			case LITERAL_signed:
			case LITERAL___signed:
			case LITERAL___signed__:
			case LITERAL_unsigned:
			case LITERAL_float:
			case LITERAL_double:
			case LITERAL_void:
			case LITERAL__declspec:
			case LITERAL___declspec:
			case LITERAL___builtin_va_list:
			case OPERATOR:
			case LITERAL_this:
			case LITERAL_true:
			case LITERAL_false:
			case OCTALINT:
			case STAR:
			case AMPERSAND:
			case TILDE:
			case SCOPE:
			case PLUS:
			case MINUS:
			case PLUSPLUS:
			case MINUSMINUS:
			case LITERAL_sizeof:
			case LITERAL_dynamic_cast:
			case LITERAL_static_cast:
			case LITERAL_reinterpret_cast:
			case NOT:
			case LITERAL_new:
			case LITERAL_delete:
			case DECIMALINT:
			case HEXADECIMALINT:
			case CharLiteral:
			case FLOATONE:
			case FLOATTWO:
			{
				x=expression();
				if ( inputState->guessing==0 ) {
#line 1813 "CPP_parser_cpp.g"
					node->add_child(x);
#line 10948 "CPPParser.cpp"
				}
				break;
			}
			case SEMICOLON:
			{
				break;
			}
			default:
			{
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
			}
			}
			}
			match(SEMICOLON);
			if ( inputState->guessing==0 ) {
#line 1814 "CPP_parser_cpp.g"
				in_return = false; end_of_stmt();
#line 10966 "CPPParser.cpp"
			}
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_109);
		} else {
			throw;
		}
	}
	return node;
}

void CPPParser::try_block() {
#line 1818 "CPP_parser_cpp.g"
	
		ASTNode *compstat;
		
#line 10993 "CPPParser.cpp"
	
	try {      // for error handling
		match(LITERAL_try);
		compstat=compound_statement();
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == LITERAL_catch)) {
				handler();
			}
			else {
				goto _loop349;
			}
			
		}
		_loop349:;
		} // ( ... )*
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_109);
		} else {
			throw;
		}
	}
}

void CPPParser::throw_statement() {
#line 1850 "CPP_parser_cpp.g"
	
		ASTNode *ax;
		
#line 11026 "CPPParser.cpp"
	
	try {      // for error handling
		match(LITERAL_throw);
		{
		switch ( LA(1)) {
		case ID:
		case StringLiteral:
		case LPAREN:
		case LITERAL_const_cast:
		case LITERAL_char:
		case LITERAL_bool:
		case LITERAL_short:
		case LITERAL_int:
		case 73:
		case 74:
		case 75:
		case LITERAL_long:
		case LITERAL_signed:
		case LITERAL___signed:
		case LITERAL___signed__:
		case LITERAL_unsigned:
		case LITERAL_float:
		case LITERAL_double:
		case LITERAL_void:
		case LITERAL__declspec:
		case LITERAL___declspec:
		case LITERAL___builtin_va_list:
		case OPERATOR:
		case LITERAL_this:
		case LITERAL_true:
		case LITERAL_false:
		case OCTALINT:
		case STAR:
		case AMPERSAND:
		case TILDE:
		case SCOPE:
		case PLUS:
		case MINUS:
		case PLUSPLUS:
		case MINUSMINUS:
		case LITERAL_sizeof:
		case LITERAL_dynamic_cast:
		case LITERAL_static_cast:
		case LITERAL_reinterpret_cast:
		case NOT:
		case LITERAL_new:
		case LITERAL_delete:
		case DECIMALINT:
		case HEXADECIMALINT:
		case CharLiteral:
		case FLOATONE:
		case FLOATTWO:
		{
			ax=assignment_expression();
			break;
		}
		case SEMICOLON:
		{
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		match(SEMICOLON);
		if ( inputState->guessing==0 ) {
#line 1854 "CPP_parser_cpp.g"
			end_of_stmt();
#line 11097 "CPPParser.cpp"
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_109);
		} else {
			throw;
		}
	}
}

ASTNode * CPPParser::asm_block() {
#line 1869 "CPP_parser_cpp.g"
	ASTNode *node;
#line 11113 "CPPParser.cpp"
#line 1869 "CPP_parser_cpp.g"
	node = mk_node("asm_block");
#line 11116 "CPPParser.cpp"
	
	try {      // for error handling
		switch ( LA(1)) {
		case LITERAL___asm:
		case LITERAL__asm:
		{
			std_asm_block();
			break;
		}
		case LITERAL_asm:
		case LITERAL___asm__:
		{
			gcc_asm_block();
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_109);
		} else {
			throw;
		}
	}
	return node;
}

void CPPParser::handler() {
#line 1826 "CPP_parser_cpp.g"
	
		ASTNode *compstat;
		
#line 11154 "CPPParser.cpp"
	
	try {      // for error handling
		match(LITERAL_catch);
		if ( inputState->guessing==0 ) {
#line 1831 "CPP_parser_cpp.g"
			exceptionBeginHandler();
#line 11161 "CPPParser.cpp"
		}
		if ( inputState->guessing==0 ) {
#line 1832 "CPP_parser_cpp.g"
			declaratorParameterList(1);
#line 11166 "CPPParser.cpp"
		}
		match(LPAREN);
		exception_declaration();
		match(RPAREN);
		if ( inputState->guessing==0 ) {
#line 1834 "CPP_parser_cpp.g"
			declaratorEndParameterList(1);
#line 11174 "CPPParser.cpp"
		}
		compstat=compound_statement();
		if ( inputState->guessing==0 ) {
#line 1836 "CPP_parser_cpp.g"
			exceptionEndHandler();
#line 11180 "CPPParser.cpp"
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_112);
		} else {
			throw;
		}
	}
}

void CPPParser::exception_declaration() {
#line 1839 "CPP_parser_cpp.g"
	
		ASTNode *pdl;
		
#line 11198 "CPPParser.cpp"
	
	try {      // for error handling
		pdl=parameter_declaration_list();
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_55);
		} else {
			throw;
		}
	}
}

ASTNode * CPPParser::assignment_expression() {
#line 1914 "CPP_parser_cpp.g"
	ASTNode *node;
#line 11216 "CPPParser.cpp"
#line 1914 "CPP_parser_cpp.g"
	
		ASTNode *cx, *rx;
		char *leaf;
		
#line 11222 "CPPParser.cpp"
	
	try {      // for error handling
		cx=conditional_expression();
		if ( inputState->guessing==0 ) {
#line 1919 "CPP_parser_cpp.g"
			node = cx;
#line 11229 "CPPParser.cpp"
		}
		{
		switch ( LA(1)) {
		case ASSIGNEQUAL:
		case TIMESEQUAL:
		case DIVIDEEQUAL:
		case MINUSEQUAL:
		case PLUSEQUAL:
		case MODEQUAL:
		case SHIFTLEFTEQUAL:
		case SHIFTRIGHTEQUAL:
		case BITWISEANDEQUAL:
		case BITWISEXOREQUAL:
		case BITWISEOREQUAL:
		{
			{
			switch ( LA(1)) {
			case ASSIGNEQUAL:
			{
				match(ASSIGNEQUAL);
				if ( inputState->guessing==0 ) {
#line 1920 "CPP_parser_cpp.g"
					leaf="equals";
#line 11253 "CPPParser.cpp"
				}
				break;
			}
			case TIMESEQUAL:
			{
				match(TIMESEQUAL);
				if ( inputState->guessing==0 ) {
#line 1920 "CPP_parser_cpp.g"
					leaf="timesequals";
#line 11263 "CPPParser.cpp"
				}
				break;
			}
			case DIVIDEEQUAL:
			{
				match(DIVIDEEQUAL);
				if ( inputState->guessing==0 ) {
#line 1920 "CPP_parser_cpp.g"
					leaf="dividesequals";
#line 11273 "CPPParser.cpp"
				}
				break;
			}
			case MINUSEQUAL:
			{
				match(MINUSEQUAL);
				if ( inputState->guessing==0 ) {
#line 1921 "CPP_parser_cpp.g"
					leaf="minusequals";
#line 11283 "CPPParser.cpp"
				}
				break;
			}
			case PLUSEQUAL:
			{
				match(PLUSEQUAL);
				if ( inputState->guessing==0 ) {
#line 1921 "CPP_parser_cpp.g"
					leaf="plusequals";
#line 11293 "CPPParser.cpp"
				}
				break;
			}
			case MODEQUAL:
			{
				match(MODEQUAL);
				if ( inputState->guessing==0 ) {
#line 1922 "CPP_parser_cpp.g"
					leaf="modequals";
#line 11303 "CPPParser.cpp"
				}
				break;
			}
			case SHIFTLEFTEQUAL:
			{
				match(SHIFTLEFTEQUAL);
				if ( inputState->guessing==0 ) {
#line 1923 "CPP_parser_cpp.g"
					leaf="shiftleftequals";
#line 11313 "CPPParser.cpp"
				}
				break;
			}
			case SHIFTRIGHTEQUAL:
			{
				match(SHIFTRIGHTEQUAL);
				if ( inputState->guessing==0 ) {
#line 1924 "CPP_parser_cpp.g"
					leaf="shiftrightequals";
#line 11323 "CPPParser.cpp"
				}
				break;
			}
			case BITWISEANDEQUAL:
			{
				match(BITWISEANDEQUAL);
				if ( inputState->guessing==0 ) {
#line 1925 "CPP_parser_cpp.g"
					leaf="bitwiseandequals";
#line 11333 "CPPParser.cpp"
				}
				break;
			}
			case BITWISEXOREQUAL:
			{
				match(BITWISEXOREQUAL);
				if ( inputState->guessing==0 ) {
#line 1926 "CPP_parser_cpp.g"
					leaf="bitwisexorequals";
#line 11343 "CPPParser.cpp"
				}
				break;
			}
			case BITWISEOREQUAL:
			{
				match(BITWISEOREQUAL);
				if ( inputState->guessing==0 ) {
#line 1927 "CPP_parser_cpp.g"
					leaf="bitwiseorequals";
#line 11353 "CPPParser.cpp"
				}
				break;
			}
			default:
			{
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
			}
			}
			}
			rx=remainder_expression();
			if ( inputState->guessing==0 ) {
#line 1929 "CPP_parser_cpp.g"
				node = exprnode_acc(node, rx, leaf);
#line 11367 "CPPParser.cpp"
			}
			break;
		}
		case GREATERTHAN:
		case SEMICOLON:
		case RCURLY:
		case COLON:
		case COMMA:
		case LITERAL___attribute__:
		case RPAREN:
		case RSQUARE:
		case ELLIPSIS:
		{
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_75);
		} else {
			throw;
		}
	}
	return node;
}

void CPPParser::std_asm_block() {
	
	try {      // for error handling
		{
		switch ( LA(1)) {
		case LITERAL__asm:
		{
			match(LITERAL__asm);
			break;
		}
		case LITERAL___asm:
		{
			match(LITERAL___asm);
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		match(LCURLY);
		{ // ( ... )*
		for (;;) {
			if ((_tokenSet_113.member(LA(1)))) {
				matchNot(RCURLY);
			}
			else {
				goto _loop360;
			}
			
		}
		_loop360:;
		} // ( ... )*
		match(RCURLY);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_109);
		} else {
			throw;
		}
	}
}

void CPPParser::gcc_asm_block() {
	
	try {      // for error handling
		{
		switch ( LA(1)) {
		case LITERAL___asm__:
		{
			match(LITERAL___asm__);
			break;
		}
		case LITERAL_asm:
		{
			match(LITERAL_asm);
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		{
		switch ( LA(1)) {
		case LITERAL_volatile:
		{
			match(LITERAL_volatile);
			break;
		}
		case LITERAL___volatile__:
		{
			match(LITERAL___volatile__);
			break;
		}
		case LITERAL___volatile:
		{
			match(LITERAL___volatile);
			break;
		}
		case LPAREN:
		{
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		match(LPAREN);
		asm_code();
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == COLON || LA(1) == SCOPE)) {
				{
				switch ( LA(1)) {
				case COLON:
				{
					match(COLON);
					break;
				}
				case SCOPE:
				{
					match(SCOPE);
					break;
				}
				default:
				{
					throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
				}
				}
				}
				{
				switch ( LA(1)) {
				case StringLiteral:
				case LSQUARE:
				{
					asm_reg_spec();
					break;
				}
				case COLON:
				case RPAREN:
				case SCOPE:
				{
					break;
				}
				default:
				{
					throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
				}
				}
				}
			}
			else {
				goto _loop367;
			}
			
		}
		_loop367:;
		} // ( ... )*
		match(RPAREN);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_109);
		} else {
			throw;
		}
	}
}

void CPPParser::asm_code() {
	
	try {      // for error handling
		{ // ( ... )+
		int _cnt370=0;
		for (;;) {
			if ((LA(1) == StringLiteral)) {
				match(StringLiteral);
			}
			else {
				if ( _cnt370>=1 ) { goto _loop370; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());}
			}
			
			_cnt370++;
		}
		_loop370:;
		}  // ( ... )+
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_114);
		} else {
			throw;
		}
	}
}

void CPPParser::asm_reg_spec() {
#line 1889 "CPP_parser_cpp.g"
	
		ASTNode *ex1, *ex2;
		
#line 11591 "CPPParser.cpp"
	
	try {      // for error handling
		{
		switch ( LA(1)) {
		case LSQUARE:
		{
			match(LSQUARE);
			match(ID);
			match(RSQUARE);
			break;
		}
		case StringLiteral:
		{
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		match(StringLiteral);
		{
		switch ( LA(1)) {
		case LPAREN:
		{
			match(LPAREN);
			ex1=expression();
			match(RPAREN);
			break;
		}
		case COLON:
		case COMMA:
		case RPAREN:
		case SCOPE:
		{
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == COMMA)) {
				match(COMMA);
				{
				switch ( LA(1)) {
				case LSQUARE:
				{
					match(LSQUARE);
					match(ID);
					match(RSQUARE);
					break;
				}
				case StringLiteral:
				{
					break;
				}
				default:
				{
					throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
				}
				}
				}
				match(StringLiteral);
				{
				switch ( LA(1)) {
				case LPAREN:
				{
					match(LPAREN);
					ex2=expression();
					match(RPAREN);
					break;
				}
				case COLON:
				case COMMA:
				case RPAREN:
				case SCOPE:
				{
					break;
				}
				default:
				{
					throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
				}
				}
				}
			}
			else {
				goto _loop377;
			}
			
		}
		_loop377:;
		} // ( ... )*
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_114);
		} else {
			throw;
		}
	}
}

ASTNode * CPPParser::conditional_expression() {
#line 1958 "CPP_parser_cpp.g"
	ASTNode *node;
#line 11704 "CPPParser.cpp"
#line 1958 "CPP_parser_cpp.g"
	
		ASTNode *lx, *e, *cx;
		
#line 11709 "CPPParser.cpp"
	
	try {      // for error handling
		lx=logical_or_expression();
		if ( inputState->guessing==0 ) {
#line 1963 "CPP_parser_cpp.g"
			node = lx;
#line 11716 "CPPParser.cpp"
		}
		{
		switch ( LA(1)) {
		case QUESTIONMARK:
		{
			match(QUESTIONMARK);
			e=expression();
			match(COLON);
			cx=conditional_expression();
			if ( inputState->guessing==0 ) {
#line 1965 "CPP_parser_cpp.g"
				node = mk_node("expression"); node->add_child(lx);
						node->add_child(e); node->add_child(cx);
						node->set_leaf("c_ternary");
#line 11731 "CPPParser.cpp"
			}
			break;
		}
		case GREATERTHAN:
		case SEMICOLON:
		case RCURLY:
		case ASSIGNEQUAL:
		case COLON:
		case COMMA:
		case LITERAL___attribute__:
		case RPAREN:
		case RSQUARE:
		case ELLIPSIS:
		case TIMESEQUAL:
		case DIVIDEEQUAL:
		case MINUSEQUAL:
		case PLUSEQUAL:
		case MODEQUAL:
		case SHIFTLEFTEQUAL:
		case SHIFTRIGHTEQUAL:
		case BITWISEANDEQUAL:
		case BITWISEXOREQUAL:
		case BITWISEOREQUAL:
		{
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_115);
		} else {
			throw;
		}
	}
	return node;
}

ASTNode * CPPParser::logical_or_expression() {
#line 1979 "CPP_parser_cpp.g"
	ASTNode *node;
#line 11779 "CPPParser.cpp"
#line 1979 "CPP_parser_cpp.g"
	
		ASTNode *x;
		
#line 11784 "CPPParser.cpp"
	
	try {      // for error handling
		x=logical_and_expression();
		if ( inputState->guessing==0 ) {
#line 1984 "CPP_parser_cpp.g"
			node = x;
#line 11791 "CPPParser.cpp"
		}
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == OR)) {
				match(OR);
				x=logical_and_expression();
				if ( inputState->guessing==0 ) {
#line 1985 "CPP_parser_cpp.g"
					node = exprnode_acc(node, x, "logicalor");
#line 11801 "CPPParser.cpp"
				}
			}
			else {
				goto _loop394;
			}
			
		}
		_loop394:;
		} // ( ... )*
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_116);
		} else {
			throw;
		}
	}
	return node;
}

ASTNode * CPPParser::logical_and_expression() {
#line 1988 "CPP_parser_cpp.g"
	ASTNode *node;
#line 11826 "CPPParser.cpp"
#line 1988 "CPP_parser_cpp.g"
	
		ASTNode *x;
		
#line 11831 "CPPParser.cpp"
	
	try {      // for error handling
		x=inclusive_or_expression();
		if ( inputState->guessing==0 ) {
#line 1993 "CPP_parser_cpp.g"
			node = x;
#line 11838 "CPPParser.cpp"
		}
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == AND)) {
				match(AND);
				x=inclusive_or_expression();
				if ( inputState->guessing==0 ) {
#line 1994 "CPP_parser_cpp.g"
					node = exprnode_acc(node, x, "logicaland");
#line 11848 "CPPParser.cpp"
				}
			}
			else {
				goto _loop397;
			}
			
		}
		_loop397:;
		} // ( ... )*
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_117);
		} else {
			throw;
		}
	}
	return node;
}

ASTNode * CPPParser::inclusive_or_expression() {
#line 1997 "CPP_parser_cpp.g"
	ASTNode *node;
#line 11873 "CPPParser.cpp"
#line 1997 "CPP_parser_cpp.g"
	
		ASTNode *x;
		
#line 11878 "CPPParser.cpp"
	
	try {      // for error handling
		x=exclusive_or_expression();
		if ( inputState->guessing==0 ) {
#line 2002 "CPP_parser_cpp.g"
			node = x;
#line 11885 "CPPParser.cpp"
		}
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == BITWISEOR)) {
				match(BITWISEOR);
				x=exclusive_or_expression();
				if ( inputState->guessing==0 ) {
#line 2003 "CPP_parser_cpp.g"
					node = exprnode_acc(node, x, "bitwiseor");
#line 11895 "CPPParser.cpp"
				}
			}
			else {
				goto _loop400;
			}
			
		}
		_loop400:;
		} // ( ... )*
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_118);
		} else {
			throw;
		}
	}
	return node;
}

ASTNode * CPPParser::exclusive_or_expression() {
#line 2006 "CPP_parser_cpp.g"
	ASTNode *node;
#line 11920 "CPPParser.cpp"
#line 2006 "CPP_parser_cpp.g"
	
		ASTNode *x;
		
#line 11925 "CPPParser.cpp"
	
	try {      // for error handling
		x=and_expression();
		if ( inputState->guessing==0 ) {
#line 2011 "CPP_parser_cpp.g"
			node = x;
#line 11932 "CPPParser.cpp"
		}
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == BITWISEXOR)) {
				match(BITWISEXOR);
				x=and_expression();
				if ( inputState->guessing==0 ) {
#line 2012 "CPP_parser_cpp.g"
					node = exprnode_acc(node, x, "bitwisexor");
#line 11942 "CPPParser.cpp"
				}
			}
			else {
				goto _loop403;
			}
			
		}
		_loop403:;
		} // ( ... )*
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_119);
		} else {
			throw;
		}
	}
	return node;
}

ASTNode * CPPParser::and_expression() {
#line 2015 "CPP_parser_cpp.g"
	ASTNode *node;
#line 11967 "CPPParser.cpp"
#line 2015 "CPP_parser_cpp.g"
	
		ASTNode *x;
		
#line 11972 "CPPParser.cpp"
	
	try {      // for error handling
		x=equality_expression();
		if ( inputState->guessing==0 ) {
#line 2020 "CPP_parser_cpp.g"
			node = x;
#line 11979 "CPPParser.cpp"
		}
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == AMPERSAND)) {
				match(AMPERSAND);
				x=equality_expression();
				if ( inputState->guessing==0 ) {
#line 2021 "CPP_parser_cpp.g"
					node = exprnode_acc(node, x, "bitwiseand");
#line 11989 "CPPParser.cpp"
				}
			}
			else {
				goto _loop406;
			}
			
		}
		_loop406:;
		} // ( ... )*
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_120);
		} else {
			throw;
		}
	}
	return node;
}

ASTNode * CPPParser::equality_expression() {
#line 2024 "CPP_parser_cpp.g"
	ASTNode *node;
#line 12014 "CPPParser.cpp"
#line 2024 "CPP_parser_cpp.g"
	
		ASTNode *x;
		char *leaf;
		
#line 12020 "CPPParser.cpp"
	
	try {      // for error handling
		x=relational_expression();
		if ( inputState->guessing==0 ) {
#line 2030 "CPP_parser_cpp.g"
			node = x;
#line 12027 "CPPParser.cpp"
		}
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == NOTEQUAL || LA(1) == EQUAL)) {
				{
				switch ( LA(1)) {
				case NOTEQUAL:
				{
					match(NOTEQUAL);
					if ( inputState->guessing==0 ) {
#line 2031 "CPP_parser_cpp.g"
						leaf = "notequals";
#line 12040 "CPPParser.cpp"
					}
					break;
				}
				case EQUAL:
				{
					match(EQUAL);
					if ( inputState->guessing==0 ) {
#line 2031 "CPP_parser_cpp.g"
						leaf = "equals";
#line 12050 "CPPParser.cpp"
					}
					break;
				}
				default:
				{
					throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
				}
				}
				}
				x=relational_expression();
				if ( inputState->guessing==0 ) {
#line 2032 "CPP_parser_cpp.g"
					node = exprnode_acc(node, x, leaf);
#line 12064 "CPPParser.cpp"
				}
			}
			else {
				goto _loop410;
			}
			
		}
		_loop410:;
		} // ( ... )*
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_121);
		} else {
			throw;
		}
	}
	return node;
}

ASTNode * CPPParser::relational_expression() {
#line 2035 "CPP_parser_cpp.g"
	ASTNode *node;
#line 12089 "CPPParser.cpp"
#line 2035 "CPP_parser_cpp.g"
	
		ASTNode *x;
		char *leaf;
		
#line 12095 "CPPParser.cpp"
	
	try {      // for error handling
		x=shift_expression();
		if ( inputState->guessing==0 ) {
#line 2040 "CPP_parser_cpp.g"
			node = x;
#line 12102 "CPPParser.cpp"
		}
		{ // ( ... )*
		for (;;) {
			if ((_tokenSet_122.member(LA(1))) && (_tokenSet_71.member(LA(2)))) {
				{
				switch ( LA(1)) {
				case LESSTHAN:
				{
					match(LESSTHAN);
					if ( inputState->guessing==0 ) {
#line 2042 "CPP_parser_cpp.g"
						leaf = "lessthan";
#line 12115 "CPPParser.cpp"
					}
					break;
				}
				case GREATERTHAN:
				{
					match(GREATERTHAN);
					if ( inputState->guessing==0 ) {
#line 2043 "CPP_parser_cpp.g"
						leaf = "greaterthan";
#line 12125 "CPPParser.cpp"
					}
					break;
				}
				case LESSTHANOREQUALTO:
				{
					match(LESSTHANOREQUALTO);
					if ( inputState->guessing==0 ) {
#line 2044 "CPP_parser_cpp.g"
						leaf = "lessthanorequals";
#line 12135 "CPPParser.cpp"
					}
					break;
				}
				case GREATERTHANOREQUALTO:
				{
					match(GREATERTHANOREQUALTO);
					if ( inputState->guessing==0 ) {
#line 2045 "CPP_parser_cpp.g"
						leaf = "greaterthanorequals";
#line 12145 "CPPParser.cpp"
					}
					break;
				}
				default:
				{
					throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
				}
				}
				}
				x=shift_expression();
				if ( inputState->guessing==0 ) {
#line 2047 "CPP_parser_cpp.g"
					node = exprnode_acc(node, x, leaf);
#line 12159 "CPPParser.cpp"
				}
			}
			else {
				goto _loop414;
			}
			
		}
		_loop414:;
		} // ( ... )*
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_123);
		} else {
			throw;
		}
	}
	return node;
}

ASTNode * CPPParser::additive_expression() {
#line 2062 "CPP_parser_cpp.g"
	ASTNode *node;
#line 12184 "CPPParser.cpp"
#line 2062 "CPP_parser_cpp.g"
	
		ASTNode *x;
		char *leaf;
		
#line 12190 "CPPParser.cpp"
	
	try {      // for error handling
		x=multiplicative_expression();
		if ( inputState->guessing==0 ) {
#line 2067 "CPP_parser_cpp.g"
			node = x;
#line 12197 "CPPParser.cpp"
		}
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == PLUS || LA(1) == MINUS)) {
				{
				switch ( LA(1)) {
				case PLUS:
				{
					match(PLUS);
					if ( inputState->guessing==0 ) {
#line 2069 "CPP_parser_cpp.g"
						leaf = "add";
#line 12210 "CPPParser.cpp"
					}
					break;
				}
				case MINUS:
				{
					match(MINUS);
					if ( inputState->guessing==0 ) {
#line 2069 "CPP_parser_cpp.g"
						leaf = "subtract";
#line 12220 "CPPParser.cpp"
					}
					break;
				}
				default:
				{
					throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
				}
				}
				}
				x=multiplicative_expression();
				if ( inputState->guessing==0 ) {
#line 2071 "CPP_parser_cpp.g"
					node = exprnode_acc(node, x, leaf);
#line 12234 "CPPParser.cpp"
				}
			}
			else {
				goto _loop422;
			}
			
		}
		_loop422:;
		} // ( ... )*
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_124);
		} else {
			throw;
		}
	}
	return node;
}

ASTNode * CPPParser::multiplicative_expression() {
#line 2082 "CPP_parser_cpp.g"
	ASTNode *node;
#line 12259 "CPPParser.cpp"
#line 2082 "CPP_parser_cpp.g"
	
		ASTNode *x;
		char *leaf;
		
#line 12265 "CPPParser.cpp"
	
	try {      // for error handling
		x=pm_expression();
		if ( inputState->guessing==0 ) {
#line 2087 "CPP_parser_cpp.g"
			node = x;
#line 12272 "CPPParser.cpp"
		}
		{ // ( ... )*
		for (;;) {
			if ((_tokenSet_125.member(LA(1)))) {
				{
				switch ( LA(1)) {
				case STAR:
				{
					match(STAR);
					if ( inputState->guessing==0 ) {
#line 2089 "CPP_parser_cpp.g"
						leaf = "multiply";
#line 12285 "CPPParser.cpp"
					}
					break;
				}
				case DIVIDE:
				{
					match(DIVIDE);
					if ( inputState->guessing==0 ) {
#line 2089 "CPP_parser_cpp.g"
						leaf = "divide";
#line 12295 "CPPParser.cpp"
					}
					break;
				}
				case MOD:
				{
					match(MOD);
					if ( inputState->guessing==0 ) {
#line 2089 "CPP_parser_cpp.g"
						leaf = "modulus";
#line 12305 "CPPParser.cpp"
					}
					break;
				}
				default:
				{
					throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
				}
				}
				}
				x=pm_expression();
				if ( inputState->guessing==0 ) {
#line 2090 "CPP_parser_cpp.g"
					node = exprnode_acc(node, x, leaf);
#line 12319 "CPPParser.cpp"
				}
			}
			else {
				goto _loop426;
			}
			
		}
		_loop426:;
		} // ( ... )*
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_126);
		} else {
			throw;
		}
	}
	return node;
}

ASTNode * CPPParser::pm_expression() {
#line 2095 "CPP_parser_cpp.g"
	ASTNode *node;
#line 12344 "CPPParser.cpp"
#line 2095 "CPP_parser_cpp.g"
	
		ASTNode *x;
		char *leaf;
		
#line 12350 "CPPParser.cpp"
	
	try {      // for error handling
		x=cast_expression();
		if ( inputState->guessing==0 ) {
#line 2100 "CPP_parser_cpp.g"
			node = x;
#line 12357 "CPPParser.cpp"
		}
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == DOTMBR || LA(1) == POINTERTOMBR)) {
				{
				switch ( LA(1)) {
				case DOTMBR:
				{
					match(DOTMBR);
					if ( inputState->guessing==0 ) {
#line 2101 "CPP_parser_cpp.g"
						leaf = ".*";
#line 12370 "CPPParser.cpp"
					}
					break;
				}
				case POINTERTOMBR:
				{
					match(POINTERTOMBR);
					if ( inputState->guessing==0 ) {
#line 2101 "CPP_parser_cpp.g"
						leaf = "->*";
#line 12380 "CPPParser.cpp"
					}
					break;
				}
				default:
				{
					throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
				}
				}
				}
				x=cast_expression();
				if ( inputState->guessing==0 ) {
#line 2102 "CPP_parser_cpp.g"
					node = exprnode_acc(node, x, leaf);
#line 12394 "CPPParser.cpp"
				}
			}
			else {
				goto _loop430;
			}
			
		}
		_loop430:;
		} // ( ... )*
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_127);
		} else {
			throw;
		}
	}
	return node;
}

ASTNode * CPPParser::cast_expression() {
#line 2141 "CPP_parser_cpp.g"
	ASTNode *node;
#line 12419 "CPPParser.cpp"
#line 2141 "CPP_parser_cpp.g"
	
		char *tq, *po;
		ASTNode *ts, *indirection, *cx, *x;
		
#line 12425 "CPPParser.cpp"
	
	try {      // for error handling
		bool synPredMatched433 = false;
		if (((LA(1) == LPAREN) && (_tokenSet_13.member(LA(2))))) {
			int _m433 = mark();
			synPredMatched433 = true;
			inputState->guessing++;
			try {
				{
				compound_literal();
				}
			}
			catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
				synPredMatched433 = false;
			}
			rewind(_m433);
			inputState->guessing--;
		}
		if ( synPredMatched433 ) {
			x=compound_literal();
			if ( inputState->guessing==0 ) {
#line 2147 "CPP_parser_cpp.g"
				node = x;
#line 12449 "CPPParser.cpp"
			}
		}
		else {
			bool synPredMatched438 = false;
			if (((LA(1) == LPAREN) && (_tokenSet_128.member(LA(2))))) {
				int _m438 = mark();
				synPredMatched438 = true;
				inputState->guessing++;
				try {
					{
					match(LPAREN);
					{
					switch ( LA(1)) {
					case LITERAL_const:
					case LITERAL___const:
					case LITERAL_const_cast:
					case LITERAL_volatile:
					case LITERAL___volatile:
					case LITERAL___restrict:
					case LITERAL___restrict__:
					{
						type_qualifier();
						break;
					}
					case LITERAL_enum:
					case ID:
					case LITERAL_char:
					case LITERAL_bool:
					case LITERAL_short:
					case LITERAL_int:
					case 73:
					case 74:
					case 75:
					case LITERAL_long:
					case LITERAL_signed:
					case LITERAL___signed:
					case LITERAL___signed__:
					case LITERAL_unsigned:
					case LITERAL_float:
					case LITERAL_double:
					case LITERAL_void:
					case LITERAL__declspec:
					case LITERAL___declspec:
					case LITERAL___builtin_va_list:
					case LITERAL_class:
					case LITERAL_struct:
					case LITERAL_union:
					case SCOPE:
					{
						break;
					}
					default:
					{
						throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
					}
					}
					}
					type_specifier(dsInvalid);
					{ // ( ... )*
					for (;;) {
						if ((_tokenSet_24.member(LA(1)))) {
							ptr_operator();
						}
						else {
							goto _loop437;
						}
						
					}
					_loop437:;
					} // ( ... )*
					match(RPAREN);
					}
				}
				catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
					synPredMatched438 = false;
				}
				rewind(_m438);
				inputState->guessing--;
			}
			if ( synPredMatched438 ) {
				if ( inputState->guessing==0 ) {
#line 2149 "CPP_parser_cpp.g"
					node = mk_node("expression");
#line 12533 "CPPParser.cpp"
				}
				if ( inputState->guessing==0 ) {
#line 2152 "CPP_parser_cpp.g"
					node->set_leaf("cast");
#line 12538 "CPPParser.cpp"
				}
				match(LPAREN);
				{
				switch ( LA(1)) {
				case LITERAL_const:
				case LITERAL___const:
				case LITERAL_const_cast:
				case LITERAL_volatile:
				case LITERAL___volatile:
				case LITERAL___restrict:
				case LITERAL___restrict__:
				{
					tq=type_qualifier();
					if ( inputState->guessing==0 ) {
#line 2153 "CPP_parser_cpp.g"
						node->add_child(mk_node(tq));
#line 12555 "CPPParser.cpp"
					}
					break;
				}
				case LITERAL_enum:
				case ID:
				case LITERAL_char:
				case LITERAL_bool:
				case LITERAL_short:
				case LITERAL_int:
				case 73:
				case 74:
				case 75:
				case LITERAL_long:
				case LITERAL_signed:
				case LITERAL___signed:
				case LITERAL___signed__:
				case LITERAL_unsigned:
				case LITERAL_float:
				case LITERAL_double:
				case LITERAL_void:
				case LITERAL__declspec:
				case LITERAL___declspec:
				case LITERAL___builtin_va_list:
				case LITERAL_class:
				case LITERAL_struct:
				case LITERAL_union:
				case SCOPE:
				{
					break;
				}
				default:
				{
					throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
				}
				}
				}
				ts=type_specifier(dsInvalid);
				if ( inputState->guessing==0 ) {
#line 2154 "CPP_parser_cpp.g"
					node->add_child(ts);
#line 12596 "CPPParser.cpp"
				}
				if ( inputState->guessing==0 ) {
#line 2155 "CPP_parser_cpp.g"
					indirection = mk_node("indirection"); node->add_child(indirection);
#line 12601 "CPPParser.cpp"
				}
				{ // ( ... )*
				for (;;) {
					if ((_tokenSet_24.member(LA(1)))) {
						po=ptr_operator();
						if ( inputState->guessing==0 ) {
#line 2156 "CPP_parser_cpp.g"
							indirection->extend_leaf(po);
#line 12610 "CPPParser.cpp"
						}
					}
					else {
						goto _loop441;
					}
					
				}
				_loop441:;
				} // ( ... )*
				match(RPAREN);
				cx=cast_expression();
				if ( inputState->guessing==0 ) {
#line 2157 "CPP_parser_cpp.g"
					node->add_child(cx);
#line 12625 "CPPParser.cpp"
				}
			}
			else if ((_tokenSet_71.member(LA(1))) && (_tokenSet_74.member(LA(2)))) {
				x=unary_expression();
				if ( inputState->guessing==0 ) {
#line 2159 "CPP_parser_cpp.g"
					node = x;
#line 12633 "CPPParser.cpp"
				}
			}
		else {
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_129);
		} else {
			throw;
		}
	}
	return node;
}

ASTNode * CPPParser::compound_literal() {
#line 2256 "CPP_parser_cpp.g"
	ASTNode *node;
#line 12655 "CPPParser.cpp"
#line 2256 "CPP_parser_cpp.g"
	
		ASTNode *tn, *i;
		node = mk_node("compound_literal");
		
#line 12661 "CPPParser.cpp"
	
	try {      // for error handling
		match(LPAREN);
		tn=type_name();
		if ( inputState->guessing==0 ) {
#line 2262 "CPP_parser_cpp.g"
			node->add_child(tn);
#line 12669 "CPPParser.cpp"
		}
		match(RPAREN);
		match(LCURLY);
		i=initializer();
		if ( inputState->guessing==0 ) {
#line 2263 "CPP_parser_cpp.g"
			node->add_child(i);
#line 12677 "CPPParser.cpp"
		}
		match(RCURLY);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_129);
		} else {
			throw;
		}
	}
	return node;
}

ASTNode * CPPParser::unary_expression() {
#line 2162 "CPP_parser_cpp.g"
	ASTNode *node;
#line 12695 "CPPParser.cpp"
	ANTLR_USE_NAMESPACE(antlr)RefToken  s = ANTLR_USE_NAMESPACE(antlr)nullToken;
#line 2162 "CPP_parser_cpp.g"
	
		char *op;
		ASTNode *x;
		
#line 12702 "CPPParser.cpp"
	
	try {      // for error handling
		{
		switch ( LA(1)) {
		case PLUSPLUS:
		{
			match(PLUSPLUS);
			x=unary_expression();
			if ( inputState->guessing==0 ) {
#line 2171 "CPP_parser_cpp.g"
				node = mk_node("expression"); node->set_leaf("++."); node->add_child(x);
#line 12714 "CPPParser.cpp"
			}
			break;
		}
		case MINUSMINUS:
		{
			match(MINUSMINUS);
			x=unary_expression();
			if ( inputState->guessing==0 ) {
#line 2173 "CPP_parser_cpp.g"
				node = mk_node("expression"); node->set_leaf("--."); node->add_child(x);
#line 12725 "CPPParser.cpp"
			}
			break;
		}
		case LITERAL_sizeof:
		{
			match(LITERAL_sizeof);
			if ( inputState->guessing==0 ) {
#line 2176 "CPP_parser_cpp.g"
				node = mk_node("expression"); node->set_leaf("sizeof");
#line 12735 "CPPParser.cpp"
			}
			{
			if (((LA(1) == LPAREN) && (_tokenSet_13.member(LA(2))))&&((!(((LA(1)==LPAREN&&(LA(2)==ID))))||(isTypeName((LT(2)->getText()).data()))))) {
				match(LPAREN);
				x=type_name();
				match(RPAREN);
				if ( inputState->guessing==0 ) {
#line 2183 "CPP_parser_cpp.g"
					node->add_child(x);
#line 12745 "CPPParser.cpp"
				}
			}
			else if ((_tokenSet_71.member(LA(1))) && (_tokenSet_74.member(LA(2)))) {
				x=unary_expression();
				if ( inputState->guessing==0 ) {
#line 2184 "CPP_parser_cpp.g"
					node->add_child(x);
#line 12753 "CPPParser.cpp"
				}
			}
			else {
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
			}
			
			}
			break;
		}
		default:
			bool synPredMatched445 = false;
			if (((_tokenSet_130.member(LA(1))) && (_tokenSet_131.member(LA(2))))) {
				int _m445 = mark();
				synPredMatched445 = true;
				inputState->guessing++;
				try {
					{
					postfix_expression();
					}
				}
				catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
					synPredMatched445 = false;
				}
				rewind(_m445);
				inputState->guessing--;
			}
			if ( synPredMatched445 ) {
				x=postfix_expression();
				if ( inputState->guessing==0 ) {
#line 2169 "CPP_parser_cpp.g"
					node = x;
#line 12785 "CPPParser.cpp"
				}
			}
			else if ((_tokenSet_132.member(LA(1))) && (_tokenSet_71.member(LA(2)))) {
				op=unary_operator();
				x=cast_expression();
				if ( inputState->guessing==0 ) {
#line 2175 "CPP_parser_cpp.g"
					node = mk_node("expression"); node->set_leaf(op); node->add_child(x);
#line 12794 "CPPParser.cpp"
				}
			}
			else if ((_tokenSet_133.member(LA(1))) && (_tokenSet_134.member(LA(2)))) {
				if ( inputState->guessing==0 ) {
#line 2186 "CPP_parser_cpp.g"
					node = mk_node("expression");
#line 12801 "CPPParser.cpp"
				}
				{
				switch ( LA(1)) {
				case SCOPE:
				{
					s = LT(1);
					match(SCOPE);
					if ( inputState->guessing==0 ) {
#line 2187 "CPP_parser_cpp.g"
						node->set_leaf(s->getText().data());
#line 12812 "CPPParser.cpp"
					}
					break;
				}
				case LITERAL_new:
				case LITERAL_delete:
				{
					break;
				}
				default:
				{
					throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
				}
				}
				}
				{
				switch ( LA(1)) {
				case LITERAL_new:
				{
					x=new_expression();
					if ( inputState->guessing==0 ) {
#line 2188 "CPP_parser_cpp.g"
						node->add_child(x);
#line 12835 "CPPParser.cpp"
					}
					break;
				}
				case LITERAL_delete:
				{
					x=delete_expression();
					if ( inputState->guessing==0 ) {
#line 2189 "CPP_parser_cpp.g"
						node->add_child(x);
#line 12845 "CPPParser.cpp"
					}
					break;
				}
				default:
				{
					throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
				}
				}
				}
			}
		else {
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_129);
		} else {
			throw;
		}
	}
	return node;
}

ASTNode * CPPParser::postfix_expression() {
#line 2194 "CPP_parser_cpp.g"
	ASTNode *node;
#line 12876 "CPPParser.cpp"
#line 2194 "CPP_parser_cpp.g"
	
		char *po;
		ASTNode *ts, *el, *x, *x2, *idxpr, *idxpr2, *prevnode, *px;
		// Purpose?
		DeclSpecifier ds = dsInvalid;
		
#line 12884 "CPPParser.cpp"
	
	try {      // for error handling
		{
		bool synPredMatched452 = false;
		if ((((_tokenSet_102.member(LA(1))) && (_tokenSet_135.member(LA(2))))&&(!(LA(1)==LPAREN)))) {
			int _m452 = mark();
			synPredMatched452 = true;
			inputState->guessing++;
			try {
				{
				ts=simple_type_specifier();
				match(LPAREN);
				match(RPAREN);
				match(LPAREN);
				}
			}
			catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
				synPredMatched452 = false;
			}
			rewind(_m452);
			inputState->guessing--;
		}
		if ( synPredMatched452 ) {
			ts=simple_type_specifier();
			match(LPAREN);
			match(RPAREN);
			match(LPAREN);
			{
			switch ( LA(1)) {
			case ID:
			case StringLiteral:
			case LPAREN:
			case LITERAL_const_cast:
			case LITERAL_char:
			case LITERAL_bool:
			case LITERAL_short:
			case LITERAL_int:
			case 73:
			case 74:
			case 75:
			case LITERAL_long:
			case LITERAL_signed:
			case LITERAL___signed:
			case LITERAL___signed__:
			case LITERAL_unsigned:
			case LITERAL_float:
			case LITERAL_double:
			case LITERAL_void:
			case LITERAL__declspec:
			case LITERAL___declspec:
			case LITERAL___builtin_va_list:
			case OPERATOR:
			case LITERAL_this:
			case LITERAL_true:
			case LITERAL_false:
			case OCTALINT:
			case STAR:
			case AMPERSAND:
			case TILDE:
			case SCOPE:
			case PLUS:
			case MINUS:
			case PLUSPLUS:
			case MINUSMINUS:
			case LITERAL_sizeof:
			case LITERAL_dynamic_cast:
			case LITERAL_static_cast:
			case LITERAL_reinterpret_cast:
			case NOT:
			case LITERAL_new:
			case LITERAL_delete:
			case DECIMALINT:
			case HEXADECIMALINT:
			case CharLiteral:
			case FLOATONE:
			case FLOATTWO:
			{
				el=expression_list();
				break;
			}
			case RPAREN:
			{
				break;
			}
			default:
			{
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
			}
			}
			}
			match(RPAREN);
			if ( inputState->guessing==0 ) {
#line 2208 "CPP_parser_cpp.g"
				node = mk_node("unfinished_postfix-expr");
#line 12979 "CPPParser.cpp"
			}
		}
		else {
			bool synPredMatched455 = false;
			if ((((_tokenSet_102.member(LA(1))) && (_tokenSet_135.member(LA(2))))&&((LA(1) != LPAREN)))) {
				int _m455 = mark();
				synPredMatched455 = true;
				inputState->guessing++;
				try {
					{
					ts=simple_type_specifier();
					match(LPAREN);
					}
				}
				catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
					synPredMatched455 = false;
				}
				rewind(_m455);
				inputState->guessing--;
			}
			if ( synPredMatched455 ) {
				ts=simple_type_specifier();
				match(LPAREN);
				{
				switch ( LA(1)) {
				case ID:
				case StringLiteral:
				case LPAREN:
				case LITERAL_const_cast:
				case LITERAL_char:
				case LITERAL_bool:
				case LITERAL_short:
				case LITERAL_int:
				case 73:
				case 74:
				case 75:
				case LITERAL_long:
				case LITERAL_signed:
				case LITERAL___signed:
				case LITERAL___signed__:
				case LITERAL_unsigned:
				case LITERAL_float:
				case LITERAL_double:
				case LITERAL_void:
				case LITERAL__declspec:
				case LITERAL___declspec:
				case LITERAL___builtin_va_list:
				case OPERATOR:
				case LITERAL_this:
				case LITERAL_true:
				case LITERAL_false:
				case OCTALINT:
				case STAR:
				case AMPERSAND:
				case TILDE:
				case SCOPE:
				case PLUS:
				case MINUS:
				case PLUSPLUS:
				case MINUSMINUS:
				case LITERAL_sizeof:
				case LITERAL_dynamic_cast:
				case LITERAL_static_cast:
				case LITERAL_reinterpret_cast:
				case NOT:
				case LITERAL_new:
				case LITERAL_delete:
				case DECIMALINT:
				case HEXADECIMALINT:
				case CharLiteral:
				case FLOATONE:
				case FLOATTWO:
				{
					el=expression_list();
					break;
				}
				case RPAREN:
				{
					break;
				}
				default:
				{
					throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
				}
				}
				}
				match(RPAREN);
				if ( inputState->guessing==0 ) {
#line 2213 "CPP_parser_cpp.g"
					node = mk_node("unfinished_postfix-expr-2");
#line 13070 "CPPParser.cpp"
				}
			}
			else if ((_tokenSet_136.member(LA(1))) && (_tokenSet_131.member(LA(2)))) {
				px=primary_expression();
				if ( inputState->guessing==0 ) {
#line 2215 "CPP_parser_cpp.g"
					node = px;
#line 13078 "CPPParser.cpp"
				}
				if ( inputState->guessing==0 ) {
#line 2216 "CPP_parser_cpp.g"
					prevnode = node;
#line 13083 "CPPParser.cpp"
				}
				{ // ( ... )*
				for (;;) {
					switch ( LA(1)) {
					case LSQUARE:
					{
						match(LSQUARE);
						x=expression();
						match(RSQUARE);
						if ( inputState->guessing==0 ) {
#line 2219 "CPP_parser_cpp.g"
							node = mk_node("expression"); node->set_leaf("arrayindex"); node->add_child(prevnode); node->add_child(x); prevnode = node;
#line 13096 "CPPParser.cpp"
						}
						break;
					}
					case LPAREN:
					{
						match(LPAREN);
						if ( inputState->guessing==0 ) {
#line 2221 "CPP_parser_cpp.g"
							node = mk_node("expression"); node->set_leaf("call"); node->add_child(prevnode);
#line 13106 "CPPParser.cpp"
						}
						{
						switch ( LA(1)) {
						case ID:
						case StringLiteral:
						case LPAREN:
						case LITERAL_const_cast:
						case LITERAL_char:
						case LITERAL_bool:
						case LITERAL_short:
						case LITERAL_int:
						case 73:
						case 74:
						case 75:
						case LITERAL_long:
						case LITERAL_signed:
						case LITERAL___signed:
						case LITERAL___signed__:
						case LITERAL_unsigned:
						case LITERAL_float:
						case LITERAL_double:
						case LITERAL_void:
						case LITERAL__declspec:
						case LITERAL___declspec:
						case LITERAL___builtin_va_list:
						case OPERATOR:
						case LITERAL_this:
						case LITERAL_true:
						case LITERAL_false:
						case OCTALINT:
						case STAR:
						case AMPERSAND:
						case TILDE:
						case SCOPE:
						case PLUS:
						case MINUS:
						case PLUSPLUS:
						case MINUSMINUS:
						case LITERAL_sizeof:
						case LITERAL_dynamic_cast:
						case LITERAL_static_cast:
						case LITERAL_reinterpret_cast:
						case NOT:
						case LITERAL_new:
						case LITERAL_delete:
						case DECIMALINT:
						case HEXADECIMALINT:
						case CharLiteral:
						case FLOATONE:
						case FLOATTWO:
						{
							x2=expression_list();
							if ( inputState->guessing==0 ) {
#line 2222 "CPP_parser_cpp.g"
								node->add_child(x2);
#line 13162 "CPPParser.cpp"
							}
							break;
						}
						case RPAREN:
						{
							break;
						}
						default:
						{
							throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
						}
						}
						}
						if ( inputState->guessing==0 ) {
#line 2223 "CPP_parser_cpp.g"
							prevnode = node;
#line 13179 "CPPParser.cpp"
						}
						match(RPAREN);
						break;
					}
					case DOT:
					{
						match(DOT);
						idxpr=id_expression();
						if ( inputState->guessing==0 ) {
#line 2226 "CPP_parser_cpp.g"
							node = mk_node("expression"); node->set_leaf("postfixdot"); node->add_child(prevnode); node->add_child(idxpr);
#line 13191 "CPPParser.cpp"
						}
						if ( inputState->guessing==0 ) {
#line 2227 "CPP_parser_cpp.g"
							prevnode = node;
#line 13196 "CPPParser.cpp"
						}
						break;
					}
					case POINTERTO:
					{
						match(POINTERTO);
						idxpr2=id_expression();
						if ( inputState->guessing==0 ) {
#line 2229 "CPP_parser_cpp.g"
							node = mk_node("expression"); node->set_leaf("postfixptr");
#line 13207 "CPPParser.cpp"
						}
						if ( inputState->guessing==0 ) {
#line 2230 "CPP_parser_cpp.g"
							node->add_child(prevnode); node->add_child(idxpr2); prevnode = node;
#line 13212 "CPPParser.cpp"
						}
						break;
					}
					case PLUSPLUS:
					{
						match(PLUSPLUS);
						if ( inputState->guessing==0 ) {
#line 2232 "CPP_parser_cpp.g"
							node = mk_node("expression"); node->set_leaf("plusplus"); node->add_child(prevnode);
#line 13222 "CPPParser.cpp"
						}
						if ( inputState->guessing==0 ) {
#line 2233 "CPP_parser_cpp.g"
							prevnode = node;
#line 13227 "CPPParser.cpp"
						}
						break;
					}
					case MINUSMINUS:
					{
						match(MINUSMINUS);
						if ( inputState->guessing==0 ) {
#line 2235 "CPP_parser_cpp.g"
							node = mk_node("expression"); node->set_leaf("minusminus"); node->add_child(prevnode);
#line 13237 "CPPParser.cpp"
						}
						if ( inputState->guessing==0 ) {
#line 2236 "CPP_parser_cpp.g"
							prevnode = node;
#line 13242 "CPPParser.cpp"
						}
						break;
					}
					default:
					{
						goto _loop459;
					}
					}
				}
				_loop459:;
				} // ( ... )*
			}
			else if ((_tokenSet_137.member(LA(1)))) {
				{
				switch ( LA(1)) {
				case LITERAL_dynamic_cast:
				{
					match(LITERAL_dynamic_cast);
					break;
				}
				case LITERAL_static_cast:
				{
					match(LITERAL_static_cast);
					break;
				}
				case LITERAL_reinterpret_cast:
				{
					match(LITERAL_reinterpret_cast);
					break;
				}
				case LITERAL_const_cast:
				{
					match(LITERAL_const_cast);
					break;
				}
				default:
				{
					throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
				}
				}
				}
				match(LESSTHAN);
				ts=type_specifier(ds);
				{
				switch ( LA(1)) {
				case ID:
				case LITERAL__stdcall:
				case LITERAL___stdcall:
				case STAR:
				case AMPERSAND:
				case SCOPE:
				case LITERAL__cdecl:
				case LITERAL___cdecl:
				case LITERAL__near:
				case LITERAL___near:
				case LITERAL__far:
				case LITERAL___far:
				case LITERAL___interrupt:
				case LITERAL_pascal:
				case LITERAL__pascal:
				case LITERAL___pascal:
				{
					po=ptr_operator();
					break;
				}
				case GREATERTHAN:
				{
					break;
				}
				default:
				{
					throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
				}
				}
				}
				match(GREATERTHAN);
				match(LPAREN);
				x=expression();
				match(RPAREN);
				if ( inputState->guessing==0 ) {
#line 2242 "CPP_parser_cpp.g"
					node = mk_node("unfinished_postfix-expr-3");
#line 13325 "CPPParser.cpp"
				}
			}
		else {
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_129);
		} else {
			throw;
		}
	}
	return node;
}

char * CPPParser::unary_operator() {
#line 2282 "CPP_parser_cpp.g"
	char *op;
#line 13348 "CPPParser.cpp"
	
	try {      // for error handling
		switch ( LA(1)) {
		case AMPERSAND:
		{
			match(AMPERSAND);
			if ( inputState->guessing==0 ) {
#line 2283 "CPP_parser_cpp.g"
				op = "unary_ampersand";
#line 13358 "CPPParser.cpp"
			}
			break;
		}
		case STAR:
		{
			match(STAR);
			if ( inputState->guessing==0 ) {
#line 2284 "CPP_parser_cpp.g"
				op = "unary_pointsto";
#line 13368 "CPPParser.cpp"
			}
			break;
		}
		case PLUS:
		{
			match(PLUS);
			if ( inputState->guessing==0 ) {
#line 2285 "CPP_parser_cpp.g"
				op = "unary_plus";
#line 13378 "CPPParser.cpp"
			}
			break;
		}
		case MINUS:
		{
			match(MINUS);
			if ( inputState->guessing==0 ) {
#line 2286 "CPP_parser_cpp.g"
				op = "unary_minus";
#line 13388 "CPPParser.cpp"
			}
			break;
		}
		case TILDE:
		{
			match(TILDE);
			if ( inputState->guessing==0 ) {
#line 2287 "CPP_parser_cpp.g"
				op = "unary_tilde";
#line 13398 "CPPParser.cpp"
			}
			break;
		}
		case NOT:
		{
			match(NOT);
			if ( inputState->guessing==0 ) {
#line 2288 "CPP_parser_cpp.g"
				op = "unary_not";
#line 13408 "CPPParser.cpp"
			}
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_71);
		} else {
			throw;
		}
	}
	return op;
}

ASTNode * CPPParser::new_expression() {
#line 2299 "CPP_parser_cpp.g"
	ASTNode *node;
#line 13432 "CPPParser.cpp"
#line 2299 "CPP_parser_cpp.g"
	
		ASTNode *el, *tn;
		
#line 13437 "CPPParser.cpp"
	
	try {      // for error handling
		{
		match(LITERAL_new);
		{
		bool synPredMatched472 = false;
		if (((LA(1) == LPAREN) && (_tokenSet_71.member(LA(2))))) {
			int _m472 = mark();
			synPredMatched472 = true;
			inputState->guessing++;
			try {
				{
				match(LPAREN);
				expression_list();
				match(RPAREN);
				}
			}
			catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& pe) {
				synPredMatched472 = false;
			}
			rewind(_m472);
			inputState->guessing--;
		}
		if ( synPredMatched472 ) {
			match(LPAREN);
			el=expression_list();
			match(RPAREN);
		}
		else if ((_tokenSet_138.member(LA(1))) && (_tokenSet_139.member(LA(2)))) {
		}
		else {
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		
		}
		{
		switch ( LA(1)) {
		case LITERAL_typedef:
		case LITERAL_enum:
		case ID:
		case LITERAL_inline:
		case LITERAL_extern:
		case LITERAL___extension__:
		case LITERAL__inline:
		case LITERAL___inline:
		case LITERAL___inline__:
		case LITERAL_virtual:
		case LITERAL_explicit:
		case LITERAL_friend:
		case LITERAL__stdcall:
		case LITERAL___stdcall:
		case LITERAL_typename:
		case LITERAL_const:
		case LITERAL___const:
		case LITERAL_auto:
		case LITERAL_register:
		case LITERAL_static:
		case LITERAL_mutable:
		case LITERAL_const_cast:
		case LITERAL_volatile:
		case LITERAL___volatile:
		case LITERAL___restrict:
		case LITERAL___restrict__:
		case LITERAL_char:
		case LITERAL_bool:
		case LITERAL_short:
		case LITERAL_int:
		case 73:
		case 74:
		case 75:
		case LITERAL_long:
		case LITERAL_signed:
		case LITERAL___signed:
		case LITERAL___signed__:
		case LITERAL_unsigned:
		case LITERAL_float:
		case LITERAL_double:
		case LITERAL_void:
		case LITERAL__declspec:
		case LITERAL___declspec:
		case LITERAL___builtin_va_list:
		case LITERAL_class:
		case LITERAL_struct:
		case LITERAL_union:
		case SCOPE:
		{
			new_type_id();
			break;
		}
		case LPAREN:
		{
			match(LPAREN);
			tn=type_name();
			match(RPAREN);
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		{
		switch ( LA(1)) {
		case LPAREN:
		{
			new_initializer();
			break;
		}
		case LESSTHAN:
		case GREATERTHAN:
		case SEMICOLON:
		case RCURLY:
		case ASSIGNEQUAL:
		case COLON:
		case COMMA:
		case LITERAL___attribute__:
		case RPAREN:
		case STAR:
		case AMPERSAND:
		case RSQUARE:
		case ELLIPSIS:
		case TIMESEQUAL:
		case DIVIDEEQUAL:
		case MINUSEQUAL:
		case PLUSEQUAL:
		case MODEQUAL:
		case SHIFTLEFTEQUAL:
		case SHIFTRIGHTEQUAL:
		case BITWISEANDEQUAL:
		case BITWISEXOREQUAL:
		case BITWISEOREQUAL:
		case QUESTIONMARK:
		case OR:
		case AND:
		case BITWISEOR:
		case BITWISEXOR:
		case NOTEQUAL:
		case EQUAL:
		case LESSTHANOREQUALTO:
		case GREATERTHANOREQUALTO:
		case SHIFTLEFT:
		case SHIFTRIGHT:
		case PLUS:
		case MINUS:
		case DIVIDE:
		case MOD:
		case DOTMBR:
		case POINTERTOMBR:
		{
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		}
		if ( inputState->guessing==0 ) {
#line 2312 "CPP_parser_cpp.g"
			node = mk_node("unfinished_new-expr");
#line 13600 "CPPParser.cpp"
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_129);
		} else {
			throw;
		}
	}
	return node;
}

ASTNode * CPPParser::delete_expression() {
#line 2415 "CPP_parser_cpp.g"
	ASTNode *node;
#line 13617 "CPPParser.cpp"
#line 2415 "CPP_parser_cpp.g"
	
		ASTNode *cx;
		node = mk_node("delete");
		
#line 13623 "CPPParser.cpp"
	
	try {      // for error handling
		match(LITERAL_delete);
		{
		switch ( LA(1)) {
		case LSQUARE:
		{
			match(LSQUARE);
			match(RSQUARE);
			if ( inputState->guessing==0 ) {
#line 2420 "CPP_parser_cpp.g"
				node->set_leaf("arrayindex");
#line 13636 "CPPParser.cpp"
			}
			break;
		}
		case ID:
		case StringLiteral:
		case LPAREN:
		case LITERAL_const_cast:
		case LITERAL_char:
		case LITERAL_bool:
		case LITERAL_short:
		case LITERAL_int:
		case 73:
		case 74:
		case 75:
		case LITERAL_long:
		case LITERAL_signed:
		case LITERAL___signed:
		case LITERAL___signed__:
		case LITERAL_unsigned:
		case LITERAL_float:
		case LITERAL_double:
		case LITERAL_void:
		case LITERAL__declspec:
		case LITERAL___declspec:
		case LITERAL___builtin_va_list:
		case OPERATOR:
		case LITERAL_this:
		case LITERAL_true:
		case LITERAL_false:
		case OCTALINT:
		case STAR:
		case AMPERSAND:
		case TILDE:
		case SCOPE:
		case PLUS:
		case MINUS:
		case PLUSPLUS:
		case MINUSMINUS:
		case LITERAL_sizeof:
		case LITERAL_dynamic_cast:
		case LITERAL_static_cast:
		case LITERAL_reinterpret_cast:
		case NOT:
		case LITERAL_new:
		case LITERAL_delete:
		case DECIMALINT:
		case HEXADECIMALINT:
		case CharLiteral:
		case FLOATONE:
		case FLOATTWO:
		{
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		cx=cast_expression();
		if ( inputState->guessing==0 ) {
#line 2421 "CPP_parser_cpp.g"
			node->add_child(cx);
#line 13700 "CPPParser.cpp"
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_129);
		} else {
			throw;
		}
	}
	return node;
}

ASTNode * CPPParser::primary_expression() {
#line 2246 "CPP_parser_cpp.g"
	ASTNode *node;
#line 13717 "CPPParser.cpp"
#line 2246 "CPP_parser_cpp.g"
	
		ASTNode *x;
		
#line 13722 "CPPParser.cpp"
	
	try {      // for error handling
		switch ( LA(1)) {
		case ID:
		case OPERATOR:
		case TILDE:
		case SCOPE:
		{
			x=id_expression();
			if ( inputState->guessing==0 ) {
#line 2250 "CPP_parser_cpp.g"
				node = x;
#line 13735 "CPPParser.cpp"
			}
			break;
		}
		case StringLiteral:
		case LITERAL_true:
		case LITERAL_false:
		case OCTALINT:
		case DECIMALINT:
		case HEXADECIMALINT:
		case CharLiteral:
		case FLOATONE:
		case FLOATTWO:
		{
			x=constant();
			if ( inputState->guessing==0 ) {
#line 2251 "CPP_parser_cpp.g"
				node = x;
#line 13753 "CPPParser.cpp"
			}
			break;
		}
		case LITERAL_this:
		{
			match(LITERAL_this);
			if ( inputState->guessing==0 ) {
#line 2252 "CPP_parser_cpp.g"
				node = mk_node("expression"), node->set_leaf("this");
#line 13763 "CPPParser.cpp"
			}
			break;
		}
		case LPAREN:
		{
			match(LPAREN);
			x=expression();
			match(RPAREN);
			if ( inputState->guessing==0 ) {
#line 2253 "CPP_parser_cpp.g"
				node = x;
#line 13775 "CPPParser.cpp"
			}
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_56);
		} else {
			throw;
		}
	}
	return node;
}

ASTNode * CPPParser::id_expression() {
#line 2266 "CPP_parser_cpp.g"
	ASTNode *node;
#line 13799 "CPPParser.cpp"
	ANTLR_USE_NAMESPACE(antlr)RefToken  id1 = ANTLR_USE_NAMESPACE(antlr)nullToken;
	ANTLR_USE_NAMESPACE(antlr)RefToken  id2 = ANTLR_USE_NAMESPACE(antlr)nullToken;
#line 2266 "CPP_parser_cpp.g"
	
		const char *s, *o;
		node = mk_node("expression");
		node->set_leaf("id_expression");
		
#line 13808 "CPPParser.cpp"
	
	try {      // for error handling
		s=scope_override();
		if ( inputState->guessing==0 ) {
#line 2273 "CPP_parser_cpp.g"
			node->set_value(s);
#line 13815 "CPPParser.cpp"
		}
		{
		switch ( LA(1)) {
		case ID:
		{
			id1 = LT(1);
			match(ID);
			if ( inputState->guessing==0 ) {
#line 2274 "CPP_parser_cpp.g"
				node->extend_value(id1->getText().data());
#line 13826 "CPPParser.cpp"
			}
			break;
		}
		case OPERATOR:
		{
			match(OPERATOR);
			o=optor();
			if ( inputState->guessing==0 ) {
#line 2275 "CPP_parser_cpp.g"
				node->extend_value("operator"); node->extend_value(o);
#line 13837 "CPPParser.cpp"
			}
			break;
		}
		case TILDE:
		{
			match(TILDE);
			if ( inputState->guessing==0 ) {
#line 2276 "CPP_parser_cpp.g"
				node->extend_value("~");
#line 13847 "CPPParser.cpp"
			}
			{
			switch ( LA(1)) {
			case STAR:
			{
				match(STAR);
				if ( inputState->guessing==0 ) {
#line 2277 "CPP_parser_cpp.g"
					node->extend_value("*");
#line 13857 "CPPParser.cpp"
				}
				break;
			}
			case ID:
			{
				break;
			}
			default:
			{
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
			}
			}
			}
			id2 = LT(1);
			match(ID);
			if ( inputState->guessing==0 ) {
#line 2278 "CPP_parser_cpp.g"
				node->extend_value(id2->getText().data());
#line 13876 "CPPParser.cpp"
			}
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_56);
		} else {
			throw;
		}
	}
	return node;
}

void CPPParser::new_type_id() {
#line 2322 "CPP_parser_cpp.g"
	
		ASTNode *ds;
		
#line 13903 "CPPParser.cpp"
	
	try {      // for error handling
		ds=declaration_specifiers();
		{
		if ((_tokenSet_140.member(LA(1))) && (_tokenSet_141.member(LA(2)))) {
			new_declarator();
		}
		else if ((_tokenSet_142.member(LA(1))) && (_tokenSet_38.member(LA(2)))) {
		}
		else {
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_142);
		} else {
			throw;
		}
	}
}

void CPPParser::new_initializer() {
#line 2315 "CPP_parser_cpp.g"
	
		ASTNode *el;
		
#line 13934 "CPPParser.cpp"
	
	try {      // for error handling
		match(LPAREN);
		{
		switch ( LA(1)) {
		case ID:
		case StringLiteral:
		case LPAREN:
		case LITERAL_const_cast:
		case LITERAL_char:
		case LITERAL_bool:
		case LITERAL_short:
		case LITERAL_int:
		case 73:
		case 74:
		case 75:
		case LITERAL_long:
		case LITERAL_signed:
		case LITERAL___signed:
		case LITERAL___signed__:
		case LITERAL_unsigned:
		case LITERAL_float:
		case LITERAL_double:
		case LITERAL_void:
		case LITERAL__declspec:
		case LITERAL___declspec:
		case LITERAL___builtin_va_list:
		case OPERATOR:
		case LITERAL_this:
		case LITERAL_true:
		case LITERAL_false:
		case OCTALINT:
		case STAR:
		case AMPERSAND:
		case TILDE:
		case SCOPE:
		case PLUS:
		case MINUS:
		case PLUSPLUS:
		case MINUSMINUS:
		case LITERAL_sizeof:
		case LITERAL_dynamic_cast:
		case LITERAL_static_cast:
		case LITERAL_reinterpret_cast:
		case NOT:
		case LITERAL_new:
		case LITERAL_delete:
		case DECIMALINT:
		case HEXADECIMALINT:
		case CharLiteral:
		case FLOATONE:
		case FLOATTWO:
		{
			el=expression_list();
			break;
		}
		case RPAREN:
		{
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		match(RPAREN);
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_129);
		} else {
			throw;
		}
	}
}

void CPPParser::new_declarator() {
#line 2333 "CPP_parser_cpp.g"
	
		char *po;
		
#line 14018 "CPPParser.cpp"
	
	try {      // for error handling
		switch ( LA(1)) {
		case ID:
		case LITERAL__stdcall:
		case LITERAL___stdcall:
		case STAR:
		case AMPERSAND:
		case SCOPE:
		case LITERAL__cdecl:
		case LITERAL___cdecl:
		case LITERAL__near:
		case LITERAL___near:
		case LITERAL__far:
		case LITERAL___far:
		case LITERAL___interrupt:
		case LITERAL_pascal:
		case LITERAL__pascal:
		case LITERAL___pascal:
		{
			po=ptr_operator();
			{
			if ((_tokenSet_140.member(LA(1))) && (_tokenSet_141.member(LA(2)))) {
				new_declarator();
			}
			else if ((_tokenSet_142.member(LA(1))) && (_tokenSet_38.member(LA(2)))) {
			}
			else {
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
			}
			
			}
			break;
		}
		case LSQUARE:
		{
			direct_new_declarator();
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_142);
		} else {
			throw;
		}
	}
}

void CPPParser::direct_new_declarator() {
#line 2405 "CPP_parser_cpp.g"
	
		ASTNode *x;
		
#line 14079 "CPPParser.cpp"
	
	try {      // for error handling
		{ // ( ... )+
		int _cnt498=0;
		for (;;) {
			if ((LA(1) == LSQUARE)) {
				match(LSQUARE);
				x=expression();
				match(RSQUARE);
			}
			else {
				if ( _cnt498>=1 ) { goto _loop498; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());}
			}
			
			_cnt498++;
		}
		_loop498:;
		}  // ( ... )+
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_142);
		} else {
			throw;
		}
	}
}

char * CPPParser::ptr_to_member() {
#line 2362 "CPP_parser_cpp.g"
	char *ptm;
#line 14112 "CPPParser.cpp"
#line 2362 "CPP_parser_cpp.g"
	
		char *so, *cvs;
		static char s[CPPParser_MaxQualifiedItemSize+1];
		s[0] = '\0';
		
#line 14119 "CPPParser.cpp"
	
	try {      // for error handling
		so=scope_override();
		if ( inputState->guessing==0 ) {
#line 2369 "CPP_parser_cpp.g"
			strcat(s, so);
#line 14126 "CPPParser.cpp"
		}
		match(STAR);
		if ( inputState->guessing==0 ) {
#line 2369 "CPP_parser_cpp.g"
			strcat(s, "*");
#line 14132 "CPPParser.cpp"
		}
		if ( inputState->guessing==0 ) {
#line 2369 "CPP_parser_cpp.g"
			is_pointer = true;
#line 14137 "CPPParser.cpp"
		}
		cvs=cv_qualifier_seq();
		if ( inputState->guessing==0 ) {
#line 2370 "CPP_parser_cpp.g"
			strcat(s, cvs);
#line 14143 "CPPParser.cpp"
		}
		if ( inputState->guessing==0 ) {
#line 2371 "CPP_parser_cpp.g"
			ptm = s;
#line 14148 "CPPParser.cpp"
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_17);
		} else {
			throw;
		}
	}
	return ptm;
}

const char * CPPParser::optor_simple_tokclass() {
#line 2473 "CPP_parser_cpp.g"
	const char *out;
#line 14165 "CPPParser.cpp"
	
	try {      // for error handling
		{
		switch ( LA(1)) {
		case PLUS:
		{
			match(PLUS);
			break;
		}
		case MINUS:
		{
			match(MINUS);
			break;
		}
		case STAR:
		{
			match(STAR);
			break;
		}
		case DIVIDE:
		{
			match(DIVIDE);
			break;
		}
		case MOD:
		{
			match(MOD);
			break;
		}
		case BITWISEXOR:
		{
			match(BITWISEXOR);
			break;
		}
		case AMPERSAND:
		{
			match(AMPERSAND);
			break;
		}
		case BITWISEOR:
		{
			match(BITWISEOR);
			break;
		}
		case TILDE:
		{
			match(TILDE);
			break;
		}
		case NOT:
		{
			match(NOT);
			break;
		}
		case SHIFTLEFT:
		{
			match(SHIFTLEFT);
			break;
		}
		case SHIFTRIGHT:
		{
			match(SHIFTRIGHT);
			break;
		}
		case ASSIGNEQUAL:
		{
			match(ASSIGNEQUAL);
			break;
		}
		case TIMESEQUAL:
		{
			match(TIMESEQUAL);
			break;
		}
		case DIVIDEEQUAL:
		{
			match(DIVIDEEQUAL);
			break;
		}
		case MODEQUAL:
		{
			match(MODEQUAL);
			break;
		}
		case PLUSEQUAL:
		{
			match(PLUSEQUAL);
			break;
		}
		case MINUSEQUAL:
		{
			match(MINUSEQUAL);
			break;
		}
		case SHIFTLEFTEQUAL:
		{
			match(SHIFTLEFTEQUAL);
			break;
		}
		case SHIFTRIGHTEQUAL:
		{
			match(SHIFTRIGHTEQUAL);
			break;
		}
		case BITWISEANDEQUAL:
		{
			match(BITWISEANDEQUAL);
			break;
		}
		case BITWISEXOREQUAL:
		{
			match(BITWISEXOREQUAL);
			break;
		}
		case BITWISEOREQUAL:
		{
			match(BITWISEOREQUAL);
			break;
		}
		case EQUAL:
		{
			match(EQUAL);
			break;
		}
		case NOTEQUAL:
		{
			match(NOTEQUAL);
			break;
		}
		case LESSTHAN:
		{
			match(LESSTHAN);
			break;
		}
		case GREATERTHAN:
		{
			match(GREATERTHAN);
			break;
		}
		case LESSTHANOREQUALTO:
		{
			match(LESSTHANOREQUALTO);
			break;
		}
		case GREATERTHANOREQUALTO:
		{
			match(GREATERTHANOREQUALTO);
			break;
		}
		case OR:
		{
			match(OR);
			break;
		}
		case AND:
		{
			match(AND);
			break;
		}
		case PLUSPLUS:
		{
			match(PLUSPLUS);
			break;
		}
		case MINUSMINUS:
		{
			match(MINUSMINUS);
			break;
		}
		case COMMA:
		{
			match(COMMA);
			break;
		}
		case POINTERTO:
		{
			match(POINTERTO);
			break;
		}
		case POINTERTOMBR:
		{
			match(POINTERTOMBR);
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		if ( inputState->guessing==0 ) {
#line 2482 "CPP_parser_cpp.g"
			out = LT(1)->getText().data();
#line 14359 "CPPParser.cpp"
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		if( inputState->guessing == 0 ) {
			reportError(ex);
			recover(ex,_tokenSet_64);
		} else {
			throw;
		}
	}
	return out;
}

void CPPParser::initializeASTFactory( ANTLR_USE_NAMESPACE(antlr)ASTFactory& )
{
}
const char* CPPParser::tokenNames[] = {
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
	"Number",
	0
};

const unsigned long CPPParser::_tokenSet_0_data_[] = { 4294065040UL, 4026531852UL, 1073741823UL, 2415919408UL, 0UL, 16368UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "template" "typedef" "enum" ID SEMICOLON "inline" "namespace" "extern" 
// "__extension__" "_inline" "__inline" "__inline__" "virtual" "explicit" 
// "friend" "_stdcall" "__stdcall" "typename" "__attribute__" LPAREN "const" 
// "__const" "auto" "register" "static" "mutable" "const_cast" "volatile" 
// "__volatile" "__restrict" "__restrict__" "char" "bool" "short" "int" 
// "_int64" "__int64" "__w64" "long" "signed" "__signed" "__signed__" "unsigned" 
// "float" "double" "void" "_declspec" "__declspec" "__builtin_va_list" 
// "class" "struct" "union" "operator" "this" "true" "false" STAR AMPERSAND 
// TILDE "using" SCOPE "_cdecl" "__cdecl" "_near" "__near" "_far" "__far" 
// "__interrupt" "pascal" "_pascal" "__pascal" 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_0(_tokenSet_0_data_,12);
const unsigned long CPPParser::_tokenSet_1_data_[] = { 2UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// EOF 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_1(_tokenSet_1_data_,8);
const unsigned long CPPParser::_tokenSet_2_data_[] = { 1072829312UL, 4026531852UL, 67108863UL, 2415919104UL, 0UL, 0UL, 0UL, 0UL };
// "typedef" "enum" ID "inline" "extern" "__extension__" "_inline" "__inline" 
// "__inline__" "virtual" "explicit" "friend" "_stdcall" "__stdcall" "typename" 
// "const" "__const" "auto" "register" "static" "mutable" "const_cast" 
// "volatile" "__volatile" "__restrict" "__restrict__" "char" "bool" "short" 
// "int" "_int64" "__int64" "__w64" "long" "signed" "__signed" "__signed__" 
// "unsigned" "float" "double" "void" "_declspec" "__declspec" "__builtin_va_list" 
// "class" "struct" "union" "using" SCOPE 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_2(_tokenSet_2_data_,8);
const unsigned long CPPParser::_tokenSet_3_data_[] = { 3757981600UL, 4026531852UL, 1073741823UL, 2147483952UL, 0UL, 16368UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// LESSTHAN "typedef" "enum" ID LCURLY SEMICOLON "inline" "namespace" "extern" 
// StringLiteral COMMA "__extension__" "_inline" "__inline" "__inline__" 
// "virtual" "explicit" "friend" "_stdcall" "__stdcall" "__attribute__" 
// LPAREN "const" "__const" "auto" "register" "static" "mutable" "const_cast" 
// "volatile" "__volatile" "__restrict" "__restrict__" "char" "bool" "short" 
// "int" "_int64" "__int64" "__w64" "long" "signed" "__signed" "__signed__" 
// "unsigned" "float" "double" "void" "_declspec" "__declspec" "__builtin_va_list" 
// "class" "struct" "union" "operator" "this" "true" "false" STAR AMPERSAND 
// TILDE SCOPE "_cdecl" "__cdecl" "_near" "__near" "_far" "__far" "__interrupt" 
// "pascal" "_pascal" "__pascal" 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_3(_tokenSet_3_data_,12);
const unsigned long CPPParser::_tokenSet_4_data_[] = { 23073296UL, 0UL, 0UL, 2147483904UL, 0UL, 0UL, 0UL, 0UL };
// "template" ID "inline" "_inline" "__inline" "virtual" TILDE SCOPE 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_4(_tokenSet_4_data_,8);
const unsigned long CPPParser::_tokenSet_5_data_[] = { 23073312UL, 0UL, 0UL, 2147483904UL, 0UL, 0UL, 0UL, 0UL };
// LESSTHAN ID "inline" "_inline" "__inline" "virtual" TILDE SCOPE 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_5(_tokenSet_5_data_,8);
const unsigned long CPPParser::_tokenSet_6_data_[] = { 39850496UL, 0UL, 0UL, 2147483648UL, 0UL, 0UL, 0UL, 0UL };
// ID "inline" "_inline" "__inline" "explicit" SCOPE 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_6(_tokenSet_6_data_,8);
const unsigned long CPPParser::_tokenSet_7_data_[] = { 2187334176UL, 0UL, 0UL, 2147483648UL, 0UL, 0UL, 0UL, 0UL };
// LESSTHAN ID "inline" "_inline" "__inline" "explicit" LPAREN SCOPE 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_7(_tokenSet_7_data_,8);
const unsigned long CPPParser::_tokenSet_8_data_[] = { 4608UL, 0UL, 67108864UL, 2147483648UL, 0UL, 0UL, 0UL, 0UL };
// ID "inline" "operator" SCOPE 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_8(_tokenSet_8_data_,8);
const unsigned long CPPParser::_tokenSet_9_data_[] = { 1072829344UL, 4026531852UL, 134217727UL, 2147483648UL, 0UL, 0UL, 0UL, 0UL };
// LESSTHAN "typedef" "enum" ID "inline" "extern" "__extension__" "_inline" 
// "__inline" "__inline__" "virtual" "explicit" "friend" "_stdcall" "__stdcall" 
// "typename" "const" "__const" "auto" "register" "static" "mutable" "const_cast" 
// "volatile" "__volatile" "__restrict" "__restrict__" "char" "bool" "short" 
// "int" "_int64" "__int64" "__w64" "long" "signed" "__signed" "__signed__" 
// "unsigned" "float" "double" "void" "_declspec" "__declspec" "__builtin_va_list" 
// "class" "struct" "union" "operator" SCOPE 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_9(_tokenSet_9_data_,8);
const unsigned long CPPParser::_tokenSet_10_data_[] = { 4294054784UL, 4026531852UL, 1073741823UL, 2147483696UL, 0UL, 16368UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "typedef" "enum" ID "inline" "extern" "__extension__" "_inline" "__inline" 
// "__inline__" "virtual" "explicit" "friend" "_stdcall" "__stdcall" "typename" 
// "__attribute__" LPAREN "const" "__const" "auto" "register" "static" 
// "mutable" "const_cast" "volatile" "__volatile" "__restrict" "__restrict__" 
// "char" "bool" "short" "int" "_int64" "__int64" "__w64" "long" "signed" 
// "__signed" "__signed__" "unsigned" "float" "double" "void" "_declspec" 
// "__declspec" "__builtin_va_list" "class" "struct" "union" "operator" 
// "this" "true" "false" STAR AMPERSAND SCOPE "_cdecl" "__cdecl" "_near" 
// "__near" "_far" "__far" "__interrupt" "pascal" "_pascal" "__pascal" 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_10(_tokenSet_10_data_,12);
const unsigned long CPPParser::_tokenSet_11_data_[] = { 3757742048UL, 4026531852UL, 1073741823UL, 2147484016UL, 1577057279UL, 32764UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// LESSTHAN GREATERTHAN "typedef" "enum" ID LCURLY "inline" ASSIGNEQUAL 
// "extern" COMMA "__extension__" "_inline" "__inline" "__inline__" "virtual" 
// "explicit" "friend" "_stdcall" "__stdcall" "__attribute__" LPAREN "const" 
// "__const" "auto" "register" "static" "mutable" "const_cast" "volatile" 
// "__volatile" "__restrict" "__restrict__" "char" "bool" "short" "int" 
// "_int64" "__int64" "__w64" "long" "signed" "__signed" "__signed__" "unsigned" 
// "float" "double" "void" "_declspec" "__declspec" "__builtin_va_list" 
// "class" "struct" "union" "operator" "this" "true" "false" STAR AMPERSAND 
// LSQUARE TILDE SCOPE TIMESEQUAL DIVIDEEQUAL MINUSEQUAL PLUSEQUAL MODEQUAL 
// SHIFTLEFTEQUAL SHIFTRIGHTEQUAL BITWISEANDEQUAL BITWISEXOREQUAL BITWISEOREQUAL 
// OR AND BITWISEOR BITWISEXOR NOTEQUAL EQUAL LESSTHANOREQUALTO GREATERTHANOREQUALTO 
// SHIFTLEFT SHIFTRIGHT PLUS MINUS DIVIDE MOD POINTERTOMBR PLUSPLUS MINUSMINUS 
// POINTERTO NOT "new" "_cdecl" "__cdecl" "_near" "__near" "_far" "__far" 
// "__interrupt" "pascal" "_pascal" "__pascal" "delete" 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_11(_tokenSet_11_data_,12);
const unsigned long CPPParser::_tokenSet_12_data_[] = { 4294081426UL, 4026531852UL, 1073741823UL, 2415919408UL, 0UL, 16368UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// EOF "template" "typedef" "enum" ID SEMICOLON "inline" "namespace" RCURLY 
// "extern" "__extension__" "_inline" "__inline" "__inline__" "virtual" 
// "explicit" "friend" "_stdcall" "__stdcall" "typename" "__attribute__" 
// LPAREN "const" "__const" "auto" "register" "static" "mutable" "const_cast" 
// "volatile" "__volatile" "__restrict" "__restrict__" "char" "bool" "short" 
// "int" "_int64" "__int64" "__w64" "long" "signed" "__signed" "__signed__" 
// "unsigned" "float" "double" "void" "_declspec" "__declspec" "__builtin_va_list" 
// "class" "struct" "union" "operator" "this" "true" "false" STAR AMPERSAND 
// TILDE "using" SCOPE "_cdecl" "__cdecl" "_near" "__near" "_far" "__far" 
// "__interrupt" "pascal" "_pascal" "__pascal" 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_12(_tokenSet_12_data_,12);
const unsigned long CPPParser::_tokenSet_13_data_[] = { 1072829312UL, 4026531852UL, 67108863UL, 2147483648UL, 0UL, 0UL, 0UL, 0UL };
// "typedef" "enum" ID "inline" "extern" "__extension__" "_inline" "__inline" 
// "__inline__" "virtual" "explicit" "friend" "_stdcall" "__stdcall" "typename" 
// "const" "__const" "auto" "register" "static" "mutable" "const_cast" 
// "volatile" "__volatile" "__restrict" "__restrict__" "char" "bool" "short" 
// "int" "_int64" "__int64" "__w64" "long" "signed" "__signed" "__signed__" 
// "unsigned" "float" "double" "void" "_declspec" "__declspec" "__builtin_va_list" 
// "class" "struct" "union" SCOPE 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_13(_tokenSet_13_data_,8);
const unsigned long CPPParser::_tokenSet_14_data_[] = { 3757711264UL, 4026531852UL, 1073741823UL, 2147483952UL, 0UL, 16368UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// LESSTHAN "typedef" "enum" ID LCURLY SEMICOLON "inline" "extern" COMMA 
// "__extension__" "_inline" "__inline" "__inline__" "virtual" "explicit" 
// "friend" "_stdcall" "__stdcall" "__attribute__" LPAREN "const" "__const" 
// "auto" "register" "static" "mutable" "const_cast" "volatile" "__volatile" 
// "__restrict" "__restrict__" "char" "bool" "short" "int" "_int64" "__int64" 
// "__w64" "long" "signed" "__signed" "__signed__" "unsigned" "float" "double" 
// "void" "_declspec" "__declspec" "__builtin_va_list" "class" "struct" 
// "union" "operator" "this" "true" "false" STAR AMPERSAND TILDE SCOPE 
// "_cdecl" "__cdecl" "_near" "__near" "_far" "__far" "__interrupt" "pascal" 
// "_pascal" "__pascal" 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_14(_tokenSet_14_data_,12);
const unsigned long CPPParser::_tokenSet_15_data_[] = { 4294344594UL, 4026531852UL, 1073741823UL, 3087003455UL, 3093299200UL, 1048575UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// EOF "template" "typedef" "enum" ID LCURLY SEMICOLON "inline" "namespace" 
// RCURLY "extern" StringLiteral "__extension__" "_inline" "__inline" "__inline__" 
// "virtual" "explicit" "friend" "_stdcall" "__stdcall" "typename" "__attribute__" 
// LPAREN "const" "__const" "auto" "register" "static" "mutable" "const_cast" 
// "volatile" "__volatile" "__restrict" "__restrict__" "char" "bool" "short" 
// "int" "_int64" "__int64" "__w64" "long" "signed" "__signed" "__signed__" 
// "unsigned" "float" "double" "void" "_declspec" "__declspec" "__builtin_va_list" 
// "class" "struct" "union" "operator" "this" "true" "false" "public" "protected" 
// "private" OCTALINT STAR AMPERSAND TILDE "asm" "__asm" "__asm__" "throw" 
// "case" "default" "if" "else" "switch" "while" "do" "for" "goto" "continue" 
// "break" "return" "try" "using" "_asm" SCOPE PLUS MINUS PLUSPLUS MINUSMINUS 
// "sizeof" "dynamic_cast" "static_cast" "reinterpret_cast" NOT "new" "_cdecl" 
// "__cdecl" "_near" "__near" "_far" "__far" "__interrupt" "pascal" "_pascal" 
// "__pascal" "delete" DECIMALINT HEXADECIMALINT CharLiteral FLOATONE FLOATTWO 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_15(_tokenSet_15_data_,12);
const unsigned long CPPParser::_tokenSet_16_data_[] = { 4294054784UL, 4026531852UL, 1073741823UL, 2415919408UL, 0UL, 16368UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "typedef" "enum" ID "inline" "extern" "__extension__" "_inline" "__inline" 
// "__inline__" "virtual" "explicit" "friend" "_stdcall" "__stdcall" "typename" 
// "__attribute__" LPAREN "const" "__const" "auto" "register" "static" 
// "mutable" "const_cast" "volatile" "__volatile" "__restrict" "__restrict__" 
// "char" "bool" "short" "int" "_int64" "__int64" "__w64" "long" "signed" 
// "__signed" "__signed__" "unsigned" "float" "double" "void" "_declspec" 
// "__declspec" "__builtin_va_list" "class" "struct" "union" "operator" 
// "this" "true" "false" STAR AMPERSAND TILDE "using" SCOPE "_cdecl" "__cdecl" 
// "_near" "__near" "_far" "__far" "__interrupt" "pascal" "_pascal" "__pascal" 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_16(_tokenSet_16_data_,12);
const unsigned long CPPParser::_tokenSet_17_data_[] = { 3624520288UL, 1UL, 1006632960UL, 2147488240UL, 134217727UL, 16368UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// LESSTHAN GREATERTHAN ID SEMICOLON RCURLY ASSIGNEQUAL COLON COMMA "_stdcall" 
// "__stdcall" "__attribute__" LPAREN RPAREN "operator" "this" "true" "false" 
// STAR AMPERSAND LSQUARE RSQUARE TILDE ELLIPSIS SCOPE TIMESEQUAL DIVIDEEQUAL 
// MINUSEQUAL PLUSEQUAL MODEQUAL SHIFTLEFTEQUAL SHIFTRIGHTEQUAL BITWISEANDEQUAL 
// BITWISEXOREQUAL BITWISEOREQUAL QUESTIONMARK OR AND BITWISEOR BITWISEXOR 
// NOTEQUAL EQUAL LESSTHANOREQUALTO GREATERTHANOREQUALTO SHIFTLEFT SHIFTRIGHT 
// PLUS MINUS DIVIDE MOD DOTMBR POINTERTOMBR "_cdecl" "__cdecl" "_near" 
// "__near" "_far" "__far" "__interrupt" "pascal" "_pascal" "__pascal" 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_17(_tokenSet_17_data_,12);
const unsigned long CPPParser::_tokenSet_18_data_[] = { 2048UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// SEMICOLON 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_18(_tokenSet_18_data_,8);
const unsigned long CPPParser::_tokenSet_19_data_[] = { 3072UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// LCURLY SEMICOLON 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_19(_tokenSet_19_data_,8);
const unsigned long CPPParser::_tokenSet_20_data_[] = { 4294081426UL, 4026531852UL, 1073741823UL, 2415919415UL, 0UL, 16368UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// EOF "template" "typedef" "enum" ID SEMICOLON "inline" "namespace" RCURLY 
// "extern" "__extension__" "_inline" "__inline" "__inline__" "virtual" 
// "explicit" "friend" "_stdcall" "__stdcall" "typename" "__attribute__" 
// LPAREN "const" "__const" "auto" "register" "static" "mutable" "const_cast" 
// "volatile" "__volatile" "__restrict" "__restrict__" "char" "bool" "short" 
// "int" "_int64" "__int64" "__w64" "long" "signed" "__signed" "__signed__" 
// "unsigned" "float" "double" "void" "_declspec" "__declspec" "__builtin_va_list" 
// "class" "struct" "union" "operator" "this" "true" "false" "public" "protected" 
// "private" STAR AMPERSAND TILDE "using" SCOPE "_cdecl" "__cdecl" "_near" 
// "__near" "_far" "__far" "__interrupt" "pascal" "_pascal" "__pascal" 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_20(_tokenSet_20_data_,12);
const unsigned long CPPParser::_tokenSet_21_data_[] = { 512UL, 0UL, 0UL, 2147483648UL, 0UL, 0UL, 0UL, 0UL };
// ID SCOPE 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_21(_tokenSet_21_data_,8);
const unsigned long CPPParser::_tokenSet_22_data_[] = { 512UL, 0UL, 1006632960UL, 272UL, 0UL, 0UL, 0UL, 0UL };
// ID "operator" "this" "true" "false" STAR TILDE 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_22(_tokenSet_22_data_,8);
const unsigned long CPPParser::_tokenSet_23_data_[] = { 535957632UL, 4026531852UL, 31UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "typedef" "inline" "extern" "__extension__" "_inline" "__inline" "__inline__" 
// "virtual" "explicit" "friend" "_stdcall" "__stdcall" "const" "__const" 
// "auto" "register" "static" "mutable" "const_cast" "volatile" "__volatile" 
// "__restrict" "__restrict__" 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_23(_tokenSet_23_data_,8);
const unsigned long CPPParser::_tokenSet_24_data_[] = { 402653696UL, 0UL, 0UL, 2147483696UL, 0UL, 16368UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// ID "_stdcall" "__stdcall" STAR AMPERSAND SCOPE "_cdecl" "__cdecl" "_near" 
// "__near" "_far" "__far" "__interrupt" "pascal" "_pascal" "__pascal" 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_24(_tokenSet_24_data_,12);
const unsigned long CPPParser::_tokenSet_25_data_[] = { 3623879200UL, 12UL, 1006632991UL, 2147483696UL, 0UL, 16368UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// LESSTHAN ID "_stdcall" "__stdcall" "__attribute__" LPAREN "const" "__const" 
// "const_cast" "volatile" "__volatile" "__restrict" "__restrict__" "operator" 
// "this" "true" "false" STAR AMPERSAND SCOPE "_cdecl" "__cdecl" "_near" 
// "__near" "_far" "__far" "__interrupt" "pascal" "_pascal" "__pascal" 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_25(_tokenSet_25_data_,12);
const unsigned long CPPParser::_tokenSet_26_data_[] = { 3221225984UL, 0UL, 1006632960UL, 2147483648UL, 0UL, 0UL, 0UL, 0UL };
// ID "__attribute__" LPAREN "operator" "this" "true" "false" SCOPE 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_26(_tokenSet_26_data_,8);
const unsigned long CPPParser::_tokenSet_27_data_[] = { 2148041312UL, 0UL, 1006632960UL, 2147484016UL, 1577057279UL, 16396UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// LESSTHAN GREATERTHAN ID ASSIGNEQUAL COMMA LPAREN "operator" "this" "true" 
// "false" STAR AMPERSAND LSQUARE TILDE SCOPE TIMESEQUAL DIVIDEEQUAL MINUSEQUAL 
// PLUSEQUAL MODEQUAL SHIFTLEFTEQUAL SHIFTRIGHTEQUAL BITWISEANDEQUAL BITWISEXOREQUAL 
// BITWISEOREQUAL OR AND BITWISEOR BITWISEXOR NOTEQUAL EQUAL LESSTHANOREQUALTO 
// GREATERTHANOREQUALTO SHIFTLEFT SHIFTRIGHT PLUS MINUS DIVIDE MOD POINTERTOMBR 
// PLUSPLUS MINUSMINUS POINTERTO NOT "new" "delete" 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_27(_tokenSet_27_data_,12);
const unsigned long CPPParser::_tokenSet_28_data_[] = { 1072832384UL, 4026531852UL, 67108863UL, 2415919104UL, 0UL, 0UL, 0UL, 0UL };
// "typedef" "enum" ID LCURLY SEMICOLON "inline" "extern" "__extension__" 
// "_inline" "__inline" "__inline__" "virtual" "explicit" "friend" "_stdcall" 
// "__stdcall" "typename" "const" "__const" "auto" "register" "static" 
// "mutable" "const_cast" "volatile" "__volatile" "__restrict" "__restrict__" 
// "char" "bool" "short" "int" "_int64" "__int64" "__w64" "long" "signed" 
// "__signed" "__signed__" "unsigned" "float" "double" "void" "_declspec" 
// "__declspec" "__builtin_va_list" "class" "struct" "union" "using" SCOPE 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_28(_tokenSet_28_data_,8);
const unsigned long CPPParser::_tokenSet_29_data_[] = { 3757184928UL, 4026531852UL, 1073741823UL, 2147483952UL, 0UL, 16368UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// LESSTHAN "typedef" "enum" ID LCURLY "inline" "extern" "__extension__" 
// "_inline" "__inline" "__inline__" "virtual" "explicit" "friend" "_stdcall" 
// "__stdcall" "__attribute__" LPAREN "const" "__const" "auto" "register" 
// "static" "mutable" "const_cast" "volatile" "__volatile" "__restrict" 
// "__restrict__" "char" "bool" "short" "int" "_int64" "__int64" "__w64" 
// "long" "signed" "__signed" "__signed__" "unsigned" "float" "double" 
// "void" "_declspec" "__declspec" "__builtin_va_list" "class" "struct" 
// "union" "operator" "this" "true" "false" STAR AMPERSAND TILDE SCOPE 
// "_cdecl" "__cdecl" "_near" "__near" "_far" "__far" "__interrupt" "pascal" 
// "_pascal" "__pascal" 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_29(_tokenSet_29_data_,12);
const unsigned long CPPParser::_tokenSet_30_data_[] = { 1072830336UL, 4026531852UL, 67108863UL, 2415919104UL, 0UL, 0UL, 0UL, 0UL };
// "typedef" "enum" ID LCURLY "inline" "extern" "__extension__" "_inline" 
// "__inline" "__inline__" "virtual" "explicit" "friend" "_stdcall" "__stdcall" 
// "typename" "const" "__const" "auto" "register" "static" "mutable" "const_cast" 
// "volatile" "__volatile" "__restrict" "__restrict__" "char" "bool" "short" 
// "int" "_int64" "__int64" "__w64" "long" "signed" "__signed" "__signed__" 
// "unsigned" "float" "double" "void" "_declspec" "__declspec" "__builtin_va_list" 
// "class" "struct" "union" "using" SCOPE 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_30(_tokenSet_30_data_,8);
const unsigned long CPPParser::_tokenSet_31_data_[] = { 4294868896UL, 4026531852UL, 1073741823UL, 3086872376UL, 3093299200UL, 1048575UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// LESSTHAN "typedef" "enum" ID LCURLY SEMICOLON "inline" "namespace" RCURLY 
// "extern" StringLiteral COMMA "__extension__" "_inline" "__inline" "__inline__" 
// "virtual" "explicit" "friend" "_stdcall" "__stdcall" "typename" "__attribute__" 
// LPAREN "const" "__const" "auto" "register" "static" "mutable" "const_cast" 
// "volatile" "__volatile" "__restrict" "__restrict__" "char" "bool" "short" 
// "int" "_int64" "__int64" "__w64" "long" "signed" "__signed" "__signed__" 
// "unsigned" "float" "double" "void" "_declspec" "__declspec" "__builtin_va_list" 
// "class" "struct" "union" "operator" "this" "true" "false" OCTALINT STAR 
// AMPERSAND TILDE "asm" "__asm" "__asm__" "throw" "case" "default" "if" 
// "switch" "while" "do" "for" "goto" "continue" "break" "return" "try" 
// "using" "_asm" SCOPE PLUS MINUS PLUSPLUS MINUSMINUS "sizeof" "dynamic_cast" 
// "static_cast" "reinterpret_cast" NOT "new" "_cdecl" "__cdecl" "_near" 
// "__near" "_far" "__far" "__interrupt" "pascal" "_pascal" "__pascal" 
// "delete" DECIMALINT HEXADECIMALINT CharLiteral FLOATONE FLOATTWO 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_31(_tokenSet_31_data_,12);
const unsigned long CPPParser::_tokenSet_32_data_[] = { 3220594560UL, 4026531852UL, 1073741823UL, 3086872376UL, 3093299200UL, 1032207UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "typedef" "enum" ID LCURLY SEMICOLON "inline" RCURLY "extern" StringLiteral 
// "__extension__" "_inline" "__inline" "__inline__" "virtual" "explicit" 
// "friend" "_stdcall" "__stdcall" "typename" LPAREN "const" "__const" 
// "auto" "register" "static" "mutable" "const_cast" "volatile" "__volatile" 
// "__restrict" "__restrict__" "char" "bool" "short" "int" "_int64" "__int64" 
// "__w64" "long" "signed" "__signed" "__signed__" "unsigned" "float" "double" 
// "void" "_declspec" "__declspec" "__builtin_va_list" "class" "struct" 
// "union" "operator" "this" "true" "false" OCTALINT STAR AMPERSAND TILDE 
// "asm" "__asm" "__asm__" "throw" "case" "default" "if" "switch" "while" 
// "do" "for" "goto" "continue" "break" "return" "try" "using" "_asm" SCOPE 
// PLUS MINUS PLUSPLUS MINUSMINUS "sizeof" "dynamic_cast" "static_cast" 
// "reinterpret_cast" NOT "new" "delete" DECIMALINT HEXADECIMALINT CharLiteral 
// FLOATONE FLOATTWO 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_32(_tokenSet_32_data_,12);
const unsigned long CPPParser::_tokenSet_33_data_[] = { 3623879168UL, 0UL, 1006632960UL, 2147483696UL, 0UL, 16368UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// ID "_stdcall" "__stdcall" "__attribute__" LPAREN "operator" "this" "true" 
// "false" STAR AMPERSAND SCOPE "_cdecl" "__cdecl" "_near" "__near" "_far" 
// "__far" "__interrupt" "pascal" "_pascal" "__pascal" 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_33(_tokenSet_33_data_,12);
const unsigned long CPPParser::_tokenSet_34_data_[] = { 3624436320UL, 12UL, 1006632991UL, 2147484016UL, 1577057279UL, 32764UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// LESSTHAN GREATERTHAN ID ASSIGNEQUAL COMMA "_stdcall" "__stdcall" "__attribute__" 
// LPAREN "const" "__const" "const_cast" "volatile" "__volatile" "__restrict" 
// "__restrict__" "operator" "this" "true" "false" STAR AMPERSAND LSQUARE 
// TILDE SCOPE TIMESEQUAL DIVIDEEQUAL MINUSEQUAL PLUSEQUAL MODEQUAL SHIFTLEFTEQUAL 
// SHIFTRIGHTEQUAL BITWISEANDEQUAL BITWISEXOREQUAL BITWISEOREQUAL OR AND 
// BITWISEOR BITWISEXOR NOTEQUAL EQUAL LESSTHANOREQUALTO GREATERTHANOREQUALTO 
// SHIFTLEFT SHIFTRIGHT PLUS MINUS DIVIDE MOD POINTERTOMBR PLUSPLUS MINUSMINUS 
// POINTERTO NOT "new" "_cdecl" "__cdecl" "_near" "__near" "_far" "__far" 
// "__interrupt" "pascal" "_pascal" "__pascal" "delete" 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_34(_tokenSet_34_data_,12);
const unsigned long CPPParser::_tokenSet_35_data_[] = { 4294082448UL, 4026531852UL, 1073741823UL, 2415919408UL, 0UL, 16368UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "template" "typedef" "enum" ID LCURLY SEMICOLON "inline" "namespace" 
// RCURLY "extern" "__extension__" "_inline" "__inline" "__inline__" "virtual" 
// "explicit" "friend" "_stdcall" "__stdcall" "typename" "__attribute__" 
// LPAREN "const" "__const" "auto" "register" "static" "mutable" "const_cast" 
// "volatile" "__volatile" "__restrict" "__restrict__" "char" "bool" "short" 
// "int" "_int64" "__int64" "__w64" "long" "signed" "__signed" "__signed__" 
// "unsigned" "float" "double" "void" "_declspec" "__declspec" "__builtin_va_list" 
// "class" "struct" "union" "operator" "this" "true" "false" STAR AMPERSAND 
// TILDE "using" SCOPE "_cdecl" "__cdecl" "_near" "__near" "_far" "__far" 
// "__interrupt" "pascal" "_pascal" "__pascal" 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_35(_tokenSet_35_data_,12);
const unsigned long CPPParser::_tokenSet_36_data_[] = { 3220575104UL, 4026531852UL, 1073741823UL, 2147483960UL, 3093299200UL, 1032207UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "typedef" "enum" ID "inline" "extern" StringLiteral "__extension__" 
// "_inline" "__inline" "__inline__" "virtual" "explicit" "friend" "_stdcall" 
// "__stdcall" "typename" LPAREN "const" "__const" "auto" "register" "static" 
// "mutable" "const_cast" "volatile" "__volatile" "__restrict" "__restrict__" 
// "char" "bool" "short" "int" "_int64" "__int64" "__w64" "long" "signed" 
// "__signed" "__signed__" "unsigned" "float" "double" "void" "_declspec" 
// "__declspec" "__builtin_va_list" "class" "struct" "union" "operator" 
// "this" "true" "false" OCTALINT STAR AMPERSAND TILDE SCOPE PLUS MINUS 
// PLUSPLUS MINUSMINUS "sizeof" "dynamic_cast" "static_cast" "reinterpret_cast" 
// NOT "new" "delete" DECIMALINT HEXADECIMALINT CharLiteral FLOATONE FLOATTWO 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_36(_tokenSet_36_data_,12);
const unsigned long CPPParser::_tokenSet_37_data_[] = { 3624521312UL, 1UL, 1006632960UL, 2147491824UL, 134217727UL, 16368UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// LESSTHAN GREATERTHAN ID LCURLY SEMICOLON RCURLY ASSIGNEQUAL COLON COMMA 
// "_stdcall" "__stdcall" "__attribute__" LPAREN RPAREN "operator" "this" 
// "true" "false" STAR AMPERSAND LSQUARE RSQUARE TILDE "asm" "__asm" "__asm__" 
// ELLIPSIS SCOPE TIMESEQUAL DIVIDEEQUAL MINUSEQUAL PLUSEQUAL MODEQUAL 
// SHIFTLEFTEQUAL SHIFTRIGHTEQUAL BITWISEANDEQUAL BITWISEXOREQUAL BITWISEOREQUAL 
// QUESTIONMARK OR AND BITWISEOR BITWISEXOR NOTEQUAL EQUAL LESSTHANOREQUALTO 
// GREATERTHANOREQUALTO SHIFTLEFT SHIFTRIGHT PLUS MINUS DIVIDE MOD DOTMBR 
// POINTERTOMBR "_cdecl" "__cdecl" "_near" "__near" "_far" "__far" "__interrupt" 
// "pascal" "_pascal" "__pascal" 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_37(_tokenSet_37_data_,12);
const unsigned long CPPParser::_tokenSet_38_data_[] = { 4294967282UL, 4026531853UL, 2147483647UL, 3087007743UL, 4294967295UL, 1048575UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// EOF "template" LESSTHAN GREATERTHAN "typedef" "enum" ID LCURLY SEMICOLON 
// "inline" "namespace" RCURLY ASSIGNEQUAL COLON "extern" StringLiteral 
// COMMA "__extension__" "_inline" "__inline" "__inline__" "virtual" "explicit" 
// "friend" "_stdcall" "__stdcall" "typename" "__attribute__" LPAREN RPAREN 
// "const" "__const" "auto" "register" "static" "mutable" "const_cast" 
// "volatile" "__volatile" "__restrict" "__restrict__" "char" "bool" "short" 
// "int" "_int64" "__int64" "__w64" "long" "signed" "__signed" "__signed__" 
// "unsigned" "float" "double" "void" "_declspec" "__declspec" "__builtin_va_list" 
// "class" "struct" "union" "operator" "this" "true" "false" DOT "public" 
// "protected" "private" OCTALINT STAR AMPERSAND LSQUARE RSQUARE TILDE 
// "asm" "__asm" "__asm__" ELLIPSIS "throw" "case" "default" "if" "else" 
// "switch" "while" "do" "for" "goto" "continue" "break" "return" "try" 
// "using" "_asm" SCOPE TIMESEQUAL DIVIDEEQUAL MINUSEQUAL PLUSEQUAL MODEQUAL 
// SHIFTLEFTEQUAL SHIFTRIGHTEQUAL BITWISEANDEQUAL BITWISEXOREQUAL BITWISEOREQUAL 
// QUESTIONMARK OR AND BITWISEOR BITWISEXOR NOTEQUAL EQUAL LESSTHANOREQUALTO 
// GREATERTHANOREQUALTO SHIFTLEFT SHIFTRIGHT PLUS MINUS DIVIDE MOD DOTMBR 
// POINTERTOMBR PLUSPLUS MINUSMINUS "sizeof" POINTERTO "dynamic_cast" "static_cast" 
// "reinterpret_cast" NOT "new" "_cdecl" "__cdecl" "_near" "__near" "_far" 
// "__far" "__interrupt" "pascal" "_pascal" "__pascal" "delete" DECIMALINT 
// HEXADECIMALINT CharLiteral FLOATONE FLOATTWO 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_38(_tokenSet_38_data_,12);
const unsigned long CPPParser::_tokenSet_39_data_[] = { 23073280UL, 0UL, 0UL, 2147483904UL, 0UL, 0UL, 0UL, 0UL };
// ID "inline" "_inline" "__inline" "virtual" TILDE SCOPE 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_39(_tokenSet_39_data_,8);
const unsigned long CPPParser::_tokenSet_40_data_[] = { 1072829312UL, 4026531852UL, 134217727UL, 2147483648UL, 0UL, 0UL, 0UL, 0UL };
// "typedef" "enum" ID "inline" "extern" "__extension__" "_inline" "__inline" 
// "__inline__" "virtual" "explicit" "friend" "_stdcall" "__stdcall" "typename" 
// "const" "__const" "auto" "register" "static" "mutable" "const_cast" 
// "volatile" "__volatile" "__restrict" "__restrict__" "char" "bool" "short" 
// "int" "_int64" "__int64" "__w64" "long" "signed" "__signed" "__signed__" 
// "unsigned" "float" "double" "void" "_declspec" "__declspec" "__builtin_va_list" 
// "class" "struct" "union" "operator" SCOPE 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_40(_tokenSet_40_data_,8);
const unsigned long CPPParser::_tokenSet_41_data_[] = { 512UL, 0UL, 1006632960UL, 2147483648UL, 0UL, 0UL, 0UL, 0UL };
// ID "operator" "this" "true" "false" SCOPE 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_41(_tokenSet_41_data_,8);
const unsigned long CPPParser::_tokenSet_42_data_[] = { 2148043360UL, 0UL, 1006632960UL, 2147484016UL, 1577057279UL, 16396UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// LESSTHAN GREATERTHAN ID SEMICOLON ASSIGNEQUAL COMMA LPAREN "operator" 
// "this" "true" "false" STAR AMPERSAND LSQUARE TILDE SCOPE TIMESEQUAL 
// DIVIDEEQUAL MINUSEQUAL PLUSEQUAL MODEQUAL SHIFTLEFTEQUAL SHIFTRIGHTEQUAL 
// BITWISEANDEQUAL BITWISEXOREQUAL BITWISEOREQUAL OR AND BITWISEOR BITWISEXOR 
// NOTEQUAL EQUAL LESSTHANOREQUALTO GREATERTHANOREQUALTO SHIFTLEFT SHIFTRIGHT 
// PLUS MINUS DIVIDE MOD POINTERTOMBR PLUSPLUS MINUSMINUS POINTERTO NOT 
// "new" "delete" 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_42(_tokenSet_42_data_,12);
const unsigned long CPPParser::_tokenSet_43_data_[] = { 2683510688UL, 4026531852UL, 1073741823UL, 2147483952UL, 0UL, 16368UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// LESSTHAN "typedef" "enum" ID LCURLY SEMICOLON "inline" COLON "extern" 
// "__extension__" "_inline" "__inline" "__inline__" "virtual" "explicit" 
// "friend" "_stdcall" "__stdcall" LPAREN "const" "__const" "auto" "register" 
// "static" "mutable" "const_cast" "volatile" "__volatile" "__restrict" 
// "__restrict__" "char" "bool" "short" "int" "_int64" "__int64" "__w64" 
// "long" "signed" "__signed" "__signed__" "unsigned" "float" "double" 
// "void" "_declspec" "__declspec" "__builtin_va_list" "class" "struct" 
// "union" "operator" "this" "true" "false" STAR AMPERSAND TILDE SCOPE 
// "_cdecl" "__cdecl" "_near" "__near" "_far" "__far" "__interrupt" "pascal" 
// "_pascal" "__pascal" 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_43(_tokenSet_43_data_,12);
const unsigned long CPPParser::_tokenSet_44_data_[] = { 4294073232UL, 4026531852UL, 1073741823UL, 2415919415UL, 0UL, 16368UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "template" "typedef" "enum" ID SEMICOLON "inline" RCURLY "extern" "__extension__" 
// "_inline" "__inline" "__inline__" "virtual" "explicit" "friend" "_stdcall" 
// "__stdcall" "typename" "__attribute__" LPAREN "const" "__const" "auto" 
// "register" "static" "mutable" "const_cast" "volatile" "__volatile" "__restrict" 
// "__restrict__" "char" "bool" "short" "int" "_int64" "__int64" "__w64" 
// "long" "signed" "__signed" "__signed__" "unsigned" "float" "double" 
// "void" "_declspec" "__declspec" "__builtin_va_list" "class" "struct" 
// "union" "operator" "this" "true" "false" "public" "protected" "private" 
// STAR AMPERSAND TILDE "using" SCOPE "_cdecl" "__cdecl" "_near" "__near" 
// "_far" "__far" "__interrupt" "pascal" "_pascal" "__pascal" 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_44(_tokenSet_44_data_,12);
const unsigned long CPPParser::_tokenSet_45_data_[] = { 68608UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// LCURLY SEMICOLON COLON 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_45(_tokenSet_45_data_,8);
const unsigned long CPPParser::_tokenSet_46_data_[] = { 4294344594UL, 4026531852UL, 1073741823UL, 3221221183UL, 3093299200UL, 1048575UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// EOF "template" "typedef" "enum" ID LCURLY SEMICOLON "inline" "namespace" 
// RCURLY "extern" StringLiteral "__extension__" "_inline" "__inline" "__inline__" 
// "virtual" "explicit" "friend" "_stdcall" "__stdcall" "typename" "__attribute__" 
// LPAREN "const" "__const" "auto" "register" "static" "mutable" "const_cast" 
// "volatile" "__volatile" "__restrict" "__restrict__" "char" "bool" "short" 
// "int" "_int64" "__int64" "__w64" "long" "signed" "__signed" "__signed__" 
// "unsigned" "float" "double" "void" "_declspec" "__declspec" "__builtin_va_list" 
// "class" "struct" "union" "operator" "this" "true" "false" "public" "protected" 
// "private" OCTALINT STAR AMPERSAND TILDE "asm" "__asm" "__asm__" "throw" 
// "case" "default" "if" "else" "switch" "while" "do" "for" "goto" "continue" 
// "break" "return" "try" "catch" "using" "_asm" SCOPE PLUS MINUS PLUSPLUS 
// MINUSMINUS "sizeof" "dynamic_cast" "static_cast" "reinterpret_cast" 
// NOT "new" "_cdecl" "__cdecl" "_near" "__near" "_far" "__far" "__interrupt" 
// "pascal" "_pascal" "__pascal" "delete" DECIMALINT HEXADECIMALINT CharLiteral 
// FLOATONE FLOATTWO 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_46(_tokenSet_46_data_,12);
const unsigned long CPPParser::_tokenSet_47_data_[] = { 16843264UL, 0UL, 0UL, 2147483648UL, 0UL, 0UL, 0UL, 0UL };
// ID COLON "virtual" SCOPE 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_47(_tokenSet_47_data_,8);
const unsigned long CPPParser::_tokenSet_48_data_[] = { 3623881216UL, 0UL, 1006632960UL, 2147483952UL, 0UL, 16368UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// ID SEMICOLON "_stdcall" "__stdcall" "__attribute__" LPAREN "operator" 
// "this" "true" "false" STAR AMPERSAND TILDE SCOPE "_cdecl" "__cdecl" 
// "_near" "__near" "_far" "__far" "__interrupt" "pascal" "_pascal" "__pascal" 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_48(_tokenSet_48_data_,12);
const unsigned long CPPParser::_tokenSet_49_data_[] = { 535958400UL, 4026531852UL, 67108863UL, 2147483648UL, 0UL, 0UL, 0UL, 0UL };
// "typedef" "enum" ID "inline" "extern" "__extension__" "_inline" "__inline" 
// "__inline__" "virtual" "explicit" "friend" "_stdcall" "__stdcall" "const" 
// "__const" "auto" "register" "static" "mutable" "const_cast" "volatile" 
// "__volatile" "__restrict" "__restrict__" "char" "bool" "short" "int" 
// "_int64" "__int64" "__w64" "long" "signed" "__signed" "__signed__" "unsigned" 
// "float" "double" "void" "_declspec" "__declspec" "__builtin_va_list" 
// "class" "struct" "union" SCOPE 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_49(_tokenSet_49_data_,8);
const unsigned long CPPParser::_tokenSet_50_data_[] = { 4294696928UL, 4026531853UL, 1073741823UL, 2415935472UL, 134217727UL, 16368UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// LESSTHAN GREATERTHAN "typedef" "enum" ID LCURLY SEMICOLON "inline" RCURLY 
// ASSIGNEQUAL COLON "extern" COMMA "__extension__" "_inline" "__inline" 
// "__inline__" "virtual" "explicit" "friend" "_stdcall" "__stdcall" "typename" 
// "__attribute__" LPAREN RPAREN "const" "__const" "auto" "register" "static" 
// "mutable" "const_cast" "volatile" "__volatile" "__restrict" "__restrict__" 
// "char" "bool" "short" "int" "_int64" "__int64" "__w64" "long" "signed" 
// "__signed" "__signed__" "unsigned" "float" "double" "void" "_declspec" 
// "__declspec" "__builtin_va_list" "class" "struct" "union" "operator" 
// "this" "true" "false" STAR AMPERSAND LSQUARE RSQUARE TILDE "asm" "__asm" 
// "__asm__" ELLIPSIS "throw" "using" SCOPE TIMESEQUAL DIVIDEEQUAL MINUSEQUAL 
// PLUSEQUAL MODEQUAL SHIFTLEFTEQUAL SHIFTRIGHTEQUAL BITWISEANDEQUAL BITWISEXOREQUAL 
// BITWISEOREQUAL QUESTIONMARK OR AND BITWISEOR BITWISEXOR NOTEQUAL EQUAL 
// LESSTHANOREQUALTO GREATERTHANOREQUALTO SHIFTLEFT SHIFTRIGHT PLUS MINUS 
// DIVIDE MOD DOTMBR POINTERTOMBR "_cdecl" "__cdecl" "_near" "__near" "_far" 
// "__far" "__interrupt" "pascal" "_pascal" "__pascal" 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_50(_tokenSet_50_data_,12);
const unsigned long CPPParser::_tokenSet_51_data_[] = { 0UL, 12UL, 31UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "const" "__const" "const_cast" "volatile" "__volatile" "__restrict" 
// "__restrict__" 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_51(_tokenSet_51_data_,8);
const unsigned long CPPParser::_tokenSet_52_data_[] = { 2147746304UL, 0UL, 1015021537UL, 2147484088UL, 3093299200UL, 1032207UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// ID StringLiteral LPAREN "const_cast" "char" "bool" "short" "int" "_int64" 
// "__int64" "__w64" "long" "signed" "__signed" "__signed__" "unsigned" 
// "float" "double" "void" "_declspec" "__declspec" "__builtin_va_list" 
// "operator" "this" "true" "false" OCTALINT STAR AMPERSAND RSQUARE TILDE 
// SCOPE PLUS MINUS PLUSPLUS MINUSMINUS "sizeof" "dynamic_cast" "static_cast" 
// "reinterpret_cast" NOT "new" "delete" DECIMALINT HEXADECIMALINT CharLiteral 
// FLOATONE FLOATTWO 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_52(_tokenSet_52_data_,12);
const unsigned long CPPParser::_tokenSet_53_data_[] = { 3624520288UL, 1UL, 1006632960UL, 2147491824UL, 1610612735UL, 32764UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// LESSTHAN GREATERTHAN ID SEMICOLON RCURLY ASSIGNEQUAL COLON COMMA "_stdcall" 
// "__stdcall" "__attribute__" LPAREN RPAREN "operator" "this" "true" "false" 
// STAR AMPERSAND LSQUARE RSQUARE TILDE "asm" "__asm" "__asm__" ELLIPSIS 
// SCOPE TIMESEQUAL DIVIDEEQUAL MINUSEQUAL PLUSEQUAL MODEQUAL SHIFTLEFTEQUAL 
// SHIFTRIGHTEQUAL BITWISEANDEQUAL BITWISEXOREQUAL BITWISEOREQUAL QUESTIONMARK 
// OR AND BITWISEOR BITWISEXOR NOTEQUAL EQUAL LESSTHANOREQUALTO GREATERTHANOREQUALTO 
// SHIFTLEFT SHIFTRIGHT PLUS MINUS DIVIDE MOD DOTMBR POINTERTOMBR PLUSPLUS 
// MINUSMINUS POINTERTO NOT "new" "_cdecl" "__cdecl" "_near" "__near" "_far" 
// "__far" "__interrupt" "pascal" "_pascal" "__pascal" "delete" 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_53(_tokenSet_53_data_,12);
const unsigned long CPPParser::_tokenSet_54_data_[] = { 3624520288UL, 1UL, 1006632960UL, 2147491824UL, 134217727UL, 16368UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// LESSTHAN GREATERTHAN ID SEMICOLON RCURLY ASSIGNEQUAL COLON COMMA "_stdcall" 
// "__stdcall" "__attribute__" LPAREN RPAREN "operator" "this" "true" "false" 
// STAR AMPERSAND LSQUARE RSQUARE TILDE "asm" "__asm" "__asm__" ELLIPSIS 
// SCOPE TIMESEQUAL DIVIDEEQUAL MINUSEQUAL PLUSEQUAL MODEQUAL SHIFTLEFTEQUAL 
// SHIFTRIGHTEQUAL BITWISEANDEQUAL BITWISEXOREQUAL BITWISEOREQUAL QUESTIONMARK 
// OR AND BITWISEOR BITWISEXOR NOTEQUAL EQUAL LESSTHANOREQUALTO GREATERTHANOREQUALTO 
// SHIFTLEFT SHIFTRIGHT PLUS MINUS DIVIDE MOD DOTMBR POINTERTOMBR "_cdecl" 
// "__cdecl" "_near" "__near" "_far" "__far" "__interrupt" "pascal" "_pascal" 
// "__pascal" 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_54(_tokenSet_54_data_,12);
const unsigned long CPPParser::_tokenSet_55_data_[] = { 0UL, 1UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// RPAREN 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_55(_tokenSet_55_data_,8);
const unsigned long CPPParser::_tokenSet_56_data_[] = { 3221866592UL, 1UL, 1073741824UL, 4336UL, 1610612735UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// LESSTHAN GREATERTHAN SEMICOLON RCURLY ASSIGNEQUAL COLON COMMA "__attribute__" 
// LPAREN RPAREN DOT STAR AMPERSAND LSQUARE RSQUARE ELLIPSIS TIMESEQUAL 
// DIVIDEEQUAL MINUSEQUAL PLUSEQUAL MODEQUAL SHIFTLEFTEQUAL SHIFTRIGHTEQUAL 
// BITWISEANDEQUAL BITWISEXOREQUAL BITWISEOREQUAL QUESTIONMARK OR AND BITWISEOR 
// BITWISEXOR NOTEQUAL EQUAL LESSTHANOREQUALTO GREATERTHANOREQUALTO SHIFTLEFT 
// SHIFTRIGHT PLUS MINUS DIVIDE MOD DOTMBR POINTERTOMBR PLUSPLUS MINUSMINUS 
// POINTERTO 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_56(_tokenSet_56_data_,12);
const unsigned long CPPParser::_tokenSet_57_data_[] = { 4294056848UL, 4026531852UL, 1073741823UL, 2415919415UL, 0UL, 16368UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "template" "typedef" "enum" ID SEMICOLON "inline" "extern" "__extension__" 
// "_inline" "__inline" "__inline__" "virtual" "explicit" "friend" "_stdcall" 
// "__stdcall" "typename" "__attribute__" LPAREN "const" "__const" "auto" 
// "register" "static" "mutable" "const_cast" "volatile" "__volatile" "__restrict" 
// "__restrict__" "char" "bool" "short" "int" "_int64" "__int64" "__w64" 
// "long" "signed" "__signed" "__signed__" "unsigned" "float" "double" 
// "void" "_declspec" "__declspec" "__builtin_va_list" "class" "struct" 
// "union" "operator" "this" "true" "false" "public" "protected" "private" 
// STAR AMPERSAND TILDE "using" SCOPE "_cdecl" "__cdecl" "_near" "__near" 
// "_far" "__far" "__interrupt" "pascal" "_pascal" "__pascal" 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_57(_tokenSet_57_data_,12);
const unsigned long CPPParser::_tokenSet_58_data_[] = { 3624521312UL, 1UL, 1006632960UL, 2147488240UL, 134217727UL, 16368UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// LESSTHAN GREATERTHAN ID LCURLY SEMICOLON RCURLY ASSIGNEQUAL COLON COMMA 
// "_stdcall" "__stdcall" "__attribute__" LPAREN RPAREN "operator" "this" 
// "true" "false" STAR AMPERSAND LSQUARE RSQUARE TILDE ELLIPSIS SCOPE TIMESEQUAL 
// DIVIDEEQUAL MINUSEQUAL PLUSEQUAL MODEQUAL SHIFTLEFTEQUAL SHIFTRIGHTEQUAL 
// BITWISEANDEQUAL BITWISEXOREQUAL BITWISEOREQUAL QUESTIONMARK OR AND BITWISEOR 
// BITWISEXOR NOTEQUAL EQUAL LESSTHANOREQUALTO GREATERTHANOREQUALTO SHIFTLEFT 
// SHIFTRIGHT PLUS MINUS DIVIDE MOD DOTMBR POINTERTOMBR "_cdecl" "__cdecl" 
// "_near" "__near" "_far" "__far" "__interrupt" "pascal" "_pascal" "__pascal" 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_58(_tokenSet_58_data_,12);
const unsigned long CPPParser::_tokenSet_59_data_[] = { 64UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// GREATERTHAN 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_59(_tokenSet_59_data_,8);
const unsigned long CPPParser::_tokenSet_60_data_[] = { 1024UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// LCURLY 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_60(_tokenSet_60_data_,8);
const unsigned long CPPParser::_tokenSet_61_data_[] = { 16384UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// RCURLY 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_61(_tokenSet_61_data_,8);
const unsigned long CPPParser::_tokenSet_62_data_[] = { 540672UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// RCURLY COMMA 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_62(_tokenSet_62_data_,8);
const unsigned long CPPParser::_tokenSet_63_data_[] = { 641024UL, 0UL, 0UL, 128UL, 0UL, 0UL, 0UL, 0UL };
// SEMICOLON RCURLY ASSIGNEQUAL COLON COMMA RSQUARE 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_63(_tokenSet_63_data_,8);
const unsigned long CPPParser::_tokenSet_64_data_[] = { 3624521312UL, 1UL, 2080374784UL, 2147491824UL, 1610612735UL, 16368UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// LESSTHAN GREATERTHAN ID LCURLY SEMICOLON RCURLY ASSIGNEQUAL COLON COMMA 
// "_stdcall" "__stdcall" "__attribute__" LPAREN RPAREN "operator" "this" 
// "true" "false" DOT STAR AMPERSAND LSQUARE RSQUARE TILDE "asm" "__asm" 
// "__asm__" ELLIPSIS SCOPE TIMESEQUAL DIVIDEEQUAL MINUSEQUAL PLUSEQUAL 
// MODEQUAL SHIFTLEFTEQUAL SHIFTRIGHTEQUAL BITWISEANDEQUAL BITWISEXOREQUAL 
// BITWISEOREQUAL QUESTIONMARK OR AND BITWISEOR BITWISEXOR NOTEQUAL EQUAL 
// LESSTHANOREQUALTO GREATERTHANOREQUALTO SHIFTLEFT SHIFTRIGHT PLUS MINUS 
// DIVIDE MOD DOTMBR POINTERTOMBR PLUSPLUS MINUSMINUS POINTERTO "_cdecl" 
// "__cdecl" "_near" "__near" "_far" "__far" "__interrupt" "pascal" "_pascal" 
// "__pascal" 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_64(_tokenSet_64_data_,12);
const unsigned long CPPParser::_tokenSet_65_data_[] = { 1074268160UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// SEMICOLON COMMA "__attribute__" 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_65(_tokenSet_65_data_,8);
const unsigned long CPPParser::_tokenSet_66_data_[] = { 2550137376UL, 12UL, 1006632991UL, 2147483952UL, 0UL, 16368UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// LESSTHAN ID "_stdcall" "__stdcall" LPAREN "const" "__const" "const_cast" 
// "volatile" "__volatile" "__restrict" "__restrict__" "operator" "this" 
// "true" "false" STAR AMPERSAND TILDE SCOPE "_cdecl" "__cdecl" "_near" 
// "__near" "_far" "__far" "__interrupt" "pascal" "_pascal" "__pascal" 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_66(_tokenSet_66_data_,12);
const unsigned long CPPParser::_tokenSet_67_data_[] = { 2147484160UL, 0UL, 1006632960UL, 2147483904UL, 0UL, 0UL, 0UL, 0UL };
// ID LPAREN "operator" "this" "true" "false" TILDE SCOPE 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_67(_tokenSet_67_data_,8);
const unsigned long CPPParser::_tokenSet_68_data_[] = { 3624438368UL, 1UL, 1006632960UL, 2147491696UL, 1577057279UL, 32764UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// LESSTHAN GREATERTHAN ID SEMICOLON ASSIGNEQUAL COMMA "_stdcall" "__stdcall" 
// "__attribute__" LPAREN RPAREN "operator" "this" "true" "false" STAR 
// AMPERSAND LSQUARE TILDE "asm" "__asm" "__asm__" ELLIPSIS SCOPE TIMESEQUAL 
// DIVIDEEQUAL MINUSEQUAL PLUSEQUAL MODEQUAL SHIFTLEFTEQUAL SHIFTRIGHTEQUAL 
// BITWISEANDEQUAL BITWISEXOREQUAL BITWISEOREQUAL OR AND BITWISEOR BITWISEXOR 
// NOTEQUAL EQUAL LESSTHANOREQUALTO GREATERTHANOREQUALTO SHIFTLEFT SHIFTRIGHT 
// PLUS MINUS DIVIDE MOD POINTERTOMBR PLUSPLUS MINUSMINUS POINTERTO NOT 
// "new" "_cdecl" "__cdecl" "_near" "__near" "_far" "__far" "__interrupt" 
// "pascal" "_pascal" "__pascal" "delete" 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_68(_tokenSet_68_data_,12);
const unsigned long CPPParser::_tokenSet_69_data_[] = { 3221784640UL, 1UL, 0UL, 7680UL, 0UL, 0UL, 0UL, 0UL };
// GREATERTHAN SEMICOLON ASSIGNEQUAL COMMA "__attribute__" LPAREN RPAREN 
// "asm" "__asm" "__asm__" ELLIPSIS 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_69(_tokenSet_69_data_,8);
const unsigned long CPPParser::_tokenSet_70_data_[] = { 4294614912UL, 4026531852UL, 67108863UL, 2415927296UL, 0UL, 0UL, 0UL, 0UL };
// "typedef" "enum" ID LCURLY SEMICOLON "inline" ASSIGNEQUAL "extern" COMMA 
// "__extension__" "_inline" "__inline" "__inline__" "virtual" "explicit" 
// "friend" "_stdcall" "__stdcall" "typename" "__attribute__" LPAREN "const" 
// "__const" "auto" "register" "static" "mutable" "const_cast" "volatile" 
// "__volatile" "__restrict" "__restrict__" "char" "bool" "short" "int" 
// "_int64" "__int64" "__w64" "long" "signed" "__signed" "__signed__" "unsigned" 
// "float" "double" "void" "_declspec" "__declspec" "__builtin_va_list" 
// "class" "struct" "union" "throw" "using" SCOPE 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_70(_tokenSet_70_data_,8);
const unsigned long CPPParser::_tokenSet_71_data_[] = { 2147746304UL, 0UL, 1015021537UL, 2147483960UL, 3093299200UL, 1032207UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// ID StringLiteral LPAREN "const_cast" "char" "bool" "short" "int" "_int64" 
// "__int64" "__w64" "long" "signed" "__signed" "__signed__" "unsigned" 
// "float" "double" "void" "_declspec" "__declspec" "__builtin_va_list" 
// "operator" "this" "true" "false" OCTALINT STAR AMPERSAND TILDE SCOPE 
// PLUS MINUS PLUSPLUS MINUSMINUS "sizeof" "dynamic_cast" "static_cast" 
// "reinterpret_cast" NOT "new" "delete" DECIMALINT HEXADECIMALINT CharLiteral 
// FLOATONE FLOATTWO 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_71(_tokenSet_71_data_,12);
const unsigned long CPPParser::_tokenSet_72_data_[] = { 4294892512UL, 4026531852UL, 2147483647UL, 2147484024UL, 4294967295UL, 1032207UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// LESSTHAN GREATERTHAN "typedef" "enum" ID SEMICOLON "inline" RCURLY ASSIGNEQUAL 
// "extern" StringLiteral COMMA "__extension__" "_inline" "__inline" "__inline__" 
// "virtual" "explicit" "friend" "_stdcall" "__stdcall" "typename" "__attribute__" 
// LPAREN "const" "__const" "auto" "register" "static" "mutable" "const_cast" 
// "volatile" "__volatile" "__restrict" "__restrict__" "char" "bool" "short" 
// "int" "_int64" "__int64" "__w64" "long" "signed" "__signed" "__signed__" 
// "unsigned" "float" "double" "void" "_declspec" "__declspec" "__builtin_va_list" 
// "class" "struct" "union" "operator" "this" "true" "false" DOT OCTALINT 
// STAR AMPERSAND LSQUARE TILDE SCOPE TIMESEQUAL DIVIDEEQUAL MINUSEQUAL 
// PLUSEQUAL MODEQUAL SHIFTLEFTEQUAL SHIFTRIGHTEQUAL BITWISEANDEQUAL BITWISEXOREQUAL 
// BITWISEOREQUAL QUESTIONMARK OR AND BITWISEOR BITWISEXOR NOTEQUAL EQUAL 
// LESSTHANOREQUALTO GREATERTHANOREQUALTO SHIFTLEFT SHIFTRIGHT PLUS MINUS 
// DIVIDE MOD DOTMBR POINTERTOMBR PLUSPLUS MINUSMINUS "sizeof" POINTERTO 
// "dynamic_cast" "static_cast" "reinterpret_cast" NOT "new" "delete" DECIMALINT 
// HEXADECIMALINT CharLiteral FLOATONE FLOATTWO 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_72(_tokenSet_72_data_,12);
const unsigned long CPPParser::_tokenSet_73_data_[] = { 1074284544UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// SEMICOLON RCURLY COMMA "__attribute__" 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_73(_tokenSet_73_data_,8);
const unsigned long CPPParser::_tokenSet_74_data_[] = { 4294958048UL, 4026531853UL, 2147483647UL, 2147488248UL, 4294967295UL, 1032207UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// LESSTHAN GREATERTHAN "typedef" "enum" ID SEMICOLON "inline" RCURLY ASSIGNEQUAL 
// COLON "extern" StringLiteral COMMA "__extension__" "_inline" "__inline" 
// "__inline__" "virtual" "explicit" "friend" "_stdcall" "__stdcall" "typename" 
// "__attribute__" LPAREN RPAREN "const" "__const" "auto" "register" "static" 
// "mutable" "const_cast" "volatile" "__volatile" "__restrict" "__restrict__" 
// "char" "bool" "short" "int" "_int64" "__int64" "__w64" "long" "signed" 
// "__signed" "__signed__" "unsigned" "float" "double" "void" "_declspec" 
// "__declspec" "__builtin_va_list" "class" "struct" "union" "operator" 
// "this" "true" "false" DOT OCTALINT STAR AMPERSAND LSQUARE RSQUARE TILDE 
// ELLIPSIS SCOPE TIMESEQUAL DIVIDEEQUAL MINUSEQUAL PLUSEQUAL MODEQUAL 
// SHIFTLEFTEQUAL SHIFTRIGHTEQUAL BITWISEANDEQUAL BITWISEXOREQUAL BITWISEOREQUAL 
// QUESTIONMARK OR AND BITWISEOR BITWISEXOR NOTEQUAL EQUAL LESSTHANOREQUALTO 
// GREATERTHANOREQUALTO SHIFTLEFT SHIFTRIGHT PLUS MINUS DIVIDE MOD DOTMBR 
// POINTERTOMBR PLUSPLUS MINUSMINUS "sizeof" POINTERTO "dynamic_cast" "static_cast" 
// "reinterpret_cast" NOT "new" "delete" DECIMALINT HEXADECIMALINT CharLiteral 
// FLOATONE FLOATTWO 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_74(_tokenSet_74_data_,12);
const unsigned long CPPParser::_tokenSet_75_data_[] = { 1074350144UL, 1UL, 0UL, 4224UL, 0UL, 0UL, 0UL, 0UL };
// GREATERTHAN SEMICOLON RCURLY COLON COMMA "__attribute__" RPAREN RSQUARE 
// ELLIPSIS 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_75(_tokenSet_75_data_,8);
const unsigned long CPPParser::_tokenSet_76_data_[] = { 2147747328UL, 0UL, 2088763361UL, 2147483960UL, 3093299200UL, 1032207UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// ID LCURLY StringLiteral LPAREN "const_cast" "char" "bool" "short" "int" 
// "_int64" "__int64" "__w64" "long" "signed" "__signed" "__signed__" "unsigned" 
// "float" "double" "void" "_declspec" "__declspec" "__builtin_va_list" 
// "operator" "this" "true" "false" DOT OCTALINT STAR AMPERSAND TILDE SCOPE 
// PLUS MINUS PLUSPLUS MINUSMINUS "sizeof" "dynamic_cast" "static_cast" 
// "reinterpret_cast" NOT "new" "delete" DECIMALINT HEXADECIMALINT CharLiteral 
// FLOATONE FLOATTWO 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_76(_tokenSet_76_data_,12);
const unsigned long CPPParser::_tokenSet_77_data_[] = { 525312UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// LCURLY COMMA 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_77(_tokenSet_77_data_,8);
const unsigned long CPPParser::_tokenSet_78_data_[] = { 2147811840UL, 0UL, 1015021537UL, 2147483960UL, 3093299200UL, 1032207UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// ID COLON StringLiteral LPAREN "const_cast" "char" "bool" "short" "int" 
// "_int64" "__int64" "__w64" "long" "signed" "__signed" "__signed__" "unsigned" 
// "float" "double" "void" "_declspec" "__declspec" "__builtin_va_list" 
// "operator" "this" "true" "false" OCTALINT STAR AMPERSAND TILDE SCOPE 
// PLUS MINUS PLUSPLUS MINUSMINUS "sizeof" "dynamic_cast" "static_cast" 
// "reinterpret_cast" NOT "new" "delete" DECIMALINT HEXADECIMALINT CharLiteral 
// FLOATONE FLOATTWO 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_78(_tokenSet_78_data_,12);
const unsigned long CPPParser::_tokenSet_79_data_[] = { 2550137344UL, 0UL, 1006632960UL, 2147483952UL, 0UL, 16368UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// ID "_stdcall" "__stdcall" LPAREN "operator" "this" "true" "false" STAR 
// AMPERSAND TILDE SCOPE "_cdecl" "__cdecl" "_near" "__near" "_far" "__far" 
// "__interrupt" "pascal" "_pascal" "__pascal" 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_79(_tokenSet_79_data_,12);
const unsigned long CPPParser::_tokenSet_80_data_[] = { 2550696544UL, 12UL, 1006632991UL, 2147484016UL, 1577057279UL, 32764UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// LESSTHAN GREATERTHAN ID SEMICOLON ASSIGNEQUAL COMMA "_stdcall" "__stdcall" 
// LPAREN "const" "__const" "const_cast" "volatile" "__volatile" "__restrict" 
// "__restrict__" "operator" "this" "true" "false" STAR AMPERSAND LSQUARE 
// TILDE SCOPE TIMESEQUAL DIVIDEEQUAL MINUSEQUAL PLUSEQUAL MODEQUAL SHIFTLEFTEQUAL 
// SHIFTRIGHTEQUAL BITWISEANDEQUAL BITWISEXOREQUAL BITWISEOREQUAL OR AND 
// BITWISEOR BITWISEXOR NOTEQUAL EQUAL LESSTHANOREQUALTO GREATERTHANOREQUALTO 
// SHIFTLEFT SHIFTRIGHT PLUS MINUS DIVIDE MOD POINTERTOMBR PLUSPLUS MINUSMINUS 
// POINTERTO NOT "new" "_cdecl" "__cdecl" "_near" "__near" "_far" "__far" 
// "__interrupt" "pascal" "_pascal" "__pascal" "delete" 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_80(_tokenSet_80_data_,12);
const unsigned long CPPParser::_tokenSet_81_data_[] = { 526336UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// SEMICOLON COMMA 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_81(_tokenSet_81_data_,8);
const unsigned long CPPParser::_tokenSet_82_data_[] = { 4294696928UL, 4026531853UL, 1073741823UL, 2415927280UL, 134217727UL, 16368UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// LESSTHAN GREATERTHAN "typedef" "enum" ID LCURLY SEMICOLON "inline" RCURLY 
// ASSIGNEQUAL COLON "extern" COMMA "__extension__" "_inline" "__inline" 
// "__inline__" "virtual" "explicit" "friend" "_stdcall" "__stdcall" "typename" 
// "__attribute__" LPAREN RPAREN "const" "__const" "auto" "register" "static" 
// "mutable" "const_cast" "volatile" "__volatile" "__restrict" "__restrict__" 
// "char" "bool" "short" "int" "_int64" "__int64" "__w64" "long" "signed" 
// "__signed" "__signed__" "unsigned" "float" "double" "void" "_declspec" 
// "__declspec" "__builtin_va_list" "class" "struct" "union" "operator" 
// "this" "true" "false" STAR AMPERSAND LSQUARE RSQUARE TILDE "asm" "__asm" 
// "__asm__" ELLIPSIS "using" SCOPE TIMESEQUAL DIVIDEEQUAL MINUSEQUAL PLUSEQUAL 
// MODEQUAL SHIFTLEFTEQUAL SHIFTRIGHTEQUAL BITWISEANDEQUAL BITWISEXOREQUAL 
// BITWISEOREQUAL QUESTIONMARK OR AND BITWISEOR BITWISEXOR NOTEQUAL EQUAL 
// LESSTHANOREQUALTO GREATERTHANOREQUALTO SHIFTLEFT SHIFTRIGHT PLUS MINUS 
// DIVIDE MOD DOTMBR POINTERTOMBR "_cdecl" "__cdecl" "_near" "__near" "_far" 
// "__far" "__interrupt" "pascal" "_pascal" "__pascal" 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_82(_tokenSet_82_data_,12);
const unsigned long CPPParser::_tokenSet_83_data_[] = { 3624520288UL, 1UL, 1006632960UL, 2147496432UL, 134217727UL, 16368UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// LESSTHAN GREATERTHAN ID SEMICOLON RCURLY ASSIGNEQUAL COLON COMMA "_stdcall" 
// "__stdcall" "__attribute__" LPAREN RPAREN "operator" "this" "true" "false" 
// STAR AMPERSAND LSQUARE RSQUARE TILDE ELLIPSIS "throw" SCOPE TIMESEQUAL 
// DIVIDEEQUAL MINUSEQUAL PLUSEQUAL MODEQUAL SHIFTLEFTEQUAL SHIFTRIGHTEQUAL 
// BITWISEANDEQUAL BITWISEXOREQUAL BITWISEOREQUAL QUESTIONMARK OR AND BITWISEOR 
// BITWISEXOR NOTEQUAL EQUAL LESSTHANOREQUALTO GREATERTHANOREQUALTO SHIFTLEFT 
// SHIFTRIGHT PLUS MINUS DIVIDE MOD DOTMBR POINTERTOMBR "_cdecl" "__cdecl" 
// "_near" "__near" "_far" "__far" "__interrupt" "pascal" "_pascal" "__pascal" 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_83(_tokenSet_83_data_,12);
const unsigned long CPPParser::_tokenSet_84_data_[] = { 1072865152UL, 4026531852UL, 67108863UL, 2415927296UL, 0UL, 0UL, 0UL, 0UL };
// "typedef" "enum" ID LCURLY SEMICOLON "inline" ASSIGNEQUAL "extern" "__extension__" 
// "_inline" "__inline" "__inline__" "virtual" "explicit" "friend" "_stdcall" 
// "__stdcall" "typename" "const" "__const" "auto" "register" "static" 
// "mutable" "const_cast" "volatile" "__volatile" "__restrict" "__restrict__" 
// "char" "bool" "short" "int" "_int64" "__int64" "__w64" "long" "signed" 
// "__signed" "__signed__" "unsigned" "float" "double" "void" "_declspec" 
// "__declspec" "__builtin_va_list" "class" "struct" "union" "throw" "using" 
// SCOPE 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_84(_tokenSet_84_data_,8);
const unsigned long CPPParser::_tokenSet_85_data_[] = { 66560UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// LCURLY COLON 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_85(_tokenSet_85_data_,8);
const unsigned long CPPParser::_tokenSet_86_data_[] = { 2147483648UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// LPAREN 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_86(_tokenSet_86_data_,8);
const unsigned long CPPParser::_tokenSet_87_data_[] = { 512UL, 0UL, 0UL, 2147483904UL, 0UL, 0UL, 0UL, 0UL };
// ID TILDE SCOPE 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_87(_tokenSet_87_data_,8);
const unsigned long CPPParser::_tokenSet_88_data_[] = { 0UL, 1UL, 0UL, 4096UL, 0UL, 0UL, 0UL, 0UL };
// RPAREN ELLIPSIS 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_88(_tokenSet_88_data_,8);
const unsigned long CPPParser::_tokenSet_89_data_[] = { 2684000224UL, 4026531853UL, 1073741823UL, 2147488112UL, 0UL, 16368UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// LESSTHAN GREATERTHAN "typedef" "enum" ID LCURLY "inline" ASSIGNEQUAL 
// "extern" COMMA "__extension__" "_inline" "__inline" "__inline__" "virtual" 
// "explicit" "friend" "_stdcall" "__stdcall" LPAREN RPAREN "const" "__const" 
// "auto" "register" "static" "mutable" "const_cast" "volatile" "__volatile" 
// "__restrict" "__restrict__" "char" "bool" "short" "int" "_int64" "__int64" 
// "__w64" "long" "signed" "__signed" "__signed__" "unsigned" "float" "double" 
// "void" "_declspec" "__declspec" "__builtin_va_list" "class" "struct" 
// "union" "operator" "this" "true" "false" STAR AMPERSAND LSQUARE TILDE 
// ELLIPSIS SCOPE "_cdecl" "__cdecl" "_near" "__near" "_far" "__far" "__interrupt" 
// "pascal" "_pascal" "__pascal" 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_89(_tokenSet_89_data_,12);
const unsigned long CPPParser::_tokenSet_90_data_[] = { 2550694496UL, 13UL, 1006632991UL, 2147488112UL, 1577057279UL, 32764UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// LESSTHAN GREATERTHAN ID ASSIGNEQUAL COMMA "_stdcall" "__stdcall" LPAREN 
// RPAREN "const" "__const" "const_cast" "volatile" "__volatile" "__restrict" 
// "__restrict__" "operator" "this" "true" "false" STAR AMPERSAND LSQUARE 
// TILDE ELLIPSIS SCOPE TIMESEQUAL DIVIDEEQUAL MINUSEQUAL PLUSEQUAL MODEQUAL 
// SHIFTLEFTEQUAL SHIFTRIGHTEQUAL BITWISEANDEQUAL BITWISEXOREQUAL BITWISEOREQUAL 
// OR AND BITWISEOR BITWISEXOR NOTEQUAL EQUAL LESSTHANOREQUALTO GREATERTHANOREQUALTO 
// SHIFTLEFT SHIFTRIGHT PLUS MINUS DIVIDE MOD POINTERTOMBR PLUSPLUS MINUSMINUS 
// POINTERTO NOT "new" "_cdecl" "__cdecl" "_near" "__near" "_far" "__far" 
// "__interrupt" "pascal" "_pascal" "__pascal" "delete" 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_90(_tokenSet_90_data_,12);
const unsigned long CPPParser::_tokenSet_91_data_[] = { 2550694464UL, 1UL, 0UL, 2147487856UL, 0UL, 16368UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// GREATERTHAN ID ASSIGNEQUAL COMMA "_stdcall" "__stdcall" LPAREN RPAREN 
// STAR AMPERSAND LSQUARE ELLIPSIS SCOPE "_cdecl" "__cdecl" "_near" "__near" 
// "_far" "__far" "__interrupt" "pascal" "_pascal" "__pascal" 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_91(_tokenSet_91_data_,12);
const unsigned long CPPParser::_tokenSet_92_data_[] = { 4294959072UL, 4026531853UL, 1073741823UL, 2415935480UL, 3221225471UL, 1048575UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// LESSTHAN GREATERTHAN "typedef" "enum" ID LCURLY SEMICOLON "inline" RCURLY 
// ASSIGNEQUAL COLON "extern" StringLiteral COMMA "__extension__" "_inline" 
// "__inline" "__inline__" "virtual" "explicit" "friend" "_stdcall" "__stdcall" 
// "typename" "__attribute__" LPAREN RPAREN "const" "__const" "auto" "register" 
// "static" "mutable" "const_cast" "volatile" "__volatile" "__restrict" 
// "__restrict__" "char" "bool" "short" "int" "_int64" "__int64" "__w64" 
// "long" "signed" "__signed" "__signed__" "unsigned" "float" "double" 
// "void" "_declspec" "__declspec" "__builtin_va_list" "class" "struct" 
// "union" "operator" "this" "true" "false" OCTALINT STAR AMPERSAND LSQUARE 
// RSQUARE TILDE "asm" "__asm" "__asm__" ELLIPSIS "throw" "using" SCOPE 
// TIMESEQUAL DIVIDEEQUAL MINUSEQUAL PLUSEQUAL MODEQUAL SHIFTLEFTEQUAL 
// SHIFTRIGHTEQUAL BITWISEANDEQUAL BITWISEXOREQUAL BITWISEOREQUAL QUESTIONMARK 
// OR AND BITWISEOR BITWISEXOR NOTEQUAL EQUAL LESSTHANOREQUALTO GREATERTHANOREQUALTO 
// SHIFTLEFT SHIFTRIGHT PLUS MINUS DIVIDE MOD DOTMBR POINTERTOMBR PLUSPLUS 
// MINUSMINUS "sizeof" "dynamic_cast" "static_cast" "reinterpret_cast" 
// NOT "new" "_cdecl" "__cdecl" "_near" "__near" "_far" "__far" "__interrupt" 
// "pascal" "_pascal" "__pascal" "delete" DECIMALINT HEXADECIMALINT CharLiteral 
// FLOATONE FLOATTWO 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_92(_tokenSet_92_data_,12);
const unsigned long CPPParser::_tokenSet_93_data_[] = { 524352UL, 1UL, 0UL, 4096UL, 0UL, 0UL, 0UL, 0UL };
// GREATERTHAN COMMA RPAREN ELLIPSIS 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_93(_tokenSet_93_data_,8);
const unsigned long CPPParser::_tokenSet_94_data_[] = { 557120UL, 1UL, 0UL, 4096UL, 0UL, 0UL, 0UL, 0UL };
// GREATERTHAN ASSIGNEQUAL COMMA RPAREN ELLIPSIS 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_94(_tokenSet_94_data_,8);
const unsigned long CPPParser::_tokenSet_95_data_[] = { 524352UL, 1UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// GREATERTHAN COMMA RPAREN 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_95(_tokenSet_95_data_,8);
const unsigned long CPPParser::_tokenSet_96_data_[] = { 2148040768UL, 1UL, 0UL, 4160UL, 0UL, 0UL, 0UL, 0UL };
// GREATERTHAN ASSIGNEQUAL COMMA LPAREN RPAREN LSQUARE ELLIPSIS 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_96(_tokenSet_96_data_,8);
const unsigned long CPPParser::_tokenSet_97_data_[] = { 524864UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// GREATERTHAN ID COMMA 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_97(_tokenSet_97_data_,8);
const unsigned long CPPParser::_tokenSet_98_data_[] = { 3220312960UL, 4026531852UL, 1073741823UL, 2147488048UL, 0UL, 16368UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "typedef" "enum" ID "inline" "extern" "__extension__" "_inline" "__inline" 
// "__inline__" "virtual" "explicit" "friend" "_stdcall" "__stdcall" "typename" 
// LPAREN "const" "__const" "auto" "register" "static" "mutable" "const_cast" 
// "volatile" "__volatile" "__restrict" "__restrict__" "char" "bool" "short" 
// "int" "_int64" "__int64" "__w64" "long" "signed" "__signed" "__signed__" 
// "unsigned" "float" "double" "void" "_declspec" "__declspec" "__builtin_va_list" 
// "class" "struct" "union" "operator" "this" "true" "false" STAR AMPERSAND 
// TILDE ELLIPSIS SCOPE "_cdecl" "__cdecl" "_near" "__near" "_far" "__far" 
// "__interrupt" "pascal" "_pascal" "__pascal" 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_98(_tokenSet_98_data_,12);
const unsigned long CPPParser::_tokenSet_99_data_[] = { 2684000224UL, 4026531852UL, 1073741823UL, 2147484016UL, 1577057279UL, 32764UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// LESSTHAN GREATERTHAN "typedef" "enum" ID LCURLY "inline" ASSIGNEQUAL 
// "extern" COMMA "__extension__" "_inline" "__inline" "__inline__" "virtual" 
// "explicit" "friend" "_stdcall" "__stdcall" LPAREN "const" "__const" 
// "auto" "register" "static" "mutable" "const_cast" "volatile" "__volatile" 
// "__restrict" "__restrict__" "char" "bool" "short" "int" "_int64" "__int64" 
// "__w64" "long" "signed" "__signed" "__signed__" "unsigned" "float" "double" 
// "void" "_declspec" "__declspec" "__builtin_va_list" "class" "struct" 
// "union" "operator" "this" "true" "false" STAR AMPERSAND LSQUARE TILDE 
// SCOPE TIMESEQUAL DIVIDEEQUAL MINUSEQUAL PLUSEQUAL MODEQUAL SHIFTLEFTEQUAL 
// SHIFTRIGHTEQUAL BITWISEANDEQUAL BITWISEXOREQUAL BITWISEOREQUAL OR AND 
// BITWISEOR BITWISEXOR NOTEQUAL EQUAL LESSTHANOREQUALTO GREATERTHANOREQUALTO 
// SHIFTLEFT SHIFTRIGHT PLUS MINUS DIVIDE MOD POINTERTOMBR PLUSPLUS MINUSMINUS 
// POINTERTO NOT "new" "_cdecl" "__cdecl" "_near" "__near" "_far" "__far" 
// "__interrupt" "pascal" "_pascal" "__pascal" "delete" 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_99(_tokenSet_99_data_,12);
const unsigned long CPPParser::_tokenSet_100_data_[] = { 524352UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// GREATERTHAN COMMA 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_100(_tokenSet_100_data_,8);
const unsigned long CPPParser::_tokenSet_101_data_[] = { 2550661728UL, 0UL, 0UL, 2147483760UL, 0UL, 16368UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// LESSTHAN GREATERTHAN ID COMMA "_stdcall" "__stdcall" LPAREN STAR AMPERSAND 
// LSQUARE SCOPE "_cdecl" "__cdecl" "_near" "__near" "_far" "__far" "__interrupt" 
// "pascal" "_pascal" "__pascal" 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_101(_tokenSet_101_data_,12);
const unsigned long CPPParser::_tokenSet_102_data_[] = { 512UL, 0UL, 8388576UL, 2147483648UL, 0UL, 0UL, 0UL, 0UL };
// ID "char" "bool" "short" "int" "_int64" "__int64" "__w64" "long" "signed" 
// "__signed" "__signed__" "unsigned" "float" "double" "void" "_declspec" 
// "__declspec" "__builtin_va_list" SCOPE 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_102(_tokenSet_102_data_,8);
const unsigned long CPPParser::_tokenSet_103_data_[] = { 2550661728UL, 0UL, 8388576UL, 2147483760UL, 0UL, 16368UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// LESSTHAN GREATERTHAN ID COMMA "_stdcall" "__stdcall" LPAREN "char" "bool" 
// "short" "int" "_int64" "__int64" "__w64" "long" "signed" "__signed" 
// "__signed__" "unsigned" "float" "double" "void" "_declspec" "__declspec" 
// "__builtin_va_list" STAR AMPERSAND LSQUARE SCOPE "_cdecl" "__cdecl" 
// "_near" "__near" "_far" "__far" "__interrupt" "pascal" "_pascal" "__pascal" 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_103(_tokenSet_103_data_,12);
const unsigned long CPPParser::_tokenSet_104_data_[] = { 2683967456UL, 4026531852UL, 1073741823UL, 2147484016UL, 0UL, 16368UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// LESSTHAN GREATERTHAN "typedef" "enum" ID LCURLY "inline" "extern" COMMA 
// "__extension__" "_inline" "__inline" "__inline__" "virtual" "explicit" 
// "friend" "_stdcall" "__stdcall" LPAREN "const" "__const" "auto" "register" 
// "static" "mutable" "const_cast" "volatile" "__volatile" "__restrict" 
// "__restrict__" "char" "bool" "short" "int" "_int64" "__int64" "__w64" 
// "long" "signed" "__signed" "__signed__" "unsigned" "float" "double" 
// "void" "_declspec" "__declspec" "__builtin_va_list" "class" "struct" 
// "union" "operator" "this" "true" "false" STAR AMPERSAND LSQUARE TILDE 
// SCOPE "_cdecl" "__cdecl" "_near" "__near" "_far" "__far" "__interrupt" 
// "pascal" "_pascal" "__pascal" 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_104(_tokenSet_104_data_,12);
const unsigned long CPPParser::_tokenSet_105_data_[] = { 3221132256UL, 4026531852UL, 2147483647UL, 2147484024UL, 4294966271UL, 1032207UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// LESSTHAN GREATERTHAN "typedef" "enum" ID "inline" ASSIGNEQUAL "extern" 
// StringLiteral COMMA "__extension__" "_inline" "__inline" "__inline__" 
// "virtual" "explicit" "friend" "_stdcall" "__stdcall" "typename" LPAREN 
// "const" "__const" "auto" "register" "static" "mutable" "const_cast" 
// "volatile" "__volatile" "__restrict" "__restrict__" "char" "bool" "short" 
// "int" "_int64" "__int64" "__w64" "long" "signed" "__signed" "__signed__" 
// "unsigned" "float" "double" "void" "_declspec" "__declspec" "__builtin_va_list" 
// "class" "struct" "union" "operator" "this" "true" "false" DOT OCTALINT 
// STAR AMPERSAND LSQUARE TILDE SCOPE TIMESEQUAL DIVIDEEQUAL MINUSEQUAL 
// PLUSEQUAL MODEQUAL SHIFTLEFTEQUAL SHIFTRIGHTEQUAL BITWISEANDEQUAL BITWISEXOREQUAL 
// BITWISEOREQUAL OR AND BITWISEOR BITWISEXOR NOTEQUAL EQUAL LESSTHANOREQUALTO 
// GREATERTHANOREQUALTO SHIFTLEFT SHIFTRIGHT PLUS MINUS DIVIDE MOD DOTMBR 
// POINTERTOMBR PLUSPLUS MINUSMINUS "sizeof" POINTERTO "dynamic_cast" "static_cast" 
// "reinterpret_cast" NOT "new" "delete" DECIMALINT HEXADECIMALINT CharLiteral 
// FLOATONE FLOATTWO 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_105(_tokenSet_105_data_,12);
const unsigned long CPPParser::_tokenSet_106_data_[] = { 1074382944UL, 1UL, 0UL, 4256UL, 524287UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// LESSTHAN GREATERTHAN SEMICOLON RCURLY ASSIGNEQUAL COLON COMMA "__attribute__" 
// RPAREN AMPERSAND RSQUARE ELLIPSIS TIMESEQUAL DIVIDEEQUAL MINUSEQUAL 
// PLUSEQUAL MODEQUAL SHIFTLEFTEQUAL SHIFTRIGHTEQUAL BITWISEANDEQUAL BITWISEXOREQUAL 
// BITWISEOREQUAL QUESTIONMARK OR AND BITWISEOR BITWISEXOR NOTEQUAL EQUAL 
// LESSTHANOREQUALTO GREATERTHANOREQUALTO 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_106(_tokenSet_106_data_,12);
const unsigned long CPPParser::_tokenSet_107_data_[] = { 3220578176UL, 4026531852UL, 1073741823UL, 3086872376UL, 3093299200UL, 1032207UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "typedef" "enum" ID LCURLY SEMICOLON "inline" "extern" StringLiteral 
// "__extension__" "_inline" "__inline" "__inline__" "virtual" "explicit" 
// "friend" "_stdcall" "__stdcall" "typename" LPAREN "const" "__const" 
// "auto" "register" "static" "mutable" "const_cast" "volatile" "__volatile" 
// "__restrict" "__restrict__" "char" "bool" "short" "int" "_int64" "__int64" 
// "__w64" "long" "signed" "__signed" "__signed__" "unsigned" "float" "double" 
// "void" "_declspec" "__declspec" "__builtin_va_list" "class" "struct" 
// "union" "operator" "this" "true" "false" OCTALINT STAR AMPERSAND TILDE 
// "asm" "__asm" "__asm__" "throw" "case" "default" "if" "switch" "while" 
// "do" "for" "goto" "continue" "break" "return" "try" "using" "_asm" SCOPE 
// PLUS MINUS PLUSPLUS MINUSMINUS "sizeof" "dynamic_cast" "static_cast" 
// "reinterpret_cast" NOT "new" "delete" DECIMALINT HEXADECIMALINT CharLiteral 
// FLOATONE FLOATTWO 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_107(_tokenSet_107_data_,12);
const unsigned long CPPParser::_tokenSet_108_data_[] = { 3221134304UL, 4026531852UL, 2147483647UL, 2147484024UL, 4294967295UL, 1032207UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// LESSTHAN GREATERTHAN "typedef" "enum" ID SEMICOLON "inline" ASSIGNEQUAL 
// "extern" StringLiteral COMMA "__extension__" "_inline" "__inline" "__inline__" 
// "virtual" "explicit" "friend" "_stdcall" "__stdcall" "typename" LPAREN 
// "const" "__const" "auto" "register" "static" "mutable" "const_cast" 
// "volatile" "__volatile" "__restrict" "__restrict__" "char" "bool" "short" 
// "int" "_int64" "__int64" "__w64" "long" "signed" "__signed" "__signed__" 
// "unsigned" "float" "double" "void" "_declspec" "__declspec" "__builtin_va_list" 
// "class" "struct" "union" "operator" "this" "true" "false" DOT OCTALINT 
// STAR AMPERSAND LSQUARE TILDE SCOPE TIMESEQUAL DIVIDEEQUAL MINUSEQUAL 
// PLUSEQUAL MODEQUAL SHIFTLEFTEQUAL SHIFTRIGHTEQUAL BITWISEANDEQUAL BITWISEXOREQUAL 
// BITWISEOREQUAL QUESTIONMARK OR AND BITWISEOR BITWISEXOR NOTEQUAL EQUAL 
// LESSTHANOREQUALTO GREATERTHANOREQUALTO SHIFTLEFT SHIFTRIGHT PLUS MINUS 
// DIVIDE MOD DOTMBR POINTERTOMBR PLUSPLUS MINUSMINUS "sizeof" POINTERTO 
// "dynamic_cast" "static_cast" "reinterpret_cast" NOT "new" "delete" DECIMALINT 
// HEXADECIMALINT CharLiteral FLOATONE FLOATTWO 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_108(_tokenSet_108_data_,12);
const unsigned long CPPParser::_tokenSet_109_data_[] = { 3220594560UL, 4026531852UL, 1073741823UL, 3087003448UL, 3093299200UL, 1032207UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "typedef" "enum" ID LCURLY SEMICOLON "inline" RCURLY "extern" StringLiteral 
// "__extension__" "_inline" "__inline" "__inline__" "virtual" "explicit" 
// "friend" "_stdcall" "__stdcall" "typename" LPAREN "const" "__const" 
// "auto" "register" "static" "mutable" "const_cast" "volatile" "__volatile" 
// "__restrict" "__restrict__" "char" "bool" "short" "int" "_int64" "__int64" 
// "__w64" "long" "signed" "__signed" "__signed__" "unsigned" "float" "double" 
// "void" "_declspec" "__declspec" "__builtin_va_list" "class" "struct" 
// "union" "operator" "this" "true" "false" OCTALINT STAR AMPERSAND TILDE 
// "asm" "__asm" "__asm__" "throw" "case" "default" "if" "else" "switch" 
// "while" "do" "for" "goto" "continue" "break" "return" "try" "using" 
// "_asm" SCOPE PLUS MINUS PLUSPLUS MINUSMINUS "sizeof" "dynamic_cast" 
// "static_cast" "reinterpret_cast" NOT "new" "delete" DECIMALINT HEXADECIMALINT 
// CharLiteral FLOATONE FLOATTWO 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_109(_tokenSet_109_data_,12);
const unsigned long CPPParser::_tokenSet_110_data_[] = { 67584UL, 1UL, 0UL, 128UL, 0UL, 0UL, 0UL, 0UL };
// SEMICOLON COLON RPAREN RSQUARE 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_110(_tokenSet_110_data_,8);
const unsigned long CPPParser::_tokenSet_111_data_[] = { 4294967282UL, 4026531852UL, 2147483647UL, 4294963071UL, 4294967295UL, 1048575UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// EOF "template" LESSTHAN GREATERTHAN "typedef" "enum" ID LCURLY SEMICOLON 
// "inline" "namespace" RCURLY ASSIGNEQUAL COLON "extern" StringLiteral 
// COMMA "__extension__" "_inline" "__inline" "__inline__" "virtual" "explicit" 
// "friend" "_stdcall" "__stdcall" "typename" "__attribute__" LPAREN "const" 
// "__const" "auto" "register" "static" "mutable" "const_cast" "volatile" 
// "__volatile" "__restrict" "__restrict__" "char" "bool" "short" "int" 
// "_int64" "__int64" "__w64" "long" "signed" "__signed" "__signed__" "unsigned" 
// "float" "double" "void" "_declspec" "__declspec" "__builtin_va_list" 
// "class" "struct" "union" "operator" "this" "true" "false" DOT "public" 
// "protected" "private" OCTALINT STAR AMPERSAND LSQUARE TILDE "asm" "__asm" 
// "__asm__" "throw" "case" "default" "if" "else" "switch" "while" "do" 
// "for" "goto" "continue" "break" "return" "try" "catch" "using" "_asm" 
// "__volatile__" SCOPE TIMESEQUAL DIVIDEEQUAL MINUSEQUAL PLUSEQUAL MODEQUAL 
// SHIFTLEFTEQUAL SHIFTRIGHTEQUAL BITWISEANDEQUAL BITWISEXOREQUAL BITWISEOREQUAL 
// QUESTIONMARK OR AND BITWISEOR BITWISEXOR NOTEQUAL EQUAL LESSTHANOREQUALTO 
// GREATERTHANOREQUALTO SHIFTLEFT SHIFTRIGHT PLUS MINUS DIVIDE MOD DOTMBR 
// POINTERTOMBR PLUSPLUS MINUSMINUS "sizeof" POINTERTO "dynamic_cast" "static_cast" 
// "reinterpret_cast" NOT "new" "_cdecl" "__cdecl" "_near" "__near" "_far" 
// "__far" "__interrupt" "pascal" "_pascal" "__pascal" "delete" DECIMALINT 
// HEXADECIMALINT CharLiteral FLOATONE FLOATTWO 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_111(_tokenSet_111_data_,12);
const unsigned long CPPParser::_tokenSet_112_data_[] = { 3220594560UL, 4026531852UL, 1073741823UL, 3221221176UL, 3093299200UL, 1032207UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "typedef" "enum" ID LCURLY SEMICOLON "inline" RCURLY "extern" StringLiteral 
// "__extension__" "_inline" "__inline" "__inline__" "virtual" "explicit" 
// "friend" "_stdcall" "__stdcall" "typename" LPAREN "const" "__const" 
// "auto" "register" "static" "mutable" "const_cast" "volatile" "__volatile" 
// "__restrict" "__restrict__" "char" "bool" "short" "int" "_int64" "__int64" 
// "__w64" "long" "signed" "__signed" "__signed__" "unsigned" "float" "double" 
// "void" "_declspec" "__declspec" "__builtin_va_list" "class" "struct" 
// "union" "operator" "this" "true" "false" OCTALINT STAR AMPERSAND TILDE 
// "asm" "__asm" "__asm__" "throw" "case" "default" "if" "else" "switch" 
// "while" "do" "for" "goto" "continue" "break" "return" "try" "catch" 
// "using" "_asm" SCOPE PLUS MINUS PLUSPLUS MINUSMINUS "sizeof" "dynamic_cast" 
// "static_cast" "reinterpret_cast" NOT "new" "delete" DECIMALINT HEXADECIMALINT 
// CharLiteral FLOATONE FLOATTWO 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_112(_tokenSet_112_data_,12);
const unsigned long CPPParser::_tokenSet_113_data_[] = { 4294950896UL, 4294967295UL, 4294967295UL, 4294967295UL, 4294967295UL, 4294967295UL, 127UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "template" LESSTHAN GREATERTHAN "typedef" "enum" ID LCURLY SEMICOLON 
// "inline" "namespace" ASSIGNEQUAL COLON "extern" StringLiteral COMMA 
// "__extension__" "_inline" "__inline" "__inline__" "virtual" "explicit" 
// "friend" "_stdcall" "__stdcall" "typename" "__attribute__" LPAREN RPAREN 
// "__mode__" "const" "__const" "__noreturn__" "__returns_twice__" "__noinline__" 
// "__always_inline__" "__flatten__" "__pure__" "__const__" "__nothrow__" 
// "__sentinel__" "__format__" "__format_arg__" "__no_instrument_function__" 
// "__section__" "__constructor__" "__destructor__" "__used__" "__unused__" 
// "__deprecated__" "__weak__" "__malloc__" "__alias__" "__warn_unused_result__" 
// "__nonnull__" "__externally_visible__" "auto" "register" "static" "mutable" 
// "const_cast" "volatile" "__volatile" "__restrict" "__restrict__" "char" 
// "bool" "short" "int" "_int64" "__int64" "__w64" "long" "signed" "__signed" 
// "__signed__" "unsigned" "float" "double" "void" "_declspec" "__declspec" 
// "__builtin_va_list" "class" "struct" "union" "operator" "this" "true" 
// "false" DOT EQUALS "public" "protected" "private" OCTALINT STAR AMPERSAND 
// LSQUARE RSQUARE TILDE "asm" "__asm" "__asm__" ELLIPSIS "throw" "case" 
// "default" "if" "else" "switch" "while" "do" "for" "goto" "continue" 
// "break" "return" "try" "catch" "using" "_asm" "__volatile__" SCOPE TIMESEQUAL 
// DIVIDEEQUAL MINUSEQUAL PLUSEQUAL MODEQUAL SHIFTLEFTEQUAL SHIFTRIGHTEQUAL 
// BITWISEANDEQUAL BITWISEXOREQUAL BITWISEOREQUAL QUESTIONMARK OR AND BITWISEOR 
// BITWISEXOR NOTEQUAL EQUAL LESSTHANOREQUALTO GREATERTHANOREQUALTO SHIFTLEFT 
// SHIFTRIGHT PLUS MINUS DIVIDE MOD DOTMBR POINTERTOMBR PLUSPLUS MINUSMINUS 
// "sizeof" POINTERTO "dynamic_cast" "static_cast" "reinterpret_cast" NOT 
// "new" "_cdecl" "__cdecl" "_near" "__near" "_far" "__far" "__interrupt" 
// "pascal" "_pascal" "__pascal" "delete" DECIMALINT HEXADECIMALINT CharLiteral 
// FLOATONE FLOATTWO Whitespace Comment CPPComment PREPROC_DIRECTIVE ALL_TO_NL 
// LineDirective Space Pragma Error EndOfLine Escape Digit Decimal LongSuffix 
// UnsignedSuffix FloatSuffix Exponent Vocabulary Number 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_113(_tokenSet_113_data_,16);
const unsigned long CPPParser::_tokenSet_114_data_[] = { 65536UL, 1UL, 0UL, 2147483648UL, 0UL, 0UL, 0UL, 0UL };
// COLON RPAREN SCOPE 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_114(_tokenSet_114_data_,8);
const unsigned long CPPParser::_tokenSet_115_data_[] = { 1074382912UL, 1UL, 0UL, 4224UL, 1023UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// GREATERTHAN SEMICOLON RCURLY ASSIGNEQUAL COLON COMMA "__attribute__" 
// RPAREN RSQUARE ELLIPSIS TIMESEQUAL DIVIDEEQUAL MINUSEQUAL PLUSEQUAL 
// MODEQUAL SHIFTLEFTEQUAL SHIFTRIGHTEQUAL BITWISEANDEQUAL BITWISEXOREQUAL 
// BITWISEOREQUAL 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_115(_tokenSet_115_data_,12);
const unsigned long CPPParser::_tokenSet_116_data_[] = { 1074382912UL, 1UL, 0UL, 4224UL, 2047UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// GREATERTHAN SEMICOLON RCURLY ASSIGNEQUAL COLON COMMA "__attribute__" 
// RPAREN RSQUARE ELLIPSIS TIMESEQUAL DIVIDEEQUAL MINUSEQUAL PLUSEQUAL 
// MODEQUAL SHIFTLEFTEQUAL SHIFTRIGHTEQUAL BITWISEANDEQUAL BITWISEXOREQUAL 
// BITWISEOREQUAL QUESTIONMARK 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_116(_tokenSet_116_data_,12);
const unsigned long CPPParser::_tokenSet_117_data_[] = { 1074382912UL, 1UL, 0UL, 4224UL, 4095UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// GREATERTHAN SEMICOLON RCURLY ASSIGNEQUAL COLON COMMA "__attribute__" 
// RPAREN RSQUARE ELLIPSIS TIMESEQUAL DIVIDEEQUAL MINUSEQUAL PLUSEQUAL 
// MODEQUAL SHIFTLEFTEQUAL SHIFTRIGHTEQUAL BITWISEANDEQUAL BITWISEXOREQUAL 
// BITWISEOREQUAL QUESTIONMARK OR 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_117(_tokenSet_117_data_,12);
const unsigned long CPPParser::_tokenSet_118_data_[] = { 1074382912UL, 1UL, 0UL, 4224UL, 8191UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// GREATERTHAN SEMICOLON RCURLY ASSIGNEQUAL COLON COMMA "__attribute__" 
// RPAREN RSQUARE ELLIPSIS TIMESEQUAL DIVIDEEQUAL MINUSEQUAL PLUSEQUAL 
// MODEQUAL SHIFTLEFTEQUAL SHIFTRIGHTEQUAL BITWISEANDEQUAL BITWISEXOREQUAL 
// BITWISEOREQUAL QUESTIONMARK OR AND 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_118(_tokenSet_118_data_,12);
const unsigned long CPPParser::_tokenSet_119_data_[] = { 1074382912UL, 1UL, 0UL, 4224UL, 16383UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// GREATERTHAN SEMICOLON RCURLY ASSIGNEQUAL COLON COMMA "__attribute__" 
// RPAREN RSQUARE ELLIPSIS TIMESEQUAL DIVIDEEQUAL MINUSEQUAL PLUSEQUAL 
// MODEQUAL SHIFTLEFTEQUAL SHIFTRIGHTEQUAL BITWISEANDEQUAL BITWISEXOREQUAL 
// BITWISEOREQUAL QUESTIONMARK OR AND BITWISEOR 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_119(_tokenSet_119_data_,12);
const unsigned long CPPParser::_tokenSet_120_data_[] = { 1074382912UL, 1UL, 0UL, 4224UL, 32767UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// GREATERTHAN SEMICOLON RCURLY ASSIGNEQUAL COLON COMMA "__attribute__" 
// RPAREN RSQUARE ELLIPSIS TIMESEQUAL DIVIDEEQUAL MINUSEQUAL PLUSEQUAL 
// MODEQUAL SHIFTLEFTEQUAL SHIFTRIGHTEQUAL BITWISEANDEQUAL BITWISEXOREQUAL 
// BITWISEOREQUAL QUESTIONMARK OR AND BITWISEOR BITWISEXOR 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_120(_tokenSet_120_data_,12);
const unsigned long CPPParser::_tokenSet_121_data_[] = { 1074382912UL, 1UL, 0UL, 4256UL, 32767UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// GREATERTHAN SEMICOLON RCURLY ASSIGNEQUAL COLON COMMA "__attribute__" 
// RPAREN AMPERSAND RSQUARE ELLIPSIS TIMESEQUAL DIVIDEEQUAL MINUSEQUAL 
// PLUSEQUAL MODEQUAL SHIFTLEFTEQUAL SHIFTRIGHTEQUAL BITWISEANDEQUAL BITWISEXOREQUAL 
// BITWISEOREQUAL QUESTIONMARK OR AND BITWISEOR BITWISEXOR 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_121(_tokenSet_121_data_,12);
const unsigned long CPPParser::_tokenSet_122_data_[] = { 96UL, 0UL, 0UL, 0UL, 393216UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// LESSTHAN GREATERTHAN LESSTHANOREQUALTO GREATERTHANOREQUALTO 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_122(_tokenSet_122_data_,12);
const unsigned long CPPParser::_tokenSet_123_data_[] = { 1074382912UL, 1UL, 0UL, 4256UL, 131071UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// GREATERTHAN SEMICOLON RCURLY ASSIGNEQUAL COLON COMMA "__attribute__" 
// RPAREN AMPERSAND RSQUARE ELLIPSIS TIMESEQUAL DIVIDEEQUAL MINUSEQUAL 
// PLUSEQUAL MODEQUAL SHIFTLEFTEQUAL SHIFTRIGHTEQUAL BITWISEANDEQUAL BITWISEXOREQUAL 
// BITWISEOREQUAL QUESTIONMARK OR AND BITWISEOR BITWISEXOR NOTEQUAL EQUAL 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_123(_tokenSet_123_data_,12);
const unsigned long CPPParser::_tokenSet_124_data_[] = { 1074382944UL, 1UL, 0UL, 4256UL, 2097151UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// LESSTHAN GREATERTHAN SEMICOLON RCURLY ASSIGNEQUAL COLON COMMA "__attribute__" 
// RPAREN AMPERSAND RSQUARE ELLIPSIS TIMESEQUAL DIVIDEEQUAL MINUSEQUAL 
// PLUSEQUAL MODEQUAL SHIFTLEFTEQUAL SHIFTRIGHTEQUAL BITWISEANDEQUAL BITWISEXOREQUAL 
// BITWISEOREQUAL QUESTIONMARK OR AND BITWISEOR BITWISEXOR NOTEQUAL EQUAL 
// LESSTHANOREQUALTO GREATERTHANOREQUALTO SHIFTLEFT SHIFTRIGHT 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_124(_tokenSet_124_data_,12);
const unsigned long CPPParser::_tokenSet_125_data_[] = { 0UL, 0UL, 0UL, 16UL, 25165824UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// STAR DIVIDE MOD 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_125(_tokenSet_125_data_,12);
const unsigned long CPPParser::_tokenSet_126_data_[] = { 1074382944UL, 1UL, 0UL, 4256UL, 8388607UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// LESSTHAN GREATERTHAN SEMICOLON RCURLY ASSIGNEQUAL COLON COMMA "__attribute__" 
// RPAREN AMPERSAND RSQUARE ELLIPSIS TIMESEQUAL DIVIDEEQUAL MINUSEQUAL 
// PLUSEQUAL MODEQUAL SHIFTLEFTEQUAL SHIFTRIGHTEQUAL BITWISEANDEQUAL BITWISEXOREQUAL 
// BITWISEOREQUAL QUESTIONMARK OR AND BITWISEOR BITWISEXOR NOTEQUAL EQUAL 
// LESSTHANOREQUALTO GREATERTHANOREQUALTO SHIFTLEFT SHIFTRIGHT PLUS MINUS 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_126(_tokenSet_126_data_,12);
const unsigned long CPPParser::_tokenSet_127_data_[] = { 1074382944UL, 1UL, 0UL, 4272UL, 33554431UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// LESSTHAN GREATERTHAN SEMICOLON RCURLY ASSIGNEQUAL COLON COMMA "__attribute__" 
// RPAREN STAR AMPERSAND RSQUARE ELLIPSIS TIMESEQUAL DIVIDEEQUAL MINUSEQUAL 
// PLUSEQUAL MODEQUAL SHIFTLEFTEQUAL SHIFTRIGHTEQUAL BITWISEANDEQUAL BITWISEXOREQUAL 
// BITWISEOREQUAL QUESTIONMARK OR AND BITWISEOR BITWISEXOR NOTEQUAL EQUAL 
// LESSTHANOREQUALTO GREATERTHANOREQUALTO SHIFTLEFT SHIFTRIGHT PLUS MINUS 
// DIVIDE MOD 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_127(_tokenSet_127_data_,12);
const unsigned long CPPParser::_tokenSet_128_data_[] = { 768UL, 12UL, 67108863UL, 2147483648UL, 0UL, 0UL, 0UL, 0UL };
// "enum" ID "const" "__const" "const_cast" "volatile" "__volatile" "__restrict" 
// "__restrict__" "char" "bool" "short" "int" "_int64" "__int64" "__w64" 
// "long" "signed" "__signed" "__signed__" "unsigned" "float" "double" 
// "void" "_declspec" "__declspec" "__builtin_va_list" "class" "struct" 
// "union" SCOPE 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_128(_tokenSet_128_data_,8);
const unsigned long CPPParser::_tokenSet_129_data_[] = { 1074382944UL, 1UL, 0UL, 4272UL, 134217727UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// LESSTHAN GREATERTHAN SEMICOLON RCURLY ASSIGNEQUAL COLON COMMA "__attribute__" 
// RPAREN STAR AMPERSAND RSQUARE ELLIPSIS TIMESEQUAL DIVIDEEQUAL MINUSEQUAL 
// PLUSEQUAL MODEQUAL SHIFTLEFTEQUAL SHIFTRIGHTEQUAL BITWISEANDEQUAL BITWISEXOREQUAL 
// BITWISEOREQUAL QUESTIONMARK OR AND BITWISEOR BITWISEXOR NOTEQUAL EQUAL 
// LESSTHANOREQUALTO GREATERTHANOREQUALTO SHIFTLEFT SHIFTRIGHT PLUS MINUS 
// DIVIDE MOD DOTMBR POINTERTOMBR 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_129(_tokenSet_129_data_,12);
const unsigned long CPPParser::_tokenSet_130_data_[] = { 2147746304UL, 0UL, 1015021537UL, 2147483912UL, 2147483648UL, 1015811UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// ID StringLiteral LPAREN "const_cast" "char" "bool" "short" "int" "_int64" 
// "__int64" "__w64" "long" "signed" "__signed" "__signed__" "unsigned" 
// "float" "double" "void" "_declspec" "__declspec" "__builtin_va_list" 
// "operator" "this" "true" "false" OCTALINT TILDE SCOPE "dynamic_cast" 
// "static_cast" "reinterpret_cast" DECIMALINT HEXADECIMALINT CharLiteral 
// FLOATONE FLOATTWO 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_130(_tokenSet_130_data_,12);
const unsigned long CPPParser::_tokenSet_131_data_[] = { 3222129248UL, 1UL, 2088763361UL, 2147488248UL, 4294967295UL, 1032207UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// LESSTHAN GREATERTHAN ID SEMICOLON RCURLY ASSIGNEQUAL COLON StringLiteral 
// COMMA "__attribute__" LPAREN RPAREN "const_cast" "char" "bool" "short" 
// "int" "_int64" "__int64" "__w64" "long" "signed" "__signed" "__signed__" 
// "unsigned" "float" "double" "void" "_declspec" "__declspec" "__builtin_va_list" 
// "operator" "this" "true" "false" DOT OCTALINT STAR AMPERSAND LSQUARE 
// RSQUARE TILDE ELLIPSIS SCOPE TIMESEQUAL DIVIDEEQUAL MINUSEQUAL PLUSEQUAL 
// MODEQUAL SHIFTLEFTEQUAL SHIFTRIGHTEQUAL BITWISEANDEQUAL BITWISEXOREQUAL 
// BITWISEOREQUAL QUESTIONMARK OR AND BITWISEOR BITWISEXOR NOTEQUAL EQUAL 
// LESSTHANOREQUALTO GREATERTHANOREQUALTO SHIFTLEFT SHIFTRIGHT PLUS MINUS 
// DIVIDE MOD DOTMBR POINTERTOMBR PLUSPLUS MINUSMINUS "sizeof" POINTERTO 
// "dynamic_cast" "static_cast" "reinterpret_cast" NOT "new" "delete" DECIMALINT 
// HEXADECIMALINT CharLiteral FLOATONE FLOATTWO 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_131(_tokenSet_131_data_,12);
const unsigned long CPPParser::_tokenSet_132_data_[] = { 0UL, 0UL, 0UL, 304UL, 6291456UL, 4UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// STAR AMPERSAND TILDE PLUS MINUS NOT 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_132(_tokenSet_132_data_,12);
const unsigned long CPPParser::_tokenSet_133_data_[] = { 0UL, 0UL, 0UL, 2147483648UL, 0UL, 16392UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// SCOPE "new" "delete" 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_133(_tokenSet_133_data_,12);
const unsigned long CPPParser::_tokenSet_134_data_[] = { 3220575104UL, 4026531852UL, 1073741823UL, 2147484024UL, 3093299200UL, 1032207UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "typedef" "enum" ID "inline" "extern" StringLiteral "__extension__" 
// "_inline" "__inline" "__inline__" "virtual" "explicit" "friend" "_stdcall" 
// "__stdcall" "typename" LPAREN "const" "__const" "auto" "register" "static" 
// "mutable" "const_cast" "volatile" "__volatile" "__restrict" "__restrict__" 
// "char" "bool" "short" "int" "_int64" "__int64" "__w64" "long" "signed" 
// "__signed" "__signed__" "unsigned" "float" "double" "void" "_declspec" 
// "__declspec" "__builtin_va_list" "class" "struct" "union" "operator" 
// "this" "true" "false" OCTALINT STAR AMPERSAND LSQUARE TILDE SCOPE PLUS 
// MINUS PLUSPLUS MINUSMINUS "sizeof" "dynamic_cast" "static_cast" "reinterpret_cast" 
// NOT "new" "delete" DECIMALINT HEXADECIMALINT CharLiteral FLOATONE FLOATTWO 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_134(_tokenSet_134_data_,12);
const unsigned long CPPParser::_tokenSet_135_data_[] = { 2147484192UL, 0UL, 8388576UL, 2147483648UL, 0UL, 0UL, 0UL, 0UL };
// LESSTHAN ID LPAREN "char" "bool" "short" "int" "_int64" "__int64" "__w64" 
// "long" "signed" "__signed" "__signed__" "unsigned" "float" "double" 
// "void" "_declspec" "__declspec" "__builtin_va_list" SCOPE 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_135(_tokenSet_135_data_,8);
const unsigned long CPPParser::_tokenSet_136_data_[] = { 2147746304UL, 0UL, 1006632960UL, 2147483912UL, 0UL, 1015808UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// ID StringLiteral LPAREN "operator" "this" "true" "false" OCTALINT TILDE 
// SCOPE DECIMALINT HEXADECIMALINT CharLiteral FLOATONE FLOATTWO 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_136(_tokenSet_136_data_,12);
const unsigned long CPPParser::_tokenSet_137_data_[] = { 0UL, 0UL, 1UL, 0UL, 2147483648UL, 3UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "const_cast" "dynamic_cast" "static_cast" "reinterpret_cast" 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_137(_tokenSet_137_data_,12);
const unsigned long CPPParser::_tokenSet_138_data_[] = { 3220312960UL, 4026531852UL, 67108863UL, 2147483648UL, 0UL, 0UL, 0UL, 0UL };
// "typedef" "enum" ID "inline" "extern" "__extension__" "_inline" "__inline" 
// "__inline__" "virtual" "explicit" "friend" "_stdcall" "__stdcall" "typename" 
// LPAREN "const" "__const" "auto" "register" "static" "mutable" "const_cast" 
// "volatile" "__volatile" "__restrict" "__restrict__" "char" "bool" "short" 
// "int" "_int64" "__int64" "__w64" "long" "signed" "__signed" "__signed__" 
// "unsigned" "float" "double" "void" "_declspec" "__declspec" "__builtin_va_list" 
// "class" "struct" "union" SCOPE 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_138(_tokenSet_138_data_,8);
const unsigned long CPPParser::_tokenSet_139_data_[] = { 4294696928UL, 4026531853UL, 1073741823UL, 2147488240UL, 134217727UL, 16368UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// LESSTHAN GREATERTHAN "typedef" "enum" ID LCURLY SEMICOLON "inline" RCURLY 
// ASSIGNEQUAL COLON "extern" COMMA "__extension__" "_inline" "__inline" 
// "__inline__" "virtual" "explicit" "friend" "_stdcall" "__stdcall" "typename" 
// "__attribute__" LPAREN RPAREN "const" "__const" "auto" "register" "static" 
// "mutable" "const_cast" "volatile" "__volatile" "__restrict" "__restrict__" 
// "char" "bool" "short" "int" "_int64" "__int64" "__w64" "long" "signed" 
// "__signed" "__signed__" "unsigned" "float" "double" "void" "_declspec" 
// "__declspec" "__builtin_va_list" "class" "struct" "union" "operator" 
// "this" "true" "false" STAR AMPERSAND LSQUARE RSQUARE TILDE ELLIPSIS 
// SCOPE TIMESEQUAL DIVIDEEQUAL MINUSEQUAL PLUSEQUAL MODEQUAL SHIFTLEFTEQUAL 
// SHIFTRIGHTEQUAL BITWISEANDEQUAL BITWISEXOREQUAL BITWISEOREQUAL QUESTIONMARK 
// OR AND BITWISEOR BITWISEXOR NOTEQUAL EQUAL LESSTHANOREQUALTO GREATERTHANOREQUALTO 
// SHIFTLEFT SHIFTRIGHT PLUS MINUS DIVIDE MOD DOTMBR POINTERTOMBR "_cdecl" 
// "__cdecl" "_near" "__near" "_far" "__far" "__interrupt" "pascal" "_pascal" 
// "__pascal" 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_139(_tokenSet_139_data_,12);
const unsigned long CPPParser::_tokenSet_140_data_[] = { 402653696UL, 0UL, 0UL, 2147483760UL, 0UL, 16368UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// ID "_stdcall" "__stdcall" STAR AMPERSAND LSQUARE SCOPE "_cdecl" "__cdecl" 
// "_near" "__near" "_far" "__far" "__interrupt" "pascal" "_pascal" "__pascal" 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_140(_tokenSet_140_data_,12);
const unsigned long CPPParser::_tokenSet_141_data_[] = { 3624782432UL, 13UL, 1015021567UL, 2147488248UL, 3221225471UL, 1048575UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// LESSTHAN GREATERTHAN ID SEMICOLON RCURLY ASSIGNEQUAL COLON StringLiteral 
// COMMA "_stdcall" "__stdcall" "__attribute__" LPAREN RPAREN "const" "__const" 
// "const_cast" "volatile" "__volatile" "__restrict" "__restrict__" "char" 
// "bool" "short" "int" "_int64" "__int64" "__w64" "long" "signed" "__signed" 
// "__signed__" "unsigned" "float" "double" "void" "_declspec" "__declspec" 
// "__builtin_va_list" "operator" "this" "true" "false" OCTALINT STAR AMPERSAND 
// LSQUARE RSQUARE TILDE ELLIPSIS SCOPE TIMESEQUAL DIVIDEEQUAL MINUSEQUAL 
// PLUSEQUAL MODEQUAL SHIFTLEFTEQUAL SHIFTRIGHTEQUAL BITWISEANDEQUAL BITWISEXOREQUAL 
// BITWISEOREQUAL QUESTIONMARK OR AND BITWISEOR BITWISEXOR NOTEQUAL EQUAL 
// LESSTHANOREQUALTO GREATERTHANOREQUALTO SHIFTLEFT SHIFTRIGHT PLUS MINUS 
// DIVIDE MOD DOTMBR POINTERTOMBR PLUSPLUS MINUSMINUS "sizeof" "dynamic_cast" 
// "static_cast" "reinterpret_cast" NOT "new" "_cdecl" "__cdecl" "_near" 
// "__near" "_far" "__far" "__interrupt" "pascal" "_pascal" "__pascal" 
// "delete" DECIMALINT HEXADECIMALINT CharLiteral FLOATONE FLOATTWO 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_141(_tokenSet_141_data_,12);
const unsigned long CPPParser::_tokenSet_142_data_[] = { 3221866592UL, 1UL, 0UL, 4272UL, 134217727UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// LESSTHAN GREATERTHAN SEMICOLON RCURLY ASSIGNEQUAL COLON COMMA "__attribute__" 
// LPAREN RPAREN STAR AMPERSAND RSQUARE ELLIPSIS TIMESEQUAL DIVIDEEQUAL 
// MINUSEQUAL PLUSEQUAL MODEQUAL SHIFTLEFTEQUAL SHIFTRIGHTEQUAL BITWISEANDEQUAL 
// BITWISEXOREQUAL BITWISEOREQUAL QUESTIONMARK OR AND BITWISEOR BITWISEXOR 
// NOTEQUAL EQUAL LESSTHANOREQUALTO GREATERTHANOREQUALTO SHIFTLEFT SHIFTRIGHT 
// PLUS MINUS DIVIDE MOD DOTMBR POINTERTOMBR 
const ANTLR_USE_NAMESPACE(antlr)BitSet CPPParser::_tokenSet_142(_tokenSet_142_data_,12);


