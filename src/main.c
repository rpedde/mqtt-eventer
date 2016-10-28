/*
 * mqtt-eventer Copyright (C) 2016 Ron Pedde <ron@pedde.com>
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

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <libconfig.h>

#include "cfg.h"
#include "debug.h"
#include "lua-runtime.h"
#include "mqtt-eventer.h"

#define DEFAULT_CONFIG_FILE "/etc/mqtt-eventer/mqtt-eventer.conf"

void daemonize(void) {
    ERROR("Daemonize not implemented");
    exit(EXIT_FAILURE);
}

void usage(char *a0) {
    fprintf(stderr, "Usage: %s [args]\n\n", a0);
    fprintf(stderr, "Valid args:\n\n");
    fprintf(stderr, " -c <configfile>     config file to load\n");
    fprintf(stderr, " -b                  background (daemonize)\n");
    fprintf(stderr, " -d <level>          debug level (1-5)\n");
    fprintf(stderr, "\n");
}

int main(int argc, char *argv[]) {
    int opt;
    int background = FALSE;
    int verbose_level = 2;

    char *configfile = DEFAULT_CONFIG_FILE;

    while((opt = getopt(argc, argv, "c:bd:")) != -1) {
        switch(opt)  {
        case 'c':
            configfile = optarg;
            break;
        case 'b':
            background = TRUE;
            break;
        case 'd':
            verbose_level = atoi(optarg);
            break;
        default:
            usage(argv[0]);
            exit(EXIT_FAILURE);
            break;
        }
    }

    debug_level(verbose_level);
    DEBUG("Loading config from %s", configfile);

    if(cfg_load(configfile) == -1) {
        ERROR("Error loading config.  Aborting");
        return EXIT_FAILURE;
    }

    cfg_dump();

    if(background)
        daemonize();

    INFO("Starting lua runtime...");
    if(lr_init() == -1) {
        ERROR("Error starting up lua runtime.  Aborting");
        return EXIT_FAILURE;
    }

    return(EXIT_SUCCESS);
}
