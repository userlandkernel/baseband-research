from output.v4 import V4Output

# FIXME: Refer to ArchitectureInfo currently in V4 generator -
# should use that struct rather than the hardcoded stuff below (simply so it's
# only stored in one spot)
ASM_HIGHEST_MESSAGE_REGISTER = 4 # 4 is correct as per L4 manual, other values for debugging
ARM_REGISTER_SIZE = 32 # FIXME?

class ArmBiguuidOutput(V4Output):
	def __init__(self, options):
		V4Output.__init__(self, options)
		self.context.file.arch_string = 'arm_biguuid'
		self.debug = False

	def gen_service_v4_main(self):
		self.fromtemplate('v4_generic_biguuid/service.functions.generic_biguuid.v4.main.template.c')
		self.fromtemplate('v4_arm_biguuid/service.functions.v4.send_reply.template.c')
	
	# Generic_biguuid functions
	def gen_imports(self):
		self.fromtemplate('v4_arm_biguuid/import.template.c')

	def gen_idl4_defines(self):
		V4Output.gen_idl4_defines(self)
		# Include the __L4_SYSCALL macro
		self.fromtemplate('v4_arm_biguuid/arm_ipc.template.c')

	def dbg(self, s):
		if self.debug:
			print s

	def gen_client_v4_function(self, function_ast):
		# FIXME: Until the asm template is written to deal with method calls
		# taking more than 4 parameters (it has to copy them to the utcb) we
		# currently defer to the generic method.
		# FIXME: If any of the input parameters, output parameters, or return
		# type won't fit in a register, we currently defer to the generic
		# method.
		slow_path = False

		# Pagefault client is slow path right now.
		if 'idl4_pagefault' in self.context.function.modifiers:
			slow_path = True
		#if function_ast.the('op_declaration').the('op_return_type').get_annotation().size_in_bits_inst() \
		#		> ARM_REGISTER_SIZE:
		#	slow_path = True
		parameter_list =\
				function_ast['parameter']
		for param in parameter_list:
		#	self.dbg(param)
		#	self.dbg("size is %s" % (param.get_annotation().size_in_bits_inst()))
			if param.get_annotation().size_in_bits_inst() > ARM_REGISTER_SIZE:
				slow_path = True
		max_register_both_ways = max(self.context.function.max_register_in,
				self.context.function.max_register_out)
		if max_register_both_ways > ASM_HIGHEST_MESSAGE_REGISTER:
			slow_path = True
		
		if slow_path:
			self.fromtemplate('v4_generic_biguuid/client.function.generic_biguuid.v4.template.c', ast = function_ast)
		else:
			# Asm function instead.
			self.fromtemplate('v4_arm_biguuid/client_function.template.c', ast=function_ast)

def construct(options):
	return ArmBiguuidOutput(options)

