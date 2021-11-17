#include "global.h"
#include "tree.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

static tree_node_t *tree_get_sibling2(tree_node_t *n, char val,
                                     size_t before, size_t *exact);
static void tree_dump2(tree_node_t *root, char *phrase, int phrase_pos);
static int tree_depth(tree_node_t *root, int actual);

void tree_dump(tree_node_t *root)
{
    int depth = tree_depth(root, 1);
    char *phrase = calloc(sizeof(depth) + 1, 1);
    tree_dump2(root, phrase, 0);
    free(phrase);
}

tree_node_t *tree_create_node(char val)
{
    tree_node_t *ret = calloc(1, sizeof(tree_node_t));
    ret->value = val;
    return ret;
}

void tree_destroy(tree_node_t **node)
{
    if (node == NULL || *node == NULL)
    {
        return ;
    }

    tree_destroy(&(*node)->next_child);
    (*node)->next_child = NULL;
    tree_destroy(&(*node)->next_sibling);
    (*node)->next_sibling = NULL;
    free((*node)->adapted);
    (*node)->adapted = NULL;
    free(*node);
    *node = NULL;
}

tree_node_t *tree_add_sibling(tree_node_t *n, char val)
{
    size_t exact = 0;
    tree_node_t *previous = tree_get_sibling2(n, val, 1, &exact);
    tree_node_t *aux = NULL;

    if (exact != 0)
    {
        return (previous == NULL) ? n : previous->next_sibling;
    }

    aux = tree_create_node(val);
    if (previous == NULL)
    {
        aux->next_sibling = n;
    } else {
        aux->next_sibling = previous->next_sibling;
        previous->next_sibling = aux;
    }
    return aux;
}

tree_node_t *tree_add_child(tree_node_t *n, char val)
{
    char value = tolower(val);
    if (n == NULL)
    {
        return tree_add_sibling(n, val);
    }

    if (n->next_child == NULL)
    {
        n->next_child = tree_create_node(val);
        return n->next_child;
    }

    if (value < n->next_child->value)
    {
        tree_node_t *aux = tree_create_node(val);
        aux->next_sibling = n->next_child;
        n->next_child = aux;
        return aux;
    }

    return tree_add_sibling(n->next_child, val);
}

tree_node_t *tree_insert(tree_node_t *root,
        const char *v, size_t len,
        const char *adapted,
        const char *suffix)
{
    tree_node_t *aux = root;

    aux = tree_add_sibling(aux, v[0]);
    if (root == NULL || aux->value < root->value)
    {
        root = aux;
    }

    for (size_t s = 1; s < len; s++)
    {
        aux = tree_add_child(aux, v[s]);
    }

    if (aux->adapted != NULL)
    {
        do_log(LOG_DEBUG, "Rule [%.*s] already exist!", (int)len, v);
        return root;
    }

    if (adapted)
    {
        size_t alen = strlen(adapted) + strlen(suffix) + 1;
        aux->adapted = malloc(sizeof(char) * alen);
        snprintf(aux->adapted, alen, "%s%s", adapted, suffix);
    } else {
        aux->adapted = calloc(1, sizeof(char));
    }

    return root;
}

tree_node_t *tree_get_sibling(tree_node_t *n, char val)
{
    return tree_get_sibling2(n, val, 0, NULL);
}

tree_node_t *tree_get_child(tree_node_t *n, char val)
{
    if (n == NULL)
        return NULL;

    return tree_get_sibling(n->next_child, val);
}

static tree_node_t *tree_get_sibling2(tree_node_t *n, char val,
                                      size_t before, size_t *exact)
{
    tree_node_t *prev = NULL;
    char value = tolower(val);
    while (n != NULL && value >= n->value)
    {
        if (n->value == value)
        {
            if (exact) *exact = 1;
            return (before == 0) ? n : prev;
        }

        prev = n;
        n = n->next_sibling;
    }

    return (before == 0) ? NULL : prev;
}

void tree_dump2(tree_node_t *root, char *phrase, int phrase_pos)
{
    if (root == NULL)
    {
        return ;
    }

    phrase[phrase_pos] = root->value;

    tree_dump2(root->next_child, phrase, phrase_pos + 1);

    if (root->adapted != NULL)
    {
        do_log(LOG_DEBUG, "--- Domain [%s]: %s (%lu)",
                phrase, root->adapted, strlen(root->adapted));
    }

    phrase[phrase_pos] = '\0';

    tree_dump2(root->next_sibling, phrase, phrase_pos);
}

static int tree_depth(tree_node_t *root, int actual)
{
    if (root == NULL)
        return actual;
    int s = tree_depth(root->next_sibling, actual);
    int c = tree_depth(root->next_child, actual + 1);
    if (c > s) s = c;
    return (s > actual)?s:actual;
}
