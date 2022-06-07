
// cheat time in miliseconds (2s default)
//#define CHEAT_TIME 500
#include "cheat.h"
#include "cheats.h"

#ifndef FORMATTER_H_
#include "formatter.c"
#endif

CHEAT_DECLARE(
    #define valid_case(P, N, E) \
        cheat_assert_int32(formatter_create_handler(NULL, P, N), E)
)

CHEAT_SET_UP(
)

CHEAT_TEAR_DOWN(
)

CHEAT_TEST(find_modifier_should_return_minus_one_when_not_found,

    cheat_assert_int32(find_modifier('\t'), -1);
)

CHEAT_TEST(find_modifier_should_return_position_of_element,
    cheat_assert_int32(find_modifier('%'), 0);

    int last = sizeof(registered) / sizeof(*registered) - 2;
    cheat_assert_int32(find_modifier(registered[last].modifier), last);
    cheat_assert(registered[last + 1].fnc == NULL);
)

CHEAT_TEST(is_a_valid_input_test,

    valid_case("%b",  2, 0);
    valid_case("- -", 3, 0);
    valid_case("%{aaa}i", 7, 0);
    valid_case("%{{%aaa}i", 9, 0);
    valid_case("%",  1, -2);
    valid_case("% ", 2, -2);
    valid_case("%{aaai", 6, -2);
    valid_case("%}i", 3, -2);
    valid_case("%{aaa} ", 7, -2);
    valid_case(NULL, 1, -1);
    valid_case("", 1, -1);
    valid_case("a", 0, -1);
    valid_case(NULL, 0, -1);
)
