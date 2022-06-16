
// cheat time in miliseconds (2s default)
//#define CHEAT_TIME 500
#include "cheat.h"
#include "cheats.h"

#ifndef FORMATTER_H_
#define clients_get_timestamp fixed_timestamp
// localtime->gmtime to remove offset of timezone
#define localtime_r gmtime_r
#include "formatter.c"
#endif

CHEAT_DECLARE(
    #define valid_case(Q, P, N, E) \
        cheat_assert_int32(formatter_create_handler(Q, P, N), E)
    const char complex_sample[]
                    = "req:%h [%l] \"%u\" %{%Y.%m,%d@%k%M%S-}t%{header}i!";
    struct formatter *trash_ptr = (struct formatter *) 0x10;
    struct formatter *okay_ptr = NULL;
    int last = sizeof(registered) / sizeof(*registered) - 1;
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

    const time_t *fixed_timestamp(struct client_position *c)
    {
        static time_t epoch = 0;
        return &epoch;
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

    cheat_assert_int32(find_modifier(registered[last - 1].modifier), last - 1);
    cheat_assert_int32(registered[last].modifier, 0);
)

CHEAT_TEST(formatter_create_handler_test,
    valid_case(NULL, "%b",  2, 0);
    valid_case(NULL, "- -", 3, 0);
    valid_case(NULL, "%{aaa}i %{bb}i", 14, 0);
    valid_case(NULL, "%",  1, -2);
    valid_case(NULL, "% ", 2, -2);
    valid_case(NULL, "%{aaai", 6, -2);
    valid_case(NULL, "%}i", 3, -2);
    valid_case(NULL, "%i", 2, 0);
    valid_case(NULL, "%{aaa} ", 7, -2);
    valid_case(NULL, "%{{aaa}i", 8, -2);
    valid_case(NULL, "%{%aaa}i", 8, 0);
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

CHEAT_TEST(create_chain_from_format_literal_only_ok,
    const char sample[] = "10 mais vida -- e agora jose?";
    struct formatter temp = { sample, sizeof(sample) - 1, NULL, NULL };
    int ret = create_chain_from_format(&temp);
    cheat_assert_int32(ret, 0);
    cheat_assert(temp.first == temp.last);
    cheat_assert(temp.first != NULL);
    cheat_assert_int32(temp.first->position_start, 0);
    cheat_assert_int32(temp.first->position_end, 28);
    cheat_assert_int32(temp.first->registered_index, last);
    cheat_assert(temp.first->next == NULL);
)

CHEAT_TEST(create_chain_from_format_modifier_only_ok,
    const char sample[] = "%%";
    struct formatter temp = { sample, sizeof(sample) - 1, NULL, NULL };
    int ret = create_chain_from_format(&temp);
    cheat_assert_int32(ret, 0);
    cheat_assert(temp.first == temp.last);
    cheat_assert(temp.first != NULL);
    cheat_assert_int32(temp.first->position_start, 0);
    cheat_assert_int32(temp.first->position_end, 1);
    cheat_assert_int32(temp.first->registered_index, 0);
    cheat_assert(temp.first->next == NULL);
)

CHEAT_TEST(create_chain_from_format_modifier_only_repeated_ok,
    const char sample[] = "%%%%%%";
    struct formatter temp = { sample, sizeof(sample) - 1, NULL, NULL };
    int ret = create_chain_from_format(&temp);
    cheat_assert_int32(ret, 0);
    cheat_assert(temp.first != temp.last);
    struct chain_formatter *node = temp.first;
    cheat_assert(node != NULL);
    cheat_assert(node->next != NULL);
    cheat_assert_int32(node->position_start, 0);
    cheat_assert_int32(node->position_end, 1);
    cheat_assert_int32(node->registered_index, 0);

    node = node->next;
    cheat_assert(node->next != NULL);
    cheat_assert_int32(node->position_start, 2);
    cheat_assert_int32(node->position_end, 3);
    cheat_assert_int32(node->registered_index, 0);

    node = node->next;
    cheat_assert(node == temp.last);
    cheat_assert(node->next == NULL);
    cheat_assert_int32(node->position_start, 4);
    cheat_assert_int32(node->position_end, 5);
    cheat_assert_int32(node->registered_index, 0);
)

CHEAT_TEST(create_chain_from_format_modifier_with_expression_only_fail,
    const char sample[] = "%{porcento}%";
    struct formatter temp = { sample, sizeof(sample) - 1, NULL, NULL };
    int ret = create_chain_from_format(&temp);
    cheat_assert_int32(ret, -1);
    cheat_assert(temp.first == temp.last);
    cheat_assert(temp.first == NULL);
)

CHEAT_TEST(create_chain_from_format_mixing_modifier_with_literal_ok,
    const char sample[] = "10%% mais vida";
    struct formatter temp = { sample, sizeof(sample) - 1, NULL, NULL };
    int ret = create_chain_from_format(&temp);
    cheat_assert_int32(ret, 0);
    cheat_assert(temp.first != temp.last);
    cheat_assert(temp.first != NULL);
    cheat_assert(temp.last != NULL);
    cheat_assert(temp.first->next->next == temp.last);
    cheat_assert_int32(temp.first->position_start, 0);
    cheat_assert_int32(temp.first->position_end, 1);
    cheat_assert_int32(temp.first->registered_index, last);
    cheat_assert_int32(temp.first->next->position_start, 2);
    cheat_assert_int32(temp.first->next->position_end, 3);
    cheat_assert_int32(temp.first->next->registered_index, 0);
    cheat_assert_int32(temp.last->position_start, 4);
    cheat_assert_int32(temp.last->position_end, 13);
    cheat_assert_int32(temp.last->registered_index, last);
)

CHEAT_TEST(create_chain_from_format_modifier_with_expression_ok,
    const char sample[] = "%{header}i%{cabeca}i";
    struct formatter temp = { sample, sizeof(sample) - 1, NULL, NULL };
    int ret = create_chain_from_format(&temp);
    cheat_assert_int32(ret, 0);
    cheat_assert(temp.first != temp.last);
    cheat_assert(temp.first != NULL);
    cheat_assert(temp.last != NULL);
    cheat_assert_int32(temp.first->position_start, 0);
    cheat_assert_int32(temp.first->position_end, 9);
    cheat_assert_int32(temp.first->registered_index, 8);
    cheat_assert(temp.first->next == temp.last);
    cheat_assert_int32(temp.last->position_start, 10);
    cheat_assert_int32(temp.last->position_end, 19);
    cheat_assert_int32(temp.last->registered_index, 8);
)

CHEAT_TEST(create_chain_from_format_modifier_with_expression_repeated_fail,
    const char sample[] = "%{header}i%{dia}d";
    struct formatter temp = { sample, sizeof(sample) - 1, NULL, NULL };
    int ret = create_chain_from_format(&temp);
    cheat_assert_int32(ret, -1);
    cheat_assert(temp.first == temp.last);
    cheat_assert(temp.first == NULL);
)

CHEAT_TEST(create_chain_from_format_complex_2_ok,
    struct formatter temp = { complex_sample,
                              sizeof(complex_sample) - 1,
                              NULL, NULL };
    int ret = create_chain_from_format(&temp);
    cheat_assert_int32(ret, 0);
    cheat_assert(temp.first != temp.last);
    cheat_assert(temp.first != NULL);
    cheat_assert(temp.last != NULL);
    struct chain_formatter *node = temp.first;
    cheat_assert_int32(node->position_start, 0); // "req:"
    cheat_assert_int32(node->position_end, 3);
    cheat_assert_int32(node->registered_index, last);

    node = node->next;
    cheat_assert_int32(node->position_start, 4); // "%h"
    cheat_assert_int32(node->position_end, 5);
    cheat_assert_int32(node->registered_index, 1);

    node = node->next;
    cheat_assert_int32(node->position_start, 6); // " ["
    cheat_assert_int32(node->position_end, 7);
    cheat_assert_int32(node->registered_index, last);

    node = node->next;
    cheat_assert_int32(node->position_start, 8); // "%l"
    cheat_assert_int32(node->position_end, 9);
    cheat_assert_int32(node->registered_index, 2);

    node = node->next;
    cheat_assert_int32(node->position_start, 10); // "] \""
    cheat_assert_int32(node->position_end, 12);
    cheat_assert_int32(node->registered_index, last);

    node = node->next;
    cheat_assert_int32(node->position_start, 13); // "%u"
    cheat_assert_int32(node->position_end, 14);
    cheat_assert_int32(node->registered_index, 3);

    node = node->next;
    cheat_assert_int32(node->position_start, 15); // "\" "
    cheat_assert_int32(node->position_end, 16);
    cheat_assert_int32(node->registered_index, last);

    node = node->next;
    cheat_assert_int32(node->position_start, 17); // "%{%Y.%m,%d@%k%M%S-}t"
    cheat_assert_int32(node->position_end, 36);
    cheat_assert_int32(node->registered_index, 4);

    node = node->next;
    cheat_assert_int32(node->position_start, 37); // "%{header}i"
    cheat_assert_int32(node->position_end, 46);
    cheat_assert_int32(node->registered_index, 8);

    node = node->next;
    cheat_assert_int32(node->position_start, 47); // "!"
    cheat_assert_int32(node->position_end, 47);
    cheat_assert_int32(node->registered_index, last);

    cheat_assert(node == temp.last);
    cheat_assert(node->next == NULL);
)

CHEAT_TEST(copy_literal_invalid_pos_should_fail,

    struct formatter fo = { complex_sample, 4, NULL, NULL };
    struct chain_formatter cf = { 4, 2, -1, NULL };
    struct formatter_userdata fu = { NULL, 0, 0, 0, 0 };
    int ret = modifier_copy_literal(&cf, &fo, &fu);
    cheat_assert_int32(ret, -1);

    struct chain_formatter cf2 = { 2, 4, -1, NULL };
    ret = modifier_copy_literal(&cf2, &fo, &fu);
    cheat_assert_int32(ret, -1);
)

CHEAT_TEST(copy_literal_invalid_size_should_fail,

    struct formatter fo = { complex_sample, 4, NULL, NULL };
    struct chain_formatter cf = { 0, 3, -1, NULL };
    struct formatter_userdata fu = { NULL, 2, 0, 0, 0 };
    int ret = modifier_copy_literal(&cf, &fo, &fu);
    cheat_assert_int32(ret, -1);
)

CHEAT_TEST(copy_literal_ok,

    struct formatter fo = { complex_sample, 4, NULL, NULL };
    struct chain_formatter cf = { 0, 3, -1, NULL };
    char temp[MAX_STR];
    struct formatter_userdata fu = { temp, MAX_STR, 0, 0, 0 };
    int ret = modifier_copy_literal(&cf, &fo, &fu);
    cheat_assert_int32(ret, 0);
    cheat_assert(strncmp(complex_sample, temp, 4) == 0);
)

CHEAT_TEST(modifier_percentage_size_should_fail,

    struct formatter fo = { complex_sample, 4, NULL, NULL };
    struct chain_formatter cf = { 0, 3, -1, NULL };
    struct formatter_userdata fu = { NULL, 2, 2, 0, 0 };
    int ret = modifier_percentage(&cf, &fo, &fu);
    cheat_assert_int32(ret, -1);
)

CHEAT_TEST(modifier_percentage_ok,

    struct formatter fo = { complex_sample, 4, NULL, NULL };
    struct chain_formatter cf = { 0, 3, -1, NULL };
    char temp[MAX_STR];
    struct formatter_userdata fu = { temp, 2, 1, 0, 0 };
    int ret = modifier_percentage(&cf, &fo, &fu);
    cheat_assert_int32(ret, 0);
    cheat_assert_int32(temp[1], '%');
)

CHEAT_TEST(modifier_none_size_should_fail,

    struct formatter fo = { complex_sample, 4, NULL, NULL };
    struct chain_formatter cf = { 0, 3, -1, NULL };
    struct formatter_userdata fu = { NULL, 2, 2, 0, 0 };
    int ret = modifier_none(&cf, &fo, &fu);
    cheat_assert_int32(ret, -1);
)

CHEAT_TEST(modifier_none_ok,

    struct formatter fo = { complex_sample, 4, NULL, NULL };
    struct chain_formatter cf = { 0, 3, -1, NULL };
    char temp[MAX_STR];
    struct formatter_userdata fu = { temp, 2, 0, 0, 0 };
    int ret = modifier_none(&cf, &fo, &fu);
    cheat_assert_int32(ret, 0);
    cheat_assert_int32(temp[0], '-');
)

CHEAT_TEST(modifier_requester_formatted_date_literal_too_big_is_invalid,

    struct formatter fo = { "%{a}t", 5, NULL, NULL };
    struct chain_formatter cf = { 0, MAX_STR*2, -1, NULL };
    char temp[MAX_STR] = { 0, };
    struct formatter_userdata fu = { temp, 2, 0, 0, (void *) &temp};
    int ret = modifier_requester_formatted_date(&cf, &fo, &fu);
    cheat_assert_int32(ret, -1);
    cheat_assert_int32(temp[0], '\0');
)

CHEAT_TEST(modifier_requester_formatted_date_ok,

    struct formatter fo = { " %t-", 4, NULL, NULL };
    struct chain_formatter cf = { 0, 3, -1, NULL };
    char temp[MAX_STR] = { 0, };
    struct formatter_userdata fu = { temp, MAX_STR, 0, 0,
                                     (void *) temp };
    int ret = modifier_requester_formatted_date(&cf, &fo, &fu);
    cheat_assert_int32(ret, 0);
    ret = modifier_requester_formatted_date(&cf, &fo, &fu);
    cheat_assert_int32(ret, 0);

    cheat_assert_string(temp,
                        "[01/Jan/1970:00:00:00 +0000]"
                        "[01/Jan/1970:00:00:00 +0000]");
)

CHEAT_TEST(modifier_requester_formatted_date_optional_okay,

    struct formatter fo = { " %{[%d--]}t-", 12, NULL, NULL };
    struct chain_formatter cf = { 1, 10, -1, NULL };
    char temp[MAX_STR] = { 0, };
    struct formatter_userdata fu = { temp, MAX_STR, 0, 0,
                                     (void *) temp };
    int ret = modifier_requester_formatted_date(&cf, &fo, &fu);
    cheat_assert_int32(ret, 0);
    ret = modifier_requester_formatted_date(&cf, &fo, &fu);
    cheat_assert_int32(ret, 0);

    cheat_assert_string(temp, "[01--][01--]");
)

CHEAT_TEST(modifier_authenticated_user_to_be_done,

    struct formatter fo = { complex_sample, 4, NULL, NULL };
    struct chain_formatter cf = { 0, 3, -1, NULL };
    char temp[MAX_STR];
    struct formatter_userdata fu = { temp, 2, 0, 0, 0 };
    int ret = modifier_authenticated_user(&cf, &fo, &fu);
    cheat_assert_int32(ret, 0);
    cheat_assert_int32(temp[0], '-');
)
