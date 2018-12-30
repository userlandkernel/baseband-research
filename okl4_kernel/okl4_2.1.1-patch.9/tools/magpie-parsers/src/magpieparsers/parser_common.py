#
# Australian Public Licence B (OZPLB)
# 
# Version 1-0
# 
# Copyright (c) 2004 National ICT Australia
# 
# All rights reserved. 
# 
# Developed by: Embedded Real-time and Operating Systems Group (ERTOS)
#               National ICT Australia
#               http://www.ertos.nicta.com.au
# 
# Permission is granted by National ICT Australia, free of charge, to
# any person obtaining a copy of this software and any associated
# documentation files (the "Software") to deal with the Software without
# restriction, including (without limitation) the rights to use, copy,
# modify, adapt, merge, publish, distribute, communicate to the public,
# sublicense, and/or sell, lend or rent out copies of the Software, and
# to permit persons to whom the Software is furnished to do so, subject
# to the following conditions:
# 
#    # Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimers.
# 
#    # Redistributions in binary form must reproduce the above
#       copyright notice, this list of conditions and the following
#       disclaimers in the documentation and/or other materials provided
#       with the distribution.
# 
#    # Neither the name of National ICT Australia, nor the names of its
#       contributors, may be used to endorse or promote products derived
#       from this Software without specific prior written permission.
# 
# EXCEPT AS EXPRESSLY STATED IN THIS LICENCE AND TO THE FULL EXTENT
# PERMITTED BY APPLICABLE LAW, THE SOFTWARE IS PROVIDED "AS-IS", AND
# NATIONAL ICT AUSTRALIA AND ITS CONTRIBUTORS MAKE NO REPRESENTATIONS,
# WARRANTIES OR CONDITIONS OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
# BUT NOT LIMITED TO ANY REPRESENTATIONS, WARRANTIES OR CONDITIONS
# REGARDING THE CONTENTS OR ACCURACY OF THE SOFTWARE, OR OF TITLE,
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NONINFRINGEMENT,
# THE ABSENCE OF LATENT OR OTHER DEFECTS, OR THE PRESENCE OR ABSENCE OF
# ERRORS, WHETHER OR NOT DISCOVERABLE.
# 
# TO THE FULL EXTENT PERMITTED BY APPLICABLE LAW, IN NO EVENT SHALL
# NATIONAL ICT AUSTRALIA OR ITS CONTRIBUTORS BE LIABLE ON ANY LEGAL
# THEORY (INCLUDING, WITHOUT LIMITATION, IN AN ACTION OF CONTRACT,
# NEGLIGENCE OR OTHERWISE) FOR ANY CLAIM, LOSS, DAMAGES OR OTHER
# LIABILITY, INCLUDING (WITHOUT LIMITATION) LOSS OF PRODUCTION OR
# OPERATION TIME, LOSS, DAMAGE OR CORRUPTION OF DATA OR RECORDS; OR LOSS
# OF ANTICIPATED SAVINGS, OPPORTUNITY, REVENUE, PROFIT OR GOODWILL, OR
# OTHER ECONOMIC LOSS; OR ANY SPECIAL, INCIDENTAL, INDIRECT,
# CONSEQUENTIAL, PUNITIVE OR EXEMPLARY DAMAGES, ARISING OUT OF OR IN
# CONNECTION WITH THIS LICENCE, THE SOFTWARE OR THE USE OF OR OTHER
# DEALINGS WITH THE SOFTWARE, EVEN IF NATIONAL ICT AUSTRALIA OR ITS
# CONTRIBUTORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH CLAIM, LOSS,
# DAMAGES OR OTHER LIABILITY.
# 
# If applicable legislation implies representations, warranties, or
# conditions, or imposes obligations or liability on National ICT
# Australia or one of its contributors in respect of the Software that
# cannot be wholly or partly excluded, restricted or modified, the
# liability of National ICT Australia or the contributor is limited, to
# the full extent permitted by the applicable legislation, at its
# option, to:
# a.  in the case of goods, any one or more of the following:
# i.  the replacement of the goods or the supply of equivalent goods;
# ii.  the repair of the goods;
# iii. the payment of the cost of replacing the goods or of acquiring
#  equivalent goods;
# iv.  the payment of the cost of having the goods repaired; or
# b.  in the case of services:
# i.  the supplying of the services again; or
# ii.  the payment of the cost of having the services supplied again.
# 
# The construction, validity and performance of this licence is governed
# by the laws in force in New South Wales, Australia.
#
import sys

