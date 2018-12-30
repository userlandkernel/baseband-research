#include <l4/ipc.h>
#include <l4io.h>

#include "pingpong_client.h"

void
test_client_main(L4_ThreadId_t server)
{
	CORBA_Environment env;
	int result;

	printf("Pingpong client starting\n");
	
	result = pingpong_add(server, 1, &env);

	printf("Pingpong client: result is %d\n", result);
}
