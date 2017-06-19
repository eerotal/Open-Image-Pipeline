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

#include "oipdataparser/oipdataparser.h"
#include "oipcore/abi/output.h"

/*
*  Define some error values.
*/
#define DP_SYNTAX_OK				0
#define DP_SYNTAX_ERR_MISSING_VAR_IDENTIFIER	1
#define DP_SYNTAX_ERR_MULTIPLE_ASSIGNMENTS	2
#define DP_SYNTAX_ERR_NO_ASSIGNMENT		3

#define DP_NO_ERROR				4
#define DP_ERROR_INTERNAL			5
#define DP_ERROR_INVALID_VARIABLE		6

/*
*  Define the different delimiter characters used
*  while parsing strings.
*/
#define DP_ASSIGN_CHAR       '='
#define DP_ASSIGN_STR        "="

#define DP_LINE_DELIM_STR     ";\n"

#define DP_ARRAY_DELIM_CHAR  ','
#define DP_ARRAY_DELIM_STR   ","

static int dp_err = 0;

static void dp_var_free_wrapper(void *var);
static int dp_chk_syntax(const char *str);
static int dp_print_error(int err);

void dp_var_dump(DP_VAR *var) {
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

	errno = 0;
	ret = calloc(1, sizeof(DP_VAR));
	if (!ret) {
		printerrno("calloc()");
		return NULL;
	}

	if (var) {
		errno = 0;
		ret->var = strdup(var);
		if (!ret->var) {
			printerrno("strdup()");
			free(ret);
			return NULL;
		}
	}

	ret->data = (PTRARRAY_TYPE(char)*) ptrarray_create(&free);
	if (!ret->data) {
		if (ret->var) {
			free(ret->var);
		}
		free(ret);
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

int dp_var_validate_numeric(const DP_VAR *var) {
	/*
	*  Check whether the data in a DP_VAR instance only
	*  contains the characters 0-9, + and -. Returns 1
	*  if it does and 0 otherwise.
	*/
	for (size_t i = 0; i < var->data->ptrc; i++) {
		if (strspn(var->data->ptrs[i], "0123456789+-")
			!= strlen(var->data->ptrs[i])) {
			return 0;
		}
	}
	return 1;
}


DP_VAR *dp_get_var(PTRARRAY_TYPE(DP_VAR) *vars, const char *var) {
	/*
	*  Get the variable var from the vars PTRARRAY.
	*  Returns a pointer to a DP_VAR instance on success
	*  or a NULL pointer on failure.
	*/
	for (size_t i = 0; i < vars->ptrc; i++) {
		if (strcmp(vars->ptrs[i]->var, var) == 0) {
			return vars->ptrs[i];
		}
	}
	return NULL;
}

long dp_var_lint_value(DP_VAR *var, const size_t index) {
	/*
	*  Return the value at index in var converted to
	*  a long int or 0 on failure.
	*/

	if (var && var->data) {
		if (index < var->data->ptrc) {
			if (!dp_var_validate_numeric(var)) {
				printverb_va("Attempted to convert "\
						"non-numeric value to "\
						"long int. (%s)\n",
						var->data->ptrs[index]);
				return 0;
			}
			return strtol(var->data->ptrs[index], NULL, 10);
		}
	}
	return 0;
}

char *dp_var_str_value(DP_VAR *var, const size_t index) {
	/*
	*  Return the value at index in var as a string
	*  or a NULL pointer on failure.
	*/
	if (var->data) {
		if (index < var->data->ptrc) {
			return var->data->ptrs[index];
		}
	}
	return NULL;
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
		tmp = dp_var_lint_value(var, i);
		if (!ptrarray_put_data((PTRARRAY_TYPE(void)*) res,
					&tmp, sizeof(tmp))) {
			ptrarray_free_ptrs((PTRARRAY_TYPE(void)*) res);
			ptrarray_free((PTRARRAY_TYPE(void)*) res);
			return NULL;
		}
	}
	return res;
}

DP_VAR *dp_parse_line(const char *str, const char **valid,
			const size_t validc) {
	/*
	*  Parse a single line with one variable definition.
	*  If the string array pointer valid is not NULL, only
	*  variable identifiers from that array are considered
	*  valid and all others are discarded. validc should
	*  contain the number of strings in the valid array.
	*  Returns a pointer to a DP_VAR instance on success or
	*  a NULL pointer on failure.
	*/
	DP_VAR *res = NULL;
	char *tmp = NULL;
	char *token = NULL;
	char *saveptr = NULL;
	int ret = 0;
	int var_valid = 0;

	ret = dp_chk_syntax(str);
	if (ret != DP_SYNTAX_OK) {
		dp_print_error(ret);
		dp_err = DP_ERROR_INTERNAL;
		return NULL;
	}

	errno = 0;
	tmp = strdup(str);
	if (!tmp) {
		printerrno("strdup()");
		dp_err = DP_ERROR_INTERNAL;
		return NULL;
	}

	res = dp_var_create(NULL);
	if (!res) {
		free(tmp);
		dp_err = DP_ERROR_INTERNAL;
		return NULL;
	}

	// Parse the variable identifier.
	token = strtok_r(tmp, DP_ASSIGN_STR, &saveptr);
	if (!token) {
		printerr("Failed to read variable value.\n")
		free(tmp);
		dp_var_free(res);
		dp_err = DP_ERROR_INTERNAL;
		return NULL;
	}

	// Check whether the variable identifier is valid.
	if (valid && validc) {
		for (size_t i = 0; i < validc; i++) {
			if (strcmp(valid[i], token) == 0) {
				var_valid = 1;
				break;
			}
		}
		if (!var_valid) {
			printverb_va("Discarding invalid "\
					"variable: %s\n", token);
			free(tmp);
			dp_var_free(res);
			dp_err = DP_ERROR_INVALID_VARIABLE;
			return NULL;
		}
	}

	errno = 0;
	res->var = strdup(token);
	if (!res->var) {
		printerrno("strdup()");
		free(tmp);
		dp_var_free(res);
		dp_err = DP_ERROR_INTERNAL;
		return NULL;
	}

	// Parse the variable data.
	token = strtok_r(NULL, DP_ARRAY_DELIM_STR, &saveptr);
	while (token) {
		if (!ptrarray_put_data((PTRARRAY_TYPE(void)*) res->data,
				token, (strlen(token) + 1)*sizeof(*token))) {
			printerr("Failed to add parsed value to PTRARRAY.\n");
			ptrarray_free_ptrs((PTRARRAY_TYPE(void)*) res->data);
			ptrarray_free((PTRARRAY_TYPE(void)*) res->data);
			free(tmp);
			dp_var_free(res);
			dp_err = DP_ERROR_INTERNAL;
			return NULL;
		}
		token = strtok_r(NULL, DP_ARRAY_DELIM_STR, &saveptr);
	}
	dp_err = DP_NO_ERROR;
	free(tmp);
	return res;
}

PTRARRAY_TYPE(DP_VAR) *dp_parse_multiline(const char *str,
						const char **valid,
						const size_t validc) {
	/*
	*  Parse a multiline variable definition string.
	*  If the string array pointer valid is not NULL, only
	*  variable identifiers from that array are considered
	*  valid and all others are discarded. validc should
	*  contain the number of strings in the valid array.
	*  The lines can be separated with the characters
	*  defined in the constant DP_LINE_DELIM_STR.
	*  Returns a PTRARRAY pointer on success or a NULL
	*  pointer on failure.
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

	token = strtok_r(tmp, DP_LINE_DELIM_STR, &saveptr);
	while (token) {
		dp_err = 0;
		tmp_var = dp_parse_line(token, valid, validc);
		if (!tmp_var && dp_err == DP_ERROR_INVALID_VARIABLE) {
			token = strtok_r(NULL, DP_LINE_DELIM_STR,
						&saveptr);
			continue;
		}
		if (!ptrarray_put_ptr((PTRARRAY_TYPE(void)*) vars,
					tmp_var)) {
			free(tmp);
			dp_var_free(tmp_var);
			ptrarray_free_ptrs((PTRARRAY_TYPE(void)*) vars);
			ptrarray_free((PTRARRAY_TYPE(void)*) vars);
			return NULL;
		}
		token = strtok_r(NULL, DP_LINE_DELIM_STR, &saveptr);
	}
	free(tmp);
	return vars;
}

static int dp_chk_syntax(const char *str) {
	/*
	*  Check for assignment syntax errors in str.
	*  This function only works on strings that
	*  contain one line. Otherwise the results won't
	*  be correct. Returns one of the DP_SYNTAX_*
	*  constants.
	*/
	size_t assign_cnt = 0;
	for (size_t i = 0; i < strlen(str); i++) {
		if (str[i] == DP_ASSIGN_CHAR) {
			if (i == 0) {
				// No variable identifier.
				return DP_SYNTAX_ERR_MISSING_VAR_IDENTIFIER;
			}
			assign_cnt++;
			if (assign_cnt > 1) {
				// Too many assignments.
				return DP_SYNTAX_ERR_MULTIPLE_ASSIGNMENTS;
			}
		}
	}
	if (assign_cnt == 0) {
		// No assignment.
		return DP_SYNTAX_ERR_NO_ASSIGNMENT;
	}
	return DP_SYNTAX_OK;
}

static int dp_print_error(int err) {
	/*
	*  Print an error message corresponding to the different
	*  error constants used in the dataparser system.
	*/
	switch (err) {
		case DP_SYNTAX_ERR_MISSING_VAR_IDENTIFIER:
			printerr("Syntax error: Missing variable identifier.");
			break;
		case DP_SYNTAX_ERR_MULTIPLE_ASSIGNMENTS:
			printerr("Syntax error: Multiple assignments.\n");
			break;
		case DP_SYNTAX_ERR_NO_ASSIGNMENT:
			printerr("Syntax error: No assignment.\n");
			break;
		case DP_SYNTAX_OK:
			printerr("Syntax OK.\n");
			break;
		case DP_NO_ERROR:
			printerr("No error.\n");
			break;
		case DP_ERROR_INTERNAL:
			printerr("Internal error.\n");
			break;
		case DP_ERROR_INVALID_VARIABLE:
			printerr("Invalid variable.\n");
			break;
		default:
			printerr("Unknown error.\n");
			break;
	}
	return err;
}
