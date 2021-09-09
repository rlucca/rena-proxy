#pragma once

#include "database.h"

void http_payload_init(fa_request_t *);
void http_payload_fill_output(fa_request_t *req, size_t len);
fa_request_t *http_payload_holding_loader(fa_request_t *req);
int http_payload_loader(fa_request_t *,
        const char *buffer, size_t buffer_sz);
int http_payload_get_header(fa_request_t *, char *name,
        char *value, size_t value_sz);
int http_payload_split_first_line(fa_request_t *,
        char *method, char *path, char *protocol);
int http_payload_get_first_line(fa_request_t *,
        char *value, size_t value_sz);

// release output, holding, and etc. set html_state_data to NULL
int http_payload_release(fa_request_t *);
