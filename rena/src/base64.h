#ifndef BASE64_H_
#define BASE64_H_

int base64_encode(const char *bytes, int bytes_sz, text_t *out);
int base64_decode(const char *bytes, int bytes_sz, text_t *out);

#endif