import traceback

exception_if_incomplete = False # FIXME: Set to True for final.

class Error(Exception):
	pass

class NoDefault(object):
	pass

class ParserLexerState:
	def __init__ (self):
		self.typedefs = []
		self.constants = {}
		self.in_typedef = False
	
	def set_registry(self, type_registry):
		self.type_registry = type_registry
	
	def addTypedef (self, identifier):
		raise NotImplementedError() # Obsolete
		self.typedefs.append (identifier)

	def addModifier (self, identifier):
		self.modifiers.append (identifier)
	
	def isModifier (self, identifier):
		return identifier in self.modifiers

	def isTypedef (self, identifier):
		if identifier in self.typedefs:
			return True # FIXME: deprecated
		if self.type_registry.has_key(identifier):
			return True
		return False
	
	def add_constant(self, name, const_type=None, const_value=None):
		self.constants[name] = {'type': const_type, 'value': const_value}
	
	def is_constant(self, name):
		# FIXME: Also check for constants declared in the type registry
		if name in self.constants:
			return True # FIXME: is this ever used?
		elif self.type_registry.has_key(name):
			# FIXME: This isn't necessarily a const...
			return True
		return False
	
	def get_constant_names(self):
		raise NotImplementedError('obsolete')
		return self.constants.keys()
	
	def get_constant_value(self, constant_name):
		return self.constants[constant_name]['value']
	
def print_tree (node, depth=None, parent_name=None, handle = None, print_lineinfo = True, seen_nodes = None, hide_hidden = False):
	if handle is None:
		handle = sys.stdout

	if depth is None:
		depth = 0
	
	if seen_nodes is None:
		seen_nodes = {}

	if node in seen_nodes:
		print >>handle, ' ' * depth, node.type, node.leaf, "(backref)"
		return
	seen_nodes[node] = True

	if node is None:
		print >>handle, ' ' * depth, '* NONE (Unfinished tree, parent is %s)' % (parent_name)
	elif (not node.nonprinting) or (hide_hidden is False):
		print >>handle, ' ' * depth, node.name,
		result = ''
		if hasattr(node, 'result') and node.result is not None:
			result = '= %s' % (node.result)
		if node.leaf is not None:
			print >>handle, node.leaf, result,
		else:
			print >>handle, result,

		if node.value is not None:
			print >>handle, '== %s' % (node.value),

		if hasattr(node, 'get_annotation'):
			annotation = node.get_annotation()
			if annotation:
				print >>handle, "[Ann: %s]" % (annotation),

		if print_lineinfo and hasattr(node, 'get_line_info'):
			print >>handle, '(%d, %s)' % (depth, node.get_line_info())
		else:
			print >>handle, '(%d)' % (depth)

		if hasattr(node, 'attributes'):
			for attr_name in node.attributes:
				print >>handle, ' ' * depth, ' (%s) =' % (attr_name), node.attributes[attr_name]
		for child in node.children:
			print_tree (child, depth+1, parent_name=node.name, handle = handle, seen_nodes = seen_nodes,
					hide_hidden = hide_hidden)

def _type_pl(x):
	if isinstance(x, list):
		assert len(x) == 1
		return _type_pl(x[0])
	elif isinstance(x, basestring):
		return '"%s"' % (x)
	elif x is None:
		return 'none' # prolog atom instead
	else:
		return x

