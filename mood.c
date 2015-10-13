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
#include <string.h>
#include <strophe.h>

#if defined HAVE_LIBBSD
# include <bsd/stdlib.h>
#endif

#include "common.h"
#include "mood.h"
#include "pubsub.h"

const char *moods[] = {
          "afraid",
          "amazed",
          "angry",
          "amorous",
          "annoyed",
          "anxious",
          "aroused",
          "ashamed",
          "bored",
          "brave",
          "calm",
          "cautious",
          "cold",
          "confident",
          "confused",
          "contemplative",
          "contented",
          "cranky",
          "crazy",
          "creative",
          "curious",
          "dejected",
          "depressed",
          "disappointed",
          "disgusted",
          "dismayed",
          "distracted",
          "embarrassed",
          "envious",
          "excited",
          "flirtatious",
          "frustrated",
          "grumpy",
          "guilty",
          "happy",
          "hopeful",
          "hot",
          "humbled",
          "humiliated",
          "hungry",
          "hurt",
          "impressed",
          "in_awe",
          "in_love",
          "indignant",
          "interested",
          "intoxicated",
          "invincible",
          "jealous",
          "lonely",
          "lucky",
          "mean",
          "moody",
          "nervous",
          "neutral",
          "offended",
          "outraged",
          "playful",
          "proud",
          "relaxed",
          "relieved",
          "remorseful",
          "restless",
          "sad",
          "sarcastic",
          "serious",
          "shocked",
          "shy",
          "sick",
          "sleepy",
          "spontaneous",
          "stressed",
          "strong",
          "surprised",
          "thankful",
          "thirsty",
          "tired",
          "undefined",
          "weak",
          "worried",
};

static int
mood_result_handler(xmpp_conn_t * const conn, xmpp_stanza_t * const stanza,
    void * const userdata)
{
	int status;
	char *type;
	mood_callback_t callback;

	status = 0;
	callback = (mood_callback_t)userdata;
	type = xmpp_stanza_get_type(stanza);
	if (strcmp(type, "error") == 0) {
		warnx("XEP-0092: Error");
		status = 1;
	} else if (strcmp(type, "result") != 0) {
		warnx("XEP-0092: Unknown error");
		status = 1;
	}

	if (callback != NULL) {
		callback(status);
	}
	return 0;
}

void
mood_publish(mood_callback_t callback, const char * const usermood,
    const char * const usertext)
{
	xmpp_stanza_t *iq, *pubsub, *publish, *item, *mood, *stanza, *text;

	iq = xmpp_stanza_new(ctx);
	xmpp_stanza_set_name(iq, "iq");
	xmpp_stanza_set_type(iq, "set");
	xmpp_stanza_set_id(iq, "mood1");	/* FIXME */

	pubsub = xmpp_stanza_new(ctx);
	xmpp_stanza_set_name(pubsub, "pubsub");
	xmpp_stanza_set_ns(pubsub, NS_PUBSUB);
	xmpp_stanza_add_child(iq, pubsub);
	xmpp_stanza_release(pubsub);

	publish = xmpp_stanza_new(ctx);
	xmpp_stanza_set_name(publish, "publish");
	xmpp_stanza_set_attribute(publish, "node", NS_MOOD);
	xmpp_stanza_add_child(pubsub, publish);
	xmpp_stanza_release(publish);

	item = xmpp_stanza_new(ctx);
	xmpp_stanza_set_name(item, "item");
	xmpp_stanza_add_child(publish, item);
	xmpp_stanza_release(item);

	mood = xmpp_stanza_new(ctx);
	xmpp_stanza_set_name(mood, "mood");
	xmpp_stanza_set_ns(mood, NS_MOOD);
	xmpp_stanza_add_child(item, mood);
	xmpp_stanza_release(mood);

	if (usermood != NULL) {
		stanza = xmpp_stanza_new(ctx);
		xmpp_stanza_set_name(stanza, usermood);
		xmpp_stanza_add_child(mood, stanza);
		xmpp_stanza_release(stanza);
	}

	if (usertext != NULL) {
		text = xmpp_stanza_new(ctx);
		xmpp_stanza_set_name(text, "text");
		xmpp_stanza_set_text(text, usertext);
		xmpp_stanza_add_child(mood, text);
		xmpp_stanza_release(text);
	}

	xmpp_id_handler_add(conn, mood_result_handler,
	    xmpp_stanza_get_id(iq), callback);

	xmpp_send(conn, iq);
	xmpp_stanza_release(iq);
}

