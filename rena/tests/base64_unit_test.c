
// cheat time in miliseconds (2s default)
//#define CHEAT_TIME 500
#include "cheat.h"
#include "cheats.h"

#include "global.h"
#include "base64.h"


CHEAT_DECLARE(
    // malloc e free nao posso trocar!
    char empty_case[] = "";
    char empty_result[] = "AA==";
    char long_case[] = "12345678901234567890123456789012345678901234567890123"
                       "45678901234567890123456789012345678901234567890123456"
                       "78901234567890123456789012345678901234567890123456789"
                       "01234567890123456789012345678901234567890123456789012"
                       "3456789012345678901234567890123456789012345";
    char long_result[] = "MTIzNDU2Nzg5MDEyMzQ1Njc4OTAxMjM0NTY3ODkwMTIzNDU2Nzg"
                         "5MDEyMzQ1Njc4OTAxMjM0NTY3ODkwMTIzNDU2Nzg5MDEyMzQ1Nj"
                         "c4OTAxMjM0NTY3ODkwMTIzNDU2Nzg5MDEyMzQ1Njc4OTAxMjM0N"
                         "TY3ODkwMTIzNDU2Nzg5MDEyMzQ1Njc4OTAxMjM0NTY3ODkwMTIz"
                         "NDU2Nzg5MDEyMzQ1Njc4OTAxMjM0NTY3ODkwMTIzNDU2Nzg5MDE"
                         "yMzQ1Njc4OTAxMjM0NTY3ODkwMTIzNDU2Nzg5MDEyMzQ1Njc4OT"
                         "AxMjM0NTY3ODkwMTIzNDU2Nzg5MDEyMzQ1";
)

CHEAT_TEST(base64_encode_all_NULL,

    int returned = base64_encode(NULL, 0, NULL);
    cheat_assert_int32(returned, -1);
)


CHEAT_TEST(base64_encode_input_NULL,
    text_t out;
    int returned = base64_encode(NULL, 0, &out);
    cheat_assert_int32(returned, -1);
)

CHEAT_TEST(base64_encode_empty_input,
    text_t out;
    int returned = base64_encode(empty_case, sizeof(empty_case), &out);
    cheat_assert_int32(returned, 0);
    cheat_assert_int32(out.size, sizeof(empty_result) - 1);
    cheat_assert_string(out.text, empty_result);
)

CHEAT_TEST(base64_encode_long_input,
    text_t out;
    int returned = base64_encode(long_case, sizeof(long_case)-1, &out);
    cheat_assert_int32(returned, 0);
    cheat_assert_string((const char *) out.text, long_result);
    cheat_assert_int32(out.size, sizeof(long_result)-1);
    cheat_assert(out.text[out.size] == 0);
)

CHEAT_TEST(base64_decode_all_NULL,

    int returned = base64_decode(NULL, 0, NULL);
    cheat_assert_int32(returned, -1);
)

CHEAT_TEST(base64_decode_input_NULL,
    text_t out;

    int returned = base64_decode(NULL, 0, &out);
    cheat_assert_int32(returned, -1);
)

CHEAT_TEST(base64_decode_output_NULL,
    text_t in = { 1, "" };

    int returned = base64_decode(in.text, in.size, NULL);
    cheat_assert_int32(returned, -1);
)

CHEAT_TEST(base64_decode_empty_case,
    text_t out;

    int returned = base64_decode(empty_result, sizeof(empty_result) - 1, &out);
    cheat_assert_int32(returned, 0);
    cheat_assert_int32(out.size, sizeof(empty_case));
    cheat_assert_string(out.text, empty_case);
)

CHEAT_TEST(base64_decode_long_case,
    text_t out;

    int returned = base64_decode(long_result, sizeof(long_result) - 1, &out);
    cheat_assert_int32(returned, 0);
    cheat_assert_int32(out.size, strlen(long_case));
    cheat_assert_int32(out.text[out.size], 0);
    cheat_assert_string(out.text, long_case);
)