def _print_attribute_pl(prefix_name, attr_name, value):
	key_name = '%s_%s' % (prefix_name, attr_name)
	print 'ast_node(%s, "%s").' % (key_name, attr_name)
	value_name = '%s_%s_val' % (prefix_name, attr_name)
	print 'ast_node(%s, %s).' % (value_name, _type_pl(value))

def _print_attributes_pl(unique_name, node):
	if node.leaf:
		_print_attribute_pl(unique_name, "leaf", node.leaf)
	for key, value in node.attributes.items():
		_print_attribute_pl(unique_name, key, value)

def _print_attrparent_pl(prefix_name, attr_name):
	key_name = '%s_%s' % (prefix_name, attr_name)
	print 'ast_parent(%s, %s).' % (prefix_name, key_name)
	value_name = '%s_%s_val' % (prefix_name, attr_name)
	print 'ast_parent(%s, %s).' % (key_name, value_name)
	
def _atomify(s):
	s = s.lower()
	return s.replace(' ', '_')

def _unique_name_pl(node, seen_nodes):
	sr_list = []

	while node:
		if node.leaf and isinstance(node.leaf, basestring):
			sr_list.append(_atomify(node.leaf))
		sr_list.append(_atomify(node.name))
		node = node.parent
	
	sr_list.reverse()
	sr_base = '_'.join(sr_list)
	
	sr_counter = 1
	sr_candidate = sr_base

	while sr_candidate in seen_nodes:
		sr_counter += 1
		sr_candidate = '%s_%s' % (sr_base, sr_counter)

	return sr_candidate

def _print_nodes_pl(node, caller_id = None, seen_nodes = None):
	""" Print a tree suitable for import into Prolog """
	if seen_nodes is None:
		seen_nodes = {}
	
	if node not in seen_nodes:
		unique_name = _unique_name_pl(node, seen_nodes)
		seen_nodes[node] = unique_name

		print 'ast_node(%s, "%s").' % (unique_name, node.name)
		_print_attributes_pl(unique_name, node)

		for child in node.children:
			_print_nodes_pl(child, caller_id = unique_name, seen_nodes = seen_nodes)
	
	return seen_nodes

def _print_parents_pl(node, seen_nodes, parents_seen_nodes = None):
	if parents_seen_nodes is None:
		parents_seen_nodes = {}

	if node not in parents_seen_nodes:
		parents_seen_nodes[node] = True

		unique_name = seen_nodes[node]
		for child in node.children:
			print "ast_parent(%s, %s)." % (unique_name, seen_nodes[child])
			_print_parents_pl(child, seen_nodes, parents_seen_nodes)
		
		if node.leaf:
			_print_attrparent_pl(unique_name, 'leaf')
		
		for key in node.attributes.keys():
			_print_attrparent_pl(unique_name, key)

def print_tree_pl(node, caller_id = None, seen_nodes = None):
	seen_nodes = _print_nodes_pl(node)
	print
	_print_parents_pl(node, seen_nodes)



