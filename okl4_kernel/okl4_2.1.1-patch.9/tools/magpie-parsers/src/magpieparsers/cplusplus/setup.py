from distutils.core import setup, Extension

SOURCE = ["cppparsemodule.cpp", "CPPLexer.cpp", "CPPParser.cpp", "Dictionary.cpp", "LineObject.cpp", "Support.cpp", "ASTNode.cpp"]

setup(name="cppparse", version="1",
		ext_modules=[Extension("cppparse", SOURCE,
			include_dirs = ['/usr/include'],
			libraries = ['antlr'],
			language = "c++",
		)])
