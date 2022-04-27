
#include "global.h"
#include <openssl/bio.h>
#include <openssl/evp.h>

static void do_pipe(BIO *bin, BIO *bout,
                      const char *bytes, int bytes_sz, char *out, int *out_sz)
{
    int r;

    BIO_write(bin, bytes, bytes_sz);
    BIO_flush(bin);

    r = BIO_read(bout, out, *out_sz);
    out[((r >= *out_sz) ? *out_sz - 1 : r)] = '\0';
    *out_sz = r;
}

int base64_encode(const char *bytes, int bytes_sz, char *out, int *out_sz)
{
    BIO *b64 = NULL;
    BIO *s = NULL;

    if (!bytes || !out || !out_sz)
        return -1;

    b64 = BIO_new(BIO_f_base64());
    s = BIO_new(BIO_s_mem());
    if (!b64 || !s)
        return -2;

    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    BIO_push(b64, s);

    do_pipe(b64, s, bytes, bytes_sz, out, out_sz);

    BIO_free_all(b64);
    return 0;
}

int base64_decode(const char *bytes, int bytes_sz, char *out, int *out_sz)
{
    BIO *b64 = NULL;
    BIO *s = NULL;

    if (!bytes || !out || !out_sz)
        return -1;

    b64 = BIO_new(BIO_f_base64());
    s = BIO_new(BIO_s_mem());
    if (!b64 || !s)
        return -2;

    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    BIO_push(b64, s);

    do_pipe(s, b64, bytes, bytes_sz, out, out_sz);

    BIO_free_all(b64);
    return 0;
}
