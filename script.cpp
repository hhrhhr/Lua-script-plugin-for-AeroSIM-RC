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

void send()
{
        dt = 0.1667;

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
            double lat = +0.0;
            double lon = -0.0;
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

    lua_getglobal(L, "main");
    int result = lua_pcall(L, 0, 2, 0);
    if (result) {
        printf("ERROR (%d) : %s\n", result, lua_tostring(L, -1));
        //int err = 0 / 0;
    }
}

void recieve()
{
    float v1 = lua_tonumber(L, -2);
    printf("%f ", v1);

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

void test()
{
    lua_getglobal(L, "TEST");

    lua_pushstring(L, "v1");
    lua_pushnumber(L, 11);
    lua_rawset(L, -3);

    lua_pushstring(L, "v2");
    lua_pushnumber(L, 22);
    lua_rawset(L, -3);

    lua_pushstring(L, "t1");
    lua_gettable(L, -2);

    lua_pushstring(L, "v1");
    lua_pushnumber(L, 33);
    lua_rawset(L, -3);

    lua_pushstring(L, "v2");
    lua_pushnumber(L, 44);
    lua_rawset(L, -3);

    lua_pop(L, 1);

    lua_pop(L, 1); // "TEST"

    printf("%d ", lua_gettop(L));

    lua_getglobal(L, "test");
    int result = lua_pcall(L, 0, 0, 0);
    if (result) {
        printf("ERROR (%d) : %s\n", result, lua_tostring(L, -1));
        //int err = 0 / 0;
    }
}

int main(void)
{
    L = luaL_newstate();
    luaL_openlibs(L); // Load Lua libraries
    luaL_loadfile(L, "lua\\main.lua");
    lua_pcall(L, 0, 0, 0);

    for (int i = 0; i < 5; i++) {
        send();
        recieve();
        //test();
        printf("\n");
    }

    lua_close(L);   /* Cya, Lua */
    return 0;
}
