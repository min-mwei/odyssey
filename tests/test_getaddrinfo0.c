
/*
 * machinarium.
 *
 * Cooperative multitasking engine.
*/

#include <machinarium.h>
#include <machinarium_test.h>

static void
test_gai(void *arg)
{
	struct addrinfo *res = NULL;
	int rc = machine_getaddrinfo("localhost", "http", NULL, &res, INT_MAX);
	if (rc < 0) {
		printf("failed to resolve address\n");
	} else {
		test(res != NULL);
		if (res)
			freeaddrinfo(res);
	}
}

void
test_getaddrinfo0(void)
{
	machinarium_init();

	int id;
	id = machine_create("test", test_gai, NULL);
	test(id != -1);

	int rc;
	rc = machine_wait(id);
	test(rc != -1);

	machinarium_free();
}
