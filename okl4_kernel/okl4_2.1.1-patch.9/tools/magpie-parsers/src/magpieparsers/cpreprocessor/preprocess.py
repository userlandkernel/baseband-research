"""
C preprocessor interface

The two public functions are preprocess_file and preprocess_multi. Both
take files as their first argument: cpp_file takes a single string filename,
and cpp_multi takes a list of filenames.

Both accept the following optional arguments:
  cpp = 'cpp'  : The name of the C preprocessor to use
  options = '' : Options to pass to the preprocessor as an unparsed string
  include_dirs : A list of include directories, passed as -Idir1 -Idir2 etc
  defines      : A list of C defines, passed as -Ddefine1, -Ddefine2 etc
"""

import sys, popen2

def _cpp_get_pipe(filename = None, cpp = None, options = None, include_dirs = None,
		defines = None):
	# Set defaults for optional parameters
	if cpp is None:
		cpp = 'cpp'
	if options is None:
		options = []
	if include_dirs is None:
		include_dirs = []
	if defines is None:
		defines = []

	# Build the command line.
	cpp_command_line = ' '.join ([cpp] + options \
		+ ['-I%s' % (incdir) for incdir in include_dirs]
		+ ['-D%s' % (define) for define in defines]
	)
	# We accept an optional filename - it's good to have it because
	# cpp includes its output.
	if filename:
		cpp_command_line += ' %s' % (filename)

	# Run cpp.
	child_out, child_in = popen2.popen2(cpp_command_line)
	return child_out, child_in

def cpp_file(filename, **cpp_opts):
	if filename == '-':
		child_out, child_in = _cpp_get_pipe(**cpp_opts)
		child_in.write(sys.stdin.read())
	else:
		child_out, child_in = _cpp_get_pipe(filename = filename, **cpp_opts)
	child_in.close()
	return child_out.read()

def cpp_multi(self, filename_list, **cpp_opts):
	# Make a tiny C file containing all the filenames as included.
	cpp_file = ['#include "%s"' % (filename) for filename in filename_list]
	cpp_file = '\n'.join(cpp_file) + '\n'

	child_out, child_in = _cpp_get_pipe(**cpp_opts)
	child_in.write(cpp_file)
	child_in.close()
	return child_out.read()
