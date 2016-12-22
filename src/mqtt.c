/*
 * mqtt.c Copyright (C) 2016 Ron Pedde <ron@pedde.com>
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
#include <stdlib.h>
#include <unistd.h>

#include "debug.h"
#include "mqtt.h"

typedef struct mqtt_providers_t {
    char *name;
    char *hostname;
    int port;
    int keepalive;
    struct mqtt_providers_t *next;
} mqtt_providers_t;

static mqtt_providers_t mqtt_providers;

int mqtt_init(void) {
    mqtt_providers.next = NULL;

    return 0;
}

int mqtt_lock(void) {
    return 0;
}

int mqtt_unlock(void) {
    return 0;
}

int mqtt_add_provider(char *friendly_name,
                      char *hostname, int port, int keepalive) {
    mqtt_providers_t *pnew;

    pnew = (mqtt_providers_t*)malloc(sizeof(mqtt_providers_t));
    if(!pnew)
        FATAL("malloc");

    pnew->name = strdup(friendly_name);
    pnew->hostname = strdup(hostname);
    pnew->port = port;
    pnew->keepalive = keepalive;

    if(!pnew->name || !pnew->hostname)
        FATAL("malloc");

    mqtt_lock();

    pnew->next = mqtt_providers.next;
    mqtt_providers.next = pnew;

    mqtt_unlock();

    return 0;
}

mqtt_providers_t *mqtt_find_provider(char *friendly_name) {
    mqtt_providers_t *pcurrent;

    mqtt_lock();
    pcurrent = mqtt_providers.next;
    while(pcurrent && strcasecmp(pcurrent->friendly_name, friendly_name))
        pcurrent=pcurrent->next;

    mqtt_unlock();
    return pcurrent;
}

int mqtt_maybe_listen_queue(char *provider, char *queue) {
}


int mqtt_deinit(void) {
    return 0;
}
