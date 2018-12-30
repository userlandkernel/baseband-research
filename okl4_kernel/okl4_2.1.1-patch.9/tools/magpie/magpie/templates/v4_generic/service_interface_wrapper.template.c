void /*-?interface.get_name()-*/_server(void);
void /*-?interface.get_name()-*/_discard(void);
/*-INTERFACE_NAME = interface.get_name().upper()-*/
/*-
def list_to_string(somelist, joiner, prepend = ''):
	return joiner.join([prepend + item for item in somelist])
-*/
#define /*-?INTERFACE_NAME-*/_DEFAULT_VTABLE { /*-?list_to_string(interface.get_function_names(), ', ', '(idl4_function) service_')-*/ }
#define /*-?INTERFACE_NAME-*/_DEFAULT_VTABLE_SIZE /*-?len(interface.get_function_names())-*/
/*-
full_list = []
for ktable_function in interface.get_pagefault_functions():
        full_list.append('(idl4_function) %s_discard' % (interface.get_name()))
        full_list.append('(idl4_function) %s_discard' % (interface.get_name()))
        full_list.append('(idl4_function) service_%s' % (ktable_function.get_name()))
        full_list.append('(idl4_function) %s_discard' % (interface.get_name()))
-*/
/*-if full_list != []*/
#define /*-?INTERFACE_NAME-*/_DEFAULT_KTABLE { /*-?', '.join(full_list)-*/ }
#define /*-?INTERFACE_NAME-*/_DEFAULT_KTABLE_SIZE /*-?len(full_list)-*/
#define /*-?INTERFACE_NAME-*/_KID_MASK /*-?hex(interface.get_fid_mask(len(full_list)))-*/
/*fi-*/
#define /*-?INTERFACE_NAME-*/_MAX_FID /*-?len(interface.get_function_names()) - 1-*/
#define /*-?INTERFACE_NAME-*/_MSGBUF_SIZE 0 /* Unimplemented */
#define /*-?INTERFACE_NAME-*/_STRBUF_SIZE 0 /* Unimplemented */
#define /*-?INTERFACE_NAME-*/_FID_MASK /*-?hex(interface.get_fid_mask(len(interface.get_function_names())))-*/

/*LOOP interface.get_defined_constants()*/#define _C_/*-?LOOPITEM[0]-*/ (/*-?LOOPITEM[1]-*/)
/*ENDLOOP*/

/*LOOP function = interface.get_functions()*/
/*-function.rename_args({'fpage': 'idl4_mapitem'})-*/
/*-run(templates.get('service_function'))-*/
/*-run(templates.get('service_function_reply'))-*/
/*ENDLOOP*/
