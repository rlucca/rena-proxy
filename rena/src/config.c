#include "global.h"
#include <ini.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#define SYSLOG_NAMES 1
#include <syslog.h>

struct config_rena
{
    char certificate_file[MAX_FILENAME];
    char certificate_key[MAX_FILENAME];
    char database_directory[MAX_FILENAME];
    char database_suffix[MAX_STR];
    char server_bind[MAX_STR];
    int server_port_http;
    int server_port_https;
    int logging_facility;
    int logging_options;
    int logging_minimum;
    int pool_minimum;
    int pool_maximum;
    int pool_reap_time;
    float pool_addictive;
};


static int parse_certificate_file(struct config_rena * restrict inout,
                                  const char *value)
{
    snprintf(inout->certificate_file, sizeof(inout->certificate_file),
             "%s", value);
    return 0;
}

static int parse_certificate_key(struct config_rena * restrict inout,
                                 const char *value)
{
    snprintf(inout->certificate_key, sizeof(inout->certificate_key),
             "%s", value);
    return 0;
}

static int parse_database_directory(struct config_rena * restrict inout,
                                    const char *value)
{
    snprintf(inout->database_directory, sizeof(inout->database_directory),
             "%s", value);
    return 0;
}

static int parse_database_suffix(struct config_rena * restrict inout,
                                    const char *value)
{
    if (value && value[0] == '.' && strchr(value + 1, '.'))
    {
        snprintf(inout->database_suffix, sizeof(inout->database_suffix),
                 "%s", value);
        return 0;
    }

    return -1;
}

static int parse_pool_minimum(struct config_rena * restrict inout,
                                    const char *value)
{
    double temp = strtod(value,NULL);
    if (temp <= 0)
        return -1;

    inout->pool_minimum = (int) temp;
    return 0;
}

static int parse_pool_maximum(struct config_rena * restrict inout,
                                    const char *value)
{
    double temp = strtod(value,NULL);
    if (temp <= 0)
        return -1;

    inout->pool_maximum = (int) temp;
    return 0;
}

static int parse_pool_reap_time(struct config_rena * restrict inout,
                                    const char *value)
{
    double temp = strtod(value,NULL);
    if (temp <= 0)
        return -1;

    inout->pool_reap_time = (int) temp;
    return 0;
}

static int parse_pool_addictive(struct config_rena * restrict inout,
                                    const char *value)
{
    double temp = strtod(value,NULL);
    if (temp <= 0)
        return -1;

    inout->pool_addictive = (float) temp;
    return 0;
}

static int parse_server_http(struct config_rena * restrict inout,
                                    const char *value)
{
    double temp = strtod(value,NULL);
    if (temp <= 0)
        return -1;

    inout->server_port_http = (int) temp;
    return 0;
}

static int parse_server_https(struct config_rena * restrict inout,
                                    const char *value)
{
    double temp = strtod(value,NULL);
    if (temp <= 0)
        return -1;

    inout->server_port_https = (int) temp;
    return 0;
}

static int parse_server_bind(struct config_rena * restrict inout,
                                    const char *value)
{
    if (strchr(value, '.') == NULL && strchr(value, ':') == NULL)
    {
      return -1;
    }

    snprintf(inout->server_bind, sizeof(inout->server_bind), "%s", value);
    return 0;
}

static int syslog_lookup(CODE *ptr, const char *value, int *ret)
{
    size_t value_len = strnlen(value, MAX_STR);

    for (; ptr->c_val >= 0; ptr++)
    {
        size_t name_len = strnlen(ptr->c_name, MAX_STR);
        name_len = (name_len > value_len) ? name_len : value_len;
        if (strncasecmp(ptr->c_name, value, name_len) == 0)
        {
            *ret = ptr->c_val;
            return 0;
        }
    }

    return -1;
}

static int parse_logging_facility(struct config_rena * restrict inout,
                                    const char *value)
{
    int temp = -1;

    if (syslog_lookup(facilitynames, value, &temp) == 0)
    {
        inout->logging_facility = temp;
        return 0;
    }

    return -1;
}

static int parse_logging_minimum(struct config_rena * restrict inout,
                                    const char *value)
{
    int temp = -1;

    if (syslog_lookup(prioritynames, value, &temp) == 0)
    {
        inout->logging_minimum = temp;
        return 0;
    }

    return -1;
}

static int parse_logging_options(struct config_rena * restrict inout,
                                    const char *value)
{
    double temp = strtod(value,NULL);
    if (temp <= 0)
        return -1;

    inout->logging_options = (int) temp;
    return 0;
}


static int config_set(struct config_rena * restrict inout,
                        const char *section, const char *key,
                        const char *value)
{
    struct {
        const char *sec, *key;
        int (*fnc)(struct config_rena * restrict, const char *);
    } *ptr = NULL, all_settings[] = {
        { "certificate", "file", parse_certificate_file },
        { "certificate", "key", parse_certificate_key },
        { "database", "directory", parse_database_directory },
        { "database", "suffix", parse_database_suffix },
        { "logging", "facility", parse_logging_facility },
        { "logging", "minimum", parse_logging_minimum },
        { "logging", "options", parse_logging_options },
        { "server", "port_https", parse_server_https },
        { "server", "port_http", parse_server_http },
        { "server", "bind", parse_server_bind },
        { "pool", "reap_time", parse_pool_reap_time },
        { "pool", "addictive", parse_pool_addictive },
        { "pool", "minimum", parse_pool_minimum },
        { "pool", "maximum", parse_pool_maximum },
        { NULL, NULL, NULL }
    };

    for (ptr = all_settings; ptr->sec && ptr->key && ptr->fnc; ptr++)
    {
        size_t sec_len = strnlen(ptr->sec, MAX_STR);
        size_t sky_len = strnlen(ptr->key, MAX_STR);
        size_t pky_len = strnlen(key, MAX_STR);
        size_t key_len = (sky_len > pky_len) ? sky_len : pky_len;

        if (strncasecmp(ptr->sec, section, sec_len)
                || strncasecmp(ptr->key, key, key_len))
        {
            continue;
        }

        if (ptr->fnc(inout, value) == 0)
        {
            return 0;
        }

        break;
    }

    return -1;
}