class Node(object):
	def __init__ (self,parent, name,  children=None, leaf=None, result=None, source_line = None, source_file = None, source = None, value = None):
		self.type = name
		self.attributes = {}
		self.parent = parent
		self.left_sibling = None
		self.right_sibling = None
		self.first_child = None
		self.last_child = None
		self.value = value
		self.children = []
		self.nonprinting = False
		
		if source:
			self._source_line = source.source_line
			self._source_file = source.source_file
		else:
			self._source_line = source_line
			self._source_file = source_file
			
		if children:
			self.add_children(children)

		try:
			for child in self.children:
				if not isinstance(child, Node):
					print "Node was trying to create itself but encountered a child of type", type(child)
					print "Node repr", repr(child)
					print "I am named %s with a leaf of leaf %s" % (name, leaf)
					print "RRRR"
					assert isinstance(child, Node)
					child.parent = self
		except:
			print type(self.children), self.children
			raise
		
		self.leaf = leaf
		if exception_if_incomplete is True:
			if children and None in children:
				print "Children up to this point:"
				print_tree(self)
				raise Exception("One of my children is missing! type is %s, children so far are %s" % (name, children) )
		self._annotation = None
		self.result = result

	def copy(self):
		result = Node(self.parent, self.name, leaf = self.leaf, result = self.result,
				source_line = self.source_line, source_file = self.source_file)
		for child in self.children:
			result.children.append(child.copy())
		for attribute in self.attributes.keys():
			val = self.attributes[attribute]
			assert len(val) == 1
			result.add_attribute(attribute, val[0])
		return result
		
	def get_name(self):
		return self.type
	
	def set_name(self, val):
		self.type = val
	
	name = property(get_name, set_name)

	def get_line_info(self):
		f = self.source_file
		if not f:
			f = '?'

		l = self.source_line
		if not l:
			l = '?'

		return '%s:%s' % (f, l)

	def get_source_file(self):
		return self._source_file
	source_file = property(get_source_file)
	
	def get_source_line(self):
		return self._source_line
	source_line = property(get_source_line)
		
	def getText(self):
		# for antlr
		return self.type
	
	def __str__ (self):
		return '<Node %s:%s>' % (self.type, self.leaf)
	__repr__ = __str__


	def add_child(self, child):
		if not child.parent:
			child.parent = self

		self.children.append(child)
		if self.first_child == None:
			self.first_child = child
		else:
			self.last_child.right_sibling = child
	
		child.left_sibling = self.last_child
		

		self.last_child = child
	
	def add_front(self, node):
		" Add the Node to the front of the list of children"
		# FIXME: Doesn't update first and last child
		self.children.insert(0, node)
		
	def remove_child(self, child):
		if child not in self.children:
			return False
		
		child.parent = None
		self.children.remove(child)
		
		if child.left_sibling is not None:	
			child.left_sibling.right_sibling = child.right_sibling
		if child.right_sibling is not None:
			child.right_sibling.left_sibling = child.left_sibling
		
		if self.last_child == child:
			self.last_child = child.left_sibling
		if self.first_child == child:
			self.first_child = child.right_sibling		
		child.left_sibling = child.right_sibling = None
		return True
		
		

	def add_children(self, children):
		for child in children:
			if child is None:
				print 'NoneType-child in list: ', children
				raise Exception()
			else:
				self.add_child(child)
	
	def child(self):
		assert len(self.children) == 1
		return self.children[0]

	def set_leaf(self, leaf):
		self.leaf = leaf
	
	def extend_leaf(self, leaf):
		if self.leaf is None:
			self.leaf = leaf
		else:
			self.leaf += leaf
	
	def set_result(self, result):
		self.result = result
	
	def set_value(self, value):
		self.value = value
	
	def extend_value(self, value):
		self.value += value
	
	def print_tree(self, handle = None, hide_hidden = False):
		print_tree(self, handle = handle, hide_hidden = hide_hidden)
		
	def scope(self):
		raise NotImplementeError()
		# FIXME: remove this
