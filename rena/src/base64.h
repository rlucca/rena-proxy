#ifndef BASE64_H_
#define BASE64_H_

int base64_encode(const char *bytes, int bytes_sz, char *out, int *out_sz);
int base64_decode(const char *bytes, int bytes_sz, char *out, int *out_sz);

#endif
