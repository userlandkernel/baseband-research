#include <l4/ipc.h>
#include <l4io.h>

#include "naming_client.h"

void
test_client_main(L4_ThreadId_t server)
{
	CORBA_Environment env;
	char name[256];
	int val;

	printf("Naming client starting\n");
	
	naming_set(server, "small string test", 38, &env);

	printf("naming_set completed, running naming_get...\n");

	val = naming_get(server, "small string test", &env);

	printf("client: naming_get returned %d\n", val);
}
