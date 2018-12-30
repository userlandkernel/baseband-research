/*-run(templates.get('preamble'))-*/
/*-run(templates.get('servicetemplate_create_getiid_func'))-*/
/*-
# Invent a filename here and include a suitable comment.
service_filename = generator.get_idl_filename().split('.')[0] + '_service.h'
-*/
/* NOTE: Change the following line to correctly include the service stubs. */
#include "/*-?service_filename-*/"

/*LOOP interface = generator.get_interfaces()*/
/*LOOP function = interface.get_functions()*/
/*-run(templates.get('language_specific_remapping'))-*/
/*-impl_params = ', '.join (['CORBA_Object _caller'] + ['%s %s%s%s' % (param['typename'], param['c_impl_indirection'], param['indirection'], param['name']) for param in function.get_call_params()] + ['idl4_server_environment *_env'])-*/
IDL4_INLINE /*-?function.get_return_type()-*/ /*-?function.get_name()-*/_implementation(/*-?impl_params-*/)
{
	/*-if function.get_return_type() != 'void'*//*-?function.get_return_type()-*/ retval;
	/*fi-*/
	/* Implementation of {*-?interface.get_name()-*}::{*-?function.get_name_raw()-*} */
	/*-if function.get_return_type() != 'void'*/return retval;/*fi-*/
}

/* Link the name of the function above with the name defined in the default vtable. */
IDL4_PUBLISH_/*-?function.get_name().upper()-*/(/*-?function.get_name()-*/_implementation);
/*ENDLOOP*/

/*-INTERFACE_NAME = interface.get_name().upper()-*/
/* Use the default dispatch table defined in the service header. */
void * /*-?interface.get_name()-*/_vtable[/*-?INTERFACE_NAME-*/_DEFAULT_VTABLE_SIZE]
		= /*-?INTERFACE_NAME-*/_DEFAULT_VTABLE;
/*ENDLOOP*/
void server(void)
{
	L4_ThreadId_t partner;
	L4_MsgTag_t msgtag;
	idl4_msgbuf_t msgbuf;
	long cnt;

	while (1) {
		partner = L4_nilthread; /* Our initial reply is to the nilthread. */
		msgtag.raw = 0;
		cnt = 0;

		while (1) {
			idl4_reply_and_wait(&partner, &msgtag, &msgbuf, &cnt);
			if (idl4_is_error(&msgtag)) {
				/* FIXME: Add your error handler here. */
				printf("server: error sending IPC reply\n");
				break; /* Reset thread ID to nilthread & try again */
			}
			switch(/*-?getiid_func-*/(&msgbuf)) {
				/*LOOP interface = generator.get_interfaces()*/
				case /*-?interface.get_uuid()-*/:
					idl4_process_request(&partner, &msgtag, &msgbuf, &cnt,
							/*-?interface.get_name()-*/_vtable
							[idl4_get_function_id(&msgtag) & /*-?interface.get_name().upper()-*/_FID_MASK]);
					break;
				/*ENDLOOP*/
			}
		}
	}
}


