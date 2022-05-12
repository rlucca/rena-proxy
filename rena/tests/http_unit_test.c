#define _GNU_SOURCE

// cheat time in miliseconds (2s default)
//#define CHEAT_TIME 500
#include "cheat.h"
#include "cheats.h"

#ifndef HTTP_H_
#include "http.c"
#endif

CHEAT_DECLARE(
    // database mocks! and remember to not try to mock malloc, realloc and free
    const char domain[]=".example.org";
    const char *database_get_suffix(struct rena *no_use_1)
    { return domain; }
    struct database_object *database_instance_create(struct rena *_no_use_1,
                                                     int _no_use_2)
    { return NULL; }
    di_output_e database_instance_lookup(struct database_object *_no_use_1,
                                         char _no_use_2,
                                         const char ** const _no_use_3,
                                         int * const _no_use_4)
    { return 0; }
    void database_instance_add_input(struct database_object * _no_use_1,
                                         char _no_use_2)
    { }
    void database_instance_get_holding(struct database_object * _no_use_1,
                                       const char ** const _no_use_2,
                                       int * const _no_use_3,
                                       int _no_use_4)
    { }
    int database_instance_get_holding_size(struct database_object *_no_use_1)
    { return 0; }
    void database_instance_set_context(struct database_object * _no_use_1,
                                       context_t * _no_use_2)
    {  }
    context_t *database_instance_get_context(struct database_object * no_use_1)
    { return NULL; }
    void database_instance_destroy(struct database_object ** _no_use_1)
    {  }
    int database_verify_userlist(struct rena * _no_use_1,
                                 const char * _no_use_2[2],
                                 const long unsigned int  _no_use_3[2])
    { return 0; }

    struct http test1 = {
        NULL, 0, NULL, NULL, 0, 0, 0,
        NULL, 0, 0, 0, 963,
        "HTTP/1.1 302 Found\r\nConnection: close\r\nalt-svc: h3=\":443\";ma=8"
        "6400,h3-29=\":443\";ma=86400,h3-27=\":443\";ma=86400\r\nCache-Contro"
        "l: no-cache, no-store, max-age=0, must-revalidate\r\nExpires: 0\r\nP"
        "ragma: no-cache\r\nSet-Cookie: idp_session=sVERSION_1aee3324c-a278-4"
        "be6-b7d6-6d3fe89563b4; Domain=.sample.org; Path=/; Secure; HttpOnly"
        "\r\nSet-Cookie: idp_session_http=hVERSION_18430d2aa-22de-4949-9dad-d"
        "565185bf58c; Domain=.sample.org; Path=/; HttpOnly\r\nSet-Cookie: idp"
        "_marker=1cc0e7a4-a2b1-4a86-a721-03fb3da9f143; Domain=.sample.org; Pa"
        "th=/; Max-Age=315360000; HttpOnly\r\nStrict-Transport-Security: max-"
        "age=31536000\r\nX-B3-Spanid: 02f381764de93e85\r\nX-B3-Traceid: 33499"
        "e1b1d380bad02f381764de93e85\r\nX-Content-Type-Options: nosniff\r\nX-"
        "Frame-Options: DENY\r\nX-Vcap-Request-Id: 3b91cee3-e1f0-4d77-7688-2c"
        "ef1dc6bde3\r\nX-Xss-Protection: 1; mode=block\r\nDate: Thu, 12 May 2"
        "022 14:38:39 GMT\r\nX-Served-By: cache-cgh11128-CGH\r\nX-Timer: S165"
        "2366320.587588,VS0,VE207\r\nVary: x-forwarded-proto\r\n\r\n."
    };

    int callback_test_count = 0;
    int callback_test(struct rena *r, struct http *h, char *b, char *e, int *u)
    {
        int bhistory[] = { 284, 407, 506, 0 };
        int ehistory[] = { 295, 418, 517, 0 };

        cheat_assert(callback_test_count < 3);
        cheat_assert(b == bhistory[callback_test_count] + h->buffer);
        cheat_assert(e == ehistory[callback_test_count] + h->buffer);
        cheat_assert(h == &test1);
        callback_test_count++;
        *u = 0;
        return 0;
    }
)

CHEAT_SET_UP(

    test1.payload = (const char *) (test1.buffer + test1.buffer_used - 1);
    callback_test_count = 0;
)

CHEAT_TEST(pretest,

    const char *returned = database_get_suffix(NULL);
    cheat_assert_string(returned, domain);
    cheat_assert_int32(test1.payload[0], '.');
)

CHEAT_TEST(apply_on_domain_test_callback,

    int u = 666;
    int returned = apply_on_domain(NULL, &test1, callback_test, &u);
    cheat_assert_int32(returned, 1);
    cheat_assert_int32(u, 0);
    cheat_assert_int32(callback_test_count, 3);
)

CHEAT_TEST(apply_on_domain_count_need,


    int u = 0;
    int returned = apply_on_domain(NULL, &test1, size_need_for_suffix, &u);
    cheat_assert_int32(returned, 1);
    cheat_assert_int32(u, 3);
)

CHEAT_TEST(apply_on_domain_replace_by_suffix,

    const char result[] = "HTTP/1.1 302 Found\r\nConnection: close\r\nalt-sv"
        "c: h3=\":443\";ma=86400,h3-29=\":443\";ma=86400,h3-27=\":443\";ma=86"
        "400\r\nCache-Control: no-cache, no-store, max-age=0, must-revalidate"
        "\r\nExpires: 0\r\nPragma: no-cache\r\nSet-Cookie: idp_session=sVERSI"
        "ON_1aee3324c-a278-4be6-b7d6-6d3fe89563b4; Domain=.example.org; Path="
        "/; Secure; HttpOnly\r\nSet-Cookie: idp_session_http=hVERSION_18430d2"
        "aa-22de-4949-9dad-d565185bf58c; Domain=.example.org; Path=/; HttpOnl"
        "y\r\nSet-Cookie: idp_marker=1cc0e7a4-a2b1-4a86-a721-03fb3da9f143; Do"
        "main=.example.org; Path=/; Max-Age=315360000; HttpOnly\r\nStrict-Tra"
        "nsport-Security: max-age=31536000\r\nX-B3-Spanid: 02f381764de93e85\r"
        "\nX-B3-Traceid: 33499e1b1d380bad02f381764de93e85\r\nX-Content-Type-O"
        "ptions: nosniff\r\nX-Frame-Options: DENY\r\nX-Vcap-Request-Id: 3b91c"
        "ee3-e1f0-4d77-7688-2cef1dc6bde3\r\nX-Xss-Protection: 1; mode=block\r"
        "\nDate: Thu, 12 May 2022 14:38:39 GMT\r\nX-Served-By: cache-cgh11128"
        "-CGH\r\nX-Timer: S1652366320.587588,VS0,VE207\r\nVary: x-forwarded-p"
        "roto\r\n\r\n.";

    int returned = apply_on_domain(NULL, &test1, copy_suffix, NULL);
    cheat_assert_int32(returned, 1);
    cheat_assert_string(test1.buffer, result);
    cheat_assert_int32(test1.buffer_used, sizeof(result) - 1);
)
