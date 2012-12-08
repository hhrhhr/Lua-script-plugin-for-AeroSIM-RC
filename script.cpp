#include <lua.hpp>
#include <stdio.h>

lua_State *L;

void send()
{
    lua_getglobal(L, "main");
    /* 1 dt*/
    lua_pushnumber(L, 0.01667);

    /* 2 - 4 model gyro */
    lua_pushnumber(L, 0.0);
    lua_pushnumber(L, 0.3);
    lua_pushnumber(L, 0.0);

    /* 5 - 7 model acc */
    lua_pushnumber(L, 0.0);
    lua_pushnumber(L, 0.0);
    lua_pushnumber(L, -9.81);

    /* 8 - 11 lat/lon/alt/head */
    lua_pushnumber(L, 0.0);
    lua_pushnumber(L, 0.0);
    lua_pushnumber(L, 0.0);
    lua_pushnumber(L, 0.0);

    /* 12 - 20 input channels */
    lua_pushnumber(L, 0.11);
    lua_pushnumber(L, 0.22);
    lua_pushnumber(L, 0.33);
    lua_pushnumber(L, 0.44);
    lua_pushnumber(L, 0.55);
    lua_pushnumber(L, 0.66);
    lua_pushnumber(L, 0.77);
    lua_pushnumber(L, 0.88);
    lua_pushnumber(L, 0.99);

    int result = lua_pcall(L, 20, 2, 0);
    if (result) {
        printf("ERROR (%d) : %s\n", result, lua_tostring(L, -1));
        //int err = 0 / 0;
    }
}

void recieve()
{
    float v1 = lua_tonumber(L, -2);
    printf("dt - %f ", v1);

    float rx[10];

    lua_pushnil(L);
    for (int i = 0; i < 10; i++) {
        lua_next(L, -2);
//        lua_tointeger(L, -2);
        rx[i] = lua_tonumber(L, -1);
        lua_pop(L, 1);
    }

    lua_pop(L, 3);

    printf("%+1.2f, %+1.2f, %+1.2f, %+1.2f %+1.2f ", rx[0], rx[1], rx[2], rx[3], rx[4]);
    printf("%+1.2f, %+1.2f, %+1.2f, %+1.2f %+1.2f ", rx[5], rx[6], rx[7], rx[8], rx[9]);

    printf("(%d)\n", lua_gettop(L));
}

int main(void)
{
    L = luaL_newstate();
    luaL_openlibs(L); /* Load Lua libraries */
    luaL_loadfile(L, "lua\\main.lua");
    lua_pcall(L, 0, 0, 0);

    for (int i = 0; i < 5; i++) {
        send();
        recieve();
        printf("\n");
    }

    lua_close(L);   /* Cya, Lua */
    return 0;
}
