#ifndef TREE_H_
#define TREE_H_

typedef struct tree_node {
    char value;
    struct tree_node *next_child;
    struct tree_node *next_sibling;
    char *adapted;
} tree_node_t;


void tree_dump(tree_node_t *root);
tree_node_t *tree_create_node(char val);
void tree_destroy(tree_node_t **tree);
tree_node_t *tree_add_sibling(tree_node_t *n, char val);
tree_node_t *tree_add_child(tree_node_t *n, char val);
tree_node_t *tree_insert(tree_node_t *root,
                         const char *v, size_t len,
                         const char *adapted,
                         const char *suffix);
tree_node_t *tree_get_sibling(tree_node_t *n, char val);
tree_node_t *tree_get_child(tree_node_t *n, char val);
#endif
