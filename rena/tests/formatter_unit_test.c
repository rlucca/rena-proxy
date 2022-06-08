
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
