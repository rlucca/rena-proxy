
// cheat time in miliseconds (2s default)
//#define CHEAT_TIME 500
#include "cheat.h"

#include "config.h"


CHEAT_DECLARE(
    // malloc e free nao posso trocar!
)

CHEAT_TEST(config_load_invalid_parameter,

    int returned = config_load(NULL, NULL);
    cheat_assert(returned == -1);
)

CHEAT_TEST(config_load_trash_parameter,

    struct config_rena *rena = (struct config_rena *) 0x10;
    int returned = config_load(&rena, NULL);
    cheat_assert(returned == -1);
)

CHEAT_TEST(config_load_no_default_parameter,

    struct config_rena *rena = NULL;
    int returned = config_load(&rena, "tests/files/no_file_name.ini");
    cheat_assert(returned == -3);
    cheat_assert(rena == NULL);
)

CHEAT_TEST(config_load_default_parameter,

    struct config_rena *rena = NULL;
    int returned = config_load(&rena, NULL);
    const char *stuff[5];
    int values[8];
    float pool = 0;

    cheat_assert(returned == 0);
    cheat_assert(rena != NULL);


    memset(stuff, 0, sizeof(stuff));
    config_get_server_address(&rena, &stuff[0]);
    config_get_certificate_file(&rena, &stuff[1]);
    config_get_certificate_key(&rena, &stuff[2]);
    config_get_database_directory(&rena, &stuff[3]);
    config_get_database_suffix(&rena, &stuff[4]);
    for (size_t total=sizeof(stuff)/sizeof(stuff[0]), i=0; i<total; i++)
        {
            cheat_assert(*stuff[i] != '\0');
        }


    config_get_pool_addictive(&rena, &pool);
    cheat_assert(pool > 0 && "failed to load pool_addictive");

    memset(values, 0, sizeof(values));
    config_get_server_port_http(&rena, &values[0]);
    config_get_server_port_https(&rena, &values[1]);
    config_get_logging_facility(&rena, &values[2]);
    config_get_logging_minimum(&rena, &values[3]);
    config_get_logging_options(&rena, &values[4]);
    config_get_pool_minimum(&rena, &values[5]);
    config_get_pool_maximum(&rena, &values[6]);
    config_get_pool_reap_time(&rena, &values[7]);
    for (size_t total=sizeof(values)/sizeof(values[0]), i=0; i<total; i++)
        {
            cheat_assert(values[i] > 0);
        }

    config_free(&rena);
)
