#ifndef _TEMPLATE_H_
#define _TEMPLATE_H_

int generate_redirect_to(char *out, int out_sz,
                         const char *cookie, const char *url);
int generate_error(char *out, int out_sz, int error);

#endif
