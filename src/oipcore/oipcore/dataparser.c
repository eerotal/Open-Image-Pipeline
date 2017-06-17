/*
*
*  Copyright 2017 Eero Talus
*
*  This file is part of Open Image Pipeline.
*
*  Open Image Pipeline is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation, either version 3 of the License, or
*  (at your option) any later version.
*
*  Open Image Pipeline is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with Open Image Pipeline.  If not, see <http://www.gnu.org/licenses/>.
*
*/

#define PRINT_IDENTIFIER "dataparser"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "oipcore/dataparser.h"
#include "oipcore/abi/output.h"

#define DP_ASSIGN "="
#define DP_VAR_DELIM ";"
#define DP_ARRAY_DELIM  ","

static void dp_var_free_wrapper(void *var);

void dp_dump(DP_VAR *var) {
	/*
	*  Dump the information stored in a DP_VAR
	*  instance to stdout.
	*/
	printverb_va("[ %s => ", var->var);
	for (size_t i = 0; i < var->data->ptrc; i++) {
		if (i != 0) {
			printf(", ");
		}
		printf("%s", var->data->ptrs[i]);
	}
	printf(" ]\n");
}

DP_VAR *dp_var_create(const char *var) {
	/*
	*  Create a new DP_VAR instance. Returns a pointer
	*  to the new instance on success or a NULL pointer
	*  on failure.
	*/
	DP_VAR *ret = NULL;
	ret = calloc(1, sizeof(DP_VAR));
	ret->var = strdup(var);
	if (!ret->var) {
		return NULL;
	}
	ret->data = (PTRARRAY_TYPE(char)*) ptrarray_create(&free);
	if (!ret->data) {
		free(ret->var);
		return NULL;
	}
	return ret;
}

void dp_var_free(DP_VAR *var) {
	/*
	*  Free a DP_VAR instance.
	*/
	if (var) {
		ptrarray_free_ptrs((PTRARRAY_TYPE(void)*) var->data);
		ptrarray_free((PTRARRAY_TYPE(void)*) var->data);
		if (var->var) {
			free(var->var);
		}
		free(var);
	}
}

static void dp_var_free_wrapper(void *var) {
	/*
	*  dp_var_free() wrapper for use with PTRARRAYs.
	*/
	dp_var_free((DP_VAR*) var);
}

PTRARRAY_TYPE(char) *dp_var_strarr(DP_VAR *var) {
	/*
	*  Return a pointer to the string PTRARRAY in var.
	*/
	return var->data;
}

PTRARRAY_TYPE(long) *dp_var_lintarr(DP_VAR *var) {
	/*
	*  Return the value of arg as a PTRARRAY instance
	*  where every item in the PTRARRAY has been converted
	*  to a long int value. This function returns a NULL
	*  pointer on failure.
	*/
	PTRARRAY_TYPE(long)* res = NULL;
	long tmp = 0;

	res = (PTRARRAY_TYPE(long)*) ptrarray_create(&free);
	if (!res) {
		return NULL;
	}
	for (size_t i = 0; i < var->data->ptrc; i++) {
		tmp = strtol(var->data->ptrs[i], NULL, 10);
		if (!ptrarray_put_data((PTRARRAY_TYPE(void)*) res,
					&tmp, sizeof(tmp))) {
			ptrarray_free_ptrs((PTRARRAY_TYPE(void)*) res);
			ptrarray_free((PTRARRAY_TYPE(void)*) res);
			return NULL;
		}
	}
	return res;
}

PTRARRAY_TYPE(DP_VAR) *dp_parse_multipart(const char *str) {
	/*
	*  Parse a comma separated list of variable definitions
	*  eg. a=1,b=3,c=5. This function returns a pointer to
	*  a DP_VAR PTRARRAY on success or a NULL pointer on failure.
	*/
	PTRARRAY_TYPE(DP_VAR) *vars = NULL;
	DP_VAR *tmp_var = NULL;
	char *token = NULL;
	char *saveptr = NULL;
	char *tmp = NULL;

	vars = (PTRARRAY_TYPE(DP_VAR)*) ptrarray_create(&dp_var_free_wrapper);
	if (!vars) {
		printerr("Failed to create DP_VAR PTRARRAY.\n");
		return NULL;
	}

	errno = 0;
	tmp = strdup(str);
	if (!tmp) {
		printerrno("strdup()");
		ptrarray_free((PTRARRAY_TYPE(void)*) vars);
		return NULL;
	}

	token = strtok_r(tmp, DP_VAR_DELIM, &saveptr);
	while (token) {
		tmp_var = dp_parse_single(token);
		if (!ptrarray_put_ptr((PTRARRAY_TYPE(void)*) vars, tmp_var)) {
			/*
			*  ptrarray_put_ptr will return NULL if
			*  tmp_var == NULL, which is why that one doesn't
			*  need to be tested separately.
			*/
			free(tmp);
			dp_var_free(tmp_var);
			ptrarray_free_ptrs((PTRARRAY_TYPE(void)*) vars);
			ptrarray_free((PTRARRAY_TYPE(void)*) vars);
			return NULL;
		}
		token = strtok_r(NULL, DP_VAR_DELIM, &saveptr);
	}
	free(tmp);
	return vars;
}

DP_VAR *dp_parse_single(const char *str) {
	/*
	*  Parse a single variable definition from a string.
	*  This function returns a pointer to a DP_VAR instance
	*  on success or a NULL pointer on failure.
	*/
	char *token = NULL;
	char *saveptr = NULL;
	char *tmp = NULL;
	DP_VAR *res = NULL;

	errno = 0;
	tmp = strdup(str);
	if (!tmp) {
		printerrno("strdup()");
		return NULL;
	}

	errno = 0;
	res = calloc(1, sizeof(DP_VAR));
	if (!res) {
		printerrno("calloc()");
		free(tmp);
		return NULL;
	}

	res->data = (PTRARRAY_TYPE(char)*) ptrarray_create(&free);
	if (!res->data) {
		free(tmp);
		dp_var_free(res);
		return NULL;
	}

	token = strtok_r(tmp, DP_ASSIGN, &saveptr);
	if (!token) {
		printerr("Failed to read variable value.\n")
		free(tmp);
		dp_var_free(res);
		return NULL;
	}
	errno = 0;
	res->var = strdup(token);
	if (!res->var) {
		printerrno("strdup()");
		free(tmp);
		dp_var_free(res);
		return NULL;
	}

	token = strtok_r(NULL, DP_ARRAY_DELIM, &saveptr);
	while (token) {
		if (!ptrarray_put_data((PTRARRAY_TYPE(void)*) res->data,
				token, (strlen(token) + 1)*sizeof(*token))) {
			printerr("Failed to add value to variable PTRARRAY.\n");
			ptrarray_free_ptrs((PTRARRAY_TYPE(void)*) res->data);
			ptrarray_free((PTRARRAY_TYPE(void)*) res->data);
			free(tmp);
			dp_var_free(res);
			return NULL;
		}
		token = strtok_r(NULL, DP_ARRAY_DELIM, &saveptr);
	}
	free(tmp);
	return res;
}
