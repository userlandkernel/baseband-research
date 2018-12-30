
def list_contains(thelist, elements):
	for element in elements:
		if element in thelist:
			return True
	return False
	
def normalise_type_list(name_list):
	"""
	Return a canonical C type name

	We do this because many type names are synonyms in C (eg "signed long long int" and
	"long long int").

	If the type is appropriate, the function:
	  * Adds "signed" or "unsigned" to the front
	  * Adds "int" to the end (so "long" becomes "long int")
	  * Sorts the names in order of length
	
	It thus returns the maximum-length name for any type. EG "long" becomes
	"signed long int".

	name_list is not touched.
	"""
	working_list = name_list[:]
	new_list = []

	if list_contains(working_list, ('long', 'short', 'signed', 'unsigned')) \
			and not list_contains(working_list, ('int', 'char', '__int64')):
		working_list.append('int')
	
	if 'unsigned' not in working_list and 'signed' not in working_list:
		working_list.insert(0, 'signed')

	# Bail if the type isn't one we understand
	order = ('signed', 'unsigned', 'long', 'short', 'int', 'char', '__int64')
	for name in order:
		while name in working_list:
			# Append and remove so as to correctly handle duplicated names
			# such as "long long"
			new_list.append(name)
			working_list.remove(name)
	
	if working_list:
		# Still some names left - means we probably shouldn't have touched the type
		return name_list
	else:
		return new_list
