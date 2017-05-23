#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include "plugin_priv.h"
#include "pipeline_priv.h"
#include "cli_priv.h"
#include "imgutil/imgutil.h"

int main(int argc, char **argv) {
	time_t tstamp;
	char *tstamp_str = NULL;

	if (cli_parse_opts(argc, argv) != 0) {
		printf("CLI argument parsing failed.\n");
		return 1;
	}

	if (cli_get_opts()->opt_in_path == NULL) {
		fprintf(stderr, "No input image specified. Exiting.\n");
		return 1;
	}

	IMAGE *src = img_load(cli_get_opts()->opt_in_path);
	if (src == NULL) {
		return 1;
	}
	IMAGE *result = img_alloc(0, 0);
	if (!result) {
		return 1;
	}
	printf("Loaded image.\n");

	plugin_load("plugins/", "convolution");
	plugin_load("plugins/", "convolution");

	plugin_set_arg(0, "kernel", "0,-1,0,-1,5,-1,0,-1,0");
	plugin_set_arg(0, "divisor", "1.0");

	plugin_set_arg(1, "kernel", "0,-1,0,-1,5,-1,0,-1,0");
	plugin_set_arg(1, "divisor", "1.0");

	print_plugin_config();

	// Get the current time for the pipeline cache id.
	tstamp = time(NULL);
	tstamp_str = calloc((unsigned int) round(log10(tstamp)) + 2, sizeof(char));
	if (tstamp_str == NULL) {
		img_free(src);
		img_free(result);
		return 1;
	}

	sprintf(tstamp_str, "%lli", (long long int) tstamp);
	pipeline_feed(src, result, "asdfg");
	free(tstamp_str);

	if (cli_get_opts()->opt_out_path != NULL) {
		img_save(result, cli_get_opts()->opt_out_path);
	} else {
		img_save(result, "res/out.jpg");
	}

	img_free(src);
	plugins_cleanup();
	cli_opts_cleanup();
	return 0;
}
