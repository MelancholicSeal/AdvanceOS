/*
 * This is sample code generated by rpcgen.
 * These are only templates and you can use them
 * as a guideline for developing your own functions.
 */

#include "add.h"


int
add_prog_1(char *host, numbers to_add)
{
	CLIENT *clnt;
	int  *result_1;
	numbers  add_1_arg= to_add;

#ifndef	DEBUG
	clnt = clnt_create (host, ADD_PROG, ADD_VERS, "udp");
	if (clnt == NULL) {
		clnt_pcreateerror (host);
		exit (1);
	}
#endif	/* DEBUG */

	result_1 = add_1(&add_1_arg, clnt);
	if (result_1 == (int *) NULL) {
		clnt_perror (clnt, "call failed");
	}
	return *result_1;
#ifndef	DEBUG
	clnt_destroy (clnt);
#endif	 /* DEBUG */
}


int
main (int argc, char *argv[])
{
	char *host;

	if (argc < 4) {
		printf ("usage: %s server_host\t number1 \t number2(integers)", argv[0]);
		exit (1);
	}
	host = argv[1];
	numbers sent;
	sent.a=strtol(argv[2], NULL, 10);
	sent.b=strtol(argv[3], NULL, 10);
	printf("%d",add_prog_1 (host, sent));
exit (0);
}
