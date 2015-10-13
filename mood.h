#ifndef MOOD_H_
#define MOOD_H_

#define NS_MOOD "http://jabber.org/protocol/mood"

typedef void (*mood_callback_t)(int);

extern const char *moods[80];

void mood_publish(mood_callback_t, const char * const, const char * const);

#endif /* mood.h */