#		parent = self.parent
#		scope_list = []
#		while parent != None:
#			if parent.type in SCOPES:
#				scope_list.append(perent)
#			parent = parent.parent
#		scope_list.reverse()
#		return scope_list
		

	def my_children_are(self, *child_names, **kwargs):
		maxkids = kwargs.get('maxkids', len(child_names))
		minkids = kwargs.get('minkids', len(child_names))

		if len(self.children) < minkids or len(self.children) > maxkids:
			return False

		for child, wanted_name in zip(self.children, child_names):
			if child.type != wanted_name:
				return False
		return True
	
	def add_attribute(self, key, value):
		"""
		"""
		#if key == 'target_type' and isinstance(value, Node) and value.leaf is None:
		#	print "BLAH NONE TARGET TYPE AAAAARGH"
		#	value.print_tree()
		#	raise Exception()
		if not isinstance(value, list):
			value = [value]
		if self.attributes.has_key(key):
			self.attributes[key].extend(value)
		else:
			self.attributes[key] = value
	
	def has_attribute(self, key):
		return self.attributes.has_key(key)
	
	def get_attribute(self, key, default = NoDefault):
		if default is NoDefault:
			if self.has_attribute(key):
				return self.attributes[key]
			else:
				return None
		else:
			return self.attributes.get(key, default)

	def del_attributes(self, key):
		del self.attributes[key]
	
	def get_single_attribute(self, key, default = NoDefault):
		if default is NoDefault:
			attrs = self.attributes[key]
		else:
			attrs = self.attributes.get(key, [default])
		assert len(attrs) == 1
		return attrs[0]
	attribute = get_single_attribute

	def get_child_names(self):
		return [child.type for child in self.children]
	
	def get_left_sibling(self):
		return self.left_sibling
	
	def get_right_sibling(self):
		return self.right_sibling
		
		
	def the(self, keylist):
		""" Return the only child with a type matching any in <keylist>
		Usage: somenode.the('childnode')
		Returns: a Node iff there is a single child
		Returns: None iff there are no children.
		Raises Exception iff there is more than one matching child."""
		if type(keylist) is not list:
			keylist = [keylist]
		suitableChildren = []
		for key in keylist:
			for child in self.children:
				if child.type == key:
					suitableChildren.append(child)
		if len(suitableChildren) > 1:
			print "Whoops"
			self.print_tree()
			raise Exception("Too many children for me. I am %s, wanted %s" % (self.type, keylist))
		if len(suitableChildren) == 0:
			return None
		return suitableChildren[0]
	
	get_child = the
	
	def find_single_child(self, keylist):
		return self.the(keylist)
	
	def has_child(self, child_name):
		for child in self.children:
			if child.name == child_name:
				return True
		return False

	def find_node(self, node_type, node_leaf, negative_list = []):
		#negative_list is used to prevent descending into scopes
		
		#print "trying to find %s [%s] without descending into one of these node-types: %s" %( node_type, node_leaf, negative_list)
		
		#if node_type == 'interface':
		#	print node_type, node_leaf, negative_list
		type_ok, leaf_ok = self._check_node(node_type, node_leaf)
		if type_ok and leaf_ok:
			return self
		else:
			for child in self.children:
				if not child.type in negative_list:
					#print 'child is not in negative_list: ', child.type
					result = child.find_node(node_type, node_leaf, negative_list)
					if result != None:
						return result
				else:
					#print 'child IS in negative_list: ', child
					#print node_type, '   ', node_leaf
					type_ok, leaf_ok = child._check_node(node_type, node_leaf)
					#print type_ok, leaf_ok
					if type_ok and leaf_ok:
						return child
					
			return None
			
	def _check_node(self, node_type, node_leaf):
		if node_type == None:
			type_ok = True					
		else:
			if isinstance(node_type, list):
				#print '1self.type = ', self.type
				if self.type in node_type:
					type_ok = True					
				else:
					type_ok = False
			else:
				#print '2self.type = ', self.type, '  ', node_type
				if self.type == node_type:
					type_ok = True					
				else:
					type_ok = False
		if node_leaf == None:
			leaf_ok = True					
		else:
			if isinstance(node_leaf, list):
				if self.leaf in node_leaf:
					leaf_ok = True					
				else:
					leaf_ok = False
			else:
				if self.leaf == node_leaf:
					leaf_ok = True					
				else:
					leaf_ok = False	
		return type_ok, leaf_ok
		
		
	def maybe_walk_to(self, *args):
		"""Return the descendant matching the list supplied in *args.
		Usage: somenode.maybe_walk_to('child', 'grandchild', ...)
		Returns: a Node if the descendants could be walked to a matching Node.
		Returns None, otherwise."""
		current = self
		for target in args:
			current = current.the(target)
			if current is None:
				return None
		return current

	def __getitem__ (self, key):
		""" Return a list of all children with a type matching <key> """
		suitableChildren = []
		for child in self.children:
			if child.type == key:
				suitableChildren.append (child)
		return suitableChildren
	
	get_children_named = __getitem__
	
	def annotate(self, annotation):
		assert self._annotation is None # Don't - yet - allow multiple annotations
		self._annotation = annotation
	
	def get_annotation(self):
		return self._annotation
	
	def traverse(self):
		pos = [self]
		while pos:
			yield pos[-1]
			pos.extend(pos.pop().children)

	def search(self, *search_terms):
		results = []
		self._search(results, search_terms)
		return results
	
	def _search(self, results, search_terms):
		for child in self.children:
			if self._search_matches(child, search_terms[0]):
				if len(search_terms) == 1:
					results.append(child)
				else:
					child._search(results, search_terms[1:])

	def _search_matches(self, ast, term):
		if ast.type != term[0]:
			return False
		if len(term) >= 2:
			for key in term[1]:
				attribute = ast.get_attribute(key)
				if not attribute:
					return False
				elif attribute[0] != term[1][key]:
					return False
		return True

