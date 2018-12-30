import sys
from magpie.targets.shared.options import options

def guess_word_size(hardware_arch):
	if hardware_arch == 'Generic 32':
		return 32
	elif hardware_arch == 'Generic 64':
		return 64
	else:
		print >>sys.stderr, "WARNING: Unknown hardware architecture. Assuming 32 bits per word."
		return 32

class ArchitectureInfo:
	ALIGN_NONE = -1
	ALIGN_NATURAL = -2

	def __init__(self):
		# FIXME: Obviously this should be configured somewhere
		self.word_size_in_bits = guess_word_size(options['hardware_arch'])
		self.granularity = 8
		self.mr_size = self.word_size_in_bits
		self.alignment = ArchitectureInfo.ALIGN_NATURAL
		
		self.safe_registers_in = range(2, 64) # FIXME
		self.safe_registers_out = range(1, 64) # FIXME
	
	def align_forward(self, bit, type_size):
		# Given a bit number and a type size in bits, return a new number
		# greater than or equal to "bit" reflecting the alignment of
		# a type of size type_size.
		if self.alignment == ArchitectureInfo.ALIGN_NONE:
			# No change
			return bit
		elif self.alignment == ArchitectureInfo.ALIGN_NATURAL:
			# Natural alignment of this word size.
			max_align = min(self.word_size_in_bits, type_size)
			if bit % max_align == 0:
				return bit
			else:
				return bit + (max_align - (bit % max_align))

class WordSizeAnnotator(object):
	def __init__(self, simple_params, arch_info):
		self.simple_params = simple_params
		self.arch = arch_info

	def _annotate_parameter_with_message_registers(self, param, current_bit, safe_registers):
		""" Annotate a particular parameter. 
		Called from (and conceptually internal to)
		annotate_parameters_with_message_registers.
		"""
		#print 'annotating parameter: %s with current_bit = %s' %(param, current_bit),
		current_param_size = param['idltype'].size_in_bits()
		#print 'parametersize = ', current_param_size
		if param['idltype'].size_in_bits() == 0:
			param['message_registers'] = []
			return current_bit # Void param or similar
		current_bit = self.arch.align_forward(current_bit, current_param_size)
		current_mr_idx = current_bit / self.arch.word_size_in_bits
		current_mr_offset = current_bit % self.arch.word_size_in_bits
		param['message_registers'] = [(safe_registers[current_mr_idx], current_mr_offset)]
		remaining_param_size = current_param_size - (self.arch.word_size_in_bits - current_mr_offset)
		while remaining_param_size > 0:
			current_mr_idx += 1
			param['message_registers'].append( (safe_registers[current_mr_idx], 0) )
			remaining_param_size -= self.arch.word_size_in_bits
		current_bit += current_param_size
		#print 'message_register: ', param['message_registers']
		return current_bit
		
	def _annotate_parameters_with_buffer_info(self, param, buffer_info):
		""" Annotate a particular parameter. 
		Called from (and conceptually internal to)
		annotate_parameters_with_message_registers.
		"""
		#print 'annotating parameter: %s with current_bit = %s' %(param, current_bit),
		current_param_size = param['idltype'].size_in_bits()
		#print 'parametersize = ', current_param_size
		if param['idltype'].size_in_bits() == 0:
			param['message_registers'] = []
			return None # Void param or similar
			
		# simple buffer-decision ... bin-packing
		current_buffer_idx = 0
		current_bit = self.arch.align_forward(buffer_info[current_buffer_idx]['current_bit'], current_param_size)
		buffer_size = buffer_info[current_buffer_idx]['wordsize'] * buffer_info[current_buffer_idx]['length']
		remaining_size = buffer_size - current_bit
		while  remaining_size < current_param_size:
			current_buffer_idx += 1
			current_bit = self.arch.align_forward(buffer_info[current_buffer_idx]['current_bit'], current_param_size)
			buffer_size = buffer_info[current_buffer_idx]['wordsize'] * buffer_info[current_buffer_idx]['length']
			remaining_size = buffer_size - current_bit
		
		current_mem_idx = current_bit / buffer_info[current_buffer_idx]['wordsize']
		current_mem_offset = current_bit % buffer_info[current_buffer_idx]['wordsize']
		param['message_registers'] = [buffer_info[current_buffer_idx], current_mem_idx, current_mem_offset] #[(safe_registers[current_mr_idx], current_mr_offset)]
		remaining_param_size = current_param_size - (buffer_info[current_buffer_idx]['wordsize'] - current_mem_offset)
		while remaining_param_size > 0:
			current_mem_idx += 1
			param['message_registers'].append( [buffer_info[current_buffer_idx], current_mem_idx, 0])
			#param['message_registers'].append( (safe_registers[current_mr_idx], 0) )
			remaining_param_size -= buffer_info[current_buffer_idx]['wordsize']
		current_bit += current_param_size
		#print 'message_register: ', param['message_registers']
		#return current_bit		
		
		
		
	def annotate(self):
		# Pick message registers for parameters. Not sure whether this should
		# be here or lower down. It needs lots of architecture-specific info,
		# but on the other hand the code itself is useful across all
		# architectures.
		# Annotation format:
		# ['message_registers'] = [(2, 0), (3, 0)] : two full MRs used
		# ['message_registers'] = [(2, 16)] : bits 16 to $param_word_size + 16 of mr2 used
		
		#current_bit_in = self.arch.safe_registrers_in
		#current_bit_out = self.arch.safe_registrers_out
		
		current_bit_in = 0; 
		current_bit_out = 0; 
		
		
		# The idea is simply to advance current_bit by the parameter's size in bits
		# and whenever we get a new parameter record the current bit indexes into
		# acceptable_registers. When we change parameters, increase current_bit
		# by the architecture's alignment value.
		
		for param in self.simple_params:
			assert param['direction'] in ('in', 'out', 'return') # No inout support yet.
			if param['direction'] == 'in':
				current_bit_in = self._annotate_parameter_with_message_registers(
						param, current_bit_in, self.arch.safe_registers_in)
			elif param['direction'] == 'out' or param['direction'] == 'return':
				current_bit_out = self._annotate_parameter_with_message_registers(
						param, current_bit_out, self.arch.safe_registers_out)
		'''				
		for param in self.simple_params:
			assert param['direction'] in ('in', 'out', 'return') # No inout support yet.
			if param['direction'] == 'in':
				current_bit_in = self._annotate_parameters_with_buffer_info(
						param, current_bit_in)
			elif param['direction'] == 'out' or param['direction'] == 'return':
				current_bit_out = self._annotate_parameters_with_buffer_info(
						param, current_bit_out)
		'''					

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
		for param in self.simple_params:
			if param['direction'] not in direction_list:
				continue
			for register, bits in param['message_registers']:
				register_map.append( {'mr': register, 'param': param, 'bits': bits} )
		return register_map
		
	def max_register_for(self, direction_list):
		max_register = 0
		for param in self.simple_params:
			if param['direction'] not in direction_list:
				continue
			for register in param['message_registers']:
				max_register = max(max_register, register[0])
		return max_register
