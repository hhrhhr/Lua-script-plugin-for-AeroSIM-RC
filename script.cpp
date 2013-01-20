#include <lua.hpp>
#include <windows.h>
#include <stdio.h>
#include <math.h>

char g_strDebugInfo[10000] = "";
char g_luaDebugInfo[1024] = "";
char g_lua2DebugInfo[1024] = "";

lua_State *L;
float acc[3] = { 0.0, 0.0, 0.0 };
float rx[39];

int errorHandler(lua_State* L)
{
    //stack: err
    const char* err = lua_tostring(L, 1);
    printf("\nError: %s\n", err);

    lua_getglobal(L, "debug"); // stack: err debug
    lua_getfield(L, -1, "traceback"); // stack: err debug debug.traceback

    // debug.traceback() возвращает 1 значение
    if(lua_pcall(L, 0, 1, 0)) {
        const char* err = lua_tostring(L, -1);
        printf("Error in debug.traceback() call: %s\n", err);
    } else {
        const char* stackTrace = lua_tostring(L, -1);
        printf("C++ stack traceback: %s\n", stackTrace);
    }

    return 1;
}

void send()
{
    lua_getglobal(L, "RAW");

    /* menu */
    lua_pushstring(L, "menu");
    lua_pushinteger(L, 999);
    lua_rawset(L, -3);

    /* dt*/
    lua_pushstring(L, "dt");
    lua_pushnumber(L, 0.016666666);
    lua_rawset(L, -3);

    /* model gyro */
    lua_pushstring(L, "gyr");
    lua_gettable(L, -2);
    {
        lua_pushstring(L, "x");
        lua_pushnumber(L, 0.00001);
        lua_rawset(L, -3);

        lua_pushstring(L, "y");
        lua_pushnumber(L, 0.00002);
        lua_rawset(L, -3);

        lua_pushstring(L, "z");
        lua_pushnumber(L, 0.00003);
        lua_rawset(L, -3);
    }
    lua_pop(L, 1);

    /* model acc */
    lua_pushstring(L, "acc");
    lua_gettable(L, -2);
    {
        lua_pushstring(L, "x");
        lua_pushnumber(L, 0.0);
        lua_rawset(L, -3);

        lua_pushstring(L, "y");
        lua_pushnumber(L, 0.0);
        lua_rawset(L, -3);

        lua_pushstring(L, "z");
        lua_pushnumber(L, -9.81);
        lua_rawset(L, -3);
    }
    lua_pop(L, 1);

    /* lat/lon/alt */
    lua_pushstring(L, "gps");
    lua_gettable(L, -2);
    {
        lua_pushstring(L, "lat");
        lua_pushnumber(L, 58.5);
        lua_rawset(L, -3);

        lua_pushstring(L, "lon");
        lua_pushnumber(L, 31.25);
        lua_rawset(L, -3);

        lua_pushstring(L, "alt");
        lua_pushnumber(L, 22);
        lua_rawset(L, -3);
    }
    lua_pop(L, 1);

    lua_pop(L, 1);


    lua_getglobal(L, "TX");
    for (int i = 0; i < 39; i++) {
        lua_pushinteger(L, i+1);
        lua_pushnumber(L, (float)(i+1) * 0.01);
        lua_rawset(L, -3);
    }
    lua_pop(L, 1);

    lua_pushcfunction(L, errorHandler);
    lua_getglobal(L, "main");
    int result = lua_pcall(L, 0, 0, -2);
    if (result) {
        printf("send() failed\n");
    }
    lua_pop(L, 1);
}

void recieve()
{
    strcpy(g_luaDebugInfo, "no errors");
    printf("1) %s\n", g_luaDebugInfo);

    lua_getglobal(L, "DBGSTR");
    strcpy(g_lua2DebugInfo, lua_tostring(L, -1));
    lua_pop(L, 1);
    printf("2) %s\n", g_lua2DebugInfo);

    lua_getglobal(L, "RX");
    for (int i = 0; i < 39; i++) {
        lua_rawgeti(L, -1, i+1);
        rx[i] = lua_tonumber(L, -1);
        lua_pop(L, 1);
    }
    lua_pop(L, 1);


    printf("%+1.2f, %+1.2f, %+1.2f, %+1.2f %+1.2f ", rx[0], rx[1], rx[2], rx[3], rx[4]);
    printf("%+1.2f, %+1.2f, %+1.2f, %+1.2f %+1.2f ", rx[5], rx[6], rx[7], rx[8], rx[9]);
    printf("(%d)\n", lua_gettop(L));
}


int main(void)
{
    L = luaL_newstate();
    luaL_openlibs(L); // Load Lua libraries

    lua_pushcfunction(L, errorHandler);

    luaL_loadfile(L, "lua\\main.lua");
    int result = lua_pcall(L, 0, 0, -2);
    lua_pop(L, 1);

    if (result) {
        printf("\nloadfile failed\n");
    } else {
/*
        for (int i = 0; i < 6; i++) {
            send();
            recieve();
        }
*/
        for (int i = 0; i < 6; i++) {
            lua_getglobal(L, "next_resolution");
            lua_pcall(L, 0, 4, 0);
            int x = 111, y = 222, w = 333, h = 444;
            x = lua_tointeger(L, 1);
            y = lua_tointeger(L, 2);
            w = lua_tointeger(L, 3);
            h = lua_tointeger(L, 4);
            lua_pop(L, 4);
            printf("%d, %d, %d, %d (%d)\n", x, y, w, h, lua_gettop(L));
        }
    }

    lua_close(L);   /* Cya, Lua */
    return 0;
}

