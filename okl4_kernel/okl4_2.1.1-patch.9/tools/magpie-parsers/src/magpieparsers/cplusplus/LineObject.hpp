/* 1999-2004 Version 3.0 July 2004
 * Modified by David Wigg at London South Bank University for CPP_parser.g
 *
 * See MyReadMe.txt for further information
 *
 * This file is best viewed in courier font with tabs set to 4 spaces
 */

#ifndef INC_LineObject_hpp__
#define INC_LineObject_hpp__

#include "antlr/config.hpp"
#include <string>

ANTLR_BEGIN_NAMESPACE(antlr)

class LineObject 
	{
private:
	//LineObject parent;
	ANTLR_USE_NAMESPACE(std)string source;
	int line;
	bool enteringFile;
	bool returningToFile;
	bool systemHeader;
	bool treatAsC;

public:
	LineObject();

/*
In CPParser.g file, we will use "
	LineObject newLineObject1;
	LineObject newLineObject2=newLineObject1;
	"
	instead of following function.

	LineObject(LineObject lobj)
		{
		parent = lobj.getParent();
		source = lobj.getSource();
		line = lobj.getLine();
		enteringFile = lobj.getEnteringFile();
		returningToFile = lobj.getReturningToFile();
		systemHeader = lobj.getSystemHeader();
		treatAsC = lobj.getTreatAsC();
		}
*/
	LineObject(ANTLR_USE_NAMESPACE(std)string src);

	void setSource(ANTLR_USE_NAMESPACE(std)string src);
	
	ANTLR_USE_NAMESPACE(std)string getSource();
 
//zuo: No use in CPParser.g file.
/*
	void setParent(LineObject par) 
	{
	parent = par;
	}

	LineObject getParent() 
	{
	return parent;
	}
 */

	void setLine(int l);
	int getLine();

	void newline();

	void setEnteringFile(bool v);
	bool getEnteringFile();

	void setReturningToFile(bool v);
	bool getReturningToFile();

	void setSystemHeader(bool v);
	bool getSystemHeader();

	void setTreatAsC(bool v);
	bool getTreatAsC() ;

//zuo:No use in CPParser.g file.
/*
	ANTLR_USE_NAMESPACE(std)string toString()
	{
	StringBuffer ret;
	ret = new StringBuffer("# " + line + " \"" + source + "\"");
	if (enteringFile) 
		{
		ret.append(" 1");
		}
	if (returningToFile)
		{
		ret.append(" 2");
		}
	if (systemHeader)
		{
		ret.append(" 3");
		}
	if (treatAsC)
		{
		ret.append(" 4");
		}
	return ret.toString();
	}
*/
	};

ANTLR_END_NAMESPACE

#endif //INC_LineObject_hpp__

