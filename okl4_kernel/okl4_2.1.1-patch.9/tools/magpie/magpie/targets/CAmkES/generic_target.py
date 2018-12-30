from generator.CAmkES_generator import CAmkESGenerator
import targets.idl4.generic_l4v4_target as generic
from targets import base

class Generator(CAmkESGenerator):
	pass


class Templates(base.Templates):
	MAPPING = 'CAmkES'
	
	component_interface_header = 'main_client_header.template.h'
	
	IguanaRPC_client_template = 'main_client.template.h'
	IguanaRPC_client_header = 'main_client_header.template.h'
	
	IguanaRPC_server = 'main_service.template.c'
		
	client_function_wrapper =  'client_function_wrapper.template.c'
	client_header_function_wrapper =  'client_header_function_wrapper.template.c'
	client_function_body =  'client_function_body.template.c'

	clientservice_function_params_maker = 'function_params_maker.template.c'

	basic_includes =  'basic_includes.template.c'
	preamble =  'preamble.template.c'
	interface_wrapper =  'interface_wrapper.template.c'
	interface_header_wrapper =  'interface_header_wrapper.template.c'
	
	#imports = 'v4_generic/import.template.c'

	language_specific_remapping = 'v4_generic/null_remapping.template.c'
	
	public = ['client', 'service', 'servicetemplate', 'types']
	
class IguanaRPC(object):
	IguanaRPC_client_template = 'main_client.template.h'
	IguanaRPC_client_header = 'main_client_header.template.h'
	
	
class DataStorage(generic.DataStorage):
	def __init__(self):
		generic.DataStorage.__init__(self)

	def get_filename(self):
		return self.data_dict['filename']
		
	def set_filename(self, filename):
		self.data_dict['filename'] = filename	
		
	def set_include_list(self, include_list):
		self.data_dict['include_list'] = include_list
		
	def get_include_list(self):
		return self.data_dict['include_list']
		
	def set_header_location(self, header_location):
		self.data_dict['header_location'] = header_location
				
	def get_header_location(self):
		return self.data_dict['header_location']
		
