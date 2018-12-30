/* This implements a very simple lookup service which maps a string to
 * an integer. It returns 0 on lookup if the string is incorrect.
*/
#include <l4/ipc.h>
#include <l4io.h>
#include "naming_service.h"

static char thename[256];
static int theval;


void
test_server_main()
{
        printf("Smallstring server starting.\n");
		server_loop();
}

void
naming_set_impl(L4_ThreadId_t caller, char *name, int val, idl4_server_environment *env)
{
	char *s = thename;

	theval = val;

	printf("server: val set to %d, string is %s\n", val, name);
	/* Of course this is terrible code security wise... */
	while(*s++ = *name++)
		;
}

int
naming_get_impl(L4_ThreadId_t caller, char *name, idl4_server_environment *env)
{
	char *s = thename;

	while(*s && *name) {
		if (*s != *name)
			return 0;
		s++;
		name++;
	}

	return theval;
}
