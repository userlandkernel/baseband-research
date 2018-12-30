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

#include "MyCode.hpp"

extern bool in_user_file;	// Set in process_line_directive() in main.cpp

// Constructor
MyCode::MyCode(ANTLR_USE_NAMESPACE(antlr)TokenStream& lexer)
	: CPPParser(lexer)
	{
	myLog = 1;	// Can be used to produce selective MyCode output in
				//  filename.log for testing and/or verification between versions
				// 1 = standard log data designed to detect any significant change
				//		between one version and the next when run on the same data
				// Use higher numbers to provide any occasional extra data for 
				//  special testing
	}

// Destructor
MyCode::~MyCode()
	{
	}

// This function holds all processing for my application which needs 
//  to be done before parsing starts
void MyCode::myCode_pre_processing(int argc,char *argv[])
	{
	now = time(NULL);  // See Joy of C p.618
	function_number = 0;	
	current_fun_scope = 0;	// Zero indicates global scope here
	int pointer = 0;

	// To put the following printf output to a file use indirection
	//  as follows,
	// ...>CPP_parser programName.i (/a|/A) > programName.trace

	printf("------------------------------------------------------\n");
	printf("Program to demonstrate subclassing of application code\n");
	printf("Version 3.0 -  July 2004\n");
	printf("(c) London South Bank University 2004\n");
	printf("------------------------------------------------------\n");

	// check for the /a (process all files, ie. including header files)
	process_all_files = false;
	
	if (argc > 2)
		if (strcmp(argv[2], "/a") == 0 || strcmp(argv[2], "/A") == 0)
			process_all_files = true;
	
	// Strip extension from name of input file
	strcpy(my_input_file,argv[1]);
	pointer = strlen(my_input_file);
	while((my_input_file[pointer] != '.') && (pointer>0))
		{
		my_input_file[pointer] = '\0';
		pointer -= 1;
		}

	if (pointer<=0)
		{
		printf("Failed to find '.' in input file name %s\n",argv[1]);
		exit(1);
		}
		
	// Create demo data file
	strcpy(my_dat_file,my_input_file);
	my_dat_file[pointer+1] = 'd';
	my_dat_file[pointer+2] = 'a';
	my_dat_file[pointer+3] = 't';
	my_dat_file[pointer+4] = '\0';

	// Open and initialise data file to summarise application run
	printf("open data file to write... at %s\n",ctime(&now));
	myDatFilePtr = fopen(my_dat_file,"w");
	fprintf(myDatFilePtr,"Program to demonstrate subclassing of application code\n");
	fprintf(myDatFilePtr,"Version 3.0 -  July 2004\n");
	fprintf(myDatFilePtr,"(c) London South Bank University 2004\n\n");
	fprintf(myDatFilePtr,"%s\n",ctime(&now));
	fprintf(myDatFilePtr,"Function definitions found in %s\n",argv[1]);
	
	// Create functions list file
	strcpy(my_lis_file,my_input_file);
	my_lis_file[pointer+1] = 'l';
	my_lis_file[pointer+2] = 'i';
	my_lis_file[pointer+3] = 's';
	my_lis_file[pointer+4] = '\0';

	// This simulates application output for subsequent processing by 
	//  other programs
	myLisFilePtr = fopen(my_lis_file, "w");
	if (myLisFilePtr == NULL)
		{
		printf("Failed to create functions list file \"%s\"\n",my_lis_file);
		exit(1);
		}

	// Check if myLog set in MyCode constructor
	if (myLog)
		{
		// create demo log file
		strcpy(my_log_file,my_input_file);
		my_log_file[pointer+1] = 'l';
		my_log_file[pointer+2] = 'o';
		my_log_file[pointer+3] = 'g';
		my_log_file[pointer+4] = '\0';
		
		// This can be used to record static data between runs to enable
		//  checks to be carried out between different versions
		// Note: This file goes to same directory as *.i file came from
		myLogFilePtr = fopen(my_log_file, "w");
		if (myLogFilePtr == NULL)
			{
			printf("Failed to create log file \"%s\"\n", my_log_file);
			exit(1);
			}
		}

	}

// This function holds all processing for my application which needs 
//  to be done after parsing finished
void MyCode::myCode_post_processing()
	{
	printf("\nRun summary in %s\n",my_dat_file);
	printf("\nListed %d functions in %s\n",function_number,my_lis_file);

	if (myLog)
		{
		printf("\nAlso see list in %s\n",my_log_file);
		}

	// Update output data file
	fprintf(myDatFilePtr,"\nFunctions read %d\n",function_number);

	// close  files
	if (function_number>0)
		{
		if (myDatFilePtr != NULL)
			fclose(myDatFilePtr);
		if (myLisFilePtr != NULL)
			fclose (myLisFilePtr);
		if (myLogFilePtr != NULL && myLog)
			fclose (myLogFilePtr);
		}
	}

void MyCode::myCode_end_of_stmt()
	{
	//printf("MyCode::myCode_end_of_stmt entered\n");
	}

void MyCode::myCode_function_direct_declarator(const char *q)
	{	// q points to the name of a function being defined
	bool ignore_this_function = false;
	// Optional output for testing
	//printf("%d myCode_function_direct_declarator entered with %s\n",LT(1)->getLine(),q);
	// Optional to pause processing
	//printf("%d %d \n",process_all_files, in_user_file);
	//printf("Press enter to continue\n");
	//getchar();
	
	if (process_all_files)
		{
		// Optional output for verification between versions
		if (myLog)	
			fprintf(myLogFilePtr, "Function: %s (line %d)\n", q, LT(1)->getLine());
		}
	else
		if (in_user_file)
			{
			// Optional output for verification between versions
			if (myLog)
				fprintf(myLogFilePtr, "Function: %s (line %d)\n", q, LT(1)->getLine());
			}
		else
			{
			ignore_this_function = true;
			}

	// Count number of functions
	if (!ignore_this_function)
		function_number += 1;
	
	// Output for subsequent application processing
	if (!ignore_this_function)
		{
		fprintf(myLisFilePtr, "File Fun%d = function %s \n", function_number, q);
		}
	}