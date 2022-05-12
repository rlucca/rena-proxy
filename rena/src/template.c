#include "global.h"
#include "template.h"

#include <string.h>

static struct template_codes {
    int code;
    const char *label;
    int label_sz;
} valid_codes[] = {
    {302, "302 Redirect", 13},
    {401, "401 Unauthorized", 17},
    {403, "403 Forbidden", 14},
    {404, "404 Not Found", 14},
    {500, "500 Transient Error", 20},
    { -1, "", 0}
};

static const char header_location[] = "Location: ";

static const char header_format[] = "HTTP/1.0 %s\r\n"
                    "Content-Type: text/html\r\n"
                    "Connection: Close\r\n"
                    "Server: RenaProxy\r\n"
                    "Content-Length: %d\r\n";

static const char payload_format[] = "<html><head><title>ERROR CODE %1$s"
                    "</title></head><body><h1 style=\"text-align:center;"
                    "margin-top: 300px\">ERROR CODE %1$s</h1></body></html>";


static int calculate_payload(int pos)
{
    static int format_size = 0;
    if (format_size == 0)
    {
        // why 10? len(%1$s)*2 + len('\\')*2
        format_size = strnlen(payload_format, MAX_STR) - 10;
    }
    return format_size + 2 * valid_codes[pos].label_sz;
}

int generate_redirect_to(text_t *out, const char *cookie, const char *url)
{
    int left = 0;

    if (!out || !url)
    {
        do_log(LOG_DEBUG, "out %p url %p (%s)", out, url, url);
        return -1;
    }

    left = sizeof(out->text);
    out->size = snprintf(out->text, left,
                        header_format,
                        valid_codes->label, 0);
    left -= out->size;
    if (cookie)
    {
        int written  = snprintf(out->text + out->size, left,
                                "Set-Cookie: renaproxy=%s; Max-Age=86400\r\n",
                                cookie);
        left -= written;
        out->size += written;
    }
    out->size += snprintf(out->text + out->size, left, "%s%s\r\n\r\n",
                        header_location, url);
    return out->size;
}

int generate_error(text_t *out, int error)
{
    int total = 0;
    int error_code = 1;
    const char *label_ptr = NULL;

    if (!out || error <= 0)
    {
        do_log(LOG_DEBUG, "out %p error %d", out, error);
        return -1;
    }

    out->size = 0;

    while (error > valid_codes[error_code].code)
    {
        error_code++;
    }

    if (valid_codes[error_code].code != error
            || valid_codes[error_code].code <= 0)
    {
        do_log(LOG_ERROR, "Build error page [%d] failed", error);
        return -1;
    }

    total = sizeof(out->text);
    label_ptr = valid_codes[error_code].label;
    out->size += snprintf(out->text, total, header_format, label_ptr,
                          calculate_payload(error_code));
    out->size += snprintf(out->text + out->size, total - out->size, "\r\n");
    out->size += snprintf(out->text + out->size, total - out->size,
                          payload_format, label_ptr);
    return out->size;
}
