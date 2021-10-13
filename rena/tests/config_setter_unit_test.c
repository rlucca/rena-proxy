
// cheat time in miliseconds (2s default)
//#define CHEAT_TIME 500
#include "cheat.h"

#include "config.h"


CHEAT_DECLARE(
    // malloc e free nao posso trocar!
    const char *g_filename = "tests/files/setter.ini";
    struct config_rena_t *rena = NULL;
)

CHEAT_SET_UP(
    int returned = config_load(&rena, g_filename);
    cheat_assert(returned == 0);
    cheat_assert(rena != NULL);
)

CHEAT_TEAR_DOWN(
    config_free(&rena);
    rena = NULL;
)

CHEAT_TEST(config_verify_server_parameters,
    const char *bind_to = NULL;
    int ports[2];

    config_get_server_address(&rena, &bind_to);
    config_get_server_port_http(&rena, &ports[0]);
    config_get_server_port_https(&rena, &ports[1]);

    cheat_assert(strcmp(bind_to, "10.10.10.10") == 0 && "bind");
    cheat_assert(ports[0] == 8080 && "port_http");
    cheat_assert(ports[1] == 80443 && "port_https");
)

CHEAT_TEST(config_verify_server,
    const char *bind_to = NULL;

    config_get_server_address(&rena, &bind_to);
    cheat_assert(strcmp(bind_to, "10.10.10.10") == 0);
)

CHEAT_TEST(config_verify_certificates,
    const char *file, *key;
    config_get_certificate_file(&rena, &file);
    config_get_certificate_key(&rena, &key);

    cheat_assert(strcmp(file, "certificate.pem1") == 0 && "file");
    cheat_assert(strcmp(key, "pem2") == 0 && "key");
)

CHEAT_TEST(config_verify_database,
    const char *directory, *suffix;
    config_get_database_directory(&rena, &directory);
    config_get_database_suffix(&rena, &suffix);

    cheat_assert(strcmp(directory, "dbs3") == 0 && "directory");
    cheat_assert(strcmp(suffix, ".marcoon.example.com") == 0 && "suffix");
)

CHEAT_TEST(config_verify_pool,
    int min, max, reap;
    float add;

    config_get_pool_minimum(&rena, &min);
    config_get_pool_maximum(&rena, &max);
    config_get_pool_reap_time(&rena, &reap);
    config_get_pool_addictive(&rena, &add);

    cheat_assert(min==2 && "minimum");
    cheat_assert(max==8 && "maximum");
    cheat_assert(reap==3600 && "reap_time");
    cheat_assert(add >= 0.119 && add <= 0.121 && "addictive");
)

CHEAT_TEST(config_verify_logging,
    int facility, min, options;

    config_get_logging_facility(&rena, &facility);
    config_get_logging_minimum(&rena, &min);
    config_get_logging_options(&rena, &options);

    cheat_assert(facility==168 && "facility");
    cheat_assert(min==127 && "minimum");
    cheat_assert(options==57 && "options");
)
