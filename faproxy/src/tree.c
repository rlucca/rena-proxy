#include "globals.h"
#include "tree.h"
#include <ctype.h>

#undef DEBUG
#define DEBUG(...)

void tree_dump(tree_node_t *root, char *phrase, int phrase_pos)
{
    if (root == NULL)
    {
        return ;
    }

    phrase[phrase_pos] = root->value;

    if (root->next_child != NULL)
    {
        tree_dump(root->next_child, phrase, phrase_pos + 1);
    }

    if (root->adapted[0] != '\0' || root->metadata)
    {
        DEBUG("--- Domain [%s]: %s (%lu)",
                phrase, root->adapted, strlen(root->adapted));
    }

    if (root->next_sibling != NULL)
    {
        tree_dump(root->next_sibling, phrase, phrase_pos);
    }

    phrase[phrase_pos] = '\0';
}

tree_node_t *tree_create_node(char val)
{
    tree_node_t *ret = calloc(1, sizeof(tree_node_t));
    ret->value = val;
    return ret;
}

tree_node_t *tree_add_sibling(tree_node_t *n, char val)
{
    tree_node_t *aux = NULL;
    tree_node_t *prev = NULL;

    if (n == NULL) 
    {
        return NULL; 
    }
  
    while (n != NULL && val > n->value)
    {
        //DEBUG("val [%c] > [%c]", val, n->value);
        prev = n;
        n = n->next_sibling; 
    }

    //DEBUG("val [%c] <= [%c]", val, n->value);
    if (n != NULL && val == n->value)
    {
        return n;
    }

    aux = tree_create_node(val);
    //DEBUG("prev %p\n", prev);
    if (prev == NULL)
    {
        aux->next_sibling = n->next_sibling;
        n->next_sibling = aux;
    } else {
        aux->next_sibling = n;
        prev->next_sibling = aux;
    }
    return aux;
}
  
tree_node_t *tree_add_child(tree_node_t *n, char val)
{ 
    if (n == NULL) 
    {
        return NULL; 
    }
  
    if (n->next_child) 
    {
        if (val < n->next_child->value)
        {
            tree_node_t *aux = tree_create_node(val);
            aux->next_sibling = n->next_child;
            n->next_child = aux;
            return aux;
        }
        return tree_add_sibling(n->next_child, val); 
    }

    n->next_child = tree_create_node(val); 
    return n->next_child;
} 

tree_node_t *tree_insert(tree_node_t *root,
                         const char *v, size_t len,
                         const char *adapted,
                         const char *suffix)
{
    tree_node_t *aux = root;
    char v_l = tolower(v[0]);

    //DEBUG("v[0] = %c", v[0]);
    if (aux == NULL)
    {
        //DEBUG("new root!");
        aux = tree_create_node(v_l);
        root = aux;
    } else {
        if (v_l < root->value)
        {
            //DEBUG("new root! v[0] %c < %c", v[0], root->value);
            aux = tree_create_node(v_l);
            aux->next_sibling = root;
            root = aux;
        } else {
            //DEBUG("keep root");
            aux = tree_add_sibling(aux, v_l);
        }
    }

    //aux->metadata = 1;
    //return root;

    for (size_t s = 1; s < len; s++)
    {
        v_l = tolower(v[s]);
        aux = tree_add_child(aux, v_l);
        //break;
    }

    aux->metadata = 1;
    if (adapted)
    {
        snprintf(aux->adapted, sizeof(aux->adapted),
                 "%s%s", adapted, suffix);
    }

    return root;
}

tree_node_t *tree_get_sibling(tree_node_t *n, char val)
{
    char value = tolower(val);
    while (n != NULL && value >= n->value)
    {
        //DEBUG("n %p val '%u' '%c' == nv '%c'", n, value, value, n->value);
        if (n->value == value)
        {
            //DEBUG("found");
            return n;
        }

        n = n->next_sibling;
    }

    //DEBUG("not found"); // TODO erase?
    return NULL;
}

tree_node_t *tree_get_child(tree_node_t *n, char val)
{
    if (n == NULL)
    {
        //DEBUG("no node to traverse");
        return NULL;
    }

    //DEBUG("n %p val '%u' '%c'", n, val, val);
    return tree_get_sibling(n->next_child, val);
}
