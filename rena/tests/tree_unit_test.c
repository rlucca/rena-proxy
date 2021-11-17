// cheat time in miliseconds (2s default)
//#define CHEAT_TIME 500
#include "cheat.h"

#include "tree.h"

CHEAT_TEST(call_create_and_destroy,
    void *o = tree_create_node('2');
    cheat_assert(o != NULL);
    tree_destroy((tree_node_t **) &o);
    cheat_assert(o == NULL);
)

CHEAT_TEST(call_create_and_sibling_ending,
    tree_node_t *o = tree_create_node('2');
    cheat_assert(o != NULL);
    tree_node_t *s = tree_add_sibling(o, '5');
    cheat_assert(o->next_sibling == s);
    cheat_assert(o->next_sibling->value == s->value);
    tree_destroy(&o);
    cheat_assert(o == NULL);
)

CHEAT_TEST(call_create_and_sibling_before_ending,
    tree_node_t *o = tree_create_node('2');
    cheat_assert(o != NULL);
    tree_add_sibling(o, '5');
    tree_node_t *s = tree_add_sibling(o, '3');
    cheat_assert(o->next_sibling == s);
    cheat_assert(o->next_sibling->value == s->value);
    tree_destroy(&o);
    cheat_assert(o == NULL);
)

CHEAT_TEST(call_create_and_sibling_before_start,
    tree_node_t *o = tree_create_node('2');
    cheat_assert(o != NULL);
    tree_add_sibling(o, '5');
    tree_node_t *s = tree_add_sibling(o, '1');
    cheat_assert(s->next_sibling == o);
    cheat_assert(s->next_sibling->value == o->value);
    tree_destroy(&s);
    cheat_assert(s == NULL);
)

CHEAT_TEST(call_create_and_sibling_ending_repeat,
    tree_node_t *o = tree_create_node('2');
    cheat_assert(o != NULL);
    tree_node_t *s = tree_add_sibling(o, '5');
    tree_node_t *d = tree_add_sibling(o, '5');
    cheat_assert(o->next_sibling == s);
    cheat_assert(o->next_sibling->value == s->value);
    cheat_assert(s == d);
    tree_destroy(&o);
    cheat_assert(o == NULL);
)

CHEAT_TEST(call_create_and_sibling_middle_repeat,
    tree_node_t *o = tree_create_node('2');
    cheat_assert(o != NULL);
    tree_add_sibling(o, '5');
    tree_node_t *s = tree_add_sibling(o, '3');
    tree_node_t *d = tree_add_sibling(o, '3');
    cheat_assert(o->next_sibling == s);
    cheat_assert(o->next_sibling->value == s->value);
    cheat_assert(s == d);
    tree_destroy(&o);
    cheat_assert(o == NULL);
)

CHEAT_TEST(call_create_and_sibling_start_repeat,
    tree_node_t *o = tree_create_node('2');
    cheat_assert(o != NULL);
    tree_add_sibling(o, '5');
    tree_node_t *s = tree_add_sibling(o, '1');
    tree_node_t *d = tree_add_sibling(s, '1');
    cheat_assert(s->next_sibling == o);
    cheat_assert(s->next_sibling->value == o->value);
    cheat_assert(s == d);
    tree_destroy(&s);
    cheat_assert(s == NULL);
)

CHEAT_TEST(call_create_and_sibling_start2_repeat,
    tree_node_t *o = tree_create_node('2');
    cheat_assert(o != NULL);
    tree_node_t *s = tree_add_sibling(o, '5');
    tree_node_t *d = tree_add_sibling(o, '2');
    cheat_assert(o->next_sibling == s);
    cheat_assert(o->next_sibling->value == s->value);
    cheat_assert(o == d);
    tree_destroy(&o);
    cheat_assert(o == NULL);
)

CHEAT_TEST(call_create_and_child,
    tree_node_t *o = tree_create_node('2');
    cheat_assert(o != NULL);
    tree_node_t *s = tree_add_child(o, '5');
    cheat_assert(o->next_child == s);
    cheat_assert(o->next_child->value == s->value);
    tree_destroy(&o);
    cheat_assert(o == NULL);
)

CHEAT_TEST(call_create_and_child_before_other,
    tree_node_t *o = tree_create_node('2');
    cheat_assert(o != NULL);
    tree_node_t *s = tree_add_child(o, '5');
    tree_node_t *f = tree_add_child(o, '4');
    cheat_assert(o->next_child == f);
    cheat_assert(o->next_child->value == f->value);
    cheat_assert(f->next_sibling->value == s->value);
    tree_destroy(&o);
    cheat_assert(o == NULL);
)

CHEAT_TEST(call_create_and_child_after_other,
    tree_node_t *o = tree_create_node('2');
    cheat_assert(o != NULL);
    tree_node_t *s = tree_add_child(o, '5');
    tree_node_t *m = tree_add_child(o, '6');
    tree_node_t *f = tree_add_child(o, '8');
    cheat_assert(o->next_child == s);
    cheat_assert(o->next_child->value == s->value);
    cheat_assert(m->next_sibling->value == f->value);
    tree_destroy(&o);
    cheat_assert(o == NULL);
)

CHEAT_TEST(call_get_sibling,
    tree_node_t *o = tree_create_node('2');
    tree_node_t *s5 = tree_add_sibling(o, '5');
    tree_add_sibling(o, '3');
    tree_node_t *s7 = tree_get_sibling(o, '7');
    cheat_assert(s7 == NULL);
    tree_node_t *s = tree_get_sibling(o, '5');
    cheat_assert(s == s5);
    tree_destroy(&o);
    cheat_assert(o == NULL);
)

CHEAT_TEST(call_get_child,
    tree_node_t *o = tree_create_node('2');
    tree_node_t *s5 = tree_add_child(o, '5');
    tree_add_sibling(s5, '3');
    tree_node_t *s7 = tree_get_child(o, '7');
    cheat_assert(s7 == NULL);
    tree_node_t *s = tree_get_child(o, '5');
    cheat_assert(s == s5);
    tree_destroy(&o);
    cheat_assert(o == NULL);
)

CHEAT_TEST(call_insert,
    char phrase[] = "casaco";
    char final[] = "m-w.com";
    size_t s = 0;
    tree_node_t *o = tree_insert(NULL, phrase, sizeof(phrase)-1,
                                 "m-w", ".com");
    tree_node_t *a = o;
    cheat_assert(o != NULL);
    while (a && a->next_child && phrase[s] != 0)
    {
        cheat_assert(phrase[s] == a->value);
        a = a->next_child;
        s++;
    }
    cheat_assert(a->adapted != NULL);
    cheat_assert(!strcmp(a->adapted,final));
    tree_destroy(&o);
    cheat_assert(o == NULL);
)

CHEAT_TEST(call_insert2,
    tree_node_t *o = tree_insert(NULL, "casaco", 6, "m-w", ".com");
    tree_node_t *s = tree_insert(o, "casas", 5, "dotvalue", ".com");
    tree_dump(o);
    cheat_assert(o==s);
    tree_destroy(&o);
)

CHEAT_TEST(call_insert3,
    tree_node_t *o = tree_insert(NULL, "casa", 4, "minha", ".com");
    tree_node_t *s = tree_insert(o, "balada", 6, "dotvalue", ".com");
    tree_node_t *t = tree_insert(s, "ada", 3, "dotvalue", ".com");
    tree_node_t *f = tree_insert(t, "baleia", 6, "dotvalue", ".com");
    tree_dump(t);
    cheat_assert(o!=s);
    cheat_assert(t!=s);
    cheat_assert(t==f);
    tree_destroy(&t);
)
