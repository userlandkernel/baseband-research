/* 1999-2004 Version 3.0 July 2004
 * Modified by London South Bank University for CPP_parser.g
 *
 * See MyReadMe.txt for further information
 *
 * This file is best viewed in courier font with tabs set to 4 spaces
 * 
 * 2004
 * This object was originally created for one of our application projects
 *	and is not used very much within the parser. It has been left in for 
 *	the present because it could be useful to other users. If you do use
 *	it please let me know, otherwise it may get deleted in future versions.
 * David Wigg
 *	wiggjd@bcs.org.uk
 */

#include "LineObject.hpp"

ANTLR_USING_NAMESPACE(antlr)

LineObject::LineObject() 
	{
	source = "";
	line = 1;
	enteringFile = false;
    returningToFile = false;
    systemHeader = false;
    treatAsC = false;
	}


LineObject::LineObject( ANTLR_USE_NAMESPACE(std)string src)
	{
	source = src;
	}

void LineObject::setSource(ANTLR_USE_NAMESPACE(std)string src)
	{
	source = src;
	}

ANTLR_USE_NAMESPACE(std)string LineObject::getSource()
	{
	return source;
	}
 
//zuo:No use in CPParser.g file.
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

void LineObject::setLine(int l)
	{
	line = l;
	}

int LineObject::getLine() 
	{
	return line;
	}

void LineObject::newline()
	{
	line++;
	}

void LineObject::setEnteringFile(bool v) 
	{
	enteringFile = v;
	}

bool LineObject::getEnteringFile()
	{
	return enteringFile;
	}

void LineObject::setReturningToFile(bool v) 
	{
	returningToFile = v;
	}

bool LineObject::getReturningToFile()
	{
	return returningToFile;
	}

void LineObject::setSystemHeader(bool v) 
	{
	systemHeader = v;
	}

bool LineObject::getSystemHeader() 
	{
	return systemHeader;
	}

void LineObject::setTreatAsC(bool v) 
	{
	treatAsC = v;
	}

bool LineObject::getTreatAsC() 
	{
	return treatAsC;
	}

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


