
// cheat time in miliseconds (2s default)
//#define CHEAT_TIME 500
#include "cheat.h"
#include "cheats.h"

#include "md5.h"


CHEAT_DECLARE(
    // malloc e free nao posso trocar!
    char empty_case[] = "";
    char empty_result[] = "k7iFrf4NoInN9jSQT9WfcQ==";
    char long_case[] = "12345678901234567890123456789012345678901234567890123"
                       "45678901234567890123456789012345678901234567890123456"
                       "78901234567890123456789012345678901234567890123456789"
                       "01234567890123456789012345678901234567890123456789012"
                       "3456789012345678901234567890123456789012345";
    char long_result[] = "kd68y0OeFndaAB9cOnyYDg==";
)

CHEAT_TEST(md5_encode_all_NULL,

    int returned = md5_encode(NULL, 0, NULL, 0);
    cheat_assert_int32(returned, -1);
)


CHEAT_TEST(md5_encode_input_NULL,
    char out[128];
    int out_sz = sizeof(out);
    int returned = md5_encode(NULL, 0, out, &out_sz);
    cheat_assert_int32(returned, -1);
)

CHEAT_TEST(md5_encode_empty_input,
    char out[128];
    int out_sz = sizeof(out);
    int returned = md5_encode(empty_case, sizeof(empty_case), out, &out_sz);
    cheat_assert_int32(returned, 0);
    cheat_assert_int32(out_sz, sizeof(empty_result) - 1);
    cheat_assert_string(out, empty_result);
)

CHEAT_TEST(md5_encode_empty_input_without_output_space_informed,
    char out[128];
    int returned = md5_encode(empty_case, sizeof(empty_case), out, NULL);
    cheat_assert_int32(returned, -1);
)

CHEAT_TEST(md5_encode_long_input,
    char out[341] = { 0, };
    int out_sz = sizeof(out);
    int returned = md5_encode(long_case, sizeof(long_case)-1, out, &out_sz);
    cheat_assert_int32(returned, 0);
    cheat_assert_string((const char *) out, long_result);
    cheat_assert_int32(out_sz, sizeof(long_result)-1);
    cheat_assert(out[sizeof(out)-1] == 0);
)
