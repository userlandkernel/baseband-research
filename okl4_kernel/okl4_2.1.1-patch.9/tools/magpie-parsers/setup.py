import os
from setuptools import setup, Extension, find_packages

CPPSOURCE = [os.path.join('src', 'magpieparsers', 'cplusplus', name) for name in ("cppparsemodule.cpp", "CPPLexer.cpp", "CPPParser.cpp", "Dictionary.cpp", "LineObject.cpp", "Support.cpp", "ASTNode.cpp")]

setup(
    name = "magpieparsers",
    version = "1",
    packages = find_packages('src'),
	package_dir = {'': 'src'},
	
	# C++ parser acceleration extension
	ext_modules=[Extension("magpieparsers.cppparse", CPPSOURCE,
		include_dirs = ['/usr/include', '/usr/local/include'],
		library_dirs = ['/usr/local/lib'],
		libraries = ['antlr'],
		language = "c++",
	)]
)
