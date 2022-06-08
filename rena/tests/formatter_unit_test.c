
// cheat time in miliseconds (2s default)
//#define CHEAT_TIME 500
#include "cheat.h"
#include "cheats.h"

#ifndef FORMATTER_H_
#include "formatter.c"
#endif

CHEAT_DECLARE(
    #define valid_case(Q, P, N, E) \
        cheat_assert_int32(formatter_create_handler(Q, P, N), E)
    struct formatter *trash_ptr = (struct formatter *) 0x10;
    struct formatter *okay_ptr = NULL;
    int foreach_fnc_test_count = 0;
    int foreach_fnc_test(struct chain_formatter *chain,
                         struct formatter *inout,
                         struct formatter_userdata *userdata)
    {
        (void) inout;
        (void) userdata;
        (void) chain;
        foreach_fnc_test_count++;
        return (foreach_fnc_test_count > 100) ? -1 : 0;
    }
)

CHEAT_SET_UP(
    foreach_fnc_test_count = 0;
)

CHEAT_TEST(find_modifier_should_return_minus_one_when_not_found,

    cheat_assert_int32(find_modifier('\t'), -1);
)

CHEAT_TEST(find_modifier_should_return_position_of_element,
    cheat_assert_int32(find_modifier('%'), 0);

    int last = sizeof(registered) / sizeof(*registered) - 2;
    cheat_assert_int32(find_modifier(registered[last].modifier), last);
    cheat_assert_int32(registered[last + 1].group_id, -1);
)

CHEAT_TEST(is_a_valid_input_test,
    valid_case(NULL, "%b",  2, 0);
    valid_case(NULL, "- -", 3, 0);
    valid_case(NULL, "%{aaa}i %{bb}i", 14, 0);
    valid_case(NULL, "%",  1, -2);
    valid_case(NULL, "% ", 2, -2);
    valid_case(NULL, "%{aaai", 6, -2);
    valid_case(NULL, "%}i", 3, -2);
    valid_case(NULL, "%{aaa} ", 7, -2);
    valid_case(NULL, "%{{aaa}i", 8, -2);
    valid_case(NULL, "%{%aaa}i", 8, -2);
    valid_case(NULL, NULL, 1, -1);
    valid_case(NULL, "", 1, -1);
    valid_case(NULL, "a", 0, -1);
    valid_case(NULL, NULL, 0, -1);
    valid_case(&trash_ptr, "%b", 2, -1);
)

CHEAT_TEST(create_list_head_ok,
    const char *s = (const char *) 0x10;
    struct formatter *ret = create_list_head(s, 33);
    cheat_assert(ret != NULL);
    cheat_assert(ret->first == NULL);
    cheat_assert(ret->last == NULL);
    cheat_assert(ret->format == s);
    cheat_assert_int32(ret->format_len, 33);
    free(ret);
)

CHEAT_TEST(list_add_on_empty_elements,
    struct formatter dummy = { NULL, 0, NULL, NULL };
    list_add_at_end(&dummy, 5, 1, -1);
    cheat_assert(dummy.first != NULL);
    cheat_assert(dummy.last != NULL);
    cheat_assert(dummy.last == dummy.first);

    cheat_assert_int32(dummy.first->position_start, 5);
    cheat_assert_int32(dummy.first->position_end, 1);
    cheat_assert_int32(dummy.first->registered_index, -1);
    cheat_assert(dummy.first->next == NULL);
    free(dummy.first);
)

CHEAT_TEST(list_add_on_one_or_more_elements,
    struct chain_formatter first = { 5, 1, -1, NULL };
    struct formatter dummy = { NULL, 0, &first, &first };
    list_add_at_end(&dummy, 7, 7, 3);
    cheat_assert(dummy.first == &first);
    cheat_assert(dummy.last != NULL);
    cheat_assert(dummy.last != dummy.first);

    cheat_assert_int32(dummy.last->position_start, 7);
    cheat_assert_int32(dummy.last->position_end, 7);
    cheat_assert_int32(dummy.last->registered_index, 3);
    cheat_assert(dummy.last->next == NULL);
    cheat_assert(dummy.first->next == dummy.last);
)

CHEAT_TEST(list_foreach_nulled_formatter_fail,

    char s[1] = { '1' };
    void *q = (void *) &s;
    cheat_assert_int32(list_foreach(NULL, q, q), -1);
)

CHEAT_TEST(list_foreach_nulled_handler_fail,
    char s[1] = { '1' };
    void *q = (void *) &s;
    cheat_assert_int32(list_foreach(q, q, NULL), -2);
)

CHEAT_TEST(list_foreach_zero_element_ok,
    struct formatter dummy = { NULL, 0, NULL, NULL };
    int ret = list_foreach(&dummy, NULL, foreach_fnc_test);
    cheat_assert_int32(ret, 0);
    cheat_assert_int32(foreach_fnc_test_count, 0);
)

CHEAT_TEST(list_foreach_one_element_ok,
    struct chain_formatter first = { 1, 2, 4, NULL };
    struct formatter dummy = { NULL, 0, &first, &first };
    int ret = list_foreach(&dummy, NULL, foreach_fnc_test);
    cheat_assert_int32(ret, 0);
    cheat_assert_int32(foreach_fnc_test_count, 1);
)

CHEAT_TEST(list_foreach_one_element_fail,
    struct chain_formatter first = { 1, 2, 4, NULL };
    struct formatter dummy = { NULL, 0, &first, &first };
    foreach_fnc_test_count = 666;
    int ret = list_foreach(&dummy, NULL, foreach_fnc_test);
    cheat_assert_int32(ret, -1);
    cheat_assert_int32(foreach_fnc_test_count, 667);
)

CHEAT_TEST(list_foreach_three_element_ok,
    struct chain_formatter third = { 3, 6, 17, NULL };
    struct chain_formatter second = { 3, 4, 0, &third };
    struct chain_formatter first = { 1, 2, 4, &second };
    struct formatter dummy = { NULL, 0, &first, &third };
    int ret = list_foreach(&dummy, NULL, foreach_fnc_test);
    cheat_assert_int32(ret, 0);
    cheat_assert_int32(foreach_fnc_test_count, 3);
)
