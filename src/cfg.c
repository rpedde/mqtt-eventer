/*
 * cfg.c Copyright (C) 2016 Ron Pedde <ron@pedde.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <string.h>

#include <libconfig.h>

#include "cfg.h"
#include "debug.h"
#include "mqtt-eventer.h"

cfg_t config;

int cfg_load(char *file) {
    config_t cfg;
    const char *svalue;
    int ivalue;

    memset(&config, 0, sizeof(config));

    config.mqtt_port = 1883;
    config.mqtt_host = strdup("127.0.0.1");
    config.mqtt_keepalive = 60;

    config_init(&cfg);
    if(!config_read_file(&cfg, file)) {
        ERROR("%s:%d - %s",
              config_error_file(&cfg),
              config_error_line(&cfg),
              config_error_text(&cfg));
        config_destroy(&cfg);
        return -1;
    }

    if(config_lookup_string(&cfg, "mqtt_host", &svalue))
        config.mqtt_host = strdup(svalue);

    if(config_lookup_int(&cfg, "mqtt_port", &ivalue))
        config.mqtt_port = (uint16_t)ivalue;

    if(config_lookup_int(&cfg, "mqtt_keepalive", &ivalue))
        config.mqtt_keepalive = (uint16_t)ivalue;

    return 0;
}

void cfg_dump(void) {
    DEBUG("mqtt host:      %s", config.mqtt_host);
    DEBUG("mqtt port:      %d", config.mqtt_port);
    DEBUG("mqtt keepalive: %d", config.mqtt_keepalive);
}