void config_free(struct config_rena ** restrict inout)
{
    free(*inout);
    *inout = NULL;
}

int config_load(struct config_rena ** restrict inout,
                const char *filename)
{
    struct config_rena default_config = {
            "/etc/rena/certificate.pem",
            "/etc/rena/certificate.pem",
            "/etc/rena/dbs", ".example.org",
            "127.0.0.1", 80, 443,
            LOG_LOCAL7,
            LOG_PID|LOG_CONS|LOG_NDELAY|LOG_PERROR,
            LOG_INFO,
            4, 16, 1800, 0.1
        };
	struct INI *ini = NULL;
    int res;

    if (inout == NULL)
    {
        do_log(LOG_ERROR, "Invalid parameters [%p]", inout);
        return -1;
    }

    if (*inout != NULL)
    {
        do_log(LOG_ERROR, "Invalid parameters [%p]", inout);
        return -1;
    }

    *inout = malloc(sizeof(struct config_rena));
    // Do not test for NULL, let it crash on invalid access!

    if (!memcpy(*inout, &default_config, sizeof(default_config)))
    {
        do_log(LOG_ERROR, "Error during copy of default configuration");
        config_free(inout);
        return -2;
    }

    if (filename == NULL || *filename == '\0')
    {
        do_log(LOG_DEBUG, "Default configuration returned");
        return 0;
    }

	ini = ini_open(filename);
	if (!ini)
    {
        do_log(LOG_ERROR, "Error opening filename [%s]", filename);
        config_free(inout);
		return -3;
    }

    do_log(LOG_DEBUG, "Config file opened.");

    const char *buf, *buf2;
    size_t buf_len, buf2_len;
    while ((res = ini_next_section(ini, &buf, &buf_len)) > 0)
    {
        char section[MAX_STR];
        snprintf(section, sizeof(section), "%.*s", (int) buf_len, buf);

        while ((res = ini_read_pair(ini, &buf, &buf_len, &buf2, &buf2_len)) > 0)
        {
            char key[MAX_STR];
            char value[MAX_FILENAME];
            snprintf(key, sizeof(key), "%.*s", (int) buf_len, buf);
            snprintf(value, sizeof(value), "%.*s", (int) buf2_len, buf2);

            if (config_set(*inout, section, key, value))
            {
                do_log(LOG_ERROR,
                           "Invalid configuration entry [%s.%s] on %s:%d",
                           section, key, filename,
                           ini_get_line_number(ini, buf));
            }
        }

        if (res < 0)
        {
            break;
        }
    }

    if (res < 0)
    {
        do_log(LOG_ERROR, "Internal libini error code [%d]", res);
    }

	ini_close(ini);

    if (res < 0)
    {
        config_free(inout);
        return -4;
    }

    return 0;
}

void config_get_server_address(struct config_rena ** restrict inout,
                               const char ** const out)
{
    *out = (*inout)->server_bind;
}

void config_get_server_port_http(struct config_rena ** restrict inout,
                               int *out)
{
    *out = (*inout)->server_port_http;
}

void config_get_server_port_https(struct config_rena ** restrict inout,
                               int *out)
{
    *out = (*inout)->server_port_https;
}

void config_get_certificate_file(struct config_rena ** restrict inout,
                               const char ** const out)
{
    *out = (*inout)->certificate_file;
}

void config_get_certificate_key(struct config_rena ** restrict inout,
                               const char ** const out)
{
    *out = (*inout)->certificate_key;
}

void config_get_database_directory(struct config_rena ** restrict inout,
                               const char ** const out)
{
    *out = (*inout)->database_directory;
}

void config_get_database_suffix(struct config_rena ** restrict inout,
                               const char ** const out)
{
    *out = (*inout)->database_suffix;
}

void config_get_logging_facility(struct config_rena ** restrict inout,
                                int *out)
{
    *out = (*inout)->logging_facility;
}

void config_get_logging_minimum(struct config_rena ** restrict inout,
                                int *out)
{
    *out = (*inout)->logging_minimum;
}

void config_get_logging_options(struct config_rena ** restrict inout,
                                int *out)
{
    *out = (*inout)->logging_options;
}


void config_get_pool_minimum(struct config_rena ** restrict inout,
                                int *out)
{
    *out = (*inout)->pool_minimum;
}

void config_get_pool_maximum(struct config_rena ** restrict inout,
                                int *out)
{
    *out = (*inout)->pool_maximum;
}

void config_get_pool_reap_time(struct config_rena ** restrict inout,
                                int *out)
{
    *out = (*inout)->pool_reap_time;
}

void config_get_pool_addictive(struct config_rena ** restrict inout,
                                float *out)
{
    *out = (*inout)->pool_addictive;
}
