
// cheat time in miliseconds (2s default)
//#define CHEAT_TIME 500
#include "cheat.h"
#include "cheats.h"

#include "global.h"
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

    int returned = md5_encode(NULL, 0, NULL);
    cheat_assert_int32(returned, -1);
)


CHEAT_TEST(md5_encode_input_NULL,
    text_t out;
    int returned = md5_encode(NULL, 0, &out);
    cheat_assert_int32(returned, -1);
)

CHEAT_TEST(md5_encode_empty_input,
    text_t out;
    int returned = md5_encode(empty_case, sizeof(empty_case), &out);
    cheat_assert_int32(returned, 0);
    cheat_assert_int32(out.size, sizeof(empty_result) - 1);
    cheat_assert_string(out.text, empty_result);
)

CHEAT_TEST(md5_encode_long_input,
    text_t out;
    int returned = md5_encode(long_case, sizeof(long_case)-1, &out);
    cheat_assert_int32(returned, 0);
    cheat_assert_string((const char *) out.text, long_result);
    cheat_assert_int32(out.size, sizeof(long_result)-1);
    cheat_assert(out.text[out.size] == 0);
)
