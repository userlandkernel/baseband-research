"""
Implements the Nicta N2 ABI for Magpie.

Natural alignment for basic types up to wordsize (then word alignment)

Types inside a struct are packed

Structs are aligned to word size.
"""
#from magpieparsers.types.infogripper import find_alias_target

class N2ABI(object):
	# When marshalling, boxes 0 and 1 are reserved (for msgtag and uuid)
	client_marshal_box_start = 2
	# When unmarshalling, only msgtag is reserved (uuid is unnecessary)
	client_unmarshal_box_start = 1

	def __init__(self, arch_info):
		self.word_size = arch_info.word_size_in_bits

	def align(self, indirection, type_ast, parent_ast):
		# Set unknown alignment by default, for checking later.
		alignment = None

		# We align differently depending on whether we're in a struct.
		if parent_ast:
			parent_meta_type = parent_ast.get_single_attribute('meta_type')
		else:
			parent_meta_type = None

		if parent_ast is None:
			# No parent, so not in a struct (for example)
			if indirection:
				# Pointer types always aligned on word boundaries.
				alignment = self.word_size
			elif type_ast.get_single_attribute('meta_type') == 'basic':
				# Basic type at the root: natural alignment or word alignment,
				# whichever is smaller.
				alignment = type_ast.get_single_attribute('size')
				alignment = min(alignment, self.word_size)
			else:
				# Non-basic type at the root: word alignment.
				alignment = self.word_size
		elif parent_meta_type == 'struct':
			# We're in a struct; pack everything.
			alignment = 8
		elif parent_meta_type == 'union':
			# We're in a union - word alignment.
			alignment = self.word_size
		else:
			parent_ast.print_tree()
			raise Exception("Unknown parent ast type %s" % parent_ast)

		assert alignment is not None

		return alignment

