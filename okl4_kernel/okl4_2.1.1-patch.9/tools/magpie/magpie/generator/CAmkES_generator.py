from v4generator import V4Generator, V4Interface, V4Function

class CAmkESInterface(V4Interface):
	def _my_function_class(self):
		return CAmkESFunction

	def get_name(self):
		return self.ast.leaf

class CAmkESFunction(V4Function):
	def get_function_name(self):
		return self.ast.leaf

class CAmkESGenerator(V4Generator):
		
	def _my_interface_class(self):
		return CAmkESInterface
		
	def get_interfaces(self):
		# AST-walkin' time!
		interface_class = self._my_interface_class()
		magpieast = self.astinfo.ast
		assert magpieast is not None
		interfaces = []
		for idlfile in magpieast['IDLFile']:
			interfaces.extend(idlfile['interface'])
		#print 'defasts = ',defasts
		#if len(defasts) == 0:
		#	magpieast.print_tree()			
		for interface in interfaces:
			self.interface_map[interface.leaf] = [interface, interface]
			yield interface_class(interface_ast = interface,
					decorators_list = interface['decorator'],
					arch_info = self.arch_info,
					interface_map = self.interface_map)



def construct(output, ast):
	return CAmkESGenerator(output, ast)
