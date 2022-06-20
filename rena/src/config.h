#ifndef CONFIG_H_
#define CONFIG_H_

#define DEFAULT_HTTP_PORT 80
#define DEFAULT_HTTPS_PORT 443

struct config_rena; // forward

int config_load(struct config_rena ** restrict inout,
                const char *filename);
void config_free(struct config_rena ** restrict inout);


void config_get_server_address(struct config_rena ** restrict inout,
                               const char ** const out);
void config_get_server_port_http(struct config_rena ** restrict inout,
                               int *);
void config_get_server_port_https(struct config_rena ** restrict inout,
                               int *);
void config_get_certificate_file(struct config_rena ** restrict inout,
                               const char ** const out);
void config_get_certificate_key(struct config_rena ** restrict inout,
                               const char ** const out);
void config_get_database_directory(struct config_rena ** restrict inout,
                               const char ** const out);
void config_get_database_suffix(struct config_rena ** restrict inout,
                               const char ** const out);
void config_get_database_auth_file(struct config_rena ** restrict inout,
                               const char ** const out);
void config_get_logging_engine(struct config_rena ** restrict inout,
                               const char ** const out);
void config_get_logging_facility(struct config_rena ** restrict inout,
                                int *);
void config_get_logging_minimum(struct config_rena ** restrict inout,
                                int *);
void config_get_logging_options(struct config_rena ** restrict inout,
                                int *);
void config_get_logging_access_file(struct config_rena ** restrict inout,
                                    const char ** const);
void config_get_logging_access_format(struct config_rena ** restrict inout,
                                      const char ** const);

void config_get_pool_minimum(struct config_rena ** restrict inout,
                                int *);
void config_get_pool_maximum(struct config_rena ** restrict inout,
                                int *);
void config_get_pool_reap_time(struct config_rena ** restrict inout,
                                int *);
void config_get_pool_addictive(struct config_rena ** restrict inout,
                                float *);
int config_process_header_content_type(struct config_rena ** restrict inout,
                                       const char *value);
int config_process_header_accept(struct config_rena ** restrict inout,
                                 const char *value);

#endif
