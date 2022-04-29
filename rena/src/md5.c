
#include "global.h"
#include <base64.h>
#include <openssl/bio.h>
#include <openssl/evp.h>


int md5_encode(const char *bytes, int bytes_sz, char *out, int *out_sz)
{
    EVP_MD_CTX *mdctx;
    int ret = 0;
    unsigned int r;

    if (!bytes || !out || !out_sz)
        return -1;

    r = *out_sz;
    mdctx = EVP_MD_CTX_new();
    if (!mdctx)
        return -2;

    EVP_DigestInit_ex(mdctx, EVP_md5(), NULL);
    EVP_DigestUpdate(mdctx, bytes, bytes_sz);
    EVP_DigestFinal_ex(mdctx, (unsigned char *) out, &r);
    EVP_MD_CTX_free(mdctx);

    // md5 does not support be in a chain with base64
    if (base64_encode(out, r, out, out_sz))
        ret = -3;

    return ret;
}
