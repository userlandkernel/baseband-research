"""
"""
import os
import sys
import glob
import shutil
import imp
import subprocess

from pexpect import pexpect

TEST_MAKE_CMD = ('make', 'libnest_extras.a')
PISTACHIO_MAKE_CMD = 'make'
BOOTDISK_CMD = './support/makeboot.ia32.sh'
QEMU_CMD = 'qemu -nographic -fda %s'

ORIGDIR = os.getcwd()
MAGPIE_TOOLS = os.path.normpath(os.path.join(ORIGDIR, '../../../magpie-tools'))
MAGPIE = os.path.normpath(os.path.join(ORIGDIR, '../..'))
MAGPIE_PARSERS = os.path.normpath(os.path.join(ORIGDIR, '../../../magpie-parsers/src'))
MAGPIE_PYTHONPATH = '%s:%s' % (MAGPIE, MAGPIE_PARSERS)

TEST_CFLAGS = '-I%s/build/user/include ' % (ORIGDIR) + \
		'-I%s/include ' % (MAGPIE)

class Error(Exception):
	pass

class RunTests(object):
	"""
	Run one or more end-to-end tests.

	Tests are run like this:
	1. run_one_test is called with the name of a test corresponding to a 
	   dir in tests/
	2. All files in tests/$testname are copied to build/$testname and
	   'make libnest_extras.a' is run.
	3. The resulting library is copied to the Pistachio build directory, build/user/lib/.
	4. Pistachio userland is re-compiled to link against the new library
	5. A disk image is created using makeboot.ia32.sh
	6. Qemu is run on this disk image and the output is fed to the expect script
	   residing in the test directory (tests/$testname/expect.py)
	7. If the expect script does not raise an exception, the test passed.
	"""
	def run_one_test(self, testname):
		testdir = os.path.join('tests', testname)
		testbuilddir = os.path.join('build', testname)
		templatedir = os.path.join('pistachio', 'user', 'apps', 'nest_template')
		nestdir = os.path.join('pistachio', 'user', 'apps', 'nest')

		# Prepare test directory and create library.
		if os.path.exists(testbuilddir):
			shutil.rmtree(testbuilddir)
		os.mkdir(testbuilddir)
		# Copy all test files into build dir
		for pathname in glob.glob('%s/*' % (testdir)):
			destpath = os.path.join(testbuilddir, os.path.basename(pathname))
			shutil.copyfile(pathname, destpath)
		# Build 'libnest_extras.a'
		self.nest_extras_build(testbuilddir)

		# Copy 'libnest_extras.a' to Pistachio build dir.
		nest_extras_src = os.path.join(testbuilddir, 'libnest_extras.a')
		nest_extras_dst = os.path.join('build', 'user', 'lib', 'libnest_extras.a')
		shutil.copyfile(nest_extras_src, nest_extras_dst)

		# Build Pistachio userland with new files
		self.pistachio_rebuild_user()

		# Create a boot disk
		boot_pathname = self.bootdisk_create()

		# Import a test-specific script if necessary.
		imp_info = imp.find_module('expect', [testdir])
		testmodule = imp.load_module('expect', *imp_info)

		# Test the system
		cmdline = QEMU_CMD % (boot_pathname)
		child = pexpect.spawn(cmdline)

		# ... and use the test-specific script as well:
		testmodule.test(child)
	
	def nest_extras_build(self, build_dir):
		try:
			os.chdir(build_dir)

			# Create new environment with PYTHONPATH and updated PATH.
			env = os.environ.copy()
			env['PYTHONPATH'] = MAGPIE_PYTHONPATH
			env['PATH'] += ':' + MAGPIE_TOOLS
			env['CFLAGS'] = TEST_CFLAGS

			handle = subprocess.Popen(TEST_MAKE_CMD, env = env)
			result = handle.wait()
			if result:
				raise Error("Couldn't build test library (return code %d)" % result)
		finally:
			os.chdir(ORIGDIR)

	def pistachio_rebuild_user(self):
		try:
			os.chdir('build/user')

			# Remove the app to force the (dumb) build system to re-link it
			os.unlink('apps/nest/nest')

			result = os.spawnlp(os.P_WAIT, PISTACHIO_MAKE_CMD, PISTACHIO_MAKE_CMD)
			if result:
				raise Error("Couldn't build Pistachio userland")
		finally:
			os.chdir(ORIGDIR)

	def bootdisk_create(self):
		result = os.spawnlp(os.P_WAIT, BOOTDISK_CMD, BOOTDISK_CMD)
		if result:
			raise Error("Couldn't create bootdisk")

		return 'build/fdimage.img'

if __name__ == '__main__':
	if len(sys.argv) != 2:
		print "Usage: %s <testname>" % (sys.argv[0])
		sys.exit(1)

	runtests = RunTests()
	runtests.run_one_test(sys.argv[1])
	print "Test passed"

