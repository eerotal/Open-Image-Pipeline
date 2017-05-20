#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "plugin_priv.h"
#include "pipeline_priv.h"
#include "imgutil/imgutil.h"

int main(int argc, char **argv) {
	IMAGE *src = img_load("res/scenery.jpg");
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
	return 0;
}
