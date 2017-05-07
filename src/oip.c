#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "plugin_priv.h"
#include "pipeline_priv.h"
#include "imgutil/imgutil.h"

int main(int argc, char **argv) {
	IMAGE *src = img_load("res/scenery.jpg");
	IMAGE *result = img_alloc(0, 0);
	if (!result) {
		return 1;
	}
	printf("Loaded image.\n");

	plugin_load("plugins/", "convolution");
	print_plugin_config();
	pipeline_feed(src, result);

	img_free(src);
	plugins_cleanup();
	return 0;
}
