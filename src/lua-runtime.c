/*
 * lua-runtime.c Copyright (C) 2016 Ron Pedde <ron@pedde.com>
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

#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "cfg.h"
#include "debug.h"
#include "mqtt-eventer.h"
#include "lua-runtime.h"

static int lr_method_register_callback(lua_State *L);
static int lr_method_debug(lua_State *L);

/* #define XFRM(x) XDBG_ ## x, */
/* typedef enum debug_level_t { DBG_SYMBOLS DBG_MAXVAL } debug_level_t; */
/* #undef XFRM */

#define XFRM(x) #x,
char *debug_values[] = { DBG_SYMBOLS "DBG_MAXVAL" };
#undef XRFM

typedef struct script_env_t {
    char *path;
    lua_State *l;
    struct script_env_t *next;
} script_env_t;

static script_env_t lr_scripts = { NULL, NULL, NULL };

static int lr_add_script(char *path);
static lua_State *lr_newstate(char *path);

int lr_init(void) {
    DIR *pdir;
    int name_max;
    struct dirent *pentry, *presult;
    int res, len;
    char full_path[PATH_MAX];


    DEBUG("Checking scripts in %s", config.lua_script_dir);

    pdir = opendir(config.lua_script_dir);
    if(!pdir) {
        ERROR("Error in opendir: %s", strerror(errno));
        return -1;
    }

    pentry = presult = NULL;

    name_max = pathconf(config.lua_script_dir, _PC_NAME_MAX);
    if (name_max == -1)         /* Limit not defined, or error */
        name_max = 255;         /* Take a guess */
    len = offsetof(struct dirent, d_name) + name_max + 1;
    pentry = malloc(len);

    if (!pentry)
        FATAL("Malloc error");

    while(((res = readdir_r(pdir, pentry, &presult)) == 0) && presult) {
        if(strlen(pentry->d_name) && (pentry->d_name[0] == '.'))
            continue;

        DEBUG("found %s", pentry->d_name);

        if((strlen(pentry->d_name) > 4) &&
           (strcmp(&pentry->d_name[strlen(pentry->d_name) - 4], ".lua") == 0)) {
            INFO("loading lua script '%s'", pentry->d_name);

            snprintf(full_path, sizeof(full_path),
                     "%s/%s", config.lua_script_dir, pentry->d_name);

            if(lr_add_script(full_path) == -1) {
                WARN("Skipped lua script '%s'", full_path);
            }
        }
    }

    free(pentry);

    closedir(pdir);

    return 0;
}

int lr_deinit(void) {
    return 0;
}

static void lr_errstack(lua_State *l, char *prefix) {
    const char *err;

    err = lua_tostring(l, -1);

    if(prefix) {
        ERROR("%s: %s", prefix, err);
    } else {
        ERROR("%s", err);
    }
}

static int lr_add_script(char *path) {
    script_env_t *pnew;

    pnew = (script_env_t*)malloc(sizeof(script_env_t));
    if(!pnew) {
        ERROR("Malloc error");
        return -1;
    }

    pnew->path = strdup(path);
    pnew->l = lr_newstate(path);

    if(!pnew->l) {
        ERROR("Could not create lua state");
        return -1;
    }

    pnew->next = lr_scripts.next;
    lr_scripts.next = pnew;

    int load_result = luaL_loadfile(pnew->l, pnew->path);
    if(load_result) {
        lr_errstack(pnew->l, "Error loading");

        lua_close(pnew->l);

        free(pnew->path);
        free(pnew);
        return -1;
    }

    load_result = lua_pcall(pnew->l, 0, 0, 0);
    if (load_result) {
        DEBUG("Error %d executing lua", load_result);

        lr_errstack(pnew->l, "Error executing");
        lua_close(pnew->l);

        free(pnew->path);
        free(pnew);
        return -1;
    }


    DEBUG("Successfully loaded %s", path);
    return 0;
}

static lua_State *lr_newstate(char *path) {
    lua_State *L;

    L = luaL_newstate();
    luaL_openlibs(L);

    /* add in my methods */
    lua_pushcfunction(L, lr_method_register_callback);
    lua_setglobal(L, "register_callback");

    lua_pushcfunction(L, lr_method_debug);
    lua_setglobal(L, "debug");

    lua_pushnumber(L, DBG_ERROR);
    lua_setglobal(L, "ERROR");

    lua_pushnumber(L, DBG_WARN);
    lua_setglobal(L, "WARN");

    lua_pushnumber(L, DBG_INFO);
    lua_setglobal(L, "INFO");

    lua_pushnumber(L, DBG_DEBUG);
    lua_setglobal(L, "DEBUG");

    return L;
}

static int lr_method_register_callback(lua_State *L) {
    const char *queue = luaL_checkstring(L, 1);
    const char *fn = luaL_checkstring(L, 2);

    DEBUG("Registering callback for queue %s on %s", queue, fn);

    return 0;
}

static int lr_method_debug(lua_State *L) {
    const int level = luaL_checkinteger(L, 1);
    const char *msg = luaL_checkstring(L, 2);

    debug_printf(level, "[%s] %s\n",
                 level < DBG_MAXVAL ? debug_values[level] : "UNKNOWN",
                 msg);
    return 0;
}
