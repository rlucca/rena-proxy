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
    text_t cn={ 11, " renaproxy=" };
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

    void compare_n_string(const char *ev, const char *ex, int len, int index,
                          int line)
    {
        if (memcmp(ev, ex, len))
        {
            printf("[%d:line_test %d] failed: [%.*s] should be [%s]\n",
                   index, line, len, ev, ex);
        }
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

CHEAT_TEST(is_a_request_to_myself_host_value_test,

    const char host1[] = "www.sample.org";
    const char host2[] = "www.sample.org";
    const char host3[] = "www.example.org";
    int returned = is_a_request_to_myself(NULL, host1, NULL);
    cheat_assert_int32(returned, 0);
    returned = is_a_request_to_myself(NULL, host2, NULL);
    cheat_assert_int32(returned, 0);
    returned = is_a_request_to_myself(NULL, host3, NULL);
    cheat_assert_int32(returned, 0);
    returned = is_a_request_to_myself(NULL, NULL, NULL);
    cheat_assert_int32(returned, 1);
    returned = is_a_request_to_myself(NULL, "", NULL);
    cheat_assert_int32(returned, 1);
    returned = is_a_request_to_myself(NULL, "Example.orG", NULL);
    cheat_assert_int32(returned, 1);
)

CHEAT_TEST(is_a_request_to_myself_third_param_test,

    const char host1[] = "www.sample.org";
    const char host2[] = "www.sample.org";
    const char host3[] = "www.example.org";
    text_t expected = { 26, "ceCil; domain=.example.org" };
    text_t empty = { 5, "ceCil" };
    text_t out;

    memcpy(&out, &empty, sizeof(empty));
    is_a_request_to_myself(NULL, host1, &out);
    cheat_assert_int32(expected.size, out.size);
    cheat_assert_string(expected.text, out.text);

    memcpy(&out, &empty, sizeof(empty));
    is_a_request_to_myself(NULL, host2, &out);
    cheat_assert_int32(expected.size, out.size);
    cheat_assert_string(expected.text, out.text);

    memcpy(&out, &empty, sizeof(empty));
    is_a_request_to_myself(NULL, host3, &out);
    cheat_assert_int32(expected.size, out.size);
    cheat_assert_string(expected.text, out.text);

    memcpy(&out, &empty, sizeof(empty));
    is_a_request_to_myself(NULL, NULL, &out);
    cheat_assert_int32(empty.size, out.size);
    cheat_assert_string(empty.text, out.text);

    memcpy(&out, &empty, sizeof(empty));
    is_a_request_to_myself(NULL, "", &out);
    cheat_assert_int32(empty.size, out.size);
    cheat_assert_string(empty.text, out.text);

    memcpy(&out, &empty, sizeof(empty));
    is_a_request_to_myself(NULL, "Example.orG", &out);
    cheat_assert_int32(expected.size, out.size);
    cheat_assert_string(expected.text, out.text);

    memcpy(&out, &empty, sizeof(empty));
    out.size = 1024; // not enough space test
    is_a_request_to_myself(NULL, "Example.orG", &out);
    cheat_assert_int32(1024, out.size);
    cheat_assert_string(empty.text, out.text);
)

CHEAT_TEST(find_header_test,

    text_t jean = { 6, "X-Jean"};
    const char *no_header[] = { "X-Bean: user.uiid=X-Jean:44422" };
    const char *only_header[] = { "X-Jean: 2022-05-27-34-22+0" };
    const char *headers3_s[] = { "X-Jean: 2022-05-27-34-22+0",
                                 "X-Bean: user.uiid=X-Jean:44422",
                                 "X-Mico: 44" };
    const char *headers3_m[] = { "X-Bean: user.uiid=X-Jean:44422",
                                 "X-Jean: 2022-05-27-34-22+0",
                                 "X-Mico: 44" };
    const char *headers3_e[] = { "X-Bean: user.uiid=X-Jean:44422",
                                 "X-Mico: 44",
                                 "X-Jean: 2022-05-27-34-22+0" };
    const char *headers3_n[] = { "X-Bean: user.uiid=X-Jean:44422",
                                 "X-Mico: 44",
                                 "X-Jenn: 2022-05-27-34-22+0" };
    struct http headers1_empty = { NULL,
                               0, NULL, NULL,
                               0, 0, 0, NULL, 0, 0, 0, 0, ""};
    struct http headers2_no_header = { NULL,
                               1, no_header, NULL,
                               0, 0, 0, NULL, 0, 0, 0, 0, ""};
    struct http headers3_only_header = { NULL,
                               1, only_header, NULL,
                               0, 0, 0, NULL, 0, 0, 0, 0, ""};
    struct http headers4_no_header = { NULL,
                               3, headers3_n, NULL,
                               0, 0, 0, NULL, 0, 0, 0, 0, ""};
    struct http headers5_at_start = { NULL,
                               3, headers3_s, NULL,
                               0, 0, 0, NULL, 0, 0, 0, 0, ""};
    struct http headers6_at_middle = { NULL,
                               3, headers3_m, NULL,
                               0, 0, 0, NULL, 0, 0, 0, 0, ""};
    struct http headers7_at_end = { NULL,
                               3, headers3_e, NULL,
                               0, 0, 0, NULL, 0, 0, 0, 0, ""};
    int returned = 0;

    returned = find_header(&headers1_empty, &jean);
    cheat_assert_int32(returned, -1);

    returned = find_header(&headers2_no_header, &jean);
    cheat_assert_int32(returned, -1);

    returned = find_header(&headers3_only_header, &jean);
    cheat_assert_int32(returned, 0);

    returned = find_header(&headers4_no_header, &jean);
    cheat_assert_int32(returned, -1);

    returned = find_header(&headers5_at_start, &jean);
    cheat_assert_int32(returned, 0);

    returned = find_header(&headers6_at_middle, &jean);
    cheat_assert_int32(returned, 1);

    returned = find_header(&headers7_at_end, &jean);
    cheat_assert_int32(returned, 2);
)

CHEAT_TEST(extract_substring_location_of_cookie,

    struct http data = { NULL,
                         5, NULL, NULL,
                         0, 0, 0, NULL, 0, 0, 0, 125,
                         "GET / HTTP/1.0\r\n"
                         "X-Bean: user.uiid=X-Jean:44422\r\n"
                         "X-Mico: 44\r\n"
                         "Host: www.sample.org\r\n"
                         "X-Jean: 21312356\r\n"
                         "Cookie: renaproxy=42\r\n\r\n."};
    const char *headers[] = { data.buffer + 16, data.buffer + 48,
                              data.buffer + 60, data.buffer + 82,
                              data.buffer + 100 };
    int headers_len[] = { 30, 10, 20, 16, 20 };
    struct http data2 = { NULL,
                         5, NULL, NULL,
                         0, 0, 0, NULL, 0, 0, 0, 135,
                         "GET / HTTP/1.0\r\n"
                         "X-Bean: user.uiid=X-Jean:44422\r\n"
                         "X-Mico: 44\r\n"
                         "Host: www.sample.org\r\n"
                         "X-Jean: 21312356\r\n"
                         "Cookie: renaproxy=42; tt=12345\r\n\r\n."};
    const char *headers2[] = { data2.buffer + 16, data2.buffer + 48,
                              data2.buffer + 60, data2.buffer + 82,
                              data2.buffer + 100 };
    int headers2_len[] = { 30, 10, 20, 16, 30 };
    struct http data3 = { NULL,
                         5, NULL, NULL,
                         0, 0, 0, NULL, 0, 0, 0, 135,
                         "GET / HTTP/1.0\r\n"
                         "X-Bean: user.uiid=X-Jean:44422\r\n"
                         "X-Mico: 44\r\n"
                         "Host: www.sample.org\r\n"
                         "X-Jean: 21312356\r\n"
                         "Cookie: tt=12345; renaproxy=42\r\n\r\n."};
    const char *headers3[] = { data3.buffer + 16, data3.buffer + 48,
                              data3.buffer + 60, data3.buffer + 82,
                              data3.buffer + 100 };
    int headers3_len[] = { 30, 10, 20, 16, 30 };
    struct http data4 = { NULL,
                         5, NULL, NULL,
                         0, 0, 0, NULL, 0, 0, 0, 135,
                         "GET / HTTP/1.0\r\n"
                         "X-Bean: user.uiid=X-Jean:44422\r\n"
                         "X-Mico: 44\r\n"
                         "Host: www.sample.org\r\n"
                         "X-Jean: 21312356\r\n"
                         "Cookie: tt=12345\r\n\r\n."};
    const char *headers4[] = { data4.buffer + 16, data4.buffer + 48,
                              data4.buffer + 60, data4.buffer + 82,
                              data4.buffer + 100 };
    int headers4_len[] = { 30, 10, 20, 16, 16 };
    struct http data5 = { NULL,
                         5, NULL, NULL,
                         0, 0, 0, NULL, 0, 0, 0, 135,
                         "GET / HTTP/1.0\r\n"
                         "X-Bean: user.uiid=X-Jean:44422\r\n"
                         "X-Mico: 44\r\n"
                         "Host: www.sample.org\r\n"
                         "X-Jean: 21312356\r\n"
                         "Cookie: tt=1; renaproxy=42; mm=0\r\n\r\n."};
    const char *headers5[] = { data5.buffer + 16, data5.buffer + 48,
                              data5.buffer + 60, data5.buffer + 82,
                              data5.buffer + 100 };
    int headers5_len[] = { 30, 10, 20, 16, 32 };
    int loc_empty[2] = { -1, -1 };
    int loc[2] = { -1, -1 };
    data.headers = headers;
    data.headers_length = headers_len;
    data2.headers = headers2;
    data2.headers_length = headers2_len;
    data3.headers = headers3;
    data3.headers_length = headers3_len;
    data4.headers = headers4;
    data4.headers_length = headers4_len;
    data5.headers = headers5;
    data5.headers_length = headers5_len;

    memcpy(loc, loc_empty, sizeof(loc_empty));
    int returned = extract_substring_location_of_cookie(&loc, &data, &cn, 4);
    cheat_assert_int32(returned, 1);
    cheat_assert_int32(loc[0], 6);
    cheat_assert_int32(loc[1], 20);

    memcpy(loc, loc_empty, sizeof(loc_empty));
    returned = extract_substring_location_of_cookie(&loc, &data2, &cn, 4);
    cheat_assert_int32(returned, 0);
    cheat_assert_int32(loc[0], 6);
    cheat_assert_int32(loc[1], 20);

    memcpy(loc, loc_empty, sizeof(loc_empty));
    returned = extract_substring_location_of_cookie(&loc, &data3, &cn, 4);
    cheat_assert_int32(returned, 0);
    cheat_assert_int32(loc[0], 16);
    cheat_assert_int32(loc[1], 30);

    memcpy(loc, loc_empty, sizeof(loc_empty));
    returned = extract_substring_location_of_cookie(&loc, &data5, &cn, 4);
    cheat_assert_int32(returned, 0);
    cheat_assert_int32(loc[0], 12);
    cheat_assert_int32(loc[1], 26);

    memcpy(loc, loc_empty, sizeof(loc_empty));
    returned = extract_substring_location_of_cookie(&loc, &data4, &cn, 4);
    cheat_assert_int32(returned, 0);
    cheat_assert_int32(loc[0], -1);
    cheat_assert_int32(loc[1], -1);
)

CHEAT_TEST(remove_substring_cookie,

    struct http data = { NULL,
                         5, NULL, NULL,
                         0, 0, 0, NULL, 0, 0, 0, 125,
                         "GET / HTTP/1.0\r\n"
                         "X-Bean: user.uiid=X-Jean:44422\r\n"
                         "X-Mico: 44\r\n"
                         "Host: www.sample.org\r\n"
                         "X-Jean: 21312356\r\n"
                         "Cookie: renaproxy=42\r\n\r\n."};
    const char *headers[] = { data.buffer + 16, data.buffer + 48,
                              data.buffer + 60, data.buffer + 82,
                              data.buffer + 100 };
    int headers_len[] = { 30, 10, 20, 16, 20 };
    struct http data2 = { NULL,
                         5, NULL, NULL,
                         0, 0, 0, NULL, 0, 0, 0, 121,
                         "GET / HTTP/1.0\r\n"
                         "X-Bean: user.uiid=X-Jean:44422\r\n"
                         "X-Mico: 44\r\n"
                         "Host: www.sample.org\r\n"
                         "X-Jean: 21312356\r\n"
                         "Cookie: tt=12345\r\n\r\n."};
    const char *headers2[] = { data2.buffer + 16, data2.buffer + 48,
                              data2.buffer + 60, data2.buffer + 82,
                              data2.buffer + 100 };
    int headers2_len[] = { 30, 10, 20, 16, 16 };
    struct http data3 = { NULL,
                         5, NULL, NULL,
                         0, 0, 0, NULL, 0, 0, 0, 137,
                         "GET / HTTP/1.0\r\n"
                         "X-Bean: user.uiid=X-Jean:44422\r\n"
                         "X-Mico: 44\r\n"
                         "Host: www.sample.org\r\n"
                         "X-Jean: 21312356\r\n"
                         "Cookie: renaproxy=42; tt=1; mm=0\r\n\r\n."};
    const char *headers3[] = { data3.buffer + 16, data3.buffer + 48,
                              data3.buffer + 60, data3.buffer + 82,
                              data3.buffer + 100 };
    int headers3_len[] = { 30, 10, 20, 16, 32 };
    char buffer3[MAX_STR] = "GET / HTTP/1.0\r\n"
                         "X-Bean: user.uiid=X-Jean:44422\r\n"
                         "X-Mico: 44\r\nHost: www.sample.org\r\n"
                         "X-Jean: 21312356\r\n"
                         "Cookie: tt=1; mm=0\r\n\r\n.t=1; mm=0\r\n\r\n.";
    struct http data4 = { NULL,
                         5, NULL, NULL,
                         0, 0, 0, NULL, 0, 0, 0, 137,
                         "GET / HTTP/1.0\r\n"
                         "X-Bean: user.uiid=X-Jean:44422\r\n"
                         "X-Mico: 44\r\n"
                         "Host: www.sample.org\r\n"
                         "X-Jean: 21312356\r\n"
                         "Cookie: tt=1; renaproxy=42; mm=0\r\n\r\n."};
    char buffer4[MAX_STR] = "GET / HTTP/1.0\r\n"
                         "X-Bean: user.uiid=X-Jean:44422\r\n"
                         "X-Mico: 44\r\nHost: www.sample.org\r\n"
                         "X-Jean: 21312356\r\n"
                         "Cookie: tt=1; mm=0\r\n\r\n.=42; mm=0\r\n\r\n.";
    const char *headers4[] = { data4.buffer + 16, data4.buffer + 48,
                              data4.buffer + 60, data4.buffer + 82,
                              data4.buffer + 100 };
    int headers4_len[] = { 30, 10, 20, 16, 32 };
    struct http data5 = { NULL,
                         5, NULL, NULL,
                         0, 0, 0, (const char *) 136, 0, 0, 0, 137,
                         "GET / HTTP/1.0\r\nCookie: tt=1; mm=0; renaproxy=42"
                         "\r\nX-Bean: user.uiid=X-Jean:44422\r\nX-Mico: 44\r\n"
                         "Host: www.sample.org\r\nX-Jean: 21312356\r\n\r\n." };
    char buffer5[MAX_STR] = "GET / HTTP/1.0\r\nCookie: tt=1; mm=0\r\n"
                            "X-Bean: user.uiid=X-Jean:44422\r\nX-Mico:"
                            " 44\r\nHost: www.sample.org\r\nX-Jean: "
                            "21312356\r\n\r\n. 21312356\r\n\r\n.";
    char *headers_expected[] = { "X-Bean", "X-Mico", "Host",
                                 "X-Jean", NULL };
    char *headers1_expected[] = { "X-Bean", "X-Mico", "Host",
                                 "X-Jean", "Cookie", NULL };
    char *headers5_expected[] = { "Cookie", "X-Bean", "X-Mico", "Host",
                                 "X-Jean", NULL };
    const char *headers5[] = { data5.buffer + 16, data5.buffer + 50,
                              data5.buffer + 82, data5.buffer + 94,
                              data5.buffer + 116 };
    int headers5_len[] = { 32, 30, 10, 20, 16 };
    int loc1[2] = { 6, 20 };
    int loc2[2] = { -1, -1 };
    int loc3[2] = { 6, 20 };
    int loc4[2] = { 12, 26 };
    int loc5[2] = { 18, 32 };
    data.payload = data.buffer + 124;
    data.headers = headers;
    data.headers_length = headers_len;
    data2.headers = headers2;
    data2.headers_length = headers2_len;
    data3.headers = headers3;
    data3.headers_length = headers3_len;
    data4.headers = headers4;
    data4.headers_length = headers4_len;
    data5.headers = headers5;
    data5.headers_length = headers5_len;

    remove_substring_cookie(&data, 1, 4, &loc1);
    cheat_assert_int32(data.headers_used, 4);
    cheat_assert_int32(data.buffer_used, 103);
    cheat_assert(data.buffer + 102 == data.payload);

    for (int i = 0; headers_expected[i] != NULL; i++)
    {
        compare_n_string(data.headers[i], headers_expected[i],
                         strlen(headers_expected[i]), i, __LINE__);
    }

    remove_substring_cookie(&data2, 0, 4, &loc2);
    cheat_assert_int32(data2.headers_used, 5);
    cheat_assert_int32(data2.buffer_used, 121);
    cheat_assert(NULL == data2.payload);

    for (int i = 0; headers_expected[i] != NULL; i++)
    {
        compare_n_string(data2.headers[i], headers1_expected[i],
                         strlen(headers1_expected[i]), i, __LINE__);
    }

    remove_substring_cookie(&data3, 0, 4, &loc3);
    cheat_assert_int32(data3.headers_used, 5);
    cheat_assert_int32(data3.buffer_used, 123);
    cheat_assert((const char *) -14 == data3.payload);
    cheat_assert_string(data3.buffer, buffer3);

    for (int i = 0; headers_expected[i] != NULL; i++)
    {
        compare_n_string(data3.headers[i], headers1_expected[i],
                         strlen(headers1_expected[i]), i, __LINE__);
    }

    remove_substring_cookie(&data4, 0, 4, &loc4);
    cheat_assert_int32(data4.headers_used, 5);
    cheat_assert_int32(data4.buffer_used, 123);
    cheat_assert((const char *) -14 == data4.payload);
    cheat_assert_string(data4.buffer, buffer4);

    for (int i = 0; headers_expected[i] != NULL; i++)
    {
        compare_n_string(data4.headers[i], headers1_expected[i],
                         strlen(headers1_expected[i]), i, __LINE__);
    }

    remove_substring_cookie(&data5, 0, 0, &loc5);
    cheat_assert_int32(data5.headers_used, 5);
    cheat_assert_int32(data5.buffer_used, 123);
    cheat_assert((const char *) 122 == data5.payload);
    cheat_assert_string(data5.buffer, buffer5);

    for (int i = 0; headers5_expected[i] != NULL; i++)
    {
        compare_n_string(data5.headers[i], headers5_expected[i],
                         strlen(headers5_expected[i]), i, __LINE__);
    }
)
