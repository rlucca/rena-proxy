#include "globals.h"
#include "database_internal.h"

#define ALLOCATED_RESERVE 16

static int array_insert_based_on_depth(tree_lookup_t ***base,
                                       size_t *base_sz,
                                       size_t *base_reserve,
                                       tree_lookup_t *new_element)
{
    if (base == NULL || base_sz == NULL || new_element == NULL)
    {
        DEBUG("incorrect parms");
        return -1;
    }

    tree_lookup_t **aux = *base;

    if ((1 + *base_sz) >= *base_reserve)
    {
        size_t new_reserve = ALLOCATED_RESERVE + *base_sz;
        aux = realloc(*base, sizeof(tree_lookup_t *) * new_reserve);
        *base_reserve = new_reserve;

        if (aux)
            *base = aux;
        else
            aux = *base;
    }

    char ne_value = new_element->subtree->value;
    size_t ne_depth = new_element->depth;
    size_t u;

    for (u = 0; u < *base_sz; u++)
    {
        if (aux[u]->depth < ne_depth
                || (aux[u]->depth == ne_depth
                    && ne_value < aux[u]->subtree->value))
        {
            break;
        }
    }

    {
    // XXX falta reposicionar no u e deslocar os itens?
    if (u != *base_sz)
        DEBUG("pos %ld / %ld", u, *base_sz);
    }

    aux[*base_sz] = new_element;
    *base_sz += 1;
    return 0;
}

static void array_destroy_elements(tree_lookup_t ***base,
                                   size_t *base_sz)
{
    if (base == NULL || *base == NULL || base_sz == NULL)
    {
        return ;
    }

    tree_lookup_t **aux = *base;

    for (size_t u = 0; u < *base_sz; u++)
    {
        free(aux[u]);
    }

    *base_sz = 0;
}


void *database_transform_handler(int side)
{
    if (side == 0)
    {
        DEBUG("invalid parm");
        return NULL;
    }

    di_lookup_t *ret = calloc(1, sizeof(di_lookup_t));
    ret->never_ref = di_get_tree(NEVER_PROXY_TREE);

    if (side > 0)
    {
        ret->other_ref = di_get_tree(RENAME_FROM_SERVER);
    } else {
        ret->other_ref = di_get_tree(RENAME_FROM_CLIENT);
    }

    return ret;
}

int database_transform_lookup(void **handler, char input, int force,
                              char **phrase, size_t *phrase_consume)
{
    di_lookup_t *cast = NULL;
    tree_node_t *never = NULL;
    tree_node_t *other = NULL;

    if (handler == NULL || phrase == NULL || phrase_consume == NULL)
    {
        //DEBUG("parms incorrect %p %p %p", handler, phrase, phrase_consume);
        return -1;
    }

    cast = *handler;
    //DEBUG("nnn %ld", cast->n_lookups);

    if (cast->lookups != NULL)
    {
        for (size_t I=0; I < cast->n_lookups; I++)
        {
            tree_lookup_t *lup = cast->lookups[I];
            tree_node_t *aux;

            //DEBUG("[%ld/%ld] val '%c' :: '%c'", 1+I, cast->n_lookups, lup->subtree->value, input);

            aux = tree_get_child(lup->subtree,
                                 input);

            //DEBUG("[%ld/%ld] tgc %p '%u' '%c' from '%c' %p \"%s\" depth %lu",
            //       1 + I, cast->n_lookups, aux, input, input, lup->subtree->value,
            //       lup->subtree->adapted, lup->subtree->adapted, lup->depth);

            if (aux == NULL)
            {
                size_t tk;
                free(lup);
                for (tk = I; tk + 1 < cast->n_lookups; tk++)
                {
                    cast->lookups[tk] = cast->lookups[tk + 1];
                }
                cast->n_lookups--;
                I--;
            } else {
                lup->subtree = aux;
                lup->depth += 1;

                if (aux && aux->adapted[0] != '\0')
                {
                    *phrase = aux->adapted;
                    *phrase_consume = lup->depth;
                    array_destroy_elements(&cast->lookups,
                                            &cast->n_lookups);
                    //DEBUG("devolvi TRANSFORM OK %lu", cast->n_lookups);
                    return TRANSFORM_OK;
                }
            }
        }
    }

    if (input <= ' ')
    {
        //DEBUG("sai do for de lookups, mas caracter nao eh interessante [%d]",
        //      input);
        //DEBUG("devolvi %s %lu",
        //      ((cast->n_lookups == 0)?"NOT_HOLD":"FEED_ME_AND_HOLD"),
        //      cast->n_lookups);
        return (cast->n_lookups == 0)?NOT_HOLD:FEED_ME_AND_HOLD;
    }

    //DEBUG("%p first neverproxy '%c'", cast->never_ref, input);
    never = tree_get_sibling(cast->never_ref, input);
    //DEBUG("%p first neverproxy end", never);
    if (never != NULL)
    {
        tree_lookup_t *tl = malloc(sizeof(tree_lookup_t));
        tl->subtree = never;
        tl->depth = 1;
        array_insert_based_on_depth(&cast->lookups,
                                    &cast->n_lookups,
                                    &cast->n_reserve,
                                    tl);
    }

    //DEBUG("%p first directional '%c'", cast->other_ref, input);
    other = tree_get_sibling(cast->other_ref, input);
    //DEBUG("%p first directional end", other);
    if (other != NULL)
    {
        tree_lookup_t *tl = malloc(sizeof(tree_lookup_t));
        tl->subtree = other;
        tl->depth = 1;
        array_insert_based_on_depth(&cast->lookups,
                                    &cast->n_lookups,
                                    &cast->n_reserve,
                                    tl);
    }

    //DEBUG("devolvi %s %lu", ((cast->n_lookups == 0)?"NOT_HOLD":"FEED_ME_AND_HOLD"), cast->n_lookups);
    return (cast->n_lookups == 0)?NOT_HOLD:FEED_ME_AND_HOLD;
}

void database_transform_cleanup(void **handler)
{
    di_lookup_t *cast = NULL;
    if (handler == NULL || *handler == NULL)
    {
        DEBUG("parm nulled");
        return ;
    }

    cast = *handler;

    // never free *_refs
    // never free lookups->subtree too
    array_destroy_elements(&cast->lookups,
                           &cast->n_lookups);

    free(cast->lookups);
    cast->lookups = NULL;

    free(*handler);
    *handler = NULL;
}