class TypeNode(Node):
	def __init__(self, parent, *args, **kwargs):
		Node.__init__(self, parent,  'type', *args, **kwargs)
	
	def add_instances(self, *instances):
		self.add_attribute('instances', instances)
	
	def get_basic_type(self):
		# Walk through the type tree until we get to a type with meta_type "basic".
		if self.get_attribute('meta_type') == 'basic':
			return self
		else:
			target = self.the('info').get_attribute('target')
			return target.get_basic_type()

class UnknownNode(Node):
	def __init__(self, parent,  pt, name = "Unknown", **kwargs):
		Node.__init__(self, parent, 'Unknown_%s' % (name), **kwargs)
		self.children = [pt]
	
	# Dummy methods so we can use this class everywhere
	def add_instances(self, *args):
		pass 

class UnknownLeaf(object):
	def __init__(self, parent,  pt = None):
		self.pt = pt
	
	def __repr__(self):
		return '<Unknown leaf; PT is %s>' % (self.pt)

def UnfinishedNode(name, source = None):
	return Node(None, name, leaf = 'UNFINISHED', source = source)

def ExpressionNode(type, children):
	if len(children) == 1:
		# Collapse long chains of "empty" expression nodes
		return children[0]
		#return Node(type, children)
	elif len(children) == 3:
		# In this case it actually is parsing an expression of this type.
		# We actually try to propagate constant expressions up the tree, so we 
		# try to eval, and if that fails we leave the expanded expression.
		result = None
		# FIXME: This is a bit hackish, but t[1] may be a Node but it may be a String or
		# something
		if isinstance(children[1], Node):
			operator = children[1].leaf
		else:
			operator = children[1]
		if children[0].result is not None and children[2].result is not None:
			try:
				expression = '%sL %s %sL' % (children[0].result, operator, children[2].result)
				result = eval(expression)
			except (NameError, SyntaxError):
				result = '(%s %s %s)' % (children[0].result, operator, children[2].result)
		return Node(type, [children[0], children[2]], children[1], result=result)
	else:
		raise Exception("Expression constructor too many args!")

def UnaryOperatorNode(type, children):
	assert len(children) in (1,2)
	if len(children) == 2:
		result = eval(children[0].leaf + str(children[1].result))
	else:
		result = children[0].result
	return Node(type, children, result=result)


# Python2.4 compatibility
try:
	reversed
except NameError:
	def reversed(l):
		for counter in range(len(l)-1, -1, -1):
			yield l[counter]
else:
	reversed = __builtins__['reversed']

