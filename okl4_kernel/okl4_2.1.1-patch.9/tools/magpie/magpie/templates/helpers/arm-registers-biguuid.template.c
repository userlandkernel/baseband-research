/*-template_version = 2 -*/
/*-
# We always use at least one input register for biguuid
maximum_used_register_biguuid = max(1, function.max_register)

def real_register_for_mr(mr_number):
	assert mr_number <= 4
	return mr_number + 3 # r3 = mr0, r4 = mr1 ... r7 = mr4

# We use input_register_list and output_register_list when telling GCC what registers
# are in/out/trashed during __asm__
# NB input_register_list does NOT include r_mr1 for biguuid
input_register_list = ['r_mr%d' % (mapitem['mr']) for mapitem in function.register_map_in]
output_register_list = ['r_mr%d' % (mapitem['mr']) for mapitem in function.register_map_out]
clobber_register_list = [5, 6, 7]
for item in [mapitem['mr'] for mapitem in function.register_map_in + function.register_map_out]:
	real_register = real_register_for_mr(item)
	if real_register in clobber_register_list:
		clobber_register_list.remove(real_register)
clobber_register_string = ', '.join(['"r%d"' % (item) for item in clobber_register_list])
if clobber_register_string != '':
	clobber_register_string += ','

single_register_return_markup = """//Result fits in a single register; just return it 
	return ({*-?function.return_type-*})r_mr{*-?len(_out_params)+1-*};"""

complicated_return_markup ="""// Return result{*LOOP range(len(_return_param['message_registers']))*}
	**( ((L4_Word_t *)(void *)&__return) + {*-?LOOPITEM-*}) = r_mr{*-?_return_param['message_registers'][LOOPITEM][0]-*};
	{*ENDLOOP*}
	return __return;
"""
def markup_return_code():
	# If the return is "simple" - ie it is void or occupies a single register completely -
	# we can save a copy here by using a simplified form of the return logic.
	if len(_return_param['message_registers']) == 0:
		pass #no registers occupied == void == nothing to do
	elif len(_return_param['message_registers']) == 1:
		_markup(single_register_return_markup)
	else:
		# More complicated copying case.
		_markup(complicated_return_markup)
-*/
