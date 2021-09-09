#pragma once

#include "database.h"
#include "tree.h"

enum {
    NEVER_PROXY_TREE = 0,
    RENAME_FROM_SERVER = 1,
    RENAME_FROM_CLIENT = 2,
    LAST_N_TREE
};

tree_node_t *di_get_tree(size_t e);

typedef struct tree_lookup {
    tree_node_t *subtree;
    size_t depth; // or to be consumed characters
} tree_lookup_t;

typedef struct {
    tree_node_t *never_ref;
    tree_node_t *other_ref;

    tree_lookup_t **lookups;
    size_t n_lookups;
    size_t n_reserve;
} di_lookup_t;
