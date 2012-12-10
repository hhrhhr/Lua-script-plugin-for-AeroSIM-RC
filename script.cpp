#include <lua.hpp>
#include <stdio.h>
#include <math.h>

lua_State *L;
float dt = 0.0;
float gyro[3] = { 0.0, 0.0, 0.0 };
float acc[3] = { 0.0, 0.0, 0.0 };
double gpsd[2] = { 0.0, 0.0 };
float gpsf[2] = { 0.0, 0.0 };
float rx[10];

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
    dt = 0.0166666666666667;

    gyro[0] = 0.0;
    gyro[1] = 0.0;
    gyro[2] = 0.0;

    // rotate gravity (0.0, 0.0, -9.81) to model
    acc[0] = -9.81 * 0.0;
    acc[1] = -9.81 * 0.0;
    acc[2] = -9.81 * 1.0;
    // add gravity to acc
    acc[0] += 0.0;
    acc[1] += 0.0;
    acc[2] += 0.0;

    static float gps_dt = 0.0;
    if (gps_dt < 0.2) {
        gps_dt += dt;
    } else {
        gps_dt = 0.0;
        double lat = +0.01;
        double lon = -0.01;
        if ( (fabs(lat - gpsd[0]) > 0.0000001) or (fabs(lon - gpsd[1]) > 0.0000001) ) {
            double lat1 = gpsd[0]   * M_PI / 180.0;
            double lat2 = lat       * M_PI / 180.0;
            double lon1 = gpsd[1]   * M_PI / 180.0;
            double lon2 = lon       * M_PI / 180.0;
            double dLon = lon2 - lon1;
            double y = sin(dLon) * cos(lat2);
            double x = cos(lat1) * sin(lat2) - sin(lat1) * cos(lat2) * cos(dLon);
            float head = atan2(y, x) * 180.0 / M_PI;

            gpsd[0] = lat;
            gpsd[1] = lon;
            gpsf[0] = 0.0;
            gpsf[1] = head;
        }
    };

    // set global tables in Lua ----------------------------------------------------------------
    lua_getglobal(L, "RAW");
    {
        /* dt*/
        lua_pushstring(L, "dt");
        lua_pushnumber(L, dt);
        lua_rawset(L, -3);

        /* model gyro */
        lua_pushstring(L, "gyro");
        lua_gettable(L, -2);
        {
            lua_pushstring(L, "x");
            lua_pushnumber(L, gyro[0]);
            lua_rawset(L, -3);

            lua_pushstring(L, "y");
            lua_pushnumber(L, gyro[1]);
            lua_rawset(L, -3);

            lua_pushstring(L, "z");
            lua_pushnumber(L, gyro[2]);
            lua_rawset(L, -3);
        }
        lua_pop(L, 1);

        /* model acc */
        lua_pushstring(L, "acc");
        lua_gettable(L, -2);
        {
            lua_pushstring(L, "x");
            lua_pushnumber(L, acc[0]);
            lua_rawset(L, -3);

            lua_pushstring(L, "y");
            lua_pushnumber(L, acc[1]);
            lua_rawset(L, -3);

            lua_pushstring(L, "z");
            lua_pushnumber(L, acc[2]);
            lua_rawset(L, -3);
        }
        lua_pop(L, 1);

        /* lat/lon/alt/head */
        lua_pushstring(L, "gps");
        lua_gettable(L, -2);
        {
            lua_pushstring(L, "lat");
            lua_pushnumber(L, gpsd[0]);
            lua_rawset(L, -3);

            lua_pushstring(L, "lon");
            lua_pushnumber(L, gpsd[1]);
            lua_rawset(L, -3);

            lua_pushstring(L, "alt");
            lua_pushnumber(L, gpsf[0]);
            lua_rawset(L, -3);

            lua_pushstring(L, "head");
            lua_pushnumber(L, gpsf[1]);
            lua_rawset(L, -3);
        }
        lua_pop(L, 1);
    }
    lua_pop(L, 1);

    lua_getglobal(L, "TX");
    {
        for (int c = 0; c < 9; c++) {
            lua_pushinteger(L, c);
            lua_pushnumber(L, (float)c * 0.1);
            lua_rawset(L, -3);
        }
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
    float v1;
    lua_getglobal(L, "FD");
    {
        lua_pushstring(L, "time");
        lua_rawget(L, -2);
        v1 = lua_tonumber(L, -1);
        lua_pop(L, 1);
    }
    lua_pop(L, 1);

    lua_getglobal(L, "RX");
    for (int i = 0; i < 10; i++) {
        lua_rawgeti(L, -1, i+1);
        rx[i] = lua_tonumber(L, -1);
        lua_pop(L, 1);
    }
    lua_pop(L, 1);


    printf("%f ", v1);
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
        for (int i = 0; i < 6; i++) {
            send();
            recieve();
        }
    }

    lua_close(L);   /* Cya, Lua */
    return 0;
}

