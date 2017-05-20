#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "plugin_priv.h"
#include "pipeline_priv.h"
#include "cli_priv.h"
#include "imgutil/imgutil.h"

int main(int argc, char **argv) {
	if (cli_parse_opts(argc, argv) != 0) {
		printf("CLI argument parsing failed.\n");
		return 1;
	}

	if (cli_opts.opt_image_path == NULL) {
		fprintf(stderr, "No input image specified. Exiting.\n");
		return 1;
	}

	IMAGE *src = img_load(cli_opts.opt_image_path);
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
	plugin_set_arg(0, "divisor", "2.0");

	plugin_set_arg(1, "kernel", "0,-1,0,-1,5,-1,0,-1,0");
	plugin_set_arg(1, "divisor", "2.0");

	print_plugin_config();
	pipeline_feed(src, result);

	img_save(result, "res/kernel_output.jpg");
	img_free(src);
	plugins_cleanup();
	cli_opts_cleanup();
	return 0;
}
