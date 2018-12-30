from targets.nicta import generic_n2_target as nictageneric_n2
from targets.idl4 import generic_biguuid_l4v4_target as idl4_biguuid

Generator = nictageneric_n2.Generator

class Templates(nictageneric_n2.Templates, idl4_biguuid.Templates):
	"""
	Interestingly, this multiple inheritance only works if the base template
	inherits from Object. (Because you get the entire inheritance tree from one
	of the base classes, depending on order, rather than having Python resolve
	the "diamond inheritance" properly). Try removing it and see the fun!
	"""
	pass
