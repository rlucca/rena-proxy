
// cheat time in miliseconds (2s default)
//#define CHEAT_TIME 500
#include "cheat.h"
#include "cheats.h"

#include "global.h"
#include "template.h"


CHEAT_DECLARE(
    // malloc e free nao posso trocar!
    const char redirect_example[] = "HTTP/1.0 302 Redirect\r\n"
                                    "Content-Type: text/html\r\n"
                                    "Connection: Close\r\n"
                                    "Server: RenaProxy\r\n"
                                    "Content-Length: 0\r\n"
                                    "Location: example.com\r\n\r\n";
    const char cookie_redirect_example[] = "HTTP/1.0 302 Redirect\r\n"
                                "Content-Type: text/html\r\n"
                                "Connection: Close\r\n"
                                "Server: RenaProxy\r\n"
                                "Content-Length: 0\r\n"
                                "Set-Cookie: renaproxy=X; Max-Age=86400\r\n"
                                "Location: example.com\r\n\r\n";
    const char unauthorized[] = "HTTP/1.0 401 Unauthorized\r\n"
                                "Content-Type: text/html\r\n"
                                "Connection: Close\r\n"
                                "Server: RenaProxy\r\n"
                                "Content-Length: 161\r\n\r\n"
                                "<html><head><title>ERROR CODE 401 Unauthoriz"
                                "ed</title></head><body><h1 style=\"text-alig"
                                "n:center;margin-top: 300px\">ERROR CODE 401 "
                                "Unauthorized</h1></body></html>";
)

CHEAT_TEST(generate_invalid_params,
    text_t tmp;
    char inout[2] = { 0, };

    int returned = generate_redirect_to(NULL, NULL, inout);
    cheat_assert_int32(returned, -1);

    returned = generate_redirect_to(&tmp, NULL, NULL);
    cheat_assert_int32(returned, -1);

    returned = generate_error(&tmp, 666);
    cheat_assert_int32(returned, -1);

    returned = generate_error(&tmp, -1);
    cheat_assert_int32(returned, -1);

    returned = generate_error(NULL, 401);
    cheat_assert(returned == -1); // using it here, to make compiler happy...
)

CHEAT_TEST(generate_redirect_to_example,
    char cookie[] = "X";
    text_t out;

    int returned = generate_redirect_to(&out, NULL, "example.com");
    cheat_assert_int32(returned, sizeof(redirect_example) - 1);
    cheat_assert_string(out.text, redirect_example);

    returned = generate_redirect_to(&out, cookie, "example.com");
    cheat_assert_int32(returned, sizeof(cookie_redirect_example) - 1);
    cheat_assert_string(out.text, cookie_redirect_example);
)

CHEAT_TEST(generate_error_401,
    text_t out;

    int returned = generate_error(&out, 401);
    cheat_assert_int32(returned, sizeof(unauthorized) - 1);
    cheat_assert_string(out.text, unauthorized);
)
