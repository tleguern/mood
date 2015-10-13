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

#include <err.h>
#include <glob.h>
#include <stdlib.h>
#include <string.h>
#include <strophe.h>
#include <unistd.h>

#ifdef HAVE_LIBBSD
# include <bsd/stdlib.h>
#endif

#include "common.h"
#include "config.h"
#include "mood.h"
#include "pathnames.h"

xmpp_ctx_t *ctx;
xmpp_conn_t *conn;

static void usage(void);

static void
mood_callback(int status) {
	if (status == 1) {
		warnx("Something happened !");
	}
	xmpp_stop(ctx);
}

static void
conn_handler(xmpp_conn_t * const conn, const xmpp_conn_event_t status, 
		  const int error, xmpp_stream_error_t * const stream_error,
		  void * const userdata)
{
	char *mood;

	mood = (char *)userdata;
	if (status == XMPP_CONN_CONNECT) {
		mood_publish(&mood_callback, mood, NULL);
	} else if (status == XMPP_CONN_DISCONNECT) {
		errx(1, "Connection status: XMPP_CONN_DISCONNECT");
	} else if (status == XMPP_CONN_FAIL) {
		errx(1, "Connection status: XMPP_CONN_FAIL");
	} else {
		errx(1, "Unknown connection status");
	}
}

int
main(int argc, char **argv)
{
	char *fflag;
	char *mood;
	int lflag;
	char ch;
	struct options options;
	unsigned int i;
	glob_t gl;

	init_options(&options);
	fflag = _PATH_CONF;
	lflag = 0;
	while ((ch = getopt(argc, argv, "f:l")) != -1)
		switch (ch) {
		case 'f':
			fflag = optarg;
		        break;
		case 'l':
			lflag = 1;
			break;
		default:
		        usage();
		}
	argc -= optind;
	argv += optind;

	if (lflag == 1) {
		for (i = 0; i < sizeof(moods) / sizeof(*moods); i++) {
			printf("%s\n", moods[i]);
		}
		exit(EXIT_SUCCESS);
	}

	if (argc < 1) {
		usage();
	}

	/* Verify that the submitted mood is part of the official list from XEP-0107 */
	mood = NULL;
	for (i = 0; i < sizeof(moods) / sizeof(*moods); i++) {
		if (strcasecmp(argv[0], moods[i]) == 0) {
			mood = argv[0];
			break;
		}
	}
	if (mood == NULL)
		usage();

	/* Tild expansion for _PATH_CONF */
	(void)memset(&gl, '\0', sizeof(gl));
	if (glob(fflag, GLOB_TILDE, NULL, &gl) != 0)
		err(1, "%s", fflag);
	if (gl.gl_pathc != 1)
		errx(1, "No match for %s", fflag);
	fflag = gl.gl_pathv[0];

	if (access(fflag, F_OK) != 0) {
		err(1, "%s", fflag);
	}
	parse_config(&options, fflag);

	/* Ready to publish */
	xmpp_initialize();
	if ((ctx = xmpp_ctx_new(NULL, NULL)) == NULL)
		errx(1, "Can't get libstrophe ctx");
	if ((conn = xmpp_conn_new(ctx)) == NULL)
		errx(1, "Can't get libstrophe conn");
	xmpp_conn_set_jid(conn, options.username);
	xmpp_conn_set_pass(conn, options.password);
	globfree(&gl);

	if (xmpp_connect_client(conn, NULL, 0, conn_handler, mood) != 0)
		errx(1, "Can't connect to XMPP server");

	xmpp_run(ctx);

	free_options(&options);
	xmpp_conn_release(conn);
	xmpp_ctx_free(ctx);
	xmpp_shutdown();
	return(EXIT_SUCCESS);
}

static void
usage(void)
{
	const char *p = getprogname();

	fprintf(stderr, "usage: %s [-f xmppcfg] [-l] arg\n", p);
	exit(EXIT_FAILURE);
}

