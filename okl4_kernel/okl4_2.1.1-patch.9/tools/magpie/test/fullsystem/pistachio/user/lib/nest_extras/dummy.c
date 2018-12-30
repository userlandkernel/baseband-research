/* Dummy nest_extras file */

#include <l4/ipc.h>
#include <l4io.h>

void
test_client_main(L4_ThreadId_t server)
{
        printf("Dummy test client starting\n");
}

void
test_server_main(L4_ThreadId_t server)
{
        printf("Dummy test server starting\n");
}

