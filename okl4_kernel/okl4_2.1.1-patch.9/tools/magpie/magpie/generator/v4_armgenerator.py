from v4generator import V4Generator, V4Interface, V4Function

class V4ArmInterface(V4Interface):
	def _my_function_class(self):
		return V4ArmFunction

class V4ArmFunction(V4Function):
	def get_max_register(self, direction_list):
		max_register = 0
		for param in self.simplified_params:
			if param['direction'] not in direction_list:
				continue
			for register in param['message_registers']:
				max_register = max(max_register, register[0])
		return max_register
	
	def get_register_map(self, direction_list):
		# Looks at the "message_registers" annotation for each simplified param
		# and creates a new list mapping message registers to parameters.
		# Format is: [ (mr, simplified_param, word_number) ]
		# where mr is the L4 message register number
		#       simplified_param is the simplified_parameter list
		#       word_number says "if the parameter is viewed as a series of
		#           consecutive words, place this word in the specified
		#           message register". If it is None, the whole parameter
		#           is placed in the message register.
		register_map = []
		for param in self.simplified_params:
			if param['direction'] not in direction_list:
				continue
			for register, bits in param['message_registers']:
				register_map.append( {'mr': register, 'param': param, 'bits': bits,
						'regnum': self._mr_to_hwreg(register)} )
		return register_map
	
	def get_registers_map_in(self):
		return self.get_register_map(self.directions_in)
	
	def get_registers_map_out(self):
		return self.get_register_map(self.directions_out)

	def get_registers_clobbered(self):
		used_registers = [item['regnum'] for item in self.get_register_map(self.directions_all)]
		clobbered_registers = [item for item in [5, 6, 7] if item not in used_registers]
		return clobbered_registers

	def _or_or_equals(self, bits_num):
		# If bits_num == 0 we assign the message register
		# If bits_num != 0 it's already got stuff in it; we logically OR it instead.
		if bits_num == 0:
			return '='
		else:
			return '|='

	def _shift_bits(self, bits_num):
		# if bits_num == 0, do nothing (we don't need to do any shifting)
		# Otherwise return the correct shift amount.
		if bits_num == 0:
			return ''
		else:
			return '<< %d' % (bits_num)
	
	def _mr_to_hwreg(self, mr_num):
		return mr_num + 3

class V4ArmGenerator(V4Generator):
	def _my_interface_class(self):
		return V4ArmInterface


def construct(output, ast):
	return V4ArmGenerator(output, ast)
