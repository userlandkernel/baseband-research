#include <l4/ipc.h>
#include <l4io.h>
#include "pingpong_service.h"

void
test_server_main()
{
        printf("Pingpong server starting.\n");
		server_loop();
}

int
pingpong_add_impl(L4_ThreadId_t caller, int val, idl4_server_environment *env)
{
	printf("Pingpong! Server got %d\n", val);
	return val + 1;
}
