/*
 * Copyright (c) 2015 Tristan Le Guern <tleguern@bouledef.eu>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
/*
 * Author: Tatu Ylonen <ylo@cs.hut.fi>
 * Copyright (c) 1995 Tatu Ylonen <ylo@cs.hut.fi>, Espoo, Finland
 *                    All rights reserved
 * Functions for reading the configuration files.
 *
 * As far as I am concerned, the code I have written for this software
 * can be used freely for any purpose.  Any derived versions of this
 * software must be clearly marked as such, and if the derived work is
 * incompatible with the protocol description in the RFC file, it must be
 * called by a name other than "ssh" or "Secure Shell".
 */

#include <ctype.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <util.h>

#ifdef HAVE_LIBBSD
# include <bsd/stdlib.h>
# include <bsd/stdio.h>
#endif

#include "config.h"

typedef enum {
	BAD_OPTION,
	USERNAME,
	PASSWORD
} opcodes;

static struct {
	const char *name;
	opcodes opcode;
} keywords[] = {
	{"username", USERNAME},
	{"password", PASSWORD},
	{NULL, BAD_OPTION}
};

static opcodes
parse_token(const char *key)
{
	u_int i;

	for (i = 0; keywords[i].name; i++)
		if (strcasecmp(key, keywords[i].name) == 0)
			return keywords[i].opcode;
	return BAD_OPTION;
}

static int
parse_line(struct options *options, char *line, const char *filename,
     size_t lineno)
{
	char *key, *value, *tmp;
	opcodes opcode;

	/* Discard empty lines */
	if (!line || *line == '\0')
		return(0);

	/* Left-trim whitespaces */
	key = line;
	while (key && isspace(*key) != 0)
		key++;

	if ((value = strchr(key, ' ')) == NULL)
		if ((value = strchr(line, '\t')) == NULL)
			goto failure;

	/* Check and left-trim the value */
	*value = '\0';
	do {
		value++;
	} while (value && isspace(*value) != 0);
	if (!value || *value == '\0')
		goto failure;

	/* Right-trim whitespaces */
	tmp = value;
	while (*tmp)
		tmp++;
	tmp--;
	while (isspace(*tmp) != 0) {
		*tmp = '\0';
		tmp--;
	}

	opcode = parse_token(key);
	switch (opcode) {
	case BAD_OPTION:
		goto failure;
	case USERNAME:
		free(options->username);
		options->username = strdup(value);
		break;
	case PASSWORD:
		free(options->password);
		options->password = strdup(value);
		break;
	default:
		errx(EXIT_FAILURE, "You forgot the handler for option %s", key);
	}
	return(0);
failure:
	errx(EXIT_FAILURE, "%s:%zi: error missing value for %s",
	    filename, lineno, line);
}

void
parse_config(struct options *options, const char *filename)
{
	size_t lineno;
	char *line;
	FILE *s;

	lineno = 0;
	s = fopen(filename, "r");
	while ((line = fparseln(s, NULL, &lineno, NULL, 0)) != NULL) {
		if (parse_line(options, line, filename, lineno) == -1)
			errx(EXIT_FAILURE, "Error: %s:%zi: %s",
			     filename, lineno, line);
		free(line);
	}
}

void
init_options(struct options *options)
{
	options->username = NULL;
	options->password = NULL;
}

void
free_options(struct options *options)
{
	free(options->username);
	free(options->password);
}

