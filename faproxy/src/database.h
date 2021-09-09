#pragma once

// parse database and initialize structures
void database_set_up(const char *directory);

// returns handler
//  side -1 client (to server from client = .-)
//  side  0 unknown
//  side  1 server (to client from server = -.)
void *database_transform_handler(int side);

enum {
    FEED_ME_AND_HOLD = 0,
    TRANSFORM_OK = 1,
    NOT_HOLD = 2
};

// lookup on database to transform based on handler state
// handler, returned by database_transform_handler
// input, feed it
// force, flag to ending a conclusion that
//        is not yield (for now do not worry in use it)
// expanded, internal data - do not try to release or keep
// holding, internal data - do not keep the pointer (it should be released)
int database_transform_lookup(void **handler, char input, int force,
                              char **expanded, size_t *expanded_len);
// Should has it a way to be called without an input? makes sense?

// ...
void database_transform_cleanup(void **handler);
