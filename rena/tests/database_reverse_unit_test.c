// cheat time in miliseconds (2s default)
//#define CHEAT_TIME 500
#include "cheat.h"

#include "database.h"
#include "tree.h"

CHEAT_DECLARE(
    struct database_object
    {
        tree_node_t *never_rules;
        tree_node_t *side_rules;
        void *list;
        char *input; // not zero-ended
        int input_sz;
        int input_rs;
    } doi;

void foreach(const char *example, struct database_object *doi,
              char **output, int *output_sz)
{
    for (int i=0; i < strlen(example); i++)
    {
        const char *transformed = NULL;
        int transformed_size = 0;
        const char *holding = NULL;
        int holding_size = 0;
        int new_size;
        di_output_e di = database_instance_lookup(
                doi, example[i],
                &transformed, &transformed_size);
        if (di == DBI_FEED_ME)
        {
            database_instance_add_input(doi, example[i]);
            continue;
        }

        if (di == DBI_NOT_HOLD)
        {
            database_instance_add_input(doi, example[i]);
            database_instance_get_holding(doi,
                    &holding, &holding_size, 0);

            new_size = *output_sz + holding_size;
            *output = realloc(*output, new_size + 1);
            if (holding_size > 0)
            {
                memmove(*output + *output_sz,
                        holding, holding_size);
                *output_sz += holding_size;
            }
            (*output)[new_size] = '\0';
        } else {
            database_instance_get_holding(doi,
                    &holding, &holding_size, 1);

            new_size = *output_sz + transformed_size + holding_size;
            *output = realloc(*output, new_size + 1);
            if (holding_size > 0)
            {
                memmove(*output + *output_sz,
                        holding, holding_size);
                *output_sz += holding_size;
            }
            memmove(*output + *output_sz,
                    transformed, transformed_size);
            *output_sz += transformed_size;
            (*output)[new_size] = '\0';

            database_instance_add_input(doi, example[i]);
        }
    }
}
)

CHEAT_SET_UP(
    doi.never_rules = tree_insert(NULL, "https://www.google.com", 22, NULL);
    doi.list = NULL;
    doi.input = NULL;
    doi.input_sz = 0;
    doi.input_rs = 0;
    doi.side_rules = tree_insert(NULL, "www-medicinanet-com-br.asumi.devitest", 37, "www.medicinanet.com.br");
    doi.side_rules = tree_insert(doi.side_rules, "www-example-com.asumi.devitest", 30, "www.example.com");
    doi.side_rules = tree_insert(doi.side_rules, "medicinanet.asumi.devitest", 26, "medicinanet");
)

CHEAT_TEAR_DOWN(
    tree_destroy(&doi.side_rules);
    doi.side_rules = NULL;
)


CHEAT_TEST(tag_with_full_link_medicinanet,
    const char example[] = "											            <ul><li><a href=\"http://www-medicinanet-com-br.asumi.devitest/conteudos/revisoes/1615/dengue.htm\">Dengue</a></li></ul>";
    const char expected[] = "											            <ul><li><a href=\"http://www.medicinanet.com.br/conteudos/revisoes/1615/dengue.htm\">Dengue</a></li></ul>";
    char *output = NULL;
    int output_sz = 0;

    foreach(example, &doi, &output, &output_sz);

    //printf("\nfullink medicinanet\n\tOUTPUT [%s]\n\tEXPECT [%s]\n", output, expected);
    cheat_assert(!strcmp(expected, output));
    cheat_assert(strlen(expected) == output_sz);
)

CHEAT_TEST(tag_with_full_link_example_com,
    const char example[] = "<html>\r\n<head>http://www-example-com.asumi.devitest</head>\r\n\n\r       <body><h1>www-example-com.asumi.devitest homepage </h1></body></html>";
    const char expected[] = "<html>\r\n<head>http://www.example.com</head>\r\n\n\r       <body><h1>www.example.com homepage </h1></body></html>";
    char *output = NULL;
    int output_sz = 0;

    foreach(example, &doi, &output, &output_sz);

    cheat_assert(!strcmp(expected, output));
    cheat_assert(strlen(expected) == output_sz);
)

CHEAT_TEST(medicinannet_both,
    const char example[] = "<link rel=\"alternate\" href=\"http://www-medicinanet-com-br.asumi.devitest/medicinanet.asumi.devitest.rss\" title=\"medicinanet.asumi.devitest - RSS - Brasil\" type=\"application/rss+xml\" />  \r";
    const char expected[] = "<link rel=\"alternate\" href=\"http://www.medicinanet.com.br/medicinanet.rss\" title=\"medicinanet - RSS - Brasil\" type=\"application/rss+xml\" />  \r";
    char *output = NULL;
    int output_sz = 0;

    foreach(example, &doi, &output, &output_sz);

    //printf("\nmedicinaboth\n\tOUTPUT [%s]\n\tEXPECT [%s]\n", output, expected);
    cheat_assert(!strcmp(expected, output));
    cheat_assert(strlen(expected) == output_sz);
)

CHEAT_TEST(medicinannet_followup,
    const char example[] = "<p>medicinanet.asumi.devitest eh um site chamado medicinanet.asumi.devitest medicinanet.asumi.devitestmedicinanet.asumi.devitest.</p>";
    const char expected[] = "<p>medicinanet eh um site chamado medicinanet medicinanetmedicinanet.</p>";
    char *output = NULL;
    int output_sz = 0;

    foreach(example, &doi, &output, &output_sz);

    //printf("\nmedicina_followup\n\tOUTPUT [%s]\n\tEXPECT [%s]\n", output, expected);
    cheat_assert(!strcmp(expected, output));
    cheat_assert(strlen(expected) == output_sz);
)

CHEAT_TEST(medicinanet_answer,
    const char example[] = "HTTP/1.1 301 Moved Permanently\r\nContent-Type: text/html; charset=UTF-8\r\nLocation: https://www-medicinanet-com-br.asumi.devitest/\r\n"
                           "Server: Microsoft-IIS/8.5\r\nX-Powered-By: ASP.NET\r\nX-UA-Compatible: IE=EmulateIE8\r\nDate: Mon, 07 Mar 2022 18:24:58 GMT\r\n\r\n";
    const char expected[] = "HTTP/1.1 301 Moved Permanently\r\nContent-Type: text/html; charset=UTF-8\r\nLocation: https://www.medicinanet.com.br/\r\n"
                           "Server: Microsoft-IIS/8.5\r\nX-Powered-By: ASP.NET\r\nX-UA-Compatible: IE=EmulateIE8\r\nDate: Mon, 07 Mar 2022 18:24:58 GMT\r\n\r\n";
    char *output = NULL;
    int output_sz = 0;

    foreach(example, &doi, &output, &output_sz);

    //printf("\nmedicina_answer\n\tOUTPUT [%s]\n\tEXPECT [%s]\n", output, expected);
    cheat_assert(!strcmp(expected, output));
    cheat_assert(strlen(expected) == output_sz);
)
