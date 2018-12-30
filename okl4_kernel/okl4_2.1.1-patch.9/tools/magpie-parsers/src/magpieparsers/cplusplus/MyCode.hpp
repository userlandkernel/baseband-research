/* 1999-2004 Version 3.0 July 2004
 * Constructed by David Wigg at London South Bank University for CPP_parser.g
 *
 * The MyCode files just demonstrate how your application code can
 *	 be included as a subclass of the CPPParser class
 *
 * See MyReadMe.txt for further information
 *
 * This file is best viewed in courier font with tabs set to 4 spaces
*/

#ifndef INC_MyCode_hpp_
#define INC_MyCode_hpp_

#include <iostream>
#include <time.h>

#include "CPPParser.hpp"

ANTLR_USING_NAMESPACE(std)

CPPParser parser(ANTLR_USE_NAMESPACE(antlr)TokenStream& lexer);

#define SYS_FUNS_LIMIT 100
#define MAX_VAR_LENGTH 50
#define MAX_FUN_NAME_LEN 15

class MyCode : public CPPParser
	{
public:
//  Constructor
	MyCode(ANTLR_USE_NAMESPACE(antlr)TokenStream& lexer);

//	Destructor
	~MyCode();
	
// Mycode functions in main.cpp
	void myCode_pre_processing(int argc, char *argv[]);
	void myCode_post_processing(void);

// MyCode functions in CPP_parser.g
// The end of the name is the name of the production where it is called from
protected:
	void myCode_end_of_stmt();
	void myCode_function_direct_declarator(const char *id);
	void myCode_declaration();
	void myCode_init_declarator();
	void myCode_direct_declarator(char *id);
	void myCode_labelled_statement();
	void myCode_case_statement();
	void myCode_selection_statement(int x);
	void myCode_iteration_statement(int x);
	void myCode_unary_operator();
	void myCode_id_expression();

private:
	int myLog;	// Can be used to produce selective MyCode output in
				//  filename.log for testing or verification
				// 1 = standard log data designed to detect any significant change
				//		between one version and the next when run on the same data
				// Use higher numbers for any occasional special testing or verification

	CPPSymbol *cs;	// Local variable for MyCode

	bool process_all_files;

	int function_number;

	char current_function[MAX_VAR_LENGTH]; /* current function name */
	int current_fun_scope;
	
	// For name of input file
	char my_input_file[127];	// Name of input file
	
	// A file to contain application summary output
	FILE *myDatFilePtr;
	char my_dat_file[127];	// Name of summary file

	// A file to contain application code output
	FILE *myLisFilePtr;
	char my_lis_file[127];	// Name of application data file

	// Optional file to contain application log data controlled by myLog
	//  switch in MyCode constructor
	FILE *myLogFilePtr;
	char my_log_file[127];	// Name of version log file

	char var[MAX_VAR_LENGTH];	// Current variable name

	time_t now; // = time(NULL);  // See Joy of C p.618
	};

#endif //INC_MyCode_hpp_

