
#include "global.h"
#include <base64.h>
#include <openssl/bio.h>
#include <openssl/evp.h>


int md5_encode(const char *bytes, int bytes_sz, text_t *out)
{
    EVP_MD_CTX *mdctx;
    int ret = 0;

    if (!bytes || !out)
        return -1;

    mdctx = EVP_MD_CTX_new();
    if (!mdctx)
        return -2;

    unsigned int r = sizeof(out->text);
    EVP_DigestInit_ex(mdctx, EVP_md5(), NULL);
    EVP_DigestUpdate(mdctx, bytes, bytes_sz);
    EVP_DigestFinal_ex(mdctx,
                       (unsigned char *) out->text, &r);
    EVP_MD_CTX_free(mdctx);

    // md5 does not support be in a chain with base64
    if (base64_encode(out->text, r, out))
        ret = -3;

    return ret;
}
