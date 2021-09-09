#pragma once

typedef struct tree_node {
    char value;
    char adapted[MAX_STR];
    int metadata; // 0 not metadata 1 has metadata
    struct tree_node *next_child;
    struct tree_node *next_sibling;
} tree_node_t;


void tree_dump(tree_node_t *root, char *phrase, int phrase_pos);
tree_node_t *tree_create_node(char val);
tree_node_t *tree_add_sibling(tree_node_t *n, char val);
tree_node_t *tree_add_child(tree_node_t *n, char val);
tree_node_t *tree_insert(tree_node_t *root,
                         const char *v, size_t len,
                         const char *adapted,
                         const char *suffix);

tree_node_t *tree_get_sibling(tree_node_t *n, char val);
tree_node_t *tree_get_child(tree_node_t *n, char val);
