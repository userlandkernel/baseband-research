#!/usr/bin/env python

"""
Ensure the cache is valid by updating the internal version whenever a file has changed.

Format of the cache file:

<integer>

filename hash
filename hash
...
"""
import os
import sha

FILENAME = 'cache.version'

class Validator(object):
	def __init__(self):
		self.files = []
		self.newhashes = {}
		self.valid = True

		handle = file(FILENAME)
		self.load(handle)
		handle.close()

	def load(self, handle):
		self.version = int(handle.readline().strip())
		for line in handle:
			line = line.strip()
			if not line:
				continue

			if ':' in line:
				filename, filehash = line.split(':', 1)
			else:
				filename = line
				filehash = ''

			filename = filename.strip()
			filehash = filehash.strip()

			self.files.append(filename)

			if not os.access(filename, os.R_OK):
				raise Error("Couldn't find file %s" % filename)

			newhash = sha.new(file(filename).read()).hexdigest()
			self.newhashes[filename] = newhash

			if newhash != filehash:
				self.valid = False

	def update(self):
		if not self.valid:
			handle = file(FILENAME, 'w')
			print >>handle, self.version + 1
			print >>handle

			for filename in self.files:
				print >>handle, '%s: %s' % (filename, self.newhashes[filename])

			handle.close()
			return True
		else:
			return False


if __name__ == '__main__':
	validator = Validator()
	if validator.update():
		print FILENAME, "updated."
	else:
		print "No change."

