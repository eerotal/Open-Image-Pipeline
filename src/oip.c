#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <pthread.h>

#include "plugin_priv.h"
#include "pipeline_priv.h"
#include "cli_priv.h"
#include "imgutil/imgutil.h"

static pthread_t *thread_cli_shell;
static int exit_queued = 0;

static void oip_cleanup(void);

static void oip_cleanup(void) {
	// Run cleanup functions.
	plugins_cleanup();
	cli_opts_cleanup();

	// Cancel the CLI shell thread.
	if (pthread_cancel(*thread_cli_shell) != 0) {
		perror("pthread_cancel(): ");
	}

	if (pthread_join(*thread_cli_shell, NULL) != 0) {
		perror("pthread_join(): ");
	}
}

void oip_exit(void) {
	exit_queued = 1;
}

int main(int argc, char **argv) {
	// Read CLI options.
	if (cli_parse_opts(argc, argv) != 0) {
		printf("CLI argument parsing failed.\n");
		return 1;
	}

	// Init CLI shell.
	thread_cli_shell = cli_shell_init();
	if (thread_cli_shell == NULL) {
		cli_opts_cleanup();
	}

	while (!exit_queued) {}

	// Run cleanup.
	oip_cleanup();
	return 0;
}
