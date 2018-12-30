from magpieparsers.parser_common import *
from magpieparsers.types.infogripper import *

# AST helper functions
class ASTGen(object):
	def __init__(self):
		self.baseid = None
		self.name = None
		self.currentUUID = 0
		self.interface_node = None
		
	def convert(self,ast, pt, filename):
		self.translation_unit(ast, pt, filename)
		
	def translation_unit(self, basicast, pt, filename):
		ast = Node(basicast, "IDLFile")
		ast.leaf = filename
		basicast.add_child(ast)
		for statement in pt.children:
			node = statement.children[0]
			if node.type == 'subsystem':
				self.baseid = int(eval(node.the('subsystem_base').children[0].type))
				self.currentUUID = self.baseid
				self.name = node.the('subsystem_name').children[0].type
				
				def_node = Definition(ast, None)
				dec_node = Decorator(def_node, None)
				anno_node = Annotation(None, name = 'uuid')
				exp_node = Expression(None, name = self.currentUUID)
				exp_node.result = self.currentUUID
				
				anno_node.add_child(exp_node)
				dec_node.add_child(anno_node)
				def_node.add_child(dec_node)
				self.interface_node = Interface(ast, name=self.name)
				def_node.add_child(self.interface_node)
				ast.add_child(def_node)
			if node.type == 'type_decl':
				type_child = self.named_type_spec(ast, node.the('named_type_spec'))
				if self.interface_node != None:
					self.interface_node.add_child(type_child)
				else:
					ast.add_child(type_child)
				
			elif node.type == 'routine_decl':
				assert self.interface_node is not None
				self.routine_decl(ast, node)
				
			elif node.type == 'skip':
				self.currentUUID += 1
			elif node.type == 'import_decl':
				import_node = Node(ast, name = 'import')
				import_node.leaf = node.children[1].type
				indicant = node.the('import_indicant').children[0].type
				if indicant == 'uimport':
					import_node.add_attribute('side','client')
				if indicant == 'simport':
					import_node.add_attribute('side','server')
				import_node.add_attribute('MIG-indicant',indicant)
				ast.add_child(import_node)
			else:
				#print 'node = ', node.type
				ast.add_child(UnknownNode(None, node))
	
	def routine_decl(self, ast, pt):
		fcn_node = Function(ast, None)
		routine_node = pt.children[0]
		fcn_node.leaf = routine_node.the('identifier').children[0].type
		
		if routine_node.type == 'simple_routine':
			decorator = Decorator(fcn_node,None)
			anno = Annotation(None, name='oneway')
			decorator.add_child(anno)
			fcn_node.add_child(decorator)

		fcn_type = Node(fcn_node, None)
		fcn_type.type = 'return_type'
		#fcn_type.leaf = 'MACH_MSG_TYPE_INTEGER_32'
		target_type_node = getTypenode('MACH_MSG_TYPE_INTEGER_32', fcn_node)
		assert target_type_node is not None
		fcn_type.add_attribute('target_type', target_type_node)
		fcn_node.add_child(fcn_type)
					
		name = routine_node.children[0].type
		arg_node = routine_node.the('arguments')
		if arg_node != None:
			arg_node_list = self.argument_list(fcn_node, arg_node.the('argument_list'))	
			fcn_node.add_children(arg_node_list)
			
		self.interface_node.add_child(fcn_node)
	
	def argument_list(self,ast, pt):
		param_node_list = []
		arg_node = pt.the('argument')
		arg_list_node = pt.the('argument_list')
		if arg_node == None:
			trailer_node = self.create_mig_node(pt)
			
			#trailer_node.add_child(pt)
			param_node_list.append(trailer_node)
			if arg_list_node != None:
				param_node_list.extend(self.argument_list(arg_list_node))
			return param_node_list
		
		direction = arg_node.the('direction')
		name = arg_node.the('identifier').children[0].type
		type_name_node = arg_node.the('argument_type').children[0]
		ipc_flags = arg_node.the('ipc_flags')
		
		param_node = Node(ast, name='parameter')
		param_node.leaf = name

		if direction != None:
			param_node.add_attribute('direction', direction.children[0].type)
		
		if type_name_node.type == 'identifier':
			#instance = TypeInstance(param_node)
			#instance.leaf = type_name_node.children[0].type
			#param_node.add_child(instance)
			target_type_node = getTypenode(type_name_node.children[0].type, ast)
			assert target_type_node
			param_node.add_attribute('target_type',target_type_node)
		elif type_name_node.type == 'named_type_spec':
			type_node = self.named_type_spec(param_node, type_name_node)
			param_node_list.add_child(type_node)
			param_node.add_attribute('target_type', type_node)
			
		if ipc_flags != None:
			decorator = Decorator(None, None)
			for flag in ipc_flags.children:
				if flag.type == 'ipc_flag':
					anno_node = Annotation(None, name = flag.children[0].type)
					decorator.add_child(anno_node)
			param_node.add_child(decorator)
		param_node_list.append(param_node)
		if arg_list_node != None:
			param_node_list.extend(self.argument_list(ast, arg_list_node))
		return param_node_list
		
	def named_type_spec(self,ast, pt):
		alias_type = Type(ast, None)
		alias_type.leaf = pt.children[0].type
		alias_type.add_attribute('meta_type', 'alias')
		info = Info(alias_type)
		alias_type.add_child(info)
	
		
		#type_spec_node = None
		for child in pt.children:
			if child.type == 'trans_type_spec':
				for trans_type_child in child.children:
					if trans_type_child.type == 'type_spec':
						typenode, newtype = self.type_spec(info, trans_type_child)
						if newtype:
							info.add_child(typenode)
						info.add_attribute('target_type',typenode)
						
					elif trans_type_child.type == 'trans_type_spec_extended':
						assert typenode is not None
						info.add_child(self.create_mig_node(trans_type_child))
					else:
						pass
		return alias_type
		
	def int_exp(self, pt):
		if len(pt.children) == 3:
			expr_str = pt.children[0].type + pt.children[1].type + self.int_exp(pt.children[2])
		else:
			expr_str = pt.children[0].type
		try:
			return_value = str(int(eval(expr_str)))
		except Exception:
			return_value = expr_str
			
		return return_value
		
	def create_mig_node(self, pt):
		mignode = Node(None, None)
		mignode.type = 'special'
		mignode.leaf = 'MIG'
		if pt != None:
			mignode.add_child(pt)
		return mignode
	
	
	def type_spec(self,ast, type_spec_pt):
		pt = type_spec_pt.children[0]
		if pt.type == 'basic_type_spec':
			type_spec, newtype = self.basic_type_spec(ast, pt)
							
		elif pt.type == 'prev_type_spec':
			typename = pt.children[0].type
			type_spec = getTypenode(typename, ast)
			if type_spec is None:
				print 'search for %s was not successful in ast = %s' %(typename, ast)
			
			assert type_spec
			newtype = False
			
		elif pt.type == 'array_spec':
			type_spec, newtype = self.array_spec(ast, pt)
				
		elif pt.type == '^':
			type_spec, newtype = self.pointer_type_spec(ast, type_spec_pt)
			
		elif pt.type == 'struct_head':
			type_spec, newtype = self.struct_head_spec(ast, pt)
			
		elif pt.type == 'c_string_spec':
			type_spec, newtype = self.c_string_spec(ast, pt)
			
		elif pt.type == 'native_type_spec':
			type_spec, newtype = self.native_type_spec(ast, pt)
		else:
			type_spec = Unknown(None, pt)			
			newtype = True

		return (type_spec, newtype)
		
	def basic_type_spec(self, ast, pt):
		if len(pt.the('ipc_type').children) == 2:
			newType = Type(ast, None)
			#poly-type
			newType.add_attribute('meta_type', 'polymorphic')
			info = Info(newType)
			newType.add_child(info)
			
			# sender type
			worknode = pt.the('ipc_type').children[0]
			if worknode.the('symbolic_type') == None:
				sender = worknode.children[0].type
			else:
				sender = worknode.the('symbolic_type').children[0].type
			if sender.isdigit():
				target_type_node = self.create_anonymous_sized_node(sender)
			else:
				target_type_node = getTypenode(sender, ast)
			if target_type_node is None:
				print 'search for %s was not successful in ast = %s' %(sender, ast)
			info.add_attribute('sender_type',target_type_node)
			assert target_type_node is not None
			
			# receiver type
			worknode = pt.the('ipc_type').children[1]
			if worknode.the('symbolic_type') == None:
				receiver = worknode.children[0].type
			else:
				receiver = worknode.the('symbolic_type').children[0].type
			
			if receiver.isdigit():
				target_type_node = self.create_anonymous_sized_node(receiver)
			else:
				target_type_node = getTypenode(receiver, ast)
			if target_type_node is None:
				print 'search for %s was not successful in ast = %s' %(receiver, ast)
			info.add_attribute('receiver_type',target_type_node)
			assert target_type_node is not None
			
			
			returnlist =  (newType, True)
		else:
			worknode = pt.the('ipc_type').the('prim_ipc_type')
			if worknode.the('symbolic_type') == None:
				typename = worknode.children[0].type
			else:
				typename = worknode.the('symbolic_type').children[0].type
	
			if typename.isdigit():
				target_type_node = self.create_anonymous_sized_node(typename)
			else:
				target_type_node = getTypenode(typename, ast)
			if target_type_node is None:
				print 'search for %s was not successful in ast = %s' %(typename, ast)
			assert target_type_node is not None
			returnlist = (target_type_node, False)
			
		if len(pt.children) == 2:
			sizeType = Type(ast, None)
			sizeType.add_attribute('meta_type', 'sized_alias')
			info = Info(sizeType)
			sizeType.add_child(info)
			info.add_attribute('size', self.int_exp(pt.the('int_exp')))
			if returnlist[1]:
				sizeType.add_child(returnlist[0])
			returnlist = (sizeType, True)
			
		return returnlist


	def array_spec(self, ast, pt):
		typename = pt.children[0].type
		child = pt.children[0]
		var_array_node = pt.the('var_array_head')
		array_node = pt.the('array_head')
		
		if array_node != None:
			int_exp = self.int_exp(array_node.the('int_exp'))
			# in MIG only ONE dimension ... for more you have to cascade
			arrayType = Type(ast, None)
			arrayType.add_attribute('meta_type', 'array')
			info = Info(arrayType)
			arrayType.add_child(info)
			info.add_attribute('shape',[int_exp])
			array_type_spec_node, newtype = self.type_spec(ast, pt.the('type_spec'))
			info.add_attribute('target_type', array_type_spec_node)
			if newtype:
				info.add_child(array_type_spec_node)
			return arrayType, True
			
		elif var_array_node != None:
			arrayType = Type(ast, None)
			arrayType.add_attribute('meta_type', 'sequence')
			info = Info(arrayType)
			arrayType.add_child(info)
			if var_array_node.the('int_exp') != None:
					info.add_attribute('shape', [self.int_exp(var_array_node.the('int_exp'))])
			array_type_spec_node, newtype = self.type_spec(ast, pt.the('type_spec'))
			info.add_attribute('target_type', array_type_spec_node)
			if newtype:
				info.add_child(array_type_spec_node)
			return arrayType, True
			
		else:
			typenode = Type(ast, None)
			typenode.add_attribute('meta_type',  'sequence')
			info = Info(typenode)
			typenode.add_child(info)
			array_type_spec_node = self.type_spec(ast, pt.the('type_spec'))
			info.add_attribute('target_type', array_type_spec_node[0])
			return typenode, True
			
	def pointer_type_spec(self, ast, pt):
		newType = Type(ast, None)
		newType.add_attribute('meta_type','pointer')
		info = Info(newType)
		newType.add_child(info)
		target_type, isnewtype = self.type_spec(newType, pt.the('type_spec'))
		info.add_attribute('meta_type',target_type)
		if isnewtype:
			info.add_child(target_type)
		return newType, True
		
	def struct_head_spec(self, ast, pt):
		newType = Type(ast, None)
		newType.add_attribute('meta_type','struct')
		members = Node(newType, None)
		members.type = 'members'
		newType.add_child(members)
		#info = Info(newType)
		#newType.add_child(info)
		
		arrayType = Type(members, None)
		arrayType.add_attribute('meta_type','array')
		arrayInfo = Info(arrayType)
		arrayType.add_child(arrayInfo)
		
		int_exp = self.int_exp(pt.the('int_exp'))
		arrayInfo.add_attribute('shape',[int_exp])
		typenode, new_type = self.type_spec(ast, pt.the('type_spec'))
		arrayInfo.add_attribute('target_type',typenode)
		members.add_child(arrayType)

		typeinstance = TypeInstance(members)
		typeinstance.leaf = '_anonymous'
		typeinstance.add_attribute('target_type',arrayType)
		members.add_child(typeinstance)
		
		return newType, True		
	
	def c_string_spec(self, ast, pt):
		new_type = Type(ast, None)
		new_type.add_attribute('meta_type', 'string')
		info = Info(new_type)
		new_type.add_child(info)
		starnode = pt.the('*')
		int_exp_node = pt.the('int_exp')
		if int_exp_node != None:
			info.add_attribute('length', [int(eval(self.int_exp(int_exp_node)))])
		if starnode != None:
			info.add_attribute('variable', True)
		return new_type, True
		
	def native_type_spec(self, ast, pt):
		new_type = Type(ast, None)
		new_type.add_attribute('meta_type', 'MIG_unimplemented')
		info = Info(new_type)
		new_type.add_child(info)
		info.add_child(pt)
		return new_type, True
		
	def create_anonymous_sized_node(self, size):
		newType = Type(None, None)
		newType.add_attribute('meta_type', 'basic')
		info = Info(newType, None)
		info.add_attribute('size', int(eval(size)))
		newType.add_child(info)
		#newType.print_tree()
		return newType
		
					
def gen(pt, basicast, filename):
	astgen = ASTGen()
	astgen.convert(basicast, pt, filename)
	return basicast
			
			
			
			
			
			
			
